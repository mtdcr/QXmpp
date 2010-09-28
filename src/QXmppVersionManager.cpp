/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

#include <QCoreApplication>
#include <QDomElement>

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppGlobal.h"
#include "QXmppVersionManager.h"
#include "QXmppVersionIq.h"

QXmppVersionManager::QXmppVersionManager() : QXmppClientExtension(),
    m_clientName(qApp->applicationName()),
    m_clientVersion(qApp->applicationVersion())
{
    if(m_clientName.isEmpty())
        m_clientName = "Based on QXmpp";

#if defined(Q_OS_LINUX)
    m_clientOs = QString::fromLatin1("Linux");
#elif defined(Q_OS_MAC)
    m_clientOs = QString::fromLatin1("Mac OS");
#elif defined(Q_OS_SYMBIAN)
    m_clientOs = QString::fromLatin1("Symbian");
#elif defined(Q_OS_WIN)
    m_clientOs = QString::fromLatin1("Windows");
#endif

    if(m_clientVersion.isEmpty())
        m_clientVersion = QXmppVersion();
}

QStringList QXmppVersionManager::discoveryFeatures() const
{
    // XEP-0092: Software Version
    return QStringList() << ns_version;
}

bool QXmppVersionManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == "iq" && QXmppVersionIq::isVersionIq(element))
    {
        QXmppVersionIq versionIq;
        versionIq.parse(element);

        if(versionIq.type() == QXmppIq::Get)
        {
            // respond to query
            QXmppVersionIq responseIq;
            responseIq.setType(QXmppIq::Result);
            responseIq.setId(versionIq.id());
            responseIq.setTo(versionIq.from());

            responseIq.setName(clientName());
            responseIq.setVersion(clientVersion());
            responseIq.setOs(clientOs());

            // TODO set OS aswell
            client()->sendPacket(responseIq);
        }

        emit versionReceived(versionIq);
        return true;
    }

    return false;
}

QString QXmppVersionManager::requestVersion(const QString& jid)
{
    QXmppVersionIq request;
    request.setType(QXmppIq::Get);
    request.setFrom(client()->configuration().jid());
    request.setTo(jid);
    if(client()->sendPacket(request))
        return request.id();
    else
        return "";
}

void QXmppVersionManager::setClientName(const QString& name)
{
    m_clientName = name;
}

void QXmppVersionManager::setClientVersion(const QString& version)
{
    m_clientVersion = version;
}

void QXmppVersionManager::setClientOs(const QString& os)
{
    m_clientOs = os;
}

QString QXmppVersionManager::clientName()
{
    return m_clientName;
}

QString QXmppVersionManager::clientVersion()
{
    return m_clientVersion;
}

QString QXmppVersionManager::clientOs()
{
    return m_clientOs;
}
