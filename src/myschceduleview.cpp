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
#include "myschceduleview.h"
#include <QVBoxLayout>
#include "schceduledlg.h"
#include "scheduledatamanage.h"
#include <DMessageBox>
#include <DPushButton>
#include <DHiDPIHelper>
#include <DPalette>
#include "schcedulectrldlg.h"
#include "cdynamicicon.h"

#include <QShortcut>
#include <DFontSizeManager>
#include <QPainter>
#include <QtMath>
DGUI_USE_NAMESPACE
CMySchceduleView::CMySchceduleView(const ScheduleDtailInfo &schduleInfo,QWidget *parent)
    : DDialog(parent)
//      m_scheduleInfo(ScheduleDtailInfo)
{
    setContentsMargins(0, 0, 0, 0);
    m_scheduleInfo = schduleInfo;
    initUI();
    initConnection();
    //setTitle(tr("My Schcedule"));
    setFixedSize(380, 160);
    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();
    if (themetype == 2) {
        DPalette anipa = palette();
        QColor color = "#191919";
        color.setAlphaF(0.8);
        anipa.setColor(DPalette::Background, color);
        setPalette(anipa);
    }

    AutoFeed(m_scheduleInfo.titleName);
    //m_schceduleLabel->setText(info.titleName);
    if (m_scheduleInfo.type.ID == 4) {
        m_timeLabel->setText(m_scheduleInfo.beginDateTime.toString(("yyyy-MM-dd")));
    } else {
        m_timeLabel->setText(m_scheduleInfo.beginDateTime.toString("yyyy-MM-dd hh:mm") + " ~ " + m_scheduleInfo.endDateTime.toString("yyyy-MM-dd hh:mm"));
    }

    //setIconPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-logo.svg").scaled(QSize(34, 34) * devicePixelRatioF()));
}

CMySchceduleView::~CMySchceduleView()
{
    emit signalViewtransparentFrame(0);
}
void CMySchceduleView::AutoFeed(QString text)
{
    QString strText = text;
    QString resultStr = nullptr;
    QFont labelF;
//    labelF.setFamily("SourceHanSansSC");
    labelF.setWeight(QFont::Medium);
    labelF = DFontSizeManager::instance()->get(DFontSizeManager::T6,labelF);
    QFontMetrics fm(labelF);
    int titlewidth = fm.width(strText);
    QStringList strList;
    QString str;
    int h = fm.height();
    strList.clear();
    if (titlewidth < 330) {
        strList.append(strText);
        resultStr += strText;
//        m_schceduleLabel->setFixedHeight(h);
    } else {
        for (int i = 0; i < strText.count(); i++) {
            str += strText.at(i);
            if (fm.width(str) > 330) {
                str.remove(str.count() - 1,1);
                strList.append(str);
                resultStr += str + "\n";
                str.clear();
                --i;
            }
        }
        strList.append(str);
        resultStr += str;
//        m_schceduleLabel->setFixedHeight(strList.count() * h + 8);
    }

    if (strList.count() * h + 140 > 240) {
        area->setFixedHeight(100);
        setFixedHeight(250);
    } else {
        area->setFixedHeight(strList.count() * h);
        setFixedHeight(strList.count() * h + 152);
    }
//    if (strList.count() * h + 140 > 240) {
//        qDebug() << "1";
//        setFixedHeight(240);
//    } else {
//        qDebug() <<"2";
//        setFixedHeight(strList.count() * h + 140);
//    }

    m_schceduleLabel->setText(resultStr);
//    m_schceduleLabel->adjustSize();



//    QString strText = text;
////    QFont labelF;
////    labelF.setFamily("SourceHanSansSC");
////    labelF.setWeight(QFont::Medium);
//    labelF = DFontSizeManager::instance()->get(DFontSizeManager::T6,labelF);
////    labelF.setPixelSize(14);
//    QFontMetrics fm(labelF);
//    int row = 0;

//    QString str;
//    if (!strText.isEmpty()) {
//        QStringList sslist = strText.split("\n");
//        QString result;
//        for (int j = 0; j < sslist.count(); j++) {
//            QString currentstr = sslist.at(j);
//            for (int i = 0; i < currentstr.count(); i++) {
//                str.append(currentstr.at(i));
//                int widthT = fm.width(str) + 5;
//                if (widthT >= 340) {
//                    currentstr.insert(i - 1, "\n");
//                    i--;
//                    str.clear();
//                    row++;
//                } else {

//                }
//            }
//            if (currentstr.isEmpty()) {
//                result += "\n";
//                row++;
//            } else {
//                if (j != sslist.count() - 1) {
//                    result += currentstr + "\n";
//                    row++;
//                } else {
//                    result += currentstr;
//                }

//            }
//        }
//        strText = result;
//    }
////    m_schceduleLabel->setFixedHeight((row + 1) * 24);
//    if (((row + 1) * 24) > 100) {
//        area->setFixedHeight(100 - 20);
//    } else {
//        area->setFixedHeight((row + 1 ) * 24);
//    }

//    if ((row * 24 + 180) > 240) {
//        setFixedHeight(240);
//    } else {
//        setFixedHeight(row * 24 + 180);
//    }
//    m_schceduleLabel->setText(strText);
////    m_schceduleLabel->setText(text);
//    m_schceduleLabel->adjustSize();
}

void CMySchceduleView::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    emit signalViewtransparentFrame(1);
}

//void CMySchceduleView::paintEvent(QPaintEvent *event)
//{

//}

bool CMySchceduleView::eventFilter(QObject *o, QEvent *e)
{
//    QWidget *cell = qobject_cast<QWidget *>(o);
//    if (cell == w && e->type() == QEvent::Paint) {
//        paintLabel(cell);
//    }
    Q_UNUSED(o);
    if (e->type() == QEvent::FontChange) {
        AutoFeed(m_scheduleInfo.titleName);
    }
    return false;
}

//void CMySchceduleView::changeEvent(QEvent *event)
//{
//    if (event->type() == QEvent::FontChange) {
////        AutoFeed(m_scheduleInfo.titleName);
////        update();
////        qDebug() <<labelF.pixelSize()<< "+++";
//    }
//}

//void CMySchceduleView::paintLabel(QWidget *label)
//{
//    QPainter painter(label);
//    QColor color;
//    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();
//    painter.save();
//    if (themetype == 0 || themetype == 1) {
//        color = QColor("#2C4767");

//    } else {
//        color = QColor("#A8B7D1");
//    }
//    painter.setPen(color);

//    QString strText = m_scheduleInfo.titleName;
//    QFontMetrics fm = painter.fontMetrics();
//    int titlewidth = fm.width(strText);
//    QStringList strList;
//    QString str;
//    int h = fm.height();
//    strList.clear();
//    if (titlewidth < 340) {
//        strList.append(strText);
//        label->setFixedHeight(h);
//    } else {
//        for (int i = 0; i < strText.count(); i++) {
//            str += strText.at(i);
//            if (fm.width(str) > 340) {
//                str.remove(str.count() - 1,1);
//                strList.append(str);
//                str.clear();
//                --i;
//            }
//        }
//        strList.append(str);
//        label->setFixedHeight(strList.count() * h + 8);
//    }


//    if (strList.count() * h > 100) {
//        area->setFixedHeight(100 - 18);
//    } else {
//        area->setFixedHeight(strList.count() * h);
//    }
//    if (strList.count() * h + 182 > 240) {
//        setFixedHeight(240);
//    } else {
//        setFixedHeight(strList.count() + 182);
//    }

//    for (int i = 0; i < strList.count(); i++) {
//        painter.drawText(QRect(0, h * i, 340, h + 4),Qt::AlignHCenter,strList.at(i));
//    }

//    painter.restore();
//    painter.end();
//}

void CMySchceduleView::slotEditBt(int buttonIndex, QString buttonName)
{
    if (buttonIndex != 1 && buttonName != "Edit")
        return;

    CSchceduleDlg dlg(0, this);
    dlg.setData(m_scheduleInfo);
    if (dlg.exec() == DDialog::Accepted) {
        accept();
        emit signalsEditorDelete(1);
    }
}

void CMySchceduleView::slotDeleteBt(int buttonIndex, QString buttonName)
{
    if (buttonIndex != 0 && buttonName != "Delete")
        return;

    if (m_scheduleInfo.rpeat == 0) {
        CSchceduleCtrlDlg msgBox;
        //msgBox.setWindowFlags(Qt::FramelessWindowHint);
        // msgBox.setIconPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-logo.svg").scaled(QSize(34, 34) * devicePixelRatioF()));
        msgBox.setText(tr("You are deleting an event."));
        msgBox.setInformativeText(tr("Are you sure you want to delete this event?"));
        /*QAbstractButton *noButton = */msgBox.addPushButton(tr("Cancel"), true);
        /* QAbstractButton *yesButton = */msgBox.addWaringButton(tr("Delete"), true);
//        DPalette pa = yesButton->palette();

//        if (themetype == 0 || themetype == 1) {
//            pa.setColor(DPalette::ButtonText, Qt::red);

//        } else {
//            pa.setColor(DPalette::ButtonText, "#FF5736");

//        }
//        yesButton->setPalette(pa);
//        msgBox.updatesize();
        msgBox.exec();

        if (msgBox.clickButton() == 0) {
            return;
        } else if (msgBox.clickButton() == 1) {
            CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->deleteScheduleInfoById(m_scheduleInfo.id);
        } else {
            return;
        }
    } else {
        if (m_scheduleInfo.RecurID == 0) {
            CSchceduleCtrlDlg msgBox;
            //msgBox.setWindowFlags(Qt::FramelessWindowHint);
            //msgBox.setIconPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-logo.svg").scaled(QSize(34, 34) * devicePixelRatioF()));

            msgBox.setText(tr("You are deleting an event."));
            msgBox.setInformativeText(tr("Do you want to delete all occurrences of this event, or only the selected occurrence?"));
            /*QAbstractButton *noButton = */msgBox.addPushButton(tr("Cancel"));
            /*QAbstractButton *yesallbutton = */msgBox.addPushButton(tr("Delete All"));
            /*QAbstractButton *yesButton = */msgBox.addsuggestButton(tr("Delete Only This Event"));
//            msgBox.updatesize();
//            DPalette pa = yesButton->palette();
//            if (themetype == 0 || themetype == 1) {
//                pa.setColor(DPalette::ButtonText, Qt::white);
//                pa.setColor(DPalette::Dark, QColor("#25B7FF"));
//                pa.setColor(DPalette::Light, QColor("#0098FF"));
//            } else {
//                pa.setColor(DPalette::ButtonText, "#B8D3FF");
//                pa.setColor(DPalette::Dark, QColor("#0056C1"));
//                pa.setColor(DPalette::Light, QColor("#004C9C"));
//            }
//            yesButton->setPalette(pa);
            msgBox.exec();

            if (msgBox.clickButton() == 0) {
                return;
            } else if (msgBox.clickButton() == 1) {
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->deleteScheduleInfoById(m_scheduleInfo.id);
            } else if (msgBox.clickButton() == 2) {

                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(m_scheduleInfo.id, newschedule);
                newschedule.ignore.append(m_scheduleInfo.beginDateTime);
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);
            } else {
                return;
            }
        } else {
            CSchceduleCtrlDlg msgBox;
            //msgBox.setWindowFlags(Qt::FramelessWindowHint);
            //msgBox.setIconPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-logo.svg").scaled(QSize(34, 34) * devicePixelRatioF()));
            msgBox.setText(tr("You are deleting an event."));
            msgBox.setInformativeText(tr("Do you want to delete this and all future occurrences of this event, or only the selected occurrence?"));
            /*QAbstractButton *noButton = */msgBox.addPushButton(tr("Cancel"));
            /*QAbstractButton *yesallbutton = */msgBox.addPushButton(tr("Delete All Future Events"));
            /*QAbstractButton *yesButton = */msgBox.addsuggestButton(tr("Delete Only This Event"));
//            msgBox.updatesize();
//            DPalette pa = yesButton->palette();
//            if (themetype == 0 || themetype == 1) {
//                pa.setColor(DPalette::ButtonText, Qt::white);
//                pa.setColor(DPalette::Dark, QColor("#25B7FF"));
//                pa.setColor(DPalette::Light, QColor("#0098FF"));
//            } else {
//                pa.setColor(DPalette::ButtonText, "#B8D3FF");
//                pa.setColor(DPalette::Dark, QColor("#0056C1"));
//                pa.setColor(DPalette::Light, QColor("#004C9C"));
//            }
//            yesButton->setPalette(pa);
            msgBox.exec();

            if (msgBox.clickButton() == 0) {
                return;
            } else if (msgBox.clickButton() == 1) {
                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(m_scheduleInfo.id, newschedule);

                newschedule.enddata.type = 2;
                newschedule.enddata.date = m_scheduleInfo.beginDateTime.addDays(-1);
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);

            } else if (msgBox.clickButton() == 2) {

                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(m_scheduleInfo.id, newschedule);

                newschedule.ignore.append(m_scheduleInfo.beginDateTime);
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);
            } else {
                return;
            }
        }
    }
    accept();
    emit signalsEditorDelete(1);
}

void CMySchceduleView::initUI()
{
    //m_icon = new QLabel(this);
    //  m_icon->setFixedSize(30, 30);
    //m_icon->setPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-logo.svg")
    //         .scaled(m_icon->size() * devicePixelRatioF()));
    //m_icon->move(11, 10);

    //在点击任何对话框上的按钮后不关闭对话框，保证关闭子窗口时不被一起关掉
    setOnButtonClickedClose(false);

    m_Title = new QLabel(this);
    m_Title->setFixedSize(108, 51);
    m_Title->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    DFontSizeManager::instance()->bind(m_Title,DFontSizeManager::T5);
    QIcon t_icon(CDynamicIcon::getInstance()->getPixmap()); //= QIcon::fromTheme("dde-calendar");
    setIcon(t_icon);
    QFont labelTitle;
//    labelTitle.setFamily("SourceHanSansSC");
    labelTitle.setWeight(QFont::DemiBold);
//    labelTitle.setPixelSize(17);
    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();

    DPalette titlepa = m_Title->palette();
    if (themetype == 0 || themetype == 1) {
        titlepa.setColor(DPalette::WindowText, QColor("#001A2E"));

    } else {
        titlepa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
    }

    m_Title->setPalette(titlepa);
    m_Title->setFont(labelTitle);
    m_Title->setText(tr("My Event"));
    m_Title->move(137, 0);



    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    // mainLayout->setContentsMargins(10, 10, 10, 10);

    area = new QScrollArea (this);
    area->setFrameShape(QFrame::NoFrame);
    area->setFixedWidth(363);
    DPalette pa = area->palette();
    if (themetype == 0 || themetype == 1) {
        pa.setColor(DPalette::WindowText, QColor("#2C4767"));

    } else {
        pa.setColor(DPalette::WindowText, QColor("#A8B7D1"));
    }
    area->setBackgroundRole(QPalette::Background);
    area->setPalette(pa);
    area->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    area->setWidgetResizable(true);
    area->setAlignment(Qt::AlignCenter);

//    w = new QWidget (this);
//    w->setFixedSize(340,24);
//    w->installEventFilter(this);

//    area->setWidget(w);
//    mainLayout->addWidget(area);

    m_schceduleLabel = new QLabel(this);
//    m_schceduleLabel->setWordWrap(true);
    m_schceduleLabel->setTextFormat(Qt::PlainText);//纯文本格式
    m_schceduleLabel->installEventFilter(this);
    m_schceduleLabel->setFixedWidth(330);
    m_schceduleLabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_schceduleLabel,DFontSizeManager::T6);
//    QFont labelF;
//    labelF.setFamily("SourceHanSansSC");
    labelF.setWeight(QFont::Medium);
    DPalette wpa = m_schceduleLabel->palette();
    if (themetype == 0 || themetype == 1) {
        wpa.setColor(DPalette::WindowText, QColor("#2C4767"));

    } else {
        wpa.setColor(DPalette::WindowText, QColor("#A8B7D1"));
    }
    m_schceduleLabel->setPalette(wpa);
    m_schceduleLabel->setFont(labelF);

    area->setWidget(m_schceduleLabel);
    mainLayout->addWidget(area);

//    mainLayout->addWidget(m_schceduleLabel);


    m_timeLabel = new QLabel(this);
    m_timeLabel->setFixedHeight(26);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_timeLabel,DFontSizeManager::T6);
    QFont labelT;
//    labelT.setFamily("SourceHanSansSC");
    labelT.setWeight(QFont::Bold);
//    labelT.setPixelSize(14);
    DPalette tpa = m_timeLabel->palette();


    if (themetype == 0 || themetype == 1) {
        tpa.setColor(DPalette::WindowText, QColor("#6A829F"));
    } else {
        tpa.setColor(DPalette::WindowText, QColor("#6A829F"));
    }



    m_timeLabel->setPalette(tpa);
    m_timeLabel->setFont(labelT);
    mainLayout->addSpacing(6);
    mainLayout->addWidget(m_timeLabel);

    if (m_scheduleInfo.type.ID == 4) {
//        QHBoxLayout *h = new QHBoxLayout ();
//        m_okBt = new DPushButton (tr("OK"));
//        m_okBt->setFocusPolicy(Qt::NoFocus);
//        m_okBt->setFixedSize(360,36);
//        connect(m_okBt,&DPushButton::clicked,this,&CMySchceduleView::close);
//        mainLayout->addSpacing(20);
//        h->addStretch();
//        h->addWidget(m_okBt);
//        h->addStretch();
//        mainLayout->addLayout(h);
        addButton(tr("OK"), false, DDialog::ButtonNormal);
        QAbstractButton *button_ok = getButton(0);
        button_ok->setFixedSize(360,36);
    } else {
        addButton(tr("Delete"), false, DDialog::ButtonNormal);
        addButton(tr("Edit"), false, DDialog::ButtonRecommend);
        for (int i = 0; i < buttonCount(); i++) {
            QAbstractButton *button = getButton(i);
            button->setFixedSize(165,36);
        }
//        DVerticalLine *btframe = new DVerticalLine(this);
//        btframe->setFixedSize(3, 28);
//        QHBoxLayout *hBtLayout = new QHBoxLayout;
//        hBtLayout->setMargin(0);
//        hBtLayout->setSpacing(0);
//        hBtLayout->setContentsMargins(0, 0, 0, 0);
//        // hBtLayout->addStretch();
//        m_editBt = new DSuggestButton(tr("Edit"));
////        DPalette pa = m_editBt->palette();
////        if (themetype == 0 || themetype == 1) {
////            pa.setColor(DPalette::ButtonText, Qt::white);
////            pa.setColor(DPalette::Dark, QColor("#25B7FF"));
////            pa.setColor(DPalette::Light, QColor("#0098FF"));
////        } else {
////            pa.setColor(DPalette::ButtonText, "#B8D3FF");
////            pa.setColor(DPalette::Dark, QColor("#0056C1"));
////            pa.setColor(DPalette::Light, QColor("#004C9C"));
////        }
////        m_editBt->setPalette(pa);
//        m_editBt->setFixedSize(165, 36);
//        m_deleteBt = new DPushButton(tr("Delete"));
//        m_deleteBt->setFixedSize(165, 36);

//        hBtLayout->addWidget(m_deleteBt);
//        hBtLayout->addSpacing(5);
//        hBtLayout->addWidget(btframe);
//        hBtLayout->addSpacing(5);
//        hBtLayout->addWidget(m_editBt);
//        mainLayout->addSpacing(20);
//        mainLayout->addLayout(hBtLayout);
    }
    DFrame *gwi = new DFrame(this);
    gwi->setContentsMargins(0, 0, 0, 0);
    gwi->setLayout(mainLayout);
    DPalette anipa = gwi->palette();
    QColor color = "#F8F8F8";
    color.setAlphaF(0.0);
    anipa.setColor(DPalette::Background, color);
    gwi->setAutoFillBackground(true);
    gwi->setPalette(anipa);
    gwi->setBackgroundRole(DPalette::Background);
    // gwi->setGeometry(0, 51, 380, 110);
    addContent(gwi, Qt::AlignCenter);
}

void CMySchceduleView::initConnection()
{
    if (m_scheduleInfo.type.ID == 4) {
        connect(this, &DDialog::buttonClicked, this, &CMySchceduleView::close);
    } else {
        connect(this, &DDialog::buttonClicked, this, &CMySchceduleView::slotEditBt);
        connect(this, &DDialog::buttonClicked, this, &CMySchceduleView::slotDeleteBt);
    }

    QShortcut *shortcut = new QShortcut(this);
    shortcut->setKey(QKeySequence(QLatin1String("ESC")));
    connect(shortcut, SIGNAL(activated()), this, SLOT(close()));
}
