/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Authors:
 *  Andreas Oberritter
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

#include <QDomElement>

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppPrivacyIq.h"
#include "QXmppPrivacyManager.h"
#include "QXmppUtils.h"

class QXmppPrivacyManagerPrivate
{
public:
    // map of list name and its privacy IQ items
    QMap<QString, QList<QXmppPrivacyIq::List::Item> > lists;
    QString activeList;
    QString defaultList;
    QSet<QString> listNames;
};

QXmppPrivacyManager::QXmppPrivacyManager() :
	QXmppClientExtension(),
	d(new QXmppPrivacyManagerPrivate())
{
}

QXmppPrivacyManager::~QXmppPrivacyManager()
{
    delete d;
}

bool QXmppPrivacyManager::setActiveList(const QString &activeList)
{
    if (d->listNames.contains(activeList)) {
        QXmppPrivacyIq iq(QXmppIq::Set);
        iq.setActiveList(activeList);
        if (client()->sendPacket(iq)) {
            d->activeList = activeList;
            return true;
        }
    }

    return false;
}

bool QXmppPrivacyManager::setDefaultList(const QString &defaultList)
{
    if (d->listNames.contains(defaultList)) {
        QXmppPrivacyIq iq(QXmppIq::Set);
        iq.setDefaultList(defaultList);
        if (client()->sendPacket(iq)) {
            d->defaultList = defaultList;
            return true;
        }
    }

    return false;
}

bool QXmppPrivacyManager::editList(const QString &name, const QList<QXmppPrivacyIq::List::Item> &items)
{
    QXmppPrivacyIq::List list(name);
    list.setItems(items);

    QXmppPrivacyIq iq(QXmppIq::Set);
    iq.addList(list);

    if (client()->sendPacket(iq)) {
        if (items.isEmpty()) {
            d->listNames.remove(name);
            d->lists.remove(name);
        } else {
            d->listNames.insert(name);
            d->lists[name] = items;
        }
        return true;
    }

    return false;
}

bool QXmppPrivacyManager::removeList(const QString &name)
{
	return editList(name, QList<QXmppPrivacyIq::List::Item>());
}

QStringList QXmppPrivacyManager::discoveryFeatures() const
{
    return QStringList(ns_privacy);
}

bool QXmppPrivacyManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == "iq" && QXmppPrivacyIq::isPrivacyIq(element))
    {
        QXmppPrivacyIq privacyIq;
        privacyIq.parse(element);

        // Security check: only server should send this iq
        // from() should be either empty or bareJid of the user
        QString fromJid = privacyIq.from();
        if(fromJid.isEmpty() ||
           QXmppUtils::jidToBareJid(fromJid) == client()->configuration().jidBare())
        {
            privacyIqReceived(privacyIq);
            return true;
        }
    }

    return false;
}

void QXmppPrivacyManager::privacyIqReceived(const QXmppPrivacyIq& privacyIq)
{
    const QList<QXmppPrivacyIq::List> &lists = privacyIq.lists();
    // privacy list pushes must contain only one list child element
    const QString &activeList = privacyIq.activeList();
    const QString &defaultList = privacyIq.defaultList();
    bool maybePrivacyList = activeList.isEmpty() &&
                            defaultList.isEmpty() &&
                            lists.size() == 1;

    switch (privacyIq.type()) {
    case QXmppIq::Set:
        {
            // send result iq
            QXmppIq returnIq(QXmppIq::Result);
            returnIq.setId(privacyIq.id());
            client()->sendPacket(returnIq);

            // notify the user that a privacy list was changed
            if (maybePrivacyList)
                emit listChanged(lists.at(0).name());
            break;
        }
    case QXmppIq::Result:
        {
            // only non-empty lists are privacy lists. in other
            // cases it is an index.
            if (maybePrivacyList) {
                const QXmppPrivacyIq::List &list = lists.at(0);
                if (list.items().size() > 0) {
                    const QString &name = list.name();
                    const QList<QXmppPrivacyIq::List::Item> &items = list.items();
                    d->lists[name] = items;
                    emit listReceived(name, items);
                    break;
                }
            }

            d->activeList = activeList;
            d->defaultList = defaultList;

            d->listNames.clear();
            for (int i = 0; i < lists.size(); ++i) {
                const QXmppPrivacyIq::List &list = lists.at(i);
                d->listNames.insert(list.name());
            }

            emit listNamesReceived(d->activeList, d->defaultList, d->listNames);
            break;
        }
    default:
        break;
    }
}
