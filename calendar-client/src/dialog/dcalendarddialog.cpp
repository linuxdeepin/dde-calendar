// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcalendarddialog.h"
#include "commondef.h"
#include "constants.h"
#include "calendarmanage.h"
#include "tabletconfig.h"
#include <DTitlebar>

DCalendarDDialog::DCalendarDDialog(QWidget *parent)
    : DDialog(parent)
    , m_timeFormat(CalendarManager::getInstance()->getTimeFormat())
    , m_dateFormat(CalendarManager::getInstance()->getDateFormat())
{
    qCDebug(ClientLogger) << "DCalendarDDialog constructor";
    connect(CalendarManager::getInstance(), &CalendarManager::signalTimeFormatChanged, this, &DCalendarDDialog::setTimeFormat);
    connect(CalendarManager::getInstance(), &CalendarManager::signalDateFormatChanged, this, &DCalendarDDialog::setDateFormat);
    //获取ddialog的标题栏
    DTitlebar *titlebar = findChild<DTitlebar *>();
    if (titlebar != nullptr) {
        qCDebug(ClientLogger) << "Setting focus proxy to titlebar";
        //设置ddialog的焦点代理为标题栏
        this->setFocusProxy(titlebar);
    } else {
        qCDebug(ClientLogger) << "Titlebar not found";
    }
}

void DCalendarDDialog::mouseMoveEvent(QMouseEvent *event)
{
    //如果为平板模式使其不可移动
    if (TabletConfig::isTablet()) {
        // qCDebug(ClientLogger) << "Mouse move event ignored in tablet mode";
        Q_UNUSED(event);
    } else {
        // qCDebug(ClientLogger) << "Processing mouse move event in desktop mode";
        DDialog::mouseMoveEvent(event);
    }
}

void DCalendarDDialog::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(ClientLogger) << "Key press event with key:" << event->key();
    //如果dtk版本在5.3.0以下调用QDialog 以上调用DDialog
#if (DTK_VERSION < DTK_VERSION_CHECK(5, 3, 0, 0))
    // qCDebug(ClientLogger) << "Using QDialog::keyPressEvent for DTK < 5.3.0";
    return QDialog::keyPressEvent(event);
#else
    // qCDebug(ClientLogger) << "Using DDialog::keyPressEvent for DTK >= 5.3.0";
    return DDialog::keyPressEvent(event);
#endif
}

bool DCalendarDDialog::eventFilter(QObject *o, QEvent *e)
{
    // qCDebug(ClientLogger) << "Event filter for event type:" << e->type();
    //如果dtk版本在5.3.0以下调用QDialog 以上调用DDialog
#if (DTK_VERSION < DTK_VERSION_CHECK(5, 3, 0, 0))
    // qCDebug(ClientLogger) << "Using QDialog::eventFilter for DTK < 5.3.0";
    return QDialog::eventFilter(o, e);
#else
    // qCDebug(ClientLogger) << "Using DDialog::eventFilter for DTK >= 5.3.0";
    return DDialog::eventFilter(o, e);
#endif
}

void DCalendarDDialog::updateDateTimeFormat()
{
    qCDebug(ClientLogger) << "Updating date time format, date format:" << m_dateFormat << "time format:" << m_timeFormat;
}

/**
 * @brief DCalendarDDialog::setTimeFormat 设置短时间格式,并更新显示
 */
void DCalendarDDialog::setTimeFormat(int value)
{
    qCDebug(ClientLogger) << "Setting time format with value:" << value;
    if (value) {
        m_timeFormat = "hh:mm";
    } else {
        m_timeFormat = "h:mm";
    }
    qCDebug(ClientLogger) << "Time format set to:" << m_timeFormat;
    updateDateTimeFormat();
}

/**
 * @brief DCalendarDDialog::setTimeFormat 设置短日期格式,并更新显示
 */
void DCalendarDDialog::setDateFormat(int value)
{
    qCDebug(ClientLogger) << "Setting date format with value:" << value;
    switch (value) {
    case 0: {
        m_dateFormat = "yyyy/M/d";
    } break;
    case 1: {
        m_dateFormat = "yyyy-M-d";
    } break;
    case 2: {
        m_dateFormat = "yyyy.M.d";
    } break;
    case 3: {
        m_dateFormat = "yyyy/MM/dd";
    } break;
    case 4: {
        m_dateFormat = "yyyy-MM-dd";
    } break;
    case 5: {
        m_dateFormat = "yyyy.MM.dd";
    } break;
    case 6: {
        m_dateFormat = "yy/M/d";
    } break;
    case 7: {
        m_dateFormat = "yy-M-d";
    } break;
    case 8: {
        m_dateFormat = "yy.M.d";
    } break;
    default: {
        m_dateFormat = "yyyy-MM-dd";
    } break;
    }
    qCDebug(ClientLogger) << "Date format set to:" << m_dateFormat;
    updateDateTimeFormat();
}
