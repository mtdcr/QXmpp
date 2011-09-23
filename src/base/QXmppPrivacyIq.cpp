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
#include <QXmlStreamWriter>

#include "QXmppPrivacyIq.h"
#include "QXmppConstants.h"
#include "QXmppUtils.h"

QXmppPrivacyIq::QXmppPrivacyIq(QXmppIq::Type type) :
    QXmppIq(type)
{
}

void QXmppPrivacyIq::addList(const List& list)
{
    m_lists.append(list);
}

QList<QXmppPrivacyIq::List> QXmppPrivacyIq::lists() const
{
    return m_lists;
}

bool QXmppPrivacyIq::isPrivacyIq(const QDomElement &element)
{
    return (element.firstChildElement("query").namespaceURI() == ns_privacy);
}

void QXmppPrivacyIq::parseElementFromChild(const QDomElement &element)
{
    const QDomElement &queryElement = element.firstChildElement("query");
    const QDomElement &activeElement = queryElement.firstChildElement("active");
    const QDomElement &defaultElement = queryElement.firstChildElement("default");
    QDomElement listElement = queryElement.firstChildElement("list");

    if (!activeElement.isNull())
        setActiveList(activeElement.attribute("name"));
    if (!defaultElement.isNull())
        setDefaultList(defaultElement.attribute("name"));

    while (!listElement.isNull()) {
        QXmppPrivacyIq::List list;
        list.parse(listElement);
        m_lists.append(list);
        listElement = listElement.nextSiblingElement();
    }
}

void QXmppPrivacyIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute("xmlns", ns_privacy);

    if (!m_activeList.isEmpty()) {
        writer->writeStartElement("active");
        writer->writeAttribute("name", m_activeList);
        writer->writeEndElement();
    }

    if (!m_defaultList.isEmpty()) {
        writer->writeStartElement("default");
        writer->writeAttribute("name", m_defaultList);
        writer->writeEndElement();
    }

    for (int i = 0; i < m_lists.count(); ++i)
        m_lists.at(i).toXml(writer);

    writer->writeEndElement();
}

QString QXmppPrivacyIq::activeList() const
{
    return m_activeList;
}

void QXmppPrivacyIq::setActiveList(const QString &activeList)
{
    m_activeList = activeList;
}

QString QXmppPrivacyIq::defaultList() const
{
    return m_defaultList;
}

void QXmppPrivacyIq::setDefaultList(const QString &defaultList)
{
    m_defaultList = defaultList;
}

QXmppPrivacyIq::List::List(const QString &name) :
    m_name(name)
{
}

QString QXmppPrivacyIq::List::name() const
{
    return m_name;
}

void QXmppPrivacyIq::List::setName(const QString &name)
{
    m_name = name;
}

/// Adds an item to the privacy list.
///
/// \param item

void QXmppPrivacyIq::List::addItem(const Item& item)
{
    m_items.append(item);
}

/// Returns the privacy list's items.

QList<QXmppPrivacyIq::List::Item> QXmppPrivacyIq::List::items() const
{
    return m_items;
}

void QXmppPrivacyIq::List::setItems(const QList<QXmppPrivacyIq::List::Item> &items)
{
    m_items = items;
}

void QXmppPrivacyIq::List::parse(const QDomElement &element)
{
    QDomElement itemElement = element.firstChildElement("item");

    setName(element.attribute("name"));

    while (!itemElement.isNull())
    {
        QXmppPrivacyIq::List::Item item;
        item.parse(itemElement);
        m_items.append(item);
        itemElement = itemElement.nextSiblingElement();
    }
}

void QXmppPrivacyIq::List::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("list");
    writer->writeAttribute("name", m_name);

    for (int i = 0; i < m_items.count(); ++i)
        m_items.at(i).toXml(writer);

    writer->writeEndElement();
}

QXmppPrivacyIq::List::Item::Item() :
    m_type(QXmppPrivacyIq::List::Item::NoType),
    m_action(QXmppPrivacyIq::List::Item::NoAction),
    m_order(0),
    m_message(false),
    m_presenceIn(false),
    m_presenceOut(false),
    m_iq(false)
{
}

/// Returns the type of the privacy list item.
///
/// \return type as a Type
///

QXmppPrivacyIq::List::Item::Type QXmppPrivacyIq::List::Item::type() const
{
    return m_type;
}

/// Sets the type of the privacy list item.
///
/// \param type
///

void QXmppPrivacyIq::List::Item::setType(Type type)
{
    m_type = type;
}

QString QXmppPrivacyIq::List::Item::value() const
{
    return m_value;
}

void QXmppPrivacyIq::List::Item::setValue(const QString &value)
{
    m_value = value;
}

QXmppPrivacyIq::List::Item::Action QXmppPrivacyIq::List::Item::action() const
{
    return m_action;
}

void QXmppPrivacyIq::List::Item::setAction(Action action)
{
    m_action = action;
}

unsigned int QXmppPrivacyIq::List::Item::order() const
{
    return m_order;
}

void QXmppPrivacyIq::List::Item::setOrder(unsigned int order)
{
    m_order = order;
}

bool QXmppPrivacyIq::List::Item::message() const
{
    return m_message;
}

void QXmppPrivacyIq::List::Item::setMessage(bool message)
{
    m_message = message;
}

bool QXmppPrivacyIq::List::Item::presenceIn() const
{
    return m_presenceIn;
}

void QXmppPrivacyIq::List::Item::setPresenceIn(bool presenceIn)
{
    m_presenceIn = presenceIn;
}

bool QXmppPrivacyIq::List::Item::presenceOut() const
{
    return m_presenceOut;
}

void QXmppPrivacyIq::List::Item::setPresenceOut(bool presenceOut)
{
    m_presenceOut = presenceOut;
}

bool QXmppPrivacyIq::List::Item::iq() const
{
    return m_iq;
}

void QXmppPrivacyIq::List::Item::setIq(bool iq)
{
    m_iq = iq;
}

void QXmppPrivacyIq::List::Item::parse(const QDomElement &element)
{
    setTypeFromStr(element.attribute("type"));
    setValue(element.attribute("value"));
    setActionFromStr(element.attribute("action"));
    setOrder(element.attribute("order").toUInt());

    setMessage(!element.firstChildElement("message").isNull());
    setPresenceIn(!element.firstChildElement("presence-in").isNull());
    setPresenceOut(!element.firstChildElement("presence-out").isNull());
    setIq(!element.firstChildElement("iq").isNull());
}

void QXmppPrivacyIq::List::Item::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("item");
    helperToXmlAddAttribute(writer, "type", getTypeStr());
    helperToXmlAddAttribute(writer, "value", m_value);
    helperToXmlAddAttribute(writer, "action", getActionStr());
    helperToXmlAddAttribute(writer, "order", QString::number(m_order));

    if (m_message)
        writer->writeEmptyElement("message");
    if (m_presenceIn)
        writer->writeEmptyElement("presence-in");
    if (m_presenceOut)
        writer->writeEmptyElement("presence-out");
    if (m_iq)
        writer->writeEmptyElement("iq");

    writer->writeEndElement();
}

QString QXmppPrivacyIq::List::Item::getTypeStr() const
{
    switch (m_type)
    {
    case NoType:
        return "";
    case Jid:
        return "jid";
    case Group:
        return "group";
    case Subscription:
        return "subscription";
    default:
        {
            qWarning("QXmppPrivacyIq::List::Item::getTypeStr(): invalid type");
            return "";
        }
    }
}

void QXmppPrivacyIq::List::Item::setTypeFromStr(const QString& str)
{
    if (str == "")
        setType(NoType);
    else if (str == "jid")
        setType(Jid);
    else if (str == "group")
        setType(Group);
    else if (str == "subscription")
        setType(Subscription);
    else
        qWarning("QXmppPrivacyIq::List::Item::setTypeFromStr(): invalid type");
}

QString QXmppPrivacyIq::List::Item::getActionStr() const
{
    switch (m_action)
    {
    case NoAction:
        return "";
    case Allow:
        return "allow";
    case Deny:
        return "deny";
    default:
        {
            qWarning("QXmppPrivacyIq::List::Item::getActionStr(): invalid action: ");
            return "";
        }
    }
}

void QXmppPrivacyIq::List::Item::setActionFromStr(const QString& str)
{
    if (str == "")
        setAction(NoAction);
    else if (str == "allow")
        setAction(Allow);
    else if (str == "deny")
        setAction(Deny);
    else
        qWarning("QXmppPrivacyIq::List::Item::setActionFromStr(): invalid action");
}
