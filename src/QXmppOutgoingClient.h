/*
 * Copyright (C) 2008-2010 The QXmpp developers
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


#ifndef QXMPPOUTGOINGCLIENT_H
#define QXMPPOUTGOINGCLIENT_H

#include "QXmppClient.h"
#include "QXmppStanza.h"
#include "QXmppStream.h"

class QDomElement;
class QSslError;

class QXmppConfiguration;
class QXmppPresence;
class QXmppIq;
class QXmppMessage;
class QXmppRpcResponseIq;
class QXmppRpcErrorIq;
class QXmppSrvInfo;

class QXmppOutgoingClientPrivate;

/// \brief The QXmppOutgoingClient class represents an outgoing XMPP stream
/// to an XMPP server.
///

class QXmppOutgoingClient : public QXmppStream
{
    Q_OBJECT

public:
    QXmppOutgoingClient(QObject *parent);
    ~QXmppOutgoingClient();

    void connectToHost();
    bool isConnected() const;

    QAbstractSocket::SocketError socketError();
    QXmppStanza::Error::Condition xmppStreamError();

    QXmppConfiguration& configuration();

signals:
    /// This signal is emitted when an error is encountered.
    void error(QXmppClient::Error);

    /// This signal is emitted when an element is received.
    void elementReceived(const QDomElement &element, bool &handled);

    /// This signal is emitted when a presence is received.
    void presenceReceived(const QXmppPresence&);

    /// This signal is emitted when a message is received.
    void messageReceived(const QXmppMessage&);

    /// This signal is emitted when an IQ is received.
    void iqReceived(const QXmppIq&);

    /// \cond
    // XEP-0009: Jabber-RPC
    void rpcCallInvoke(const QXmppRpcInvokeIq &invoke);
    void rpcCallResponse(const QXmppRpcResponseIq& result);
    void rpcCallError(const QXmppRpcErrorIq &err);
    /// \endcond

protected:
    /// \cond
    // Overridable methods
    virtual void handleStart();
    virtual void handleStanza(const QDomElement &element);
    virtual void handleStream(const QDomElement &element);
    /// \endcond

private slots:
    void connectToHost(const QXmppSrvInfo &serviceInfo);
    void socketError(QAbstractSocket::SocketError);
    void socketSslErrors(const QList<QSslError>&);

    void pingStart();
    void pingStop();
    void pingSend();
    void pingTimeout();

private:
    void sendAuthDigestMD5ResponseStep1(const QString& challenge);
    void sendAuthDigestMD5ResponseStep2(const QString& challenge);
    void sendNonSASLAuth(bool plaintext);
    void sendNonSASLAuthQuery();

    QXmppOutgoingClientPrivate * const d;
};

#endif // QXMPPOUTGOINGCLIENT_H
