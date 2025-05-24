// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ccustomtimeedit.h"

#include <QMouseEvent>
#include <QLineEdit>
#include <QKeyEvent>

Q_LOGGING_CATEGORY(customTimeEditLog, "calendar.widget.customtimeedit")

CCustomTimeEdit::CCustomTimeEdit(QWidget *parent)
    : QTimeEdit(parent)
{
    qCDebug(customTimeEditLog) << "Initializing CCustomTimeEdit";
    //设置edit最大宽度，不影响其他控件使用
    setMaximumWidth(80);
    setButtonSymbols(QTimeEdit::NoButtons);
    qCDebug(customTimeEditLog) << "CCustomTimeEdit initialized";
}

/**
 * @brief CCustomTimeEdit::getLineEdit      获取编辑框
 * @return
 */
QLineEdit *CCustomTimeEdit::getLineEdit()
{
    qCDebug(customTimeEditLog) << "Getting line edit";
    return lineEdit();
}

void CCustomTimeEdit::focusInEvent(QFocusEvent *event)
{
    qCDebug(customTimeEditLog) << "Focus in event";
    QTimeEdit::focusInEvent(event);
    emit signalUpdateFocus(true);
}

void CCustomTimeEdit::focusOutEvent(QFocusEvent *event)
{
    qCDebug(customTimeEditLog) << "Focus out event";
    QTimeEdit::focusOutEvent(event);
    emit signalUpdateFocus(false);
}

void CCustomTimeEdit::mousePressEvent(QMouseEvent *event)
{
    qCDebug(customTimeEditLog) << "Mouse press event at position:" << event->pos();
    //设置父类widget焦点
    if (parentWidget() != nullptr) {
        qCDebug(customTimeEditLog) << "Setting parent widget focus";
        parentWidget()->setFocus(Qt::TabFocusReason);
    }
    //设置点击位置的光标
    lineEdit()->setCursorPosition(lineEdit()->cursorPositionAt(event->pos()));
    QAbstractSpinBox::mousePressEvent(event);
}

void CCustomTimeEdit::keyPressEvent(QKeyEvent *event)
{
    qCDebug(customTimeEditLog) << "Key press event:" << event->key();
    QTimeEdit::keyPressEvent(event);
    //鼠标左右键,切换光标位置
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
        if (parentWidget() != nullptr) {
            qCDebug(customTimeEditLog) << "Setting parent widget focus on arrow key";
            parentWidget()->setFocus(Qt::TabFocusReason);
        }
        qCDebug(customTimeEditLog) << "Setting cursor position to current section index:" << currentSectionIndex();
        lineEdit()->setCursorPosition(currentSectionIndex());
    }
}

void CCustomTimeEdit::mouseDoubleClickEvent(QMouseEvent *event)
{
    qCDebug(customTimeEditLog) << "Mouse double click event";
    QTimeEdit::mouseDoubleClickEvent(event);
    //鼠标双击,选中section
    qCDebug(customTimeEditLog) << "Selecting current section:" << currentSection();
    setSelectedSection(currentSection());
}
