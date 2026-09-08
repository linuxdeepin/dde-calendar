// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MOCK_CALDAV_SERVER_H
#define MOCK_CALDAV_SERVER_H

#include <QHash>
#include <QList>
#include <QMap>
#include <QUrl>
#include <QTcpServer>

class QSslSocket;

class MockCalDavServer : public QTcpServer
{
    Q_OBJECT
public:
    struct Request {
        QByteArray method;
        QByteArray target;
        QByteArray body;
        QMap<QByteArray, QByteArray> headers;
    };

    explicit MockCalDavServer(QObject *parent = nullptr);

    bool start(const QByteArray &certificate, const QByteArray &privateKey);
    QUrl url() const;
    const QList<Request> &requests() const;
    int connectionCount() const;
    void setResponseStatus(int status);
    void setResponseStatusForMethod(const QByteArray &method, int status);
    void clearResponseStatusForMethod(const QByteArray &method);
    void setResponseStatusForTarget(const QByteArray &target, int status);
    void setResponseBodyForTarget(const QByteArray &target, const QByteArray &body);
    void setCalendarMultiGetResponseStatus(int status);
    void setResponseEtag(const QByteArray &etag);
    void setCalendarQueryReturnsCalendarData(bool enabled);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    void processSocket(QSslSocket *socket);
    void sendResponse(QSslSocket *socket, const QByteArray &method, const QByteArray &target,
                      const QByteArray &requestBody);

    QByteArray m_certificate;
    QByteArray m_privateKey;
    int m_responseStatus = 207;
    QMap<QByteArray, int> m_methodResponseStatuses;
    QMap<QByteArray, int> m_targetResponseStatuses;
    QMap<QByteArray, QByteArray> m_targetResponseBodies;
    int m_calendarMultiGetResponseStatus = 0;
    QByteArray m_responseEtag;
    bool m_calendarQueryReturnsCalendarData = true;
    QList<Request> m_requests;
    int m_connectionCount = 0;
    QHash<QSslSocket *, QByteArray> m_buffers;
};

#endif // MOCK_CALDAV_SERVER_H
