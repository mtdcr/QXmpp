/*
 * Copyright (C) 2008-2009 QXmpp Developers
 *
 * Author:
 *	Ian Reinhart Geiser
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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


#ifndef IBBCLIENT_H
#define IBBCLIENT_H

#include "QXmppClient.h"
#include "QXmppTransferManager.h"

class QBuffer;

class xmppClient : public QXmppClient
{
    Q_OBJECT

public:
    xmppClient(QObject *parent = 0);
    void sendOnceAvailable(const QString &recipient, const QString &file);

private slots:
    void slotConnected();
    void slotError(QXmppTransferJob::Error error);
    void slotFileReceived(QXmppTransferJob *job);
    void slotFinished();
    void slotProgress(qint64 done, qint64 total);

private:
    QString m_sendFile;
    QString m_sendRecipient;
};

#endif // IBBCLIENT_H
