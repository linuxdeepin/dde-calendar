/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
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
#include "timeedit.h"
#include <QRegExpValidator>
#include <QDebug>
//#include "timeverticalscroll.h"
CTimeEdit::CTimeEdit(QWidget *parent)
    : DComboBox(parent)
{
    initUI();
    initConnection();
    setFocus(Qt::MouseFocusReason);
}

CTimeEdit::~CTimeEdit() {}

void CTimeEdit::setTime(QTime time)
{
    m_time = time;
    m_timeEdit->setText(m_time.toString("hh:mm"));
}

QTime CTimeEdit::getTime()
{
    m_timeEdit->lineEdit()->setInputMask("00:00;#");
    QString timetext = m_timeEdit->text();

    if (timetext.count() == 1) {
        timetext = "00:00";
    }
    if (timetext.right(1) == ":") {
        timetext = timetext + "00";
    }
    if (timetext.left(1) == ":") {
        timetext = "00" + timetext;
    }
    QStringList t_list = timetext.split(":");
    QString firststr = t_list.at(0);
    QString secondstr = t_list.at(1);
    if (firststr.count() == 1) {
        firststr = "0" + firststr;
    }
    if (secondstr.count() == 1) {
        secondstr = "0" + secondstr;
    }
    timetext = firststr + ":" + secondstr;

    m_time = QTime::fromString(timetext, "hh:mm");
    return m_time;
}

void CTimeEdit::initUI()
{
    //    m_pListWidget = new DListWidget(this);

    m_timeEdit = new DLineEdit(this);
    m_timeEdit->lineEdit()->setInputMask("00:00;0");
    m_timeEdit->setClearButtonEnabled(false);
    QRegExpValidator *validator = nullptr;
    QRegExp rx("0[0-9]:[0-5][0-9]|1[0-9]:[0-5][0-9]|2[0-3]:[0-5][0-9]");
    validator = new QRegExpValidator(rx, this);
    m_timeEdit->lineEdit()->setValidator(validator);
    setLineEdit(m_timeEdit->lineEdit());

    QStringList list;
    for (int i = 0; i < 24; i++) {
        list << QString("%1:%2").arg(i, 2, 10, QLatin1Char('0')).arg(0, 2, 10, QLatin1Char('0'));
        list << QString("%1:%2").arg(i, 2, 10, QLatin1Char('0')).arg(30);
    }
    this->addItems(list);

#if 0
    setModel(m_pListWidget->model());
    setView(m_pListWidget);
    //disconnect(m_timeEdit, &QLineEdit::editingFinished, m_pListWidget, &QListWidget::update);
    //[2] 年月日选择控件
    m_pListWidget->setContentsMargins(0, 0, 0, 0);
    m_pListWidget->setLineWidth(0);
    m_pListWidget->setFixedWidth(131);
    m_pitem = new QListWidgetItem(m_pListWidget);
    m_pitem->setData(Qt::UserRole, "");
    m_pListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_verticalScroll = new CTimeVerticalScroll();

    m_verticalScroll->setRange(0, 23);
    m_timeEdit->lineEdit()->setCursorPosition(1);
    m_verticalScroll->setFixedSize(m_pListWidget->width(), 300);
    m_pitem->setSizeHint(QSize(m_pListWidget->width(), 300));
    m_pListWidget->addItem(m_pitem);
    m_pitem->setFlags(Qt::ItemIsTristate );
    m_pListWidget->setItemWidget(m_pitem, m_verticalScroll);
    m_pListWidget->setFocusPolicy(Qt::NoFocus);
    m_pListWidget->viewport()->setContentsMargins(0, 0, 0, 0);
#endif
}

void CTimeEdit::initConnection()
{
#if 0
    connect(m_verticalScroll, &CTimeVerticalScroll::currentValueChanged, this, &CTimeEdit::slotcurrentValueChanged);
    connect(m_verticalScroll, &CTimeVerticalScroll::currentValueChangedClose, this, &CTimeEdit::slotcurrentValueChangedClose);
#endif
    // connect(m_timeEdit, &DLineEdit::editingFinished, this, &CTimeEdit::slotEidtChange);
    // connect(m_timeEdit, &DLineEdit::returnPressed, this, &CTimeEdit::slotEidtChange);
    m_timeEdit->disconnect(SIGNAL(returnPressed()));
    m_timeEdit->disconnect(SIGNAL(editingFinished()));
    m_timeEdit->disconnect(SIGNAL(selectionChanged()));
    m_timeEdit->disconnect(SIGNAL(textChanged(const QString &)));
    m_timeEdit->disconnect(SIGNAL(textEdited(const QString &)));
    m_timeEdit->disconnect(SIGNAL(cursorPositionChanged(int, int)));

    disconnect(SIGNAL(activated(int)));
    disconnect(SIGNAL(activated(const QString &)));
    disconnect(SIGNAL(currentIndexChanged(int)));
    disconnect(SIGNAL(currentIndexChanged(const QString &)));
    disconnect(SIGNAL(currentTextChanged(const QString &)));
    disconnect(SIGNAL(editTextChanged(const QString &)));
    disconnect(SIGNAL(highlighted(int)));
    disconnect(SIGNAL(highlighted(const QString &)));
}

#if 0
void CTimeEdit::showPopup()
{
    // QComboBox::showPopup();
    int cc = width() + 32;
    //m_pListWidget->setFixedWidth(width() + 12);
    m_pos = m_timeEdit->lineEdit()->cursorPosition();
    //m_pitem->setSizeHint(QSize(width() + 12, 300));
    //m_verticalScroll->setFixedWidth(width() + 12);
    m_timeEdit->lineEdit()->setInputMask("00:00;#");
    QString timetext = m_timeEdit->text();
    m_timeEdit->lineEdit()->setInputMask("00:00;0");
    if (timetext.count() == 1) {
        timetext = "00:00";
    }
    if (timetext.right(1) == ":") {
        timetext = timetext + "00";
    }
    if (timetext.left(1) == ":") {
        timetext = "00" + timetext;
    }
    QStringList t_list = timetext.split(":");
    QString firststr = t_list.at(0);
    QString secondstr = t_list.at(1);
    if (firststr.count() == 1) {
        firststr = "0" + firststr;
    }
    if (secondstr.count() == 1) {
        secondstr = "0" + secondstr;
    }
    timetext = firststr + ":" + secondstr;
    m_time = QTime::fromString(timetext, "hh:mm");

    if (m_pos < 3) {
        m_type = 0;
        m_verticalScroll->setRange(0, 23);
        m_verticalScroll->setCurrValue(m_time.hour());
    } else {
        m_type = 1;
        m_verticalScroll->setRange(0, 59);
        m_verticalScroll->setCurrValue(m_time.minute());
    }
    m_pListWidget->show();
    QComboBox::showPopup();
}
void CTimeEdit::hidePopup()
{
    QString timetext = m_time.toString("hh:mm");
    m_timeEdit->setText(timetext);
    QComboBox::hidePopup();
    return;
    // 移除旧item
    QListWidgetItem *item = NULL;
    int iCount = m_pListWidget->count();
    for (int i = iCount - 1; i >= 1; i--) {
        item = m_pListWidget->takeItem(i);
        delete item;
    }
}

void CTimeEdit::slotcurrentValueChanged(int value)
{
    if (m_type == 0) {

        QTime time = QTime(value, m_time.minute());
        if (!time.isValid()) return;
        m_time = time;
        m_timeEdit->setText(m_time.toString("hh:mm"));
    } else {
        QTime time = QTime(m_time.hour(), value);
        if (!time.isValid()) return;
        m_time = time;
        m_timeEdit->setText(m_time.toString("hh:mm"));
    }
    m_timeEdit->lineEdit()->setCursorPosition(m_pos);
}

void CTimeEdit::slotcurrentValueChangedClose(int value)
{
    slotcurrentValueChanged(value);
    hidePopup();
}

void CTimeEdit::slotEidtChange()
{
    m_timeEdit->lineEdit()->setInputMask("00:00;#");
    //QTime time = QTime(m_time.hour(), value);
    QTime time = QTime::fromString(m_timeEdit->text(), "hh:mm");
    m_timeEdit->lineEdit()->setInputMask("00:00;0");
    QListWidgetItem *item = NULL;
    int iCount = m_pListWidget->count();
    for (int i = iCount - 1; i >= 1; i--) {
        item = m_pListWidget->takeItem(i);
        delete item;
    }
    if (!time.isValid()) return;
    m_time = time;
    m_timeEdit->setText(m_time.toString("hh:mm"));
}

#endif
