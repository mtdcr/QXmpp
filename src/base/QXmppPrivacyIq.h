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


#ifndef QXMPPPRIVACYIQ_H
#define QXMPPPRIVACYIQ_H

#include "QXmppIq.h"
#include <QList>

/// \brief The QXmppPrivacyIq class represents a privacy list IQ.
///
/// \ingroup Stanzas

class QXmppPrivacyIq : public QXmppIq
{
public:
    /// \brief The QXmppPrivacyIq::List class represents a privacy list.
    class List
    {
    public:
        /// \brief The QXmppPrivacyIq::List::Item class represents a privacy list entry.
        class Item
        {
        public:
            /// An enumeration for type of item.
            enum Type
            {
                NoType = 0,       ///< default item
                Jid = 1,          ///< matches on a Jid
                Group = 2,        ///< matches on a Group membership
                Subscription = 3, ///< matches on a subscription status
            };

            enum Action
            {
                NoAction = 0,
                Allow = 1,
                Deny = 2,
            };

            Item();

            Type type() const;
            QString value() const;
            Action action() const;
            unsigned int order() const;
            bool message() const;
            bool presenceIn() const;
            bool presenceOut() const;
            bool iq() const;

            void setType(Type);
            void setValue(const QString&);
            void setAction(Action);
            void setOrder(unsigned int);
            void setMessage(bool);
            void setPresenceIn(bool);
            void setPresenceOut(bool);
            void setIq(bool);

            /// \cond
            void parse(const QDomElement &element);
            void toXml(QXmlStreamWriter *writer) const;
            /// \endcond

        private:
            QString getTypeStr() const;
            void setTypeFromStr(const QString&);

            QString getActionStr() const;
            void setActionFromStr(const QString&);

            Type m_type;
            QString m_value;
            Action m_action;
            unsigned int m_order;
            bool m_message;
            bool m_presenceIn;
            bool m_presenceOut;
            bool m_iq;
        };

        List(const QString &name = QString());

        /// \cond
        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;
        /// \endcond

        QString name() const;
        void setName(const QString &);

        void addItem(const Item&);
        void setItems(const QList<Item> &items);
        QList<Item> items() const;

    private:
        QString m_name;
        QList<QXmppPrivacyIq::List::Item> m_items;
    };

    QXmppPrivacyIq(QXmppIq::Type type = QXmppIq::Get);

    QString activeList() const;
    void setActiveList(const QString &);

    QString defaultList() const;
    void setDefaultList(const QString &);

    void addList(const List&);
    QList<List> lists() const;

    /// \cond
    static bool isPrivacyIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_activeList;
    QString m_defaultList;
    QList<QXmppPrivacyIq::List> m_lists;
};

#endif // QXMPPPRIVACYIQ_H
