// SPDX-FileCopyrightText: 2017 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "schedulecoormanage.h"
#include "commondef.h"

#include <QTime>

CScheduleCoorManage::CScheduleCoorManage()
{
    qCDebug(ClientLogger) << "Creating CScheduleCoorManage";
}

CScheduleCoorManage::~CScheduleCoorManage()
{
    qCDebug(ClientLogger) << "Destroying CScheduleCoorManage";
}

void CScheduleCoorManage::setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin)
{
    qCDebug(ClientLogger) << "Setting range with width:" << w << "height:" << h << "begin date:" << begindate.toString() << "end date:" << enddate.toString() << "right margin:" << rightmagin;
    m_width = w;
    m_height = h;
    m_rightmagin = rightmagin;
    m_begindate = begindate;
    m_enddate = enddate;
    m_totalDay = begindate.daysTo(enddate) + 1;
    qCDebug(ClientLogger) << "Total days:" << m_totalDay;
}

void CScheduleCoorManage::setDateRange(QDate begindate, QDate enddate)
{
    qCDebug(ClientLogger) << "Setting date range from:" << begindate.toString() << "to:" << enddate.toString();
    m_begindate = begindate;
    m_enddate = enddate;
    m_totalDay = begindate.daysTo(enddate) + 1;
    qCDebug(ClientLogger) << "Total days:" << m_totalDay;
}

QRectF CScheduleCoorManage::getDrawRegion(QDateTime begintime, QDateTime endtime)
{
    qCDebug(ClientLogger) << "Getting draw region from:" << begintime.toString() << "to:" << endtime.toString();
    QRectF rect;
    QString bb = begintime.toString("yyyyMMddhhmmsszzz");
    QString ee = endtime.toString("yyyyMMddhhmmsszzz");

    if (begintime > endtime) {
        qCDebug(ClientLogger) << "Begin time is after end time, returning empty rect";
        return rect;
    }

    QDate begindate = begintime.date();
    QDate enddate = endtime.date();
    QTime beginzero(0, 0, 0);
    QTime beginScheduleT = begintime.time();
    QTime endScheduleT = endtime.time();

    if (begindate < m_begindate || enddate > m_enddate) {
        qCDebug(ClientLogger) << "Date out of range, returning empty rect";
        return rect;
    }

    qint64 beginday = m_begindate.daysTo(begindate) + 1;
    qint64 day = begindate.daysTo(enddate) + 1;
    int ScheduleBT = beginzero.secsTo(beginScheduleT);
    int ScheduleET = beginzero.secsTo(endScheduleT);
    qreal rWidth = m_width * (1.0 * day / m_totalDay);
    qreal rHeight = m_height * ((ScheduleET - ScheduleBT) / 86400.0);
    qreal posX = m_width * (1.0 * (beginday - 1) / m_totalDay);
    qreal posY = m_height * (ScheduleBT / 86400.0);
    rect = QRectF(posX, posY, rWidth, rHeight);

    qCDebug(ClientLogger) << "Draw region calculated:" << rect;
    return rect;
}

QRectF CScheduleCoorManage::getDrawRegion(QDateTime begintime, QDateTime endtime, int index, int coount)
{
    qCDebug(ClientLogger) << "Getting draw region from:" << begintime.toString() << "to:" << endtime.toString() << "index:" << index << "count:" << coount;
    QRectF rect;
    QString bb = begintime.toString("yyyyMMddhhmmsszzz");
    QString ee = endtime.toString("yyyyMMddhhmmsszzz");

    if (begintime > endtime) {
        qCDebug(ClientLogger) << "Begin time is after end time, returning empty rect";
        return rect;
    }

    QDate begindate = begintime.date();
    QDate enddate = endtime.date();
    QTime beginzero(0, 0, 0);
    QTime beginScheduleT = begintime.time();
    QTime endScheduleT = endtime.time();

    if (begindate < m_begindate || enddate > m_enddate) {
        qCDebug(ClientLogger) << "Date out of range, returning empty rect";
        return rect;
    }
    qint64 beginday = m_begindate.daysTo(begindate) + 1;
    qint64 day = begindate.daysTo(enddate) + 1;
    int ScheduleBT = beginzero.secsTo(beginScheduleT);
    int ScheduleET = beginzero.secsTo(endScheduleT);
    qreal rWidth = m_width * (1.0 * day / m_totalDay) / coount;
    qreal rHeight = m_height * ((ScheduleET - ScheduleBT) / 86400.0);
    qreal posX = m_width * (1.0 * (beginday - 1) / m_totalDay) + (index - 1) * rWidth;
    qreal posY = m_height * (ScheduleBT / 86400.0);
    rect = QRectF(posX, posY, rWidth, rHeight);

    qCDebug(ClientLogger) << "Draw region calculated:" << rect;
    return rect;
}

QRectF CScheduleCoorManage::getDrawRegion(QDate date, QDateTime begintime, QDateTime endtime, int index, int coount, int maxNum, int type)
{
    qCDebug(ClientLogger) << "Getting draw region for date:" << date.toString() << "from:" << begintime.toString() << "to:" << endtime.toString() 
                          << "index:" << index << "count:" << coount << "maxNum:" << maxNum << "type:" << type;
    QRectF rect;
    QString bb = begintime.toString("yyyyMMddhhmmsszzz");
    QString ee = endtime.toString("yyyyMMddhhmmsszzz");

    if (begintime > endtime) {
        qCDebug(ClientLogger) << "Begin time is after end time, returning empty rect";
        return rect;
    }

    QDate begindate = begintime.date();
    QDate enddate = endtime.date();
    QTime beginzero(0, 0, 0);
    QTime beginScheduleT = begintime.time();
    QTime endScheduleT = endtime.time();

    if (begindate < date) {
        qCDebug(ClientLogger) << "Begin date is before target date, adjusting";
        begindate = date;
        beginScheduleT = beginzero;
    }
    if (enddate > date) {
        qCDebug(ClientLogger) << "End date is after target date, adjusting";
        enddate = date;
        endScheduleT = QTime(23, 59, 59);
    }

    qint64 beginday = m_begindate.daysTo(begindate) + 1;
    qint64 day = begindate.daysTo(enddate) + 1;
    int ScheduleBT = beginzero.secsTo(beginScheduleT);
    int ScheduleET = beginzero.secsTo(endScheduleT);
    qreal rWidth = m_width * (1.0 * day / m_totalDay) / coount;
    qreal rHeight = m_height * ((ScheduleET - ScheduleBT) / 86400.0);
    qreal posX = m_width * (1.0 * (beginday - 1) / m_totalDay) + (index - 1) * rWidth;
    qreal posY = m_height * (ScheduleBT / 86400.0);

    if (coount > maxNum && type == 0) {
        qCDebug(ClientLogger) << "Count exceeds maxNum, adjusting width";
        qreal sscale = 27.0 / (m_width * (1.0 * day / m_totalDay));

        if (index < maxNum + 1) {
            qCDebug(ClientLogger) << "Index within maxNum range";
            rWidth = m_width * (1.0 * day / m_totalDay) * sscale + 0.5;
            posX = m_width * (1.0 * (beginday - 1) / m_totalDay) + (index - 1) * rWidth;
        } else {
            qCDebug(ClientLogger) << "Index outside maxNum range";
            qreal trWidth = m_width * (1.0 * day / m_totalDay) * sscale + 0.5;
            rWidth = m_width * (1.0 * day / m_totalDay) - (index - 1) * trWidth;
            posX = m_width * (1.0 * (beginday - 1) / m_totalDay) + (index - 1) * trWidth;
        }
    }

    if (rHeight < 20) {
        qCDebug(ClientLogger) << "Height too small, adjusting to minimum";
        if (posY + 20 > m_height)
            posY = m_height - 20;
        rHeight = 20;
    }

    if (posX < 1) {
        qCDebug(ClientLogger) << "X position too small, adjusting to minimum";
        posX = 1;
        rWidth = rWidth - posX;
    }

    rect = QRectF(posX, posY, rWidth, rHeight);
    qCDebug(ClientLogger) << "Draw region calculated:" << rect;
    return rect;
}

QRectF CScheduleCoorManage::getDrawRegionF(QDateTime begintime, QDateTime endtime)
{
    qCDebug(ClientLogger) << "Getting draw region F from:" << begintime.toString() << "to:" << endtime.toString();
    QRectF rectf;

    if (begintime > endtime) {
        qCDebug(ClientLogger) << "Begin time is after end time, returning empty rect";
        return rectf;
    }

    QDate begindate = begintime.date();
    QDate enddate = endtime.date();
    QTime beginzero(0, 0, 0);
    QTime beginScheduleT = begintime.time();
    QTime endScheduleT = endtime.time();

    if (begindate < m_begindate || enddate > m_enddate) {
        qCDebug(ClientLogger) << "Date out of range, returning empty rect";
        return rectf;
    }

    qint64 beginday = m_begindate.daysTo(begindate) + 1;
    qint64 day = begindate.daysTo(enddate) + 1;
    int ScheduleBT = beginzero.secsTo(beginScheduleT);
    int ScheduleET = beginzero.secsTo(endScheduleT);
    qreal rWidth = m_width * (1.0 * day / m_totalDay);
    qreal rHeight = m_height * ((ScheduleET - ScheduleBT) / 86400.0);
    qreal posX = m_width * (1.0 * (beginday - 1) / m_totalDay);
    qreal posY = m_height * (ScheduleBT / 86400.0);
    rectf = QRectF(posX, posY, rWidth, rHeight);

    qCDebug(ClientLogger) << "Draw region F calculated:" << rectf;
    return rectf;
}

QRectF CScheduleCoorManage::getAllDayDrawRegion(QDate begin, QDate end)
{
    qCDebug(ClientLogger) << "Getting all day draw region from:" << begin.toString() << "to:" << end.toString();
    QRectF rect;
    if (begin > end) {
        qCDebug(ClientLogger) << "Begin date is after end date, returning empty rect";
        return rect;
    }

    QDate begindate = begin;
    QDate enddate = end;

    if (begindate < m_begindate) {
        qCDebug(ClientLogger) << "Begin date is before range, adjusting";
        begindate = m_begindate;
    }
    if (enddate > m_enddate) {
        qCDebug(ClientLogger) << "End date is after range, adjusting";
        enddate = m_enddate;
    }

    qint64 beginday = m_begindate.daysTo(begindate);
    qint64 day = begindate.daysTo(enddate) + 1;
    qreal rWidth = m_width * (1.0 * day / m_totalDay) - 12;
    qreal rHeight = m_height;
    qreal posX = m_width * (1.0 * beginday / m_totalDay);
    qreal posY = 0;
    rect = QRectF(posX + 6, posY, rWidth - m_rightmagin, rHeight);

    qCDebug(ClientLogger) << "All day draw region calculated:" << rect;
    return rect;
}

QDateTime CScheduleCoorManage::getDate(QPointF pos)
{
    qCDebug(ClientLogger) << "Getting date from position:" << pos;
    QDateTime begintime;
    // 坐标非有限值或绘图区域无效时返回无效时间，避免 NaN/Inf 转整数触发未定义行为
    if (!qIsFinite(pos.x()) || !qIsFinite(pos.y()) || m_width <= 0 || m_height <= 0 || m_totalDay <= 0) {
        return begintime;
    }

    // 先在 double 域内夹到有效范围，再转换为整数
    double dayValue = (1.0 * pos.x() / m_width) * m_totalDay;
    dayValue = qBound(0.0, dayValue, double(m_totalDay - 1));
    qint64 day = static_cast<qint64>(dayValue);

    double timeValue = (1.0 * pos.y() / m_height) * 86400.0;
    // 夹取到合法日内秒数范围 [0, 86399]，既避免超范围转换的未定义行为，也保证时间合法
    timeValue = qBound(0.0, timeValue, 86399.0);
    int time = static_cast<int>(timeValue);
    int hours = time / 3600;
    int minutes = (time - 3600 * hours) / 60;
    int secss = time - 3600 * hours - 60 * minutes;
    QDate date = m_begindate.addDays(day);
    begintime.setDate(date);
    begintime.setTime(QTime(hours, minutes, secss));

    qCDebug(ClientLogger) << "Calculated date time:" << begintime.toString();
    return begintime;
}

QDate CScheduleCoorManage::getsDate(QPointF pos)
{
    qCDebug(ClientLogger) << "Getting date from position:" << pos;
    // 坐标非有限值或绘图区域无效时返回无效日期，避免 NaN/Inf 转整数触发未定义行为
    if (!qIsFinite(pos.x()) || m_width <= 0 || m_totalDay <= 0) {
        return QDate();
    }

    // 先在 double 域内夹到有效范围，再转换为整数
    double dayValue = (1.0 * pos.x() / m_width) * m_totalDay;
    dayValue = qBound(0.0, dayValue, double(m_totalDay - 1));
    qint64 day = static_cast<qint64>(dayValue);
    QDate date = m_begindate.addDays(day);

    qCDebug(ClientLogger) << "Calculated date:" << date.toString();
    return date;
}

float CScheduleCoorManage::getHeight(const QTime &time) const
{
    qCDebug(ClientLogger) << "Getting height for time:" << time.toString();
    QTime beginzero(0, 0, 0);
    int ScheduleBT = beginzero.secsTo(time);
    float posY = static_cast<float>(m_height * (ScheduleBT / 86400.0));

    qCDebug(ClientLogger) << "Calculated height:" << posY;
    return posY;
}
