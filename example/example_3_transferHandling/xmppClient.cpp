/*
 * Copyright (C) 2008-2010 QXmpp Developers
 *
 * Authors:
 *	Ian Reinhart Geiser
 *  Jeremy Lainé
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

#include <QBuffer>
#include <QDebug>

#include "QXmppMessage.h"
#include "QXmppUtils.h"

#include "xmppClient.h"

xmppClient::xmppClient(QObject *parent)
    : QXmppClient(parent)
{
    // comment the following to use all available methods (highly recommended)
    transferManager().setSupportedMethods(QXmppTransferJob::InBandMethod);

    bool check = connect(this, SIGNAL(connected()),
                         this, SLOT(slotConnected()) );
    Q_ASSERT(check);

    check = connect(&transferManager(), SIGNAL(fileReceived(QXmppTransferJob*)),
                    this, SLOT(slotFileReceived(QXmppTransferJob*)));
    Q_ASSERT(check);
}

/// Request that the given file be sent to the recipient once he/she is online.
///
/// \param recipient
/// \param file

void xmppClient::sendOnceAvailable(const QString &recipient, const QString &file)
{
    m_sendRecipient = recipient;
    m_sendFile = file;
}

void xmppClient::slotConnected()
{
    const QLatin1String recipient("client@geiseri.com/QXmpp");

    // if we are the recipient, do nothing
    if (getConfiguration().jid() == recipient)
        return;

    QXmppTransferJob *job = transferManager().sendFile(recipient, "xmppClient.cpp");

    bool check = connect( job, SIGNAL(error(QXmppTransferJob::Error)),
             this, SLOT(slotError(QXmppTransferJob::Error)) );
    Q_ASSERT(check);

    check = connect( job, SIGNAL(finished()),
             this, SLOT(slotFinished()) );
    Q_ASSERT(check);

    check = connect( job, SIGNAL(progress(qint64,qint64)),
             this, SLOT(slotProgress(qint64,qint64)) );
    Q_ASSERT(check);
}

/// A file transfer failed.

void xmppClient::slotError(QXmppTransferJob::Error error)
{
    qDebug() << "Transmission failed:" << error;
}

/// A file transfer request was received.

void xmppClient::slotFileReceived(QXmppTransferJob *job)
{
    qDebug() << "Got transfer request from:" << job->jid();

    bool check = connect(job, SIGNAL(error(QXmppTransferJob::Error)), this, SLOT(slotError(QXmppTransferJob::Error)));
    Q_ASSERT(check);

    check = connect(job, SIGNAL(finished()), this, SLOT(slotFinished()));
    Q_ASSERT(check);

    check = connect(job, SIGNAL(progress(qint64,qint64)), this, SLOT(slotProgress(qint64,qint64)));
    Q_ASSERT(check);

    // allocate a buffer to receive the file
    QBuffer *buffer = new QBuffer(this);
    buffer->open(QIODevice::WriteOnly);
    job->accept(buffer);
}

/// A file transfer finished.

void xmppClient::slotFinished()
{
    qDebug() << "Transmission finished";
}

/// A file transfer has made progress.

void xmppClient::slotProgress(qint64 done, qint64 total)
{
    qDebug() << "Transmission progress:" << done << "/" << total;
}
