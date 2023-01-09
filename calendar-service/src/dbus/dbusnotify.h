/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     kongyunzhen <kongyunzhen@uniontech.com>
*
* Maintainer: kongyunzhen <kongyunzhen@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DBUSNOTIFY_H
#define DBUSNOTIFY_H
#include <QtDBus/QtDBus>

class DBusNotify : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit DBusNotify(const QString &service, const QString &path, const QString &interface, const QDBusConnection &connection, QObject *parent = nullptr);
    inline int Notify(const QList<QVariant> &argumentList)
    {
        QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("Notify"), argumentList);

        int notifyid = -1;
        if (QDBusMessage::ReplyMessage == reply.type()) {
            qDebug() << reply.type() << reply.errorName() << reply.errorMessage();
            QDBusReply<quint32> id = reply;
            if (id.isValid()) {
                notifyid = id.value();
            }
        } else {
            qDebug() << reply.type() << reply.errorName() << reply.errorMessage();
        }
        return notifyid;
    }

    /**
     * @brief closeNotification 根据通知ID关闭桌面顶部通知
     * @param notifyID          通知ID
     */
    void closeNotification(quint32 notifyID);
Q_SIGNALS: // SIGNALS
    Q_SCRIPTABLE void NotificationClosed(quint32 id, quint32 reason);
    Q_SCRIPTABLE void ActionInvoked(quint32 id, const QString &reason);
};

#endif // DBUSNOTIFY_H
