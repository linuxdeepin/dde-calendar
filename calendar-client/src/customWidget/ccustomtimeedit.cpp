// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ccustomtimeedit.h"
#include "commondef.h"

#include <QMouseEvent>
#include <QLineEdit>
#include <QKeyEvent>

CCustomTimeEdit::CCustomTimeEdit(QWidget *parent)
    : QTimeEdit(parent)
{
    qCDebug(ClientLogger) << "CCustomTimeEdit constructor initialized";
    //设置edit最大宽度，不影响其他控件使用
    setMaximumWidth(80);
    setButtonSymbols(QTimeEdit::NoButtons);
}

/**
 * @brief CCustomTimeEdit::getLineEdit      获取编辑框
 * @return
 */
QLineEdit *CCustomTimeEdit::getLineEdit()
{
    // qCDebug(ClientLogger) << "CCustomTimeEdit::getLineEdit - Returning line edit";
    return lineEdit();
}

void CCustomTimeEdit::focusInEvent(QFocusEvent *event)
{
    // qCDebug(ClientLogger) << "CCustomTimeEdit::focusInEvent - Focus reason:" << event->reason();
    QTimeEdit::focusInEvent(event);
    emit signalUpdateFocus(true);
}

void CCustomTimeEdit::focusOutEvent(QFocusEvent *event)
{
    // qCDebug(ClientLogger) << "CCustomTimeEdit::focusOutEvent - Focus reason:" << event->reason();
    QTimeEdit::focusOutEvent(event);
    emit signalUpdateFocus(false);
}

void CCustomTimeEdit::mousePressEvent(QMouseEvent *event)
{
    // qCDebug(ClientLogger) << "CCustomTimeEdit::mousePressEvent - Position:" << event->pos();
    //设置父类widget焦点
    if (parentWidget() != nullptr) {
        // qCDebug(ClientLogger) << "Setting focus to parent widget";
        parentWidget()->setFocus(Qt::TabFocusReason);
    }
    //设置点击位置的光标
    lineEdit()->setCursorPosition(lineEdit()->cursorPositionAt(event->pos()));
    QAbstractSpinBox::mousePressEvent(event);
}

void CCustomTimeEdit::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(ClientLogger) << "CCustomTimeEdit::keyPressEvent - Key:" << event->key();
    QTimeEdit::keyPressEvent(event);
    //鼠标左右键,切换光标位置
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
        // qCDebug(ClientLogger) << "Left/Right key pressed, setting focus to parent and updating cursor position";
        if (parentWidget() != nullptr) {
            parentWidget()->setFocus(Qt::TabFocusReason);
        }
        lineEdit()->setCursorPosition(currentSectionIndex());
    }
}

void CCustomTimeEdit::mouseDoubleClickEvent(QMouseEvent *event)
{
    // qCDebug(ClientLogger) << "CCustomTimeEdit::mouseDoubleClickEvent - Position:" << event->pos();
    QTimeEdit::mouseDoubleClickEvent(event);
    //鼠标双击,选中section
    // qCDebug(ClientLogger) << "Double click detected, selecting section:" << currentSection();
    setSelectedSection(currentSection());
}
