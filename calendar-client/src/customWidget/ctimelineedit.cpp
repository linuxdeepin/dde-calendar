/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
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
#include "ctimelineedit.h"
#include <DStyle>
#include <QPushButton>
#include <QVBoxLayout>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

CTimeLineEdit::CTimeLineEdit(int id, QWidget *parent) : QLineEdit(parent)
  , m_id(id)
{
    initView();
    //设置输入框输入限制在0到9之间
    QRegExp reg("^[0-9]*$");
    QValidator *validator = new QRegExpValidator(reg, this);
    setValidator(validator);

    connect(this, &QLineEdit::editingFinished, this, &CTimeLineEdit::slotEditingFinished);
}

void CTimeLineEdit::initView()
{
    QPushButton *addBtn = new QPushButton(this);
    QPushButton *subBtn = new QPushButton(this);
    addBtn->setIcon(DStyle().standardIcon(DStyle::SP_ArrowUp));
    addBtn->setFixedSize(16, 16);
    addBtn->setIconSize(QSize(12, 12));
    addBtn->setFlat(true);
    subBtn->setIcon(DStyle().standardIcon(DStyle::SP_ArrowDown));
    subBtn->setFixedSize(16, 16);
    subBtn->setIconSize(QSize(12, 12));
    subBtn->setFlat(true);

    connect(addBtn, &QPushButton::clicked, this, &CTimeLineEdit::slotAddBtnClicked);
    connect(subBtn, &QPushButton::clicked, this, &CTimeLineEdit::slotSubBtnClicked);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setMargin(0);
    vLayout->setSpacing(2);
    vLayout->addWidget(addBtn);
    vLayout->addWidget(subBtn);
    m_arrowWidget = new QWidget(this);
    m_arrowWidget->setLayout(vLayout);
    m_arrowWidget->move(width(), 0);

    //设置事件代理
    m_arrowWidget->installEventFilter(this);
}

/**
 * @brief CTimeLineEdit::showEvent
 * 控件显示事件
 * @param event
 */
void CTimeLineEdit::showEvent(QShowEvent *event)
{
    QLineEdit::showEvent(event);
    //调整箭头控件位置，因初始化时不能获取控件的确定大小
    m_arrowWidget->move(width()-20, 0);
}

/**
 * @brief CTimeLineEdit::setNumberRange
 * 设置数字显示范围
 * @param min 最小大小
 * @param max 最大大小
 */
void CTimeLineEdit::setNumberRange(int min, int max)
{
    m_minMun = min;
    m_maxNum = max;
    int num = m_num;
    m_num = m_num > m_minMun? m_num:m_minMun;
    m_num = m_num < m_maxNum? m_num:m_maxNum;
    if (num != m_num) {
        setText(QString("%1").arg(m_num));
    }
}

/**
 * @brief CTimeLineEdit::setNum
 * 设置显示的值
 * @param num 带显示的值
 */
void CTimeLineEdit::setNum(int num)
{
    m_num = num;
    m_num = m_num > m_minMun? m_num:m_minMun;
    m_num = m_num < m_maxNum? m_num:m_maxNum;
    setText(QString("%1").arg(m_num));
    emit signalNumChange(m_id, m_num);
}

/**
 * @brief CTimeLineEdit::setNum
 * 设置显示的值
 * @param num 带显示的数字
 * @param canCarry true: 可以跳转， false: 不可以跳转
 */
void CTimeLineEdit::setNum(int num, bool canCarry)
{
    if (!canCarry) {
        setNum(num);
        return;
    }

    //若没有超过限制范围则不必发送时间跳转事件
    if (num >= m_minMun && num <= m_maxNum) {
        m_num = num;
        setText(QString("%1").arg(m_num));
        emit signalNumChange(m_id, m_num);
        return;
    }

    //发送时间跳转信号， num-m_num: 时间差
    emit signalDateJump(m_id, num - m_num);
}

void CTimeLineEdit::slotAddBtnClicked()
{
    setNum(m_num + 1, true);
}

void CTimeLineEdit::slotSubBtnClicked()
{
    setNum(m_num - 1, true);
}

void CTimeLineEdit::slotEditingFinished()
{
    setNum(text().toInt());
}

/**
 * @brief CTimeLineEdit::eventFilter
 * 事件过滤器
 * @param obj
 * @param event
 * @return
 */
bool CTimeLineEdit::eventFilter(QObject *obj, QEvent *event)
{
    //当鼠标滑动进入箭头区域时鼠标样式显示为正常箭头状态，离开后显示为输入状态，其他情况由父布局去控制
    if (m_arrowWidget == obj && (event->type() == QEvent::Enter)) {
        setCursor(Qt::ArrowCursor);
    } else if (m_arrowWidget == obj && (event->type() == QEvent::Leave)) {
        setCursor(Qt::IBeamCursor);
    }
    return QLineEdit::eventFilter(obj, event);
}

/**
 * @brief CTimeLineEdit::keyPressEvent
 * 按键点击事件
 * @param event
 */
void CTimeLineEdit::keyPressEvent(QKeyEvent *event)
{
    //捕捉上方向键和下方向键进行数字改变
    if (event->key() == Qt::Key_Up) {
        slotAddBtnClicked();
    } else if (event->key() == Qt::Key_Down) {
        slotSubBtnClicked();
    }
    QLineEdit::keyPressEvent(event);
}
