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

#ifndef QXMPPSASLAUTH_H
#define QXMPPSASLAUTH_H

#include <QByteArray>
#include <QMap>
#include <QString>

#include "QXmppConfiguration.h"
#include "QXmppGlobal.h"
#include "QXmppLogger.h"

class QXMPP_EXPORT QXmppSaslMechanism : public QXmppLoggable
{
    Q_OBJECT
    Q_DISABLE_COPY(QXmppSaslMechanism)

public:
    QXmppSaslMechanism(QObject *parent, const QString &identifier, bool needsInitialResponse = false);
    virtual ~QXmppSaslMechanism() = 0;

    void setConfiguration(QXmppConfiguration *);
    const QString &identifier() const;
    bool needsInitialResponse() const;
    virtual QByteArray authText() const;
    virtual bool challengeResponse(const QByteArray &challenge,
                                   QByteArray &response,
                                   unsigned int step);

    static QXmppConfiguration::SASLAuthMechanism Q_DECL_DEPRECATED fromString(const QString &);
    static QString Q_DECL_DEPRECATED toString(QXmppConfiguration::SASLAuthMechanism);

protected:
    QXmppConfiguration *m_configuration;

private:
    QString m_identifier;
    bool m_needsInitialResponse;
};

class QXMPP_EXPORT QXmppSaslAnonymous : public QXmppSaslMechanism
{
    Q_OBJECT
    Q_DISABLE_COPY(QXmppSaslAnonymous)

public:
    QXmppSaslAnonymous(QObject *parent = 0);
};

class QXMPP_EXPORT QXmppSaslPlain : public QXmppSaslMechanism
{
    Q_OBJECT
    Q_DISABLE_COPY(QXmppSaslPlain)

public:
    QXmppSaslPlain(QObject *parent = 0);

    QByteArray authText() const;
};

class QXMPP_EXPORT QXmppSaslDigestMd5 : public QXmppSaslMechanism
{
    Q_OBJECT
    Q_DISABLE_COPY(QXmppSaslDigestMd5)

public:
    QXmppSaslDigestMd5(QObject *parent = 0);

    QByteArray authzid() const;
    void setAuthzid(const QByteArray &cnonce);

    QByteArray cnonce() const;
    void setCnonce(const QByteArray &cnonce);

    QByteArray digestUri() const;
    void setDigestUri(const QByteArray &digestUri);

    QByteArray nc() const;
    void setNc(const QByteArray &nc);

    QByteArray nonce() const;
    void setNonce(const QByteArray &nonce);

    QByteArray qop() const;
    void setQop(const QByteArray &qop);

    void setSecret(const QByteArray &secret);

    QByteArray calculateDigest(const QByteArray &A2) const;
    QByteArray calculateDigest(const QByteArray &A1, const QByteArray &A2) const;
    static QByteArray generateNonce();

    // message parsing and serialization
    static QMap<QByteArray, QByteArray> parseMessage(const QByteArray &ba);
    static QByteArray serializeMessage(const QMap<QByteArray, QByteArray> &map);

    bool challengeResponse(const QByteArray &challenge,
                           QByteArray &response,
                           unsigned int step);

private:
    QByteArray m_authzid;
    QByteArray m_cnonce;
    QByteArray m_digestUri;
    QByteArray m_nc;
    QByteArray m_nonce;
    QByteArray m_qop;
    QByteArray m_secret;
};

class QXMPP_EXPORT QXmppSaslFacebook : public QXmppSaslMechanism
{
    Q_OBJECT
    Q_DISABLE_COPY(QXmppSaslFacebook)

public:
    QXmppSaslFacebook(QObject *parent = 0);

    bool challengeResponse(const QByteArray &challenge,
                           QByteArray &response,
                           unsigned int step);
};

#endif
