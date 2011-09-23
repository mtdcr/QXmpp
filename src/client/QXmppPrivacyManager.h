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

#ifndef QXMPPPRIVACYMANAGER_H
#define QXMPPPRIVACYMANAGER_H

#include <QMap>
#include <QSet>
#include <QStringList>

#include "QXmppClientExtension.h"
#include "QXmppPresence.h"
#include "QXmppPrivacyIq.h"

class QXmppPrivacyManagerPrivate;

/// \brief The QXmppPrivacyManager class provides access to
/// XEP-0016 Privacy Lists.

class QXmppPrivacyManager : public QXmppClientExtension
{
    Q_OBJECT
    Q_DISABLE_COPY(QXmppPrivacyManager)

public:
    QXmppPrivacyManager();
    ~QXmppPrivacyManager();

    bool setActiveList(const QString &name);
    bool setDefaultList(const QString &name);
    bool editList(const QString &name, const QList<QXmppPrivacyIq::List::Item> &items);
    bool removeList(const QString &name);

    /// \cond
    virtual QStringList discoveryFeatures() const;
    virtual bool handleStanza(const QDomElement &element);
    /// \endcond

signals:
    void listChanged(const QString &listName);
    void listReceived(const QString &listName, const QList<QXmppPrivacyIq::List::Item> &items);
    void listNamesReceived(const QString &activeName, const QString &defaultName, const QSet<QString> &listNames);

private:
    void privacyIqReceived(const QXmppPrivacyIq&);

    QXmppPrivacyManagerPrivate * const d;
};

#endif // QXMPPPRIVACYMANAGER_H
