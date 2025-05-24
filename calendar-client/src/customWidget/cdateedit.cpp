// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cdateedit.h"
#include "lunarcalendarwidget.h"
#include "lunarmanager.h"

#include <QCoreApplication>
#include <QLineEdit>
#include <QDebug>

Q_LOGGING_CATEGORY(dateEditLog, "calendar.widget.dateedit")

CDateEdit::CDateEdit(QWidget *parent) : QDateEdit(parent)
{
    qCDebug(dateEditLog) << "Initializing CDateEdit";
    connect(this, &QDateEdit::userDateChanged, this, &CDateEdit::slotDateEidtInfo);
    connect(lineEdit(), &QLineEdit::textChanged, this, &CDateEdit::slotRefreshLineEditTextFormat);
    connect(lineEdit(), &QLineEdit::cursorPositionChanged, this, &CDateEdit::slotCursorPositionChanged);
    connect(lineEdit(), &QLineEdit::selectionChanged, this, &CDateEdit::slotSelectionChanged);
    connect(lineEdit(), &QLineEdit::editingFinished, this, [this]() {
        //监听完成输入信号，触发文本改变事件，保证退出文本编辑的情况下依旧能刷新文本样式
        //当非手动输入时间时不会触发文本改变信号
        slotRefreshLineEditTextFormat(text());
    });
    qCDebug(dateEditLog) << "CDateEdit initialized";
}

void CDateEdit::setDate(QDate date)
{
    qCDebug(dateEditLog) << "Setting date to:" << date;
    QDateEdit::setDate(date);
    QString dtFormat = m_showLunarCalendar ? m_format + getLunarName(date) : m_format;
    m_strCurrrentDate = date.toString(dtFormat);
    qCDebug(dateEditLog) << "Current date string set to:" << m_strCurrrentDate;
}

void CDateEdit::setDisplayFormat(QString format)
{
    qCDebug(dateEditLog) << "Setting display format to:" << format;
    this->m_format = format;
    slotDateEidtInfo(date());
}

QString CDateEdit::displayFormat()
{
    return m_format;
}

void CDateEdit::setLunarCalendarStatus(bool status)
{
    qCDebug(dateEditLog) << "Setting lunar calendar status to:" << status;
    m_showLunarCalendar = status;
    slotDateEidtInfo(date());
    updateCalendarWidget();
}

void CDateEdit::setLunarTextFormat(QTextCharFormat format)
{
    qCDebug(dateEditLog) << "Setting lunar text format";
    m_lunarTextFormat = format;
    slotRefreshLineEditTextFormat(text());
}

QTextCharFormat CDateEdit::getsetLunarTextFormat()
{
    return m_lunarTextFormat;
}

void CDateEdit::setCalendarPopup(bool enable)
{
    qCDebug(dateEditLog) << "Setting calendar popup to:" << enable;
    QDateEdit::setCalendarPopup(enable);
    //更新日历显示类型
    updateCalendarWidget();
}

void CDateEdit::slotDateEidtInfo(const QDate &date)
{
    qCDebug(dateEditLog) << "Updating date edit info for:" << date;
    QString format = m_format;

    if (m_showLunarCalendar) {
        if (!showGongli()) {
            format = "yyyy/";
        }
        m_lunarName = getLunarName(date);
        format += m_lunarName;
    }
    //当当前显示格式与应该显示格式一致时不再重新设置
    if (QDateEdit::displayFormat() == format) {
        qCDebug(dateEditLog) << "Display format unchanged, skipping update";
        return;
    }

    //记录刷新格式前的状态
    bool hasSelected = lineEdit()->hasSelectedText();   //是否选择状态
    int cPos = 0;
    QDateTimeEdit::Section section = QDateTimeEdit::NoSection;
    if (hasSelected) {
        section = currentSection();     //选择节
    } else {
        cPos = lineEdit()->cursorPosition();    //光标所在位置
    }

    QDateEdit::setDisplayFormat(format);

    //恢复原状
    if (hasSelected) {
        setSelectedSection(section);    //设置选中节
    } else {
        lineEdit()->setCursorPosition(cPos);    //设置光标位置
    }
    //刷新文本样式
    //当非手动输入时间时不会触发文本改变信号
    slotRefreshLineEditTextFormat(date.toString(format));
}

void CDateEdit::slotRefreshLineEditTextFormat(const QString &text)
{
    qCDebug(dateEditLog) << "Refreshing line edit text format for:" << text;
    QFont font = lineEdit()->font();
    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(text);
    int maxWidth = lineEdit()->width() - 25; //文本能正常显示的最大宽度
    if (textWidth > maxWidth) {
        setToolTip(text);
    } else {
        setToolTip("");
    }

    //不显示农历时无需处理
    if (!m_showLunarCalendar) {
        return;
    }

    QList<QTextLayout::FormatRange> formats;
    QTextLayout::FormatRange fr_tracker;

    fr_tracker.start = text.size() - m_lunarName.size();    //样式起始位置
    fr_tracker.length = m_lunarName.size();                 //样式长度

    fr_tracker.format = m_lunarTextFormat;                  //样式

    formats.append(fr_tracker);
    //更改农历文本样式
    setLineEditTextFormat(lineEdit(), formats);
}

void CDateEdit::slotCursorPositionChanged(int oldPos, int newPos)
{
    //不显示农历时无需处理
    if (!m_showLunarCalendar) {
        return;
    }
    Q_UNUSED(oldPos);

    //光标最大位置不能超过时间长度不能覆盖农历信息
    int maxPos = text().length() - m_lunarName.length();
    bool hasSelected = lineEdit()->hasSelectedText();

    if (hasSelected) {
        int startPos = lineEdit()->selectionStart();
        int endPos = lineEdit()->selectionEnd();

        //新的光标位置与选择区域末尾位置相等则是向后选择，向前选择无需处理
        if (newPos == endPos) {
            newPos = newPos > maxPos ? maxPos : newPos;
            lineEdit()->setSelection(startPos, newPos - startPos); //重新设置选择区域
        }
    } else {
        //非选择情况当新光标位置大于最大位置时设置到最大位置处，重新设置选中节位最后一节
        if (newPos > maxPos) {
            qCDebug(dateEditLog) << "Cursor position exceeds max, adjusting to:" << maxPos;
            setCurrentSectionIndex(sectionCount() - 1);
            lineEdit()->setCursorPosition(maxPos);
        }
    }
}

void CDateEdit::slotSelectionChanged()
{
    //不显示农历时无需处理
    if (!m_showLunarCalendar) {
        return;
    }
    //全选时重新设置为只选择时间不选择农历
    if (lineEdit()->hasSelectedText() && lineEdit()->selectionEnd() == text().length()) {
        int startPos = lineEdit()->selectionStart();
        qCDebug(dateEditLog) << "Adjusting full selection to exclude lunar info, new end:" << (text().length() - m_lunarName.length());
        lineEdit()->setSelection(startPos, text().length() - m_lunarName.length() - startPos);
    }
}

QString CDateEdit::getLunarName(const QDate &date)
{
    QString lunarName = gLunarManager->getHuangLiShortName(date);
    qCDebug(dateEditLog) << "Got lunar name for" << date << ":" << lunarName;
    return lunarName;
}

void CDateEdit::setLineEditTextFormat(QLineEdit *lineEdit, const QList<QTextLayout::FormatRange> &formats)
{
    if (!lineEdit) {
        qCWarning(dateEditLog) << "Attempted to set format for null line edit";
        return;
    }
    qCDebug(dateEditLog) << "Setting line edit text format with" << formats.size() << "format ranges";
    QList<QInputMethodEvent::Attribute> attributes;

    for (const QTextLayout::FormatRange &fr : formats) {
        QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;
        int start = fr.start - lineEdit->cursorPosition();
        int length = fr.length;
        QVariant value = fr.format;
        attributes.append(QInputMethodEvent::Attribute(type, start, length, value));
    }

    QInputMethodEvent event(QString(), attributes);
    QCoreApplication::sendEvent(lineEdit, &event);
}

void CDateEdit::changeEvent(QEvent *e)
{
    QDateEdit::changeEvent(e);
    if (e->type() == QEvent::FontChange && m_showLunarCalendar) {
        qCDebug(dateEditLog) << "Font changed, updating date edit info";
        slotDateEidtInfo(date());
    }
}

bool CDateEdit::showGongli()
{
    QString str = m_strCurrrentDate;
    QFontMetrics fontMetrice(lineEdit()->font());
    bool show = fontMetrice.horizontalAdvance(str) <= lineEdit()->width() - 20;
    qCDebug(dateEditLog) << "Show Gongli:" << show;
    return show;
}

void CDateEdit::updateCalendarWidget()
{
    qCDebug(dateEditLog) << "Updating calendar widget, lunar calendar:" << m_showLunarCalendar;
    if (calendarPopup()) {
        if (m_showLunarCalendar) {
            setCalendarWidget(new LunarCalendarWidget(this));
        } else {
            setCalendarWidget(new QCalendarWidget(this));
        }
    }
}

void CDateEdit::setEditCursorPos(int pos)
{
    qCDebug(dateEditLog) << "Setting edit cursor position to:" << pos;
    QLineEdit *edit = lineEdit();
    if (nullptr != edit) {
        edit->setCursorPosition(pos);
    } else {
        qCWarning(dateEditLog) << "Failed to set cursor position - null line edit";
    }
}

