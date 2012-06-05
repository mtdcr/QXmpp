/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include <QCryptographicHash>
#include <QSslSocket>
#include <QUrl>
#include "qdnslookup.h"

#include "QXmppConfiguration.h"
#include "QXmppConstants.h"
#include "QXmppIq.h"
#include "QXmppLogger.h"
#include "QXmppMessage.h"
#include "QXmppPresence.h"
#include "QXmppOutgoingClient.h"
#include "QXmppStreamFeatures.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppSaslAuth.h"
#include "QXmppUtils.h"

// IQ types
#include "QXmppBindIq.h"
#include "QXmppPingIq.h"
#include "QXmppSessionIq.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDomDocument>
#include <QStringList>
#include <QRegExp>
#include <QHostAddress>
#include <QXmlStreamWriter>
#include <QTimer>

class QXmppOutgoingClientPrivate
{
public:
    QXmppOutgoingClientPrivate();

    // This object provides the configuration
    // required for connecting to the XMPP server.
    QXmppConfiguration config;
    QXmppStanza::Error::Condition xmppStreamError;

    // DNS
    QDnsLookup dns;

    // Stream
    QString streamId;
    QString streamFrom;
    QString streamVersion;

    // Session
    QString bindId;
    QString sessionId;
    bool sessionAvailable;
    bool sessionStarted;

    // Authentication
    QString nonSASLAuthId;
    int saslStep;
    QXmppSaslMechanism *saslMechanism;
    QHash<QString, QXmppSaslMechanism *> saslMechanisms;

    // Timers
    QTimer *pingTimer;
    QTimer *timeoutTimer;
};

QXmppOutgoingClientPrivate::QXmppOutgoingClientPrivate()
    : sessionAvailable(false),
    saslStep(0),
    saslMechanism(0)
{
}

/// Constructs an outgoing client stream.
///
/// \param parent

QXmppOutgoingClient::QXmppOutgoingClient(QObject *parent)
    : QXmppStream(parent),
    d(new QXmppOutgoingClientPrivate)
{
    bool check;
    Q_UNUSED(check);

    // initialise socket
    QSslSocket *socket = new QSslSocket(this);
    setSocket(socket);

    check = connect(socket, SIGNAL(sslErrors(QList<QSslError>)),
                    this, SLOT(socketSslErrors(QList<QSslError>)));
    Q_ASSERT(check);

    check = connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(socketError(QAbstractSocket::SocketError)));
    Q_ASSERT(check);

    // DNS lookups
    check = connect(&d->dns, SIGNAL(finished()),
                    this, SLOT(_q_dnsLookupFinished()));
    Q_ASSERT(check);

    // XEP-0199: XMPP Ping
    d->pingTimer = new QTimer(this);
    check = connect(d->pingTimer, SIGNAL(timeout()),
                    this, SLOT(pingSend()));
    Q_ASSERT(check);

    d->timeoutTimer = new QTimer(this);
    d->timeoutTimer->setSingleShot(true);
    check = connect(d->timeoutTimer, SIGNAL(timeout()),
                    this, SLOT(pingTimeout()));
    Q_ASSERT(check);

    check = connect(this, SIGNAL(connected()),
                    this, SLOT(pingStart()));
    Q_ASSERT(check);

    check = connect(this, SIGNAL(disconnected()),
                    this, SLOT(pingStop()));
    Q_ASSERT(check);
}

/// Destroys an outgoing client stream.

QXmppOutgoingClient::~QXmppOutgoingClient()
{
    delete d;
}

/// Returns a reference to the stream's configuration.

QXmppConfiguration& QXmppOutgoingClient::configuration()
{
    return d->config;
}

/// Attempts to connect to the XMPP server.

void QXmppOutgoingClient::connectToHost()
{
    const QString host = configuration().host();
    const quint16 port = configuration().port();

    // override CA certificates if requested
    if (!configuration().caCertificates().isEmpty()) {
        socket()->setCaCertificates(configuration().caCertificates());
    }

    // if an explicit host was provided, connect to it
    if (!host.isEmpty() && port) {
        info(QString("Connecting to %1:%2").arg(host, QString::number(port)));
        socket()->setProxy(configuration().networkProxy());
        socket()->connectToHost(host, port);
        return;
    }

    // otherwise, lookup server
    const QString domain = configuration().domain();
    debug(QString("Looking up server for domain %1").arg(domain));
    d->dns.setName("_xmpp-client._tcp." + domain);
    d->dns.setType(QDnsLookup::SRV);
    d->dns.lookup();
}

void QXmppOutgoingClient::_q_dnsLookupFinished()
{
    QString host;
    quint16 port;

    if (d->dns.error() == QDnsLookup::NoError &&
        !d->dns.serviceRecords().isEmpty()) {
        // take the first returned record
        host = d->dns.serviceRecords().first().target();
        port = d->dns.serviceRecords().first().port();
    } else {
        // as a fallback, use domain as the host name
        warning(QString("Lookup for domain %1 failed: %2")
                .arg(d->dns.name(), d->dns.errorString()));
        host = configuration().domain();
        port = configuration().port();
    }

    // connect to server
    info(QString("Connecting to %1:%2").arg(host, QString::number(port)));
    socket()->setProxy(configuration().networkProxy());
    socket()->connectToHost(host, port);
}

/// Returns true if the socket is connected and a session has been started.
///

bool QXmppOutgoingClient::isConnected() const
{
    return QXmppStream::isConnected() && d->sessionStarted;
}

void QXmppOutgoingClient::socketSslErrors(const QList<QSslError> & error)
{
    warning("SSL errors");
    for(int i = 0; i< error.count(); ++i)
        warning(error.at(i).errorString());

    if (configuration().ignoreSslErrors())
        socket()->ignoreSslErrors();
}

void QXmppOutgoingClient::socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit error(QXmppClient::SocketError);
}

void QXmppOutgoingClient::handleStart()
{
    QXmppStream::handleStart();

    // reset stream information
    d->streamId.clear();
    d->streamFrom.clear();
    d->streamVersion.clear();

    // reset authentication step
    d->saslStep = 0;
    d->saslMechanism = 0;

    // reset session information
    d->bindId.clear();
    d->sessionId.clear();
    d->sessionAvailable = false;
    d->sessionStarted = false;

    // start stream
    QByteArray data = "<?xml version='1.0'?><stream:stream to='";
    data.append(configuration().domain().toUtf8());
    data.append("' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>");
    sendData(data);
}

void QXmppOutgoingClient::handleStream(const QDomElement &streamElement)
{
    if(d->streamId.isEmpty())
        d->streamId = streamElement.attribute("id");
    if (d->streamFrom.isEmpty())
        d->streamFrom = streamElement.attribute("from");
    if(d->streamVersion.isEmpty())
    {
        d->streamVersion = streamElement.attribute("version");

        // no version specified, signals XMPP Version < 1.0.
        // switch to old auth mechanism
        if(d->streamVersion.isEmpty())
            sendNonSASLAuthQuery();
    }
}

void QXmppOutgoingClient::handleStanza(const QDomElement &nodeRecv)
{
    // if we receive any kind of data, stop the timeout timer
    d->timeoutTimer->stop();

    const QString ns = nodeRecv.namespaceURI();

    // give client opportunity to handle stanza
    bool handled = false;
    emit elementReceived(nodeRecv, handled);
    if (handled)
        return;

    if(QXmppStreamFeatures::isStreamFeatures(nodeRecv))
    {
        QXmppStreamFeatures features;
        features.parse(nodeRecv);

        if (!socket()->isEncrypted())
        {
            // determine TLS mode to use
            const QXmppConfiguration::StreamSecurityMode localSecurity = configuration().streamSecurityMode();
            const QXmppStreamFeatures::Mode remoteSecurity = features.tlsMode();
            if (!socket()->supportsSsl() &&
                (localSecurity == QXmppConfiguration::TLSRequired ||
                 remoteSecurity == QXmppStreamFeatures::Required))
            {
                warning("Disconnecting as TLS is required, but SSL support is not available");
                disconnectFromHost();
                return;
            }
            if (localSecurity == QXmppConfiguration::TLSRequired &&
                remoteSecurity == QXmppStreamFeatures::Disabled)
            {
                warning("Disconnecting as TLS is required, but not supported by the server");
                disconnectFromHost();
                return;
            }

            if (socket()->supportsSsl() &&
                localSecurity != QXmppConfiguration::TLSDisabled &&
                remoteSecurity != QXmppStreamFeatures::Disabled)
            {
                // enable TLS as it is support by both parties
                sendData("<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
                return;
            }
        }

        // handle authentication
        const bool nonSaslAvailable = features.nonSaslAuthMode() != QXmppStreamFeatures::Disabled;
        const bool saslAvailable = !features.authMechanismsStrings().isEmpty();
        const bool useSasl = configuration().useSASLAuthentication();
        if((saslAvailable && nonSaslAvailable && !useSasl) ||
           (!saslAvailable && nonSaslAvailable))
        {
            sendNonSASLAuthQuery();
        }
        else if(saslAvailable)
        {
            // determine SASL Authentication mechanism to use
            const QList<QString> mechanisms = features.authMechanismsStrings();
            if (mechanisms.isEmpty())
            {
                warning("No supported SASL Authentication mechanism available");
                disconnectFromHost();
                return;
            }

            QString mech;
            if (!mechanisms.contains(configuration().sASLAuthMechanismString()))
            {
                info("Desired SASL Auth mechanism is not available, selecting first available one");
                mech = mechanisms.first();
            } else {
                mech = configuration().sASLAuthMechanismString();
            }

            if (!d->saslMechanisms.contains(mech)) {
                warning("No supported SASL Authentication mechanism available");
                disconnectFromHost();
                return;
            }

            // send SASL Authentication request
            d->saslMechanism = d->saslMechanisms.value(mech);

            QByteArray data = "<auth xmlns='";
            data += ns_sasl;
            data += "' mechanism='" + d->saslMechanism->identifier() + "'";

            QByteArray text = d->saslMechanism->authText();
            if (text.isEmpty() && !d->saslMechanism->needsInitialResponse()) {
                data += "/>";
            } else {
                data += ">";
                if (text.isEmpty())
                    data += "=";
                else
                    data += text.toBase64();
                data += "</auth>";
            }

            sendData(data);
        }

        // check whether bind is available
        if (features.bindMode() != QXmppStreamFeatures::Disabled)
        {
            QXmppBindIq bind;
            bind.setType(QXmppIq::Set);
            bind.setResource(configuration().resource());
            d->bindId = bind.id();
            sendPacket(bind);
        }

        // check whether session is available
        if (features.sessionMode() != QXmppStreamFeatures::Disabled)
            d->sessionAvailable = true;
    }
    else if(ns == ns_stream && nodeRecv.tagName() == "error")
    {
        if (!nodeRecv.firstChildElement("conflict").isNull())
            d->xmppStreamError = QXmppStanza::Error::Conflict;
        else
            d->xmppStreamError = QXmppStanza::Error::UndefinedCondition;
        emit error(QXmppClient::XmppStreamError);
    }
    else if(ns == ns_tls)
    {
        if(nodeRecv.tagName() == "proceed")
        {
            debug("Starting encryption");
            socket()->startClientEncryption();
            return;
        }
    }
    else if(ns == ns_sasl)
    {
        if(nodeRecv.tagName() == "success")
        {
            debug("Authenticated");
            handleStart();
        }
        else if(nodeRecv.tagName() == "challenge")
        {
            QByteArray challenge = QByteArray::fromBase64(nodeRecv.text().toAscii());
            QByteArray response;
            d->saslStep++;
            if (!d->saslMechanism || !d->saslMechanism->challengeResponse(challenge, response, d->saslStep)) {
                disconnectFromHost();
            } else {
                QByteArray data = "<response xmlns='";
                data += ns_sasl;
                data += "'";
                if (response.isEmpty())
                    data += "/>";
                else
                    data += ">" + response.toBase64() + "</response>";

                sendData(data);
            }
        }
        else if(nodeRecv.tagName() == "failure")
        {
            if (!nodeRecv.firstChildElement("not-authorized").isNull())
                d->xmppStreamError = QXmppStanza::Error::NotAuthorized;
            else
                d->xmppStreamError = QXmppStanza::Error::UndefinedCondition;
            emit error(QXmppClient::XmppStreamError);

            warning("Authentication failure");
            disconnectFromHost();
        }
    }
    else if(ns == ns_client)
    {

        if(nodeRecv.tagName() == "iq")
        {
            QDomElement element = nodeRecv.firstChildElement();
            QString id = nodeRecv.attribute("id");
            QString type = nodeRecv.attribute("type");
            if(type.isEmpty())
                warning("QXmppStream: iq type can't be empty");

            if(id == d->sessionId)
            {
                QXmppSessionIq session;
                session.parse(nodeRecv);

                // xmpp connection made
                d->sessionStarted = true;
                emit connected();
            }
            else if(QXmppBindIq::isBindIq(nodeRecv) && id == d->bindId)
            {
                QXmppBindIq bind;
                bind.parse(nodeRecv);

                // bind result
                if (bind.type() == QXmppIq::Result)
                {
                    if (!bind.jid().isEmpty())
                    {
                        QRegExp jidRegex("^([^@/]+)@([^@/]+)/(.+)$");
                        if (jidRegex.exactMatch(bind.jid()))
                        {
                            configuration().setUser(jidRegex.cap(1));
                            configuration().setDomain(jidRegex.cap(2));
                            configuration().setResource(jidRegex.cap(3));
                        } else {
                            warning("Bind IQ received with invalid JID: " + bind.jid());
                        }
                    }

                    // start session if it is available
                    if (d->sessionAvailable)
                    {
                        QXmppSessionIq session;
                        session.setType(QXmppIq::Set);
                        session.setTo(configuration().domain());
                        d->sessionId = session.id();
                        sendPacket(session);
                    }
                }
            }
            // extensions

            // XEP-0078: Non-SASL Authentication
            else if(id == d->nonSASLAuthId && type == "result")
            {
                // successful Non-SASL Authentication
                debug("Authenticated (Non-SASL)");

                // xmpp connection made
                d->sessionStarted = true;
                emit connected();
            }
            else if(QXmppNonSASLAuthIq::isNonSASLAuthIq(nodeRecv))
            {
                if(type == "result")
                {
                    bool digest = !nodeRecv.firstChildElement("query").
                         firstChildElement("digest").isNull();
                    bool plain = !nodeRecv.firstChildElement("query").
                         firstChildElement("password").isNull();
                    bool plainText = false;

                    if(plain && digest)
                    {
                        if(configuration().nonSASLAuthMechanism() ==
                           QXmppConfiguration::NonSASLDigest)
                            plainText = false;
                        else
                            plainText = true;
                    }
                    else if(plain)
                        plainText = true;
                    else if(digest)
                        plainText = false;
                    else
                    {
                        warning("No supported Non-SASL Authentication mechanism available");
                        disconnectFromHost();
                        return;
                    }
                    sendNonSASLAuth(plainText);
                }
            }
            // XEP-0199: XMPP Ping
            else if(QXmppPingIq::isPingIq(nodeRecv))
            {
                QXmppPingIq req;
                req.parse(nodeRecv);

                QXmppIq iq(QXmppIq::Result);
                iq.setId(req.id());
                iq.setTo(req.from());
                sendPacket(iq);
            }
            else
            {
                QXmppIq iqPacket;
                iqPacket.parse(nodeRecv);

                // if we didn't understant the iq, reply with error
                // except for "result" and "error" iqs
                if (type != "result" && type != "error")
                {
                    QXmppIq iq(QXmppIq::Error);
                    iq.setId(iqPacket.id());
                    iq.setTo(iqPacket.from());
                    QXmppStanza::Error error(QXmppStanza::Error::Cancel,
                        QXmppStanza::Error::FeatureNotImplemented);
                    iq.setError(error);
                    sendPacket(iq);
                } else {
                    emit iqReceived(iqPacket);
                }
            }
        }
        else if(nodeRecv.tagName() == "presence")
        {
            QXmppPresence presence;
            presence.parse(nodeRecv);

            // emit presence
            emit presenceReceived(presence);
        }
        else if(nodeRecv.tagName() == "message")
        {
            QXmppMessage message;
            message.parse(nodeRecv);

            // emit message
            emit messageReceived(message);
        }
    }
}

void QXmppOutgoingClient::pingStart()
{
    const int interval = configuration().keepAliveInterval();
    // start ping timer
    if (interval > 0)
    {
        d->pingTimer->setInterval(interval * 1000);
        d->pingTimer->start();
    }
}

void QXmppOutgoingClient::pingStop()
{
    // stop all timers
    d->pingTimer->stop();
    d->timeoutTimer->stop();
}

void QXmppOutgoingClient::pingSend()
{
    // send ping packet
    QXmppPingIq ping;
    ping.setTo(configuration().domain());
    sendPacket(ping);

    // start timeout timer
    const int timeout = configuration().keepAliveTimeout();
    if (timeout > 0)
    {
        d->timeoutTimer->setInterval(timeout * 1000);
        d->timeoutTimer->start();
    }
}

void QXmppOutgoingClient::pingTimeout()
{
    warning("Ping timeout");
    disconnectFromHost();
    emit error(QXmppClient::KeepAliveError);
}

void QXmppOutgoingClient::sendNonSASLAuth(bool plainText)
{
    QXmppNonSASLAuthIq authQuery;
    authQuery.setType(QXmppIq::Set);
    authQuery.setUsername(configuration().user());
    if (plainText)
        authQuery.setPassword(configuration().password());
    else
        authQuery.setDigest(d->streamId, configuration().password());
    authQuery.setResource(configuration().resource());
    d->nonSASLAuthId = authQuery.id();
    sendPacket(authQuery);
}

void QXmppOutgoingClient::sendNonSASLAuthQuery()
{
    QXmppNonSASLAuthIq authQuery;
    authQuery.setType(QXmppIq::Get);
    authQuery.setTo(d->streamFrom);
    // FIXME : why are we setting the username, XEP-0078 states we should
    // not attempt to guess the required fields?
    authQuery.setUsername(configuration().user());
    sendPacket(authQuery);
}

/// Returns the type of the last XMPP stream error that occured.

QXmppStanza::Error::Condition QXmppOutgoingClient::xmppStreamError()
{
    return d->xmppStreamError;
}

/// Registers a new SASL mechanism with the client.
///
/// \param mechanism

bool QXmppOutgoingClient::addSaslMechanism(QXmppSaslMechanism *mechanism)
{
    const QString &identifier = mechanism->identifier();
    if (d->saslMechanisms.contains(identifier))
    {
        qWarning("Cannot add SASL mechanism, it has already been added");
        return false;
    }

    mechanism->setParent(this);
    mechanism->setConfiguration(&configuration());
    d->saslMechanisms.insert(identifier, mechanism);
    return true;
}

/// Unregisters the given SASL mechanism from the client. If the
/// SASL mechanism is found, it will be destroyed.
///
/// \param mechanism

bool QXmppOutgoingClient::removeSaslMechanism(QXmppSaslMechanism *mechanism)
{
    const QString &identifier = mechanism->identifier();
    if (d->saslMechanisms.contains(identifier))
    {
        d->saslMechanisms.remove(identifier);
        delete mechanism;
        return true;
    } else {
        qWarning("Cannot remove SASL mechanism, it was never added");
        return false;
    }
}

/// Returns a list containing all the client's SASL mechanisms.
///

QHash<QString, QXmppSaslMechanism *> QXmppOutgoingClient::saslMechanisms()
{
    return d->saslMechanisms;
}
