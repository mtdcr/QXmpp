/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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


#include <QSslSocket>
#include "QXmppConfiguration.h"
#include "QXmppSaslAuth.h"
#include "QXmppUtils.h"

/// Creates a QXmppConfiguration object.

QXmppConfiguration::QXmppConfiguration() : m_port(5222),
                m_resource("QXmpp"),
                m_autoAcceptSubscriptions(false),
                m_sendIntialPresence(true),
                m_sendRosterRequest(true),
                m_keepAliveInterval(60),
                m_keepAliveTimeout(20),
                m_autoReconnectionEnabled(true),
                m_useSASLAuthentication(true),
                m_ignoreSslErrors(true),
                m_streamSecurityMode(QXmppConfiguration::TLSEnabled),
                m_nonSASLAuthMechanism(QXmppConfiguration::NonSASLDigest),
                m_SASLAuthMechanism("DIGEST-MD5")
{

}

/// Destructor, destroys the QXmppConfiguration object.
///

QXmppConfiguration::~QXmppConfiguration()
{

}

/// Sets the host name.
///
/// \param host host name of the XMPP server where connection has to be made
/// (e.g. "jabber.org" and "talk.google.com"). It can also be an IP address in
/// the form of a string (e.g. "192.168.1.25").
///

void QXmppConfiguration::setHost(const QString& host)
{
    m_host = host;
}

/// Sets the domain name.
///
/// \param domain Domain name e.g. "gmail.com" and "jabber.org".
/// \note host name and domain name can be different for google
/// domain name is gmail.com and host name is talk.google.com
///

void QXmppConfiguration::setDomain(const QString& domain)
{
    m_domain = domain;
}

/// Sets the port number.
///
/// \param port Port number at which the XMPP server is listening. The default
/// value is 5222.
///

void QXmppConfiguration::setPort(int port)
{
    m_port = port;
}

/// Sets the username.
///
/// \param user Username of the account at the specified XMPP server. It should
/// be the name without the domain name. E.g. "qxmpp.test1" and not
/// "qxmpp.test1@gmail.com"
///

void QXmppConfiguration::setUser(const QString& user)
{
    m_user = user;
}

/// Sets the password.
///
/// \param password Password for the specified username
///

void QXmppConfiguration::setPassword(const QString& password)
{
    m_password = password;
}

/// Sets the resource identifier.
///
/// Multiple resources (e.g., devices or locations) may connect simultaneously
/// to a server on behalf of each authorized client, with each resource
/// differentiated by the resource identifier of an XMPP address
/// (e.g. node\@domain/home vs. node\@domain/work)
///
/// The default value is "QXmpp".
///
/// \param resource Resource identifier of the client in connection.

void QXmppConfiguration::setResource(const QString& resource)
{
    m_resource = resource;
}

/// Sets the JID. If a full JID (i.e. one with a resource) is given, calling
/// this method will update the username, domain and resource. Otherwise, only
/// the username and the domain will be updated.
///
/// \param jid

void QXmppConfiguration::setJid(const QString& jid)
{
    m_user = QXmppUtils::jidToUser(jid);
    m_domain = QXmppUtils::jidToDomain(jid);
    const QString resource = QXmppUtils::jidToResource(jid);
    if (!resource.isEmpty())
        m_resource = resource;
}

/// Returns the host name.
///
/// \return host name
///

QString QXmppConfiguration::host() const
{
    return m_host;
}

/// Returns the domain name.
///
/// \return domain name
///

QString QXmppConfiguration::domain() const
{
    return m_domain;
}

/// Returns the port number.
///
/// \return port number
///

int QXmppConfiguration::port() const
{
    return m_port;
}

/// Returns the username.
///
/// \return username
///

QString QXmppConfiguration::user() const
{
    return m_user;
}

/// Returns the password.
///
/// \return password
///

QString QXmppConfiguration::password() const
{
    return m_password;
}

/// Returns the resource identifier.
///
/// \return resource identifier
///

QString QXmppConfiguration::resource() const
{
    return m_resource;
}

/// Returns the jabber id (jid).
///
/// \return jabber id (jid)
/// (e.g. "qxmpp.test1@gmail.com/resource" or qxmpptest@jabber.org/QXmpp156)
///

QString QXmppConfiguration::jid() const
{
    if (m_user.isEmpty())
        return m_domain;
    else
        return jidBare() + "/" + m_resource;
}

/// Returns the bare jabber id (jid), without the resource identifier.
///
/// \return bare jabber id (jid)
/// (e.g. "qxmpp.test1@gmail.com" or qxmpptest@jabber.org)
///

QString QXmppConfiguration::jidBare() const
{
    if (m_user.isEmpty())
        return m_domain;
    else
        return m_user+"@"+m_domain;
}

/// Returns the access token used for X-FACEBOOK-PLATFORM authentication.

QString QXmppConfiguration::facebookAccessToken() const
{
    return m_facebookAccessToken;
}

/// Sets the access token used for X-FACEBOOK-PLATFORM authentication.
///
/// This token is returned by Facebook at the end of the OAuth authentication
/// process.
///
/// \param accessToken

void QXmppConfiguration::setFacebookAccessToken(const QString& accessToken)
{
    m_facebookAccessToken = accessToken;
}

/// Returns the application ID used for X-FACEBOOK-PLATFORM authentication.

QString QXmppConfiguration::facebookAppId() const
{
    return m_facebookAppId;
}

/// Sets the application ID used for X-FACEBOOK-PLATFORM authentication.
///
/// \param appId

void QXmppConfiguration::setFacebookAppId(const QString& appId)
{
    m_facebookAppId = appId;
}

/// Returns the auto-accept-subscriptions-request configuration.
///
/// \return boolean value
/// true means that auto-accept-subscriptions-request is enabled else disabled for false
///

bool QXmppConfiguration::autoAcceptSubscriptions() const
{
    return m_autoAcceptSubscriptions;
}

/// Sets the auto-accept-subscriptions-request configuration.
///
/// \param value boolean value
/// true means that auto-accept-subscriptions-request is enabled else disabled for false
///

void QXmppConfiguration::setAutoAcceptSubscriptions(bool value)
{
    m_autoAcceptSubscriptions = value;
}

/// Returns the auto-reconnect-on-disconnection-on-error configuration.
///
/// \return boolean value
/// true means that auto-reconnect is enabled else disabled for false
///

bool QXmppConfiguration::autoReconnectionEnabled() const
{
    return m_autoReconnectionEnabled;
}

/// Sets the auto-reconnect-on-disconnection-on-error configuration.
///
/// \param value boolean value
/// true means that auto-reconnect is enabled else disabled for false
///

void QXmppConfiguration::setAutoReconnectionEnabled(bool value)
{
    m_autoReconnectionEnabled = value;
}

/// Returns whether SSL errors (such as certificate validation errors)
/// are to be ignored when connecting to the XMPP server.

bool QXmppConfiguration::ignoreSslErrors() const
{
    return m_ignoreSslErrors;
}

/// Specifies whether SSL errors (such as certificate validation errors)
/// are to be ignored when connecting to an XMPP server.

void QXmppConfiguration::setIgnoreSslErrors(bool value)
{
    m_ignoreSslErrors = value;
}

/// Returns the type of authentication system specified by the user.
/// \return true if SASL was specified else false. If the specified
/// system is not available QXmpp will resort to the other one.

bool QXmppConfiguration::useSASLAuthentication() const
{
    return m_useSASLAuthentication;
}

/// Returns the type of authentication system specified by the user.
/// \param useSASL to hint to use SASL authentication system if available.
/// false will specify to use NonSASL XEP-0078: Non-SASL Authentication
/// If the specified one is not availbe, library will use the othe one

void QXmppConfiguration::setUseSASLAuthentication(bool useSASL)
{
    m_useSASLAuthentication = useSASL;
}

/// Returns the specified security mode for the stream. The default value is
/// QXmppConfiguration::TLSEnabled.
/// \return StreamSecurityMode

QXmppConfiguration::StreamSecurityMode QXmppConfiguration::streamSecurityMode() const
{
    return m_streamSecurityMode;
}

/// Specifies the specified security mode for the stream. The default value is
/// QXmppConfiguration::TLSEnabled.
/// \param mode StreamSecurityMode

void QXmppConfiguration::setStreamSecurityMode(
        QXmppConfiguration::StreamSecurityMode mode)
{
    m_streamSecurityMode = mode;
}

/// Returns the Non-SASL authentication mechanism configuration.
///
/// \return QXmppConfiguration::NonSASLAuthMechanism
///

QXmppConfiguration::NonSASLAuthMechanism QXmppConfiguration::nonSASLAuthMechanism() const
{
    return m_nonSASLAuthMechanism;
}

/// Hints the library the Non-SASL authentication mechanism to be used for authentication.
///
/// \param mech QXmppConfiguration::NonSASLAuthMechanism
///

void QXmppConfiguration::setNonSASLAuthMechanism(
        QXmppConfiguration::NonSASLAuthMechanism mech)
{
    m_nonSASLAuthMechanism = mech;
}

/// Returns the SASL authentication mechanism configuration.
///
/// \return QXmppConfiguration::SASLAuthMechanism
///

QXmppConfiguration::SASLAuthMechanism QXmppConfiguration::sASLAuthMechanism() const
{
    return QXmppSaslMechanism::fromString(m_SASLAuthMechanism);
}

/// Hints the library the SASL authentication mechanism to be used for authentication.
///
/// \param mech QXmppConfiguration::SASLAuthMechanism
///

void QXmppConfiguration::setSASLAuthMechanism(
        QXmppConfiguration::SASLAuthMechanism mech)
{
    m_SASLAuthMechanism = QXmppSaslMechanism::toString(mech);
}

/// Returns the SASL authentication mechanism configuration.
///
/// \return QXmppConfiguration::SASLAuthMechanism
///

QString QXmppConfiguration::sASLAuthMechanismString() const
{
    return m_SASLAuthMechanism;
}

/// Hints the library the SASL authentication mechanism to be used for authentication.
///
/// \param mech QXmppConfiguration::SASLAuthMechanism
///

void QXmppConfiguration::setSASLAuthMechanismString(const QString &mech)
{
    m_SASLAuthMechanism = mech;
}

/// Specifies the network proxy used for the connection made by QXmppClient.
/// The default value is QNetworkProxy::DefaultProxy that is the proxy is
/// determined based on the application proxy set using
/// QNetworkProxy::setApplicationProxy().
/// \param proxy QNetworkProxy

void QXmppConfiguration::setNetworkProxy(const QNetworkProxy& proxy)
{
    m_networkProxy = proxy;
}

/// Returns the specified network proxy.
/// The default value is QNetworkProxy::DefaultProxy that is the proxy is
/// determined based on the application proxy set using
/// QNetworkProxy::setApplicationProxy().
/// \return QNetworkProxy

QNetworkProxy QXmppConfiguration::networkProxy() const
{
    return m_networkProxy;
}

/// Specifies the interval in seconds at which keep alive (ping) packets
/// will be sent to the server.
///
/// If set to zero, no keep alive packets will be sent.
///
/// The default value is 60 seconds.

void QXmppConfiguration::setKeepAliveInterval(int secs)
{
    m_keepAliveInterval = secs;
}

/// Returns the keep alive interval in seconds.
///
/// The default value is 60 seconds.

int QXmppConfiguration::keepAliveInterval() const
{
    return m_keepAliveInterval;
}

/// Specifies the maximum time in seconds to wait for a keep alive response
/// from the server before considering we are disconnected.
///
/// If set to zero or a value larger than the keep alive interval,
/// no timeout will occur.
///
/// The default value is 20 seconds.

void QXmppConfiguration::setKeepAliveTimeout(int secs)
{
    m_keepAliveTimeout = secs;
}

/// Returns the keep alive timeout in seconds.
///
/// The default value is 20 seconds.

int QXmppConfiguration::keepAliveTimeout() const
{
    return m_keepAliveTimeout;
}

/// Specifies a list of trusted CA certificates.

void QXmppConfiguration::setCaCertificates(const QList<QSslCertificate> &caCertificates)
{
    m_caCertificates = caCertificates;
}

/// Returns the a list of trusted CA certificates.

QList<QSslCertificate> QXmppConfiguration::caCertificates() const
{
    return m_caCertificates;
}

