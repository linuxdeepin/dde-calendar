/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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
#include "schcedulectrldlg.h"
#include <QVBoxLayout>
#include <DMessageBox>
#include <DPushButton>
#include <DHiDPIHelper>
#include <DPalette>
#include <DFontSizeManager>

#include "schceduledlg.h"
#include "scheduledatamanage.h"
#include "cdynamicicon.h"

DGUI_USE_NAMESPACE
CSchceduleCtrlDlg::CSchceduleCtrlDlg(QWidget *parent) : DDialog(parent)
{
    setContentsMargins(0, 0, 0, 0);
    initUI();
    initConnection();
    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();
    if (themetype == 2) {
        DPalette anipa = palette();
        QColor color = "#191919";
        color.setAlphaF(0.8);
        anipa.setColor(DPalette::Background, color);
        setPalette(anipa);
    }

    //setTitle(tr("My Schcedule"));
    resize(403, 260);
    setFocusPolicy(Qt::ClickFocus);
}

void CSchceduleCtrlDlg::initUI()
{

    //在点击任何对话框上的按钮后不关闭对话框，保证关闭子窗口时不被一起关掉
    setOnButtonClickedClose(false);

    // m_icon = new QLabel(this);
    // m_icon->setFixedSize(32, 32);
    //m_icon->setPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-logo.svg")
    //.scaled(m_icon->size() * devicePixelRatioF()));
    //m_icon->move(10, 9);
    QIcon t_icon(CDynamicIcon::getInstance()->getPixmap());// = QIcon::fromTheme("dde-calendar");
    setIcon(t_icon);
    m_Title = new QLabel(this);
    m_Title->setFixedHeight(48);
    m_Title->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    QFont labelTitle;
//    labelTitle.setFamily("SourceHanSansSC");
    labelTitle.setWeight(QFont::Bold);
    labelTitle.setPixelSize(17);
    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();

    DPalette titlepa = m_Title->palette();
    if (themetype == 0 || themetype == 1) {
        titlepa.setColor(DPalette::WindowText, QColor("#001A2E"));

    } else {
        titlepa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
    }

    m_Title->setPalette(titlepa);
    m_Title->setFont(labelTitle);


    m_mainBoxLayout = new QVBoxLayout();
    m_mainBoxLayout->setMargin(0);
    m_mainBoxLayout->setSpacing(0);
    //m_mainBoxLayout->setContentsMargins(10, 28, 10, 10);

    m_firstLabel = new QLabel(this);
    m_firstLabel->setAlignment(Qt::AlignCenter);
    m_firstLabel->setFixedWidth(350);
//    m_firstLabel->adjustSize();
    m_firstLabel->setWordWrap(true);
    labelF.setWeight(QFont::Medium);
    DFontSizeManager::instance()->bind(m_firstLabel,DFontSizeManager::T6);
    DPalette wpa = m_firstLabel->palette();
    if (themetype == 0 || themetype == 1) {
        wpa.setColor(DPalette::WindowText, QColor("#2C4767"));

    } else {
        wpa.setColor(DPalette::WindowText, QColor("#A8B7D1"));
    }
    m_firstLabel->setPalette(wpa);
    m_firstLabel->setForegroundRole(DPalette::WindowText);
    m_firstLabel->setFont(labelF);
    m_mainBoxLayout->addWidget(m_firstLabel);


    m_seconLabel = new QLabel(this);
    m_seconLabel->setFixedWidth(350);
    //m_seconLabel->setFixedHeight(26);
    m_seconLabel->setAlignment(Qt::AlignCenter);
    labelTitle.setWeight(QFont::Bold);
    DFontSizeManager::instance()->bind(m_seconLabel,DFontSizeManager::T6);
    DPalette tpa = m_seconLabel->palette();
    if (themetype == 0 || themetype == 1) {
        tpa.setColor(DPalette::WindowText, QColor("#6A829F"));
    } else {
        tpa.setColor(DPalette::WindowText, QColor("#6A829F"));
    }
    m_seconLabel->setPalette(tpa);
    m_seconLabel->setForegroundRole(DPalette::WindowText);
    m_seconLabel->setFont(labelT);
    m_seconLabel->setWordWrap(true);
//    m_seconLabel->adjustSize();
    m_mainBoxLayout->addSpacing(3);
    m_mainBoxLayout->addWidget(m_seconLabel);
//    m_mainBoxLayout->addSpacing(10);
//    m_btBoxLayout = new QHBoxLayout;
//    m_btBoxLayout->setMargin(0);
//    m_btBoxLayout->setSpacing(0);
//    m_btBoxLayout->setContentsMargins(0, 0, 0, 0);
//    m_Buttongroup = new QButtonGroup(this);
//    m_mainBoxLayout->addLayout(m_btBoxLayout);
    DFrame *gwi = new DFrame(this);
    gwi->setContentsMargins(0, 0, 0, 0);
    gwi->setLayout(m_mainBoxLayout);
    DPalette anipa = gwi->palette();
    QColor color = "#F8F8F8";
    color.setAlphaF(0.0);
    anipa.setColor(DPalette::Background, color);
    gwi->setAutoFillBackground(true);
    gwi->setPalette(anipa);
    gwi->setBackgroundRole(DPalette::Background);
    //gwi->setGeometry(0, 51, 380, 110);
    addContent(gwi, Qt::AlignCenter);
    setFocus();
}

void CSchceduleCtrlDlg::initConnection()
{
    connect(this, &DDialog::buttonClicked, this, &CSchceduleCtrlDlg::buttonJudge);
    //    connect( m_Buttongroup, SIGNAL(buttonClicked (int)), this, SLOT(buttonJudge(int)) );//连接信号和槽
}

void CSchceduleCtrlDlg::changeEvent(QEvent *event)
{
    Q_UNUSED(event)
    QFont font;

    QFontMetrics font_button(font);

    QFontMetrics font_firstLabel(font);
    QFontMetrics font_seconLabel(font);
    int height_firstLabel = (font_firstLabel.width(m_firstLabel->text()) / 300 + 1) * font_firstLabel.height();
    int height_seconLabel = (font_seconLabel.width(m_seconLabel->text()) / 300 + 1) * font_seconLabel.height();

    for (int i = 0; i < buttonCount(); i++) {
        QAbstractButton *button = getButton(i);
        QString str = str_btName.at(i);
        QString text_button = font_button.elidedText(str, Qt::ElideRight, 112);
        if (str.count() == 2) {
            button->setText(QString().append(str.at(0)).append(QChar::Nbsp).append(str.at(1)));
        } else {
            button->setText(text_button);
        }
    }

    setFixedHeight(36 + 48 + height_firstLabel + height_seconLabel + 30);

}
//void CSchceduleCtrlDlg::updatesize()
//{
//    if (m_buttonlist.count() == 2) {

//        int w = 0;
//        for (int i = 0; i < m_buttonlist.count(); i++) {
//            if (m_buttonlist.at(i)->width() > w) {
//                w = m_buttonlist.at(i)->width();
//            }
//        }
//        if (w < 170) w = 170;
//        for (int i = 0; i < m_buttonlist.count(); i++) {
//            m_buttonlist.at(i)->setFixedWidth(w);
//        }
//        int rw = w * m_buttonlist.count() + (m_buttonlist.count()) * 23 + 20;
//        setFixedWidth(rw);

//    } else {
//        int w = 0;
//        for (int i = 0; i < m_buttonlist.count(); i++) {
//            if (m_buttonlist.at(i)->width() > w) {
//                w = m_buttonlist.at(i)->width();
//            }
//        }
//        for (int i = 0; i < m_buttonlist.count(); i++) {
//            m_buttonlist.at(i)->setFixedWidth(w);
//        }
//        int rw = w * m_buttonlist.count() + (m_buttonlist.count() - 1) * 23 + 20;
//        setFixedWidth(rw);
//    }
//    //rect.setHeight(rect.height());
//    //m_firstLabel->setGeometry(rect); //
//    m_firstLabel->adjustSize();
//    //rect = m_seconLabel->geometry();
//    //rect.setHeight(rect.height() * 2);
//    // m_seconLabel->setGeometry(rect); //
//    m_seconLabel->adjustSize();
//}

void CSchceduleCtrlDlg::buttonJudge(int id)
{
    m_id = id;
    accept();
}

QAbstractButton *CSchceduleCtrlDlg::addPushButton(QString btName, bool type)
{
//    QFont labelTitle;
////    labelTitle.setFamily("SourceHanSansSC");
//    labelTitle.setWeight(QFont::Medium);
//    labelTitle = DFontSizeManager::instance()->get(DFontSizeManager::T6,labelTitle);
////    labelTitle.setPixelSize(14);
//    QFontMetrics fm(labelTitle);
//    int w = fm.width(btName);
//    if (w > 109) w = w + 18;
//    else {
//        w = 109;
//    }
//    button  = new DPushButton(btName);
//    DFontSizeManager::instance()->bind(button,DFontSizeManager::T6);
//    QFontMetrics font_button(labelTitle);
//    QString text_button = font_button.elidedText(btName,Qt::ElideRight,112);
//    button->setText(text_button);
//    button->setToolTip(btName);
//    button->setFixedWidth(129);
//    button->setFixedHeight(36);
//    button->setFont(labelTitle);
//    m_Buttongroup->addButton(button, m_Buttongroup->buttons().count());
//    if (m_Buttongroup->buttons().count() > 1) {
//        DVerticalLine *btframe = new DVerticalLine(this);
//        m_btBoxLayout->addSpacing(4);
//        btframe->setFixedSize(3, 28);
//        m_btBoxLayout->addWidget(btframe, 0, Qt::AlignCenter);
//        if (type == 1) {
//            m_btBoxLayout->addSpacing(4);
//        } else {
//            m_btBoxLayout->addSpacing(4);
//        }

//    }

//    m_btBoxLayout->addWidget(button, 0, Qt::AlignCenter);
//    m_buttonlist.append(button);

    addButton(btName, false, DDialog::ButtonNormal);
    int button_index = getButtonIndexByText(btName);
    QAbstractButton *button = getButton(button_index);
    if (type) {
        button->setFixedHeight(36);
        button->setFixedWidth(165);
    } else {
        button->setFixedHeight(36);
        button->setFixedWidth(129);
    }

    button->setToolTip(btName);

    str_btName.append(btName);

    return button;
}

QAbstractButton *CSchceduleCtrlDlg::addsuggestButton(QString btName, bool type)
{
    addButton(btName,false,DDialog::ButtonRecommend);
    int button_index = getButtonIndexByText(btName);
    QAbstractButton *suggestButton = getButton(button_index);
    if (type) {
        suggestButton->setFixedHeight(36);
        suggestButton->setFixedWidth(165);
    } else {
        suggestButton->setFixedHeight(36);
        suggestButton->setFixedWidth(129);
    }
    suggestButton->setToolTip(btName);

    str_btName.append(btName);

    return suggestButton;
}

QAbstractButton *CSchceduleCtrlDlg::addWaringButton(QString btName, bool type)
{
//    m_firstLabel->setFixedWidth(300);
    addButton(btName,false,DDialog::ButtonWarning);
    int button_index = getButtonIndexByText(btName);
    QAbstractButton *suggestButton = getButton(button_index);
    if (type) {
        suggestButton->setFixedHeight(36);
        suggestButton->setFixedWidth(165);
    } else {
        suggestButton->setFixedHeight(36);
        suggestButton->setFixedWidth(129);
    }
    suggestButton->setToolTip(btName);

    str_btName.append(btName);

    return suggestButton;
}

void CSchceduleCtrlDlg::setText(QString str)
{
    m_firstLabel->setText(str);
    m_firstLabel->setToolTip(str);
}

void CSchceduleCtrlDlg::setInformativeText(QString str)
{
    m_seconLabel->setText(str);
    m_seconLabel->setToolTip(str);
}

int CSchceduleCtrlDlg::clickButton()
{
//    if (m_id < 0 || m_id > m_buttonlist.count() - 1) return  nullptr;
//    return m_buttonlist[m_id];
    if (m_id < 0 || m_id > buttonCount() - 1) return buttonCount();
    return  m_id;
}

