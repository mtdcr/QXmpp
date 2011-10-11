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

#include <cstdlib>

#include <QCryptographicHash>
#include <QUrl>

#include "QXmppClient.h"
#include "QXmppConfiguration.h"
#include "QXmppSaslAuth.h"
#include "QXmppUtils.h"

QXmppSaslMechanism::QXmppSaslMechanism(QObject *parent, const QString &identifier, bool needsInitialResponse) :
    QXmppLoggable(parent),
    m_configuration(0),
    m_identifier(identifier),
    m_needsInitialResponse(needsInitialResponse)
{
}

QXmppSaslMechanism::~QXmppSaslMechanism()
{
}

QXmppConfiguration::SASLAuthMechanism QXmppSaslMechanism::fromString(const QString &mech)
{
    if (mech == "PLAIN")
        return QXmppConfiguration::SASLPlain;
    if (mech == "DIGEST-MD5")
        return QXmppConfiguration::SASLDigestMD5;
    if (mech == "ANONYMOUS")
        return QXmppConfiguration::SASLAnonymous;
    if (mech == "X-FACEBOOK-PLATFORM")
        return QXmppConfiguration::SASLXFacebookPlatform;

    return static_cast<QXmppConfiguration::SASLAuthMechanism>(-1);
}

QString QXmppSaslMechanism::toString(QXmppConfiguration::SASLAuthMechanism mech)
{
    switch (mech) {
    case QXmppConfiguration::SASLPlain:
        return QLatin1String("PLAIN");
    case QXmppConfiguration::SASLDigestMD5:
        return QLatin1String("DIGEST-MD5");
    case QXmppConfiguration::SASLAnonymous:
        return QLatin1String("ANONYMOUS");
    case QXmppConfiguration::SASLXFacebookPlatform:
        return QLatin1String("X-FACEBOOK-PLATFORM");
    }

    return QString();
}

void QXmppSaslMechanism::setConfiguration(QXmppConfiguration *configuration)
{
    m_configuration = configuration;
}

QByteArray QXmppSaslMechanism::authText() const
{
    return QByteArray();
}

bool QXmppSaslMechanism::challengeResponse(const QByteArray &challenge,
                                           QByteArray &response,
                                           unsigned int step)
{
    Q_UNUSED(challenge)
    Q_UNUSED(response)
    Q_UNUSED(step)

    warning("Unexpected SASL challenge");
    return false;
}

const QString &QXmppSaslMechanism::identifier() const
{
    return m_identifier;
}

bool QXmppSaslMechanism::needsInitialResponse() const
{
    return m_needsInitialResponse;
}



QXmppSaslAnonymous::QXmppSaslAnonymous(QObject *parent) :
    QXmppSaslMechanism(parent, QLatin1String("ANONYMOUS"))
{
}



QXmppSaslPlain::QXmppSaslPlain(QObject *parent) :
    QXmppSaslMechanism(parent, QLatin1String("PLAIN"))
{
}

QByteArray QXmppSaslPlain::authText() const
{
    return QString('\0' + m_configuration->user() + '\0' + m_configuration->password()).toUtf8();
}



QXmppSaslDigestMd5::QXmppSaslDigestMd5(QObject *parent) :
    QXmppSaslMechanism(parent, QLatin1String("DIGEST-MD5"))
{
}

QByteArray QXmppSaslDigestMd5::authzid() const
{
    return m_authzid;
}

void QXmppSaslDigestMd5::setAuthzid(const QByteArray &authzid)
{
    m_authzid = authzid;
}

QByteArray QXmppSaslDigestMd5::cnonce() const
{
    return m_cnonce;
}

void QXmppSaslDigestMd5::setCnonce(const QByteArray &cnonce)
{
    m_cnonce = cnonce;
}

QByteArray QXmppSaslDigestMd5::digestUri() const
{
    return m_digestUri;
}

void QXmppSaslDigestMd5::setDigestUri(const QByteArray &digestUri)
{
    m_digestUri = digestUri;
}

QByteArray QXmppSaslDigestMd5::nc() const
{
    return m_nc;
}

void QXmppSaslDigestMd5::setNc(const QByteArray &nc)
{
    m_nc = nc;
}

QByteArray QXmppSaslDigestMd5::nonce() const
{
    return m_nonce;
}

void QXmppSaslDigestMd5::setNonce(const QByteArray &nonce)
{
    m_nonce = nonce;
}

QByteArray QXmppSaslDigestMd5::qop() const
{
    return m_qop;
}

void QXmppSaslDigestMd5::setQop(const QByteArray &qop)
{
    m_qop = qop;
}

void QXmppSaslDigestMd5::setSecret(const QByteArray &secret)
{
    m_secret = secret;
}

QByteArray QXmppSaslDigestMd5::generateNonce()
{
    QByteArray nonce = QXmppUtils::generateRandomBytes(32);

    // The random data can the '=' char is not valid as it is a delimiter,
    // so to be safe, base64 the nonce
    return nonce.toBase64();
}

/// Calculate digest response for use with XMPP/SASL.
///
/// \param A2
///

QByteArray QXmppSaslDigestMd5::calculateDigest(const QByteArray &A2) const
{
    QByteArray ha1 = m_secret + ':' + m_nonce + ':' + m_cnonce;

    if (!m_authzid.isEmpty())
        ha1 += ':' + m_authzid;

    return calculateDigest(ha1, A2);
}

/// Calculate generic digest response.
///
/// \param A1
/// \param A2
///

QByteArray QXmppSaslDigestMd5::calculateDigest(const QByteArray &A1, const QByteArray &A2) const
{
    QByteArray HA1 = QCryptographicHash::hash(A1, QCryptographicHash::Md5).toHex();
    QByteArray HA2 = QCryptographicHash::hash(A2, QCryptographicHash::Md5).toHex();
    QByteArray KD;
    if (m_qop == "auth" || m_qop == "auth-int")
        KD = HA1 + ':' + m_nonce + ':' + m_nc + ':' + m_cnonce + ':' + m_qop + ':' + HA2;
    else
        KD = HA1 + ':' + m_nonce + ':' + HA2;
    return QCryptographicHash::hash(KD, QCryptographicHash::Md5).toHex();
}

QMap<QByteArray, QByteArray> QXmppSaslDigestMd5::parseMessage(const QByteArray &ba)
{
    QMap<QByteArray, QByteArray> map;
    int startIndex = 0;
    int pos = 0;
    while ((pos = ba.indexOf("=", startIndex)) >= 0)
    {
        // key get name and skip equals
        const QByteArray key = ba.mid(startIndex, pos - startIndex).trimmed();
        pos++;

        // check whether string is quoted
        if (ba.at(pos) == '"')
        {
            // skip opening quote
            pos++;
            int endPos = ba.indexOf('"', pos);
            // skip quoted quotes
            while (endPos >= 0 && ba.at(endPos - 1) == '\\')
                endPos = ba.indexOf('"', endPos + 1);
            if (endPos < 0)
            {
                qWarning("Unfinished quoted string");
                return map;
            }
            // unquote
            QByteArray value = ba.mid(pos, endPos - pos);
            value.replace("\\\"", "\"");
            value.replace("\\\\", "\\");
            map[key] = value;
            // skip closing quote and comma
            startIndex = endPos + 2;
        } else {
            // non-quoted string
            int endPos = ba.indexOf(',', pos);
            if (endPos < 0)
                endPos = ba.size();
            map[key] = ba.mid(pos, endPos - pos);
            // skip comma
            startIndex = endPos + 1;
        }
    }
    return map;
}

QByteArray QXmppSaslDigestMd5::serializeMessage(const QMap<QByteArray, QByteArray> &map)
{
    QByteArray ba;
    foreach (const QByteArray &key, map.keys())
    {
        if (!ba.isEmpty())
            ba.append(',');
        ba.append(key + "=");
        QByteArray value = map[key];
        const char *separators = "()<>@,;:\\\"/[]?={} \t";
        bool quote = false;
        for (const char *c = separators; *c; c++)
        {
            if (value.contains(*c))
            {
                quote = true;
                break;
            }
        }
        if (quote)
        {
            value.replace("\\", "\\\\");
            value.replace("\"", "\\\"");
            ba.append("\"" + value + "\"");
        }
        else
            ba.append(value);
    }
    return ba;
}

bool QXmppSaslDigestMd5::challengeResponse(const QByteArray &challenge,
                                           QByteArray &response,
                                           unsigned int step)
{
    if (step != 1 && step != 2) {
        warning("QXmppSaslDigestMD5: Too many authentication steps");
        return false;
    }

    QMap<QByteArray, QByteArray> map = QXmppSaslDigestMd5::parseMessage(challenge);

    if (step == 1) {
        if (!map.contains("nonce"))
        {
            warning("QXmppSaslDigestMD5: Invalid input");
            return false;
        }

        const QByteArray &user = m_configuration->user().toUtf8();
        const QByteArray &pass = m_configuration->password().toUtf8();
        const QByteArray &domain = m_configuration->domain().toUtf8();

        setAuthzid(map.value("authzid"));
        setCnonce(QXmppSaslDigestMd5::generateNonce());
        setDigestUri("xmpp/" + domain);
        setNc("00000001");
        setNonce(map.value("nonce"));
        setQop("auth");
        setSecret(QCryptographicHash::hash(user + ":" + map.value("realm") + ":" + pass, QCryptographicHash::Md5));

        // Build response
        QMap<QByteArray, QByteArray> responseMap;
        responseMap["username"] = user;
        if (map.contains("realm"))
            responseMap["realm"] = map.value("realm");
        responseMap["nonce"] = nonce();
        responseMap["cnonce"] = cnonce();
        responseMap["nc"] = nc();
        responseMap["qop"] = qop();
        responseMap["digest-uri"] = digestUri();
        responseMap["response"] = calculateDigest(QByteArray("AUTHENTICATE:") + digestUri());

        if (!authzid().isEmpty())
            responseMap["authzid"] = authzid();
        responseMap["charset"] = "utf-8";

        response = serializeMessage(responseMap);
    } else {
        if (!map.contains("rspauth"))
        {
            warning("QXmppSaslDigestMD5: Invalid input");
            return false;
        }

        // check new challenge
        if (map["rspauth"] != calculateDigest(QByteArray(":") + digestUri()))
        {
            warning("QXmppSaslDigestMD5: Bad challenge");
            return false;
        }

        response = QByteArray();
    }

    return true;
}



QXmppSaslFacebook::QXmppSaslFacebook(QObject *parent) :
    QXmppSaslMechanism(parent, QLatin1String("X-FACEBOOK-PLATFORM"))
{
}

bool QXmppSaslFacebook::challengeResponse(const QByteArray &challenge,
                                          QByteArray &response,
                                          unsigned int step)
{
    if (step != 1) {
        warning("QXmppSaslFacebook: Too many authentication steps");
        return false;
    }

    // parse request
    QUrl request;
    request.setEncodedQuery(challenge);
    if (!request.hasQueryItem("method") || !request.hasQueryItem("nonce")) {
        warning("QXmppSaslFacebook: Invalid input");
        return false;
    }

    // build response
    QUrl query;
    query.addQueryItem("access_token", m_configuration->facebookAccessToken());
    query.addQueryItem("api_key", m_configuration->facebookAppId());
    query.addQueryItem("call_id", 0);
    query.addQueryItem("method", request.queryItemValue("method"));
    query.addQueryItem("nonce", request.queryItemValue("nonce"));
    query.addQueryItem("v", "1.0");

    response = query.encodedQuery();
    return true;
}
