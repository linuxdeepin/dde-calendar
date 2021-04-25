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
#include "schceduledlg.h"
#include "schcedulectrldlg.h"
#include "scheduledatamanage.h"
#include "timeedit.h"
#include "constants.h"

#include <DHiDPIHelper>
#include <DMessageBox>
#include <DPalette>
#include <DFontSizeManager>

#include <QCalendarWidget>
#include <QHBoxLayout>
#include <QIcon>
#include <QIntValidator>
#include <QMessageBox>
#include <QShortcut>
#include <QTextBlock>
#include <QToolButton>
#include <QVBoxLayout>
#include <QKeyEvent>

DGUI_USE_NAMESPACE
CSchceduleDlg::CSchceduleDlg(int type, QWidget *parent, const bool isAllDay)
    : DDialog(parent)
    , m_createAllDay(isAllDay)
{
    setContentsMargins(0, 0, 0, 0);
    m_type = type;
    initUI();
    initConnection();

    if (type == 1) {
        m_titleLabel->setText(tr("New Event"));
        m_beginDateEdit->setDate(QDate::currentDate());
        int hours = QTime::currentTime().hour();
        int minnutes = QTime::currentTime().minute() % DDECalendar::QuarterOfAnhourWithMinute;

        if (minnutes != 0) {
            minnutes = QTime::currentTime().minute() / DDECalendar::QuarterOfAnhourWithMinute * DDECalendar::QuarterOfAnhourWithMinute + DDECalendar::QuarterOfAnhourWithMinute;
        }
        m_beginTimeEdit->setTime(QTime(hours, minnutes));
        m_endDateEdit->setDate(QDate::currentDate());
        m_endTimeEdit->setTime(QTime(hours, minnutes).addSecs(3600));
    } else {
        m_titleLabel->setText(tr("Edit Event"));
    }

    setFocusPolicy(Qt::WheelFocus);
    setFixedSize(438, 470);
}

CSchceduleDlg::~CSchceduleDlg()
{
    emit signalViewtransparentFrame(0);
}

void CSchceduleDlg::setData(const ScheduleDtailInfo &info)
{
    m_scheduleDtailInfo = info;
    m_typeComBox->setCurrentIndex(info.type.ID - 1);
    m_textEdit->setPlainText(info.titleName);
    m_beginDateEdit->setDate(info.beginDateTime.date());
    m_beginTimeEdit->setTime(info.beginDateTime.time());
    m_endDateEdit->setDate(info.endDateTime.date());
    m_endTimeEdit->setTime(info.endDateTime.time());
    m_allDayCheckbox->setChecked(info.allday);
    m_endRepeatDate->setMinimumDate(info.beginDateTime.date());

    m_currentDate = info.beginDateTime;
    m_EndDate = info.endDateTime;

    slotallDayStateChanged(info.allday);
    initRmindRpeatUI();
}

void CSchceduleDlg::setDate(const QDateTime &date)
{
    m_currentDate = date;
    int hours = date.time().hour();
    int minnutes = 0;

    if (date.date() == QDate::currentDate()) {
        minnutes = (date.time().minute() / DDECalendar::QuarterOfAnhourWithMinute + 1) * DDECalendar::QuarterOfAnhourWithMinute;
    } else {
        int minnutes = date.time().minute() % DDECalendar::QuarterOfAnhourWithMinute;
        if (minnutes != 0) {
            minnutes = (date.time().minute() / DDECalendar::QuarterOfAnhourWithMinute + 1) * DDECalendar::QuarterOfAnhourWithMinute;
        }
    }

    if (minnutes == 60) {
        if (hours + 1 == 24) {
            m_currentDate.setTime(QTime(0, 0));
            m_currentDate = m_currentDate.addDays(1);
        } else {
            m_currentDate.setTime(QTime(hours + 1, 0));
        }
    } else {
        m_currentDate.setTime(QTime(hours, minnutes));
    }

    m_beginDateEdit->setDate(m_currentDate.date());
    m_beginTimeEdit->setTime(m_currentDate.time());
    QDateTime datetime = m_currentDate.addSecs(3600);
    m_EndDate = datetime;
    m_endDateEdit->setDate(datetime.date());
    m_endTimeEdit->setTime(datetime.time());
    m_endRepeatDate->setMinimumDate(date.date());
}

ScheduleDtailInfo CSchceduleDlg::getScheduleData()
{
    return m_scheduleDtailInfo;
}

void CSchceduleDlg::setAllDay(bool flag)
{
    m_allDayCheckbox->setChecked(flag);
}

void CSchceduleDlg::slotCancelBt(int buttonIndex, QString buttonName)
{
    if (buttonIndex != 0 && buttonName != "Cancel")
        return;

    reject();
}

void CSchceduleDlg::slotOkBt(int buttonIndex, QString buttonName)
{
    if (buttonIndex != 1 && buttonName != "Save")
        return;

    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();

    ScheduleDtailInfo scheduleDtailInfo = m_scheduleDtailInfo;
    QDateTime beginDateTime, endDateTime;
    beginDateTime.setDate(m_beginDateEdit->date());
    beginDateTime.setTime(m_beginTimeEdit->getTime());
    endDateTime.setDate(m_endDateEdit->date());
    endDateTime.setTime(m_endTimeEdit->getTime());

    if (m_textEdit->toPlainText().isEmpty()) {
        scheduleDtailInfo.titleName = m_textEdit->placeholderText();
    } else {
        scheduleDtailInfo.titleName = m_textEdit->toPlainText();
    }

    if (scheduleDtailInfo.titleName.isEmpty()) {
        return;
    }

    if (beginDateTime > endDateTime) {
        DDialog *prompt = new DDialog(this);
        prompt->setIcon(QIcon(":/resources/icon/warning.svg"));
        prompt->setMessage(tr("End time must be greater than start time"));
        prompt->setWindowFlags(prompt->windowFlags() | Qt::WindowStaysOnTopHint);
        prompt->addButton(tr("OK"), true, DDialog::ButtonNormal);
        prompt->exec();
        return;
    }

    if (m_type == 1)
        scheduleDtailInfo.id = 0;
    scheduleDtailInfo.allday = m_allDayCheckbox->isChecked();

    if (m_rmindCombox->currentIndex() == 0)
        scheduleDtailInfo.remind = false;
    else {
        scheduleDtailInfo.remind = true;
    }

    if (scheduleDtailInfo.allday) {
        if (scheduleDtailInfo.remind) {
            scheduleDtailInfo.remindData.time = QTime(9, 0);
            if (m_rmindCombox->currentIndex() == 1) {
                scheduleDtailInfo.remindData.n = DDECalendar::OnStartDay;
            } else if (m_rmindCombox->currentIndex() == 2) {
                scheduleDtailInfo.remindData.n = DDECalendar::OneDayBeforeWithDay;
            } else if (m_rmindCombox->currentIndex() == 3) {
                scheduleDtailInfo.remindData.n = DDECalendar::TwoDayBeforeWithDay;
            } else if (m_rmindCombox->currentIndex() == 4) {
                scheduleDtailInfo.remindData.n = DDECalendar::OneWeekBeforeWithDay;
            }
        }
    } else {
        if (scheduleDtailInfo.remind) {
            if (m_rmindCombox->currentIndex() == 1) {
                scheduleDtailInfo.remindData.n = DDECalendar::AtTimeOfEvent;
            } else if (m_rmindCombox->currentIndex() == 2) {
                scheduleDtailInfo.remindData.n = DDECalendar::FifteenMinutesBefore;
            } else if (m_rmindCombox->currentIndex() == 3) {
                scheduleDtailInfo.remindData.n = DDECalendar::ThirtyMinutesBefore;
            } else if (m_rmindCombox->currentIndex() == 4) {
                scheduleDtailInfo.remindData.n = DDECalendar::OneHourBefore;
            } else if (m_rmindCombox->currentIndex() == 5) {
                scheduleDtailInfo.remindData.n = DDECalendar::OneDayBeforeWithMinutes;
            } else if (m_rmindCombox->currentIndex() == 6) {
                scheduleDtailInfo.remindData.n = DDECalendar::TwoDayBeforeWithMinutes;
            } else if (m_rmindCombox->currentIndex() == 7) {
                scheduleDtailInfo.remindData.n = DDECalendar::OneWeekBeforeWithMinutes;
            }
        }
    }

    scheduleDtailInfo.rpeat = m_beginrepeatCombox->currentIndex();

    if (scheduleDtailInfo.rpeat != 0) {
        scheduleDtailInfo.enddata.type = m_endrepeatCombox->currentIndex();

        if (m_endrepeatCombox->currentIndex() == 1) {
            if (m_endrepeattimes->text().isEmpty()) {
                return;
            }
            scheduleDtailInfo.enddata.tcount = m_endrepeattimes->text().toInt();
        } else if (m_endrepeatCombox->currentIndex() == 2) {
            QDateTime endrpeattime = beginDateTime;
            endrpeattime.setDate(m_endRepeatDate->date());

            if (beginDateTime > endrpeattime) {
                return;
            }
            scheduleDtailInfo.enddata.date = endrpeattime;
        }
    }

    CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->GetType(
        m_typeComBox->currentIndex() + 1, scheduleDtailInfo.type);
    scheduleDtailInfo.beginDateTime = beginDateTime;
    scheduleDtailInfo.endDateTime = endDateTime;

    if (m_type == 1) {
        CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->addSchedule(
            scheduleDtailInfo);
    } else if (m_type == 0) {
        if (m_scheduleDtailInfo.rpeat == 0 &&
                m_scheduleDtailInfo.rpeat == scheduleDtailInfo.rpeat) {
            CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(
                scheduleDtailInfo);
        } else {
            if (m_scheduleDtailInfo.allday != scheduleDtailInfo.allday) {
                CSchceduleCtrlDlg msgBox;
                msgBox.setText(
                    tr("All occurrences of a repeating event must have the same all-day status."));
                msgBox.setInformativeText(tr("Do you want to change all occurrences?"));
                msgBox.addPushButton(tr("Cancel"), true);
                msgBox.addWaringButton(tr("Change All"), true);
                msgBox.exec();

                if (msgBox.clickButton() == 0) {
                    return;
                } else if (msgBox.clickButton() == 1) {
                    CScheduleDataManage::getScheduleDataManage()
                    ->getscheduleDataCtrl()
                    ->updateScheduleInfo(scheduleDtailInfo);
                }
            } else if (m_scheduleDtailInfo.rpeat != scheduleDtailInfo.rpeat) {
                CSchceduleCtrlDlg msgBox;
                msgBox.setText(tr("You are changing the repeating rule of this event."));
                msgBox.setInformativeText(tr("Do you want to change all occurrences?"));
                msgBox.addPushButton(tr("Cancel"), true);
                msgBox.addWaringButton(tr("Change All"), true);
                msgBox.exec();

                if (msgBox.clickButton() == 0) {
                    return;
                } else if (msgBox.clickButton() == 1) {
                    CScheduleDataManage::getScheduleDataManage()
                    ->getscheduleDataCtrl()
                    ->updateScheduleInfo(scheduleDtailInfo);
                }
            } else {
                ChangeRecurInfo(this,scheduleDtailInfo,m_scheduleDtailInfo,themetype);
            }
        }
    }
    accept();
    emit  signalScheduleUpdate();
}

void CSchceduleDlg::slotTextChange()
{
    QTextCursor cursor = m_textEdit->textCursor();
    QString textContent = m_textEdit->toPlainText();
    int length = textContent.count();
    QString tStitlename = textContent;

    if (tStitlename.contains("\n")) {
        tStitlename.replace("\n", "");
        m_textEdit->setText(tStitlename);
        cursor.movePosition(QTextCursor::End);
        m_textEdit->setTextCursor(cursor);
        return;
    }

    int maxLength = 256;  // 最大字符数

    if (length > maxLength) {
        m_textEdit->setText(m_context);
        cursor.movePosition(QTextCursor::End);
        m_textEdit->setTextCursor(cursor);
    }
    m_context = m_textEdit->toPlainText();
}

void CSchceduleDlg::slotendrepeatTextchange()
{
    QAbstractButton *m_OkBt = getButton(1);

    if (m_endrepeattimes->text().isEmpty())
        m_OkBt->setEnabled(false);
    else
        m_OkBt->setEnabled(true);
}

void CSchceduleDlg::slotBDateEidtInfo(const QDate &date)
{
    m_beginDateEdit->setDate(date);
    m_endRepeatDate->setMinimumDate(date);
    m_endDateEdit->setMinimumDate(date);

    QDateTime beginDateTime, endDateTime;
    beginDateTime.setDate(m_beginDateEdit->date());
    beginDateTime.setTime(m_beginTimeEdit->getTime());
    endDateTime.setDate(m_endDateEdit->date());
    endDateTime.setTime(m_endTimeEdit->getTime());

    if (endDateTime < beginDateTime) {
        m_endTimeEdit->setTime(m_beginTimeEdit->getTime().addSecs(3600));
    }
}

void CSchceduleDlg::slotallDayStateChanged(int state)
{
    m_rmindCombox->clear();

    if (!state) {
        m_rmindCombox->addItem(tr("Never"));
        m_rmindCombox->addItem(tr("At time of event"));
        m_rmindCombox->addItem(tr("15 minutes before"));
        m_rmindCombox->addItem(tr("30 minutes before"));
        m_rmindCombox->addItem(tr("1 hour before"));
        m_rmindCombox->addItem(tr("1 day before"));
        m_rmindCombox->addItem(tr("2 days before"));
        m_rmindCombox->addItem(tr("1 week before"));
        m_beginTimeEdit->setVisible(true);
        m_endTimeEdit->setVisible(true);

        if (m_type == 0) {
            m_beginDateEdit->setDate(m_scheduleDtailInfo.beginDateTime.date());
            m_beginTimeEdit->setTime(m_scheduleDtailInfo.beginDateTime.time());
            m_endDateEdit->setDate(m_scheduleDtailInfo.endDateTime.date());
            m_endTimeEdit->setTime(m_scheduleDtailInfo.endDateTime.time());
        } else {
            m_beginDateEdit->setDate(m_currentDate.date());
            m_beginTimeEdit->setTime(m_currentDate.time());
            m_endDateEdit->setDate(m_EndDate.date());
            m_endTimeEdit->setTime(m_EndDate.time());
        }
    } else {
        m_rmindCombox->addItem(tr("Never"));
        m_rmindCombox->addItem(tr("On start day (9:00 AM)"));
        m_rmindCombox->addItem(tr("1 day before"));
        m_rmindCombox->addItem(tr("2 days before"));
        m_rmindCombox->addItem(tr("1 week before"));
        m_beginTimeEdit->setVisible(false);
        m_endTimeEdit->setVisible(false);

        if (m_type == 0) {
            m_beginDateEdit->setDate(m_scheduleDtailInfo.beginDateTime.date());
            m_beginTimeEdit->setTime(QTime(0, 0));
            m_endDateEdit->setDate(m_scheduleDtailInfo.endDateTime.date());
            m_endTimeEdit->setTime(QTime(23, 59));
        } else {
            m_beginDateEdit->setDate(m_currentDate.date());
            m_endDateEdit->setDate(m_EndDate.date());
            m_beginTimeEdit->setTime(QTime(0, 0));
            m_endTimeEdit->setTime(QTime(23, 59));
            m_rmindCombox->setCurrentIndex(2);
        }
    }
}

void CSchceduleDlg::slotbRpeatactivated(int index)
{
    if (index > 0) {
        m_endrepeatWidget->setVisible(true);
        setFixedSize(438, 506);
    } else {
        m_endrepeatWidget->setVisible(false);
        setFixedSize(438, 470);
    }
}

void CSchceduleDlg::sloteRpeatactivated(int index)
{
    if (index == 0) {
        m_endrepeattimesWidget->setVisible(false);
        m_endRepeatDate->setVisible(false);
    } else if (index == 1) {
        m_endrepeattimesWidget->setVisible(true);
        m_endRepeatDate->setVisible(false);
    } else {
        m_endrepeattimesWidget->setVisible(false);
        m_endRepeatDate->setVisible(true);
    }
}

bool CSchceduleDlg::eventFilter(QObject *obj, QEvent *pEvent)
{
    if (obj == m_textEdit) {
        if (pEvent->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(pEvent);
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                return true;
            }
            if (keyEvent->key() == Qt::Key_Tab)
                return true;
        }
        if (m_type == 1) {
            if (pEvent->type() == QEvent::FocusIn) {
                //清空编辑框默认占位符
                m_textEdit->setPlaceholderText("");
            } else if (pEvent->type() == QEvent::FocusOut) {
                //设置编辑框默认占位符
                m_textEdit->setPlaceholderText(tr("New event"));
            }
        }
    }
    return QDialog::eventFilter(obj, pEvent);
}

void CSchceduleDlg::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    emit signalViewtransparentFrame(1);
}

void CSchceduleDlg::changeEvent(QEvent *event)
{
    Q_UNUSED(event);
    QFont mlabelF;
    mlabelF.setWeight(QFont::Medium);

    QFontMetrics fontWidth_typeLabel(mlabelF);
    QString str_typelabel = fontWidth_typeLabel.elidedText(tr("Type:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_typeLabel->setText(str_typelabel);

    QFontMetrics fontWidth_contentlabel(mlabelF);
    QString str_contentlabel = fontWidth_contentlabel.elidedText(tr("Description:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_contentLabel->setText(str_contentlabel);

    QFontMetrics fontWidth_allDayLabel(mlabelF);
    QString str_allDayLabel = fontWidth_allDayLabel.elidedText(tr("All Day:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_adllDayLabel->setText(str_allDayLabel);

    QFontMetrics fontWidth_beginTimeLabel(mlabelF);
    QString str_beginTimeLabel = fontWidth_beginTimeLabel.elidedText(tr("Starts:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_beginTimeLabel->setText(str_beginTimeLabel);

    QFontMetrics fontWidth_endTimeLabel(mlabelF);
    QString str_endTimeLabel = fontWidth_endTimeLabel.elidedText(tr("Ends:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_endTimeLabel->setText(str_endTimeLabel);

    QFontMetrics fontWidth_remindSetLabel(mlabelF);
    QString str_remindSetLabel = fontWidth_remindSetLabel.elidedText(tr("Remind Me:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_remindSetLabel->setText(str_remindSetLabel);

    QFontMetrics fontWidth_beginRepeatLabel(mlabelF);
    QString str_beginRepeatLabel = fontWidth_beginRepeatLabel.elidedText(tr("Repeat:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_beginrepeatLabel->setText(str_beginRepeatLabel);

    QFontMetrics fontWidth_endrepeatLabel(mlabelF);
    QString str_endrepeatLabel = fontWidth_endrepeatLabel.elidedText(tr("End Repeat:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_endrepeatLabel->setText(str_endrepeatLabel);
}

void CSchceduleDlg::initUI()
{
    //设置对象名称和辅助显示名称
    this->setObjectName("ScheduleEditDialog");
    this->setAccessibleName("ScheduleEditDialog");
    //在点击任何对话框上的按钮后不关闭对话框，保证关闭子窗口时不被一起关掉
    setOnButtonClickedClose(false);

    m_titleLabel = new QLabel(this);
    QFont titlelabelF;
    titlelabelF.setWeight(QFont::DemiBold);
    titlelabelF.setPixelSize(17);
    m_titleLabel->setFixedSize(148, 51);
    m_titleLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_titleLabel->move(145, 0);
    m_titleLabel->setFont(titlelabelF);

    setSpacing(0);
    QFont mlabelF;
    mlabelF.setWeight(QFont::Medium);

    QVBoxLayout *maintlayout = new QVBoxLayout;
    maintlayout->setMargin(0);
    maintlayout->setSpacing(10);
    QHBoxLayout *typelayout = new QHBoxLayout;
    typelayout->setSpacing(0);
    typelayout->setMargin(0);
    m_typeLabel = new QLabel();
    m_typeLabel->setToolTip(tr("Type"));
    DFontSizeManager::instance()->bind(m_typeLabel,DFontSizeManager::T6);
    QFontMetrics fontWidth_typeLabel(mlabelF);
    QString str_typelabel = fontWidth_typeLabel.elidedText(tr("Type:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_typeLabel->setText(str_typelabel);
    m_typeLabel->setFont(mlabelF);
    m_typeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_typeLabel->setFixedSize(78, 36);
    m_typeComBox = new DComboBox();
    //设置对象名称和辅助显示名称
    m_typeComBox->setObjectName("ScheduleTypeCombobox");
    m_typeComBox->setAccessibleName("ScheduleTypeCombobox");
    m_typeComBox->setFixedSize(319, 36);
    m_typeComBox->setIconSize(QSize(24,24));
    m_typeComBox->insertItem(0,
                             QIcon(DHiDPIHelper::loadNxPixmap(":/resources/icon/icon_type_work.svg")
                                   .scaled(QSize(24, 24) * devicePixelRatioF())),
                             tr("Work"));
    m_typeComBox->insertItem(1,
                             QIcon(DHiDPIHelper::loadNxPixmap(":/resources/icon/icon_type_life.svg")
                                   .scaled(QSize(24, 24) * devicePixelRatioF())),
                             tr("Life"));
    m_typeComBox->insertItem(
        2,
        QIcon(DHiDPIHelper::loadNxPixmap(":/resources/icon/icon_type_other.svg")
              .scaled(QSize(24, 24) * devicePixelRatioF())),
        tr("Other"));
    typelayout->addWidget(m_typeLabel);
    typelayout->addWidget(m_typeComBox);
    maintlayout->addLayout(typelayout);

    QHBoxLayout *contentLabellayout = new QHBoxLayout;
    contentLabellayout->setSpacing(0);
    contentLabellayout->setMargin(0);
    QVBoxLayout *conttelabellayout = new QVBoxLayout;
    conttelabellayout->setSpacing(0);
    conttelabellayout->setMargin(0);
    m_contentLabel = new QLabel();
    DFontSizeManager::instance()->bind(m_contentLabel,DFontSizeManager::T6);
    QFontMetrics fontWidth_contentlabel(mlabelF);
    QString str_contentlabel = fontWidth_contentlabel.elidedText(tr("Description:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_contentLabel->setText(str_contentlabel);
    m_contentLabel->setFont(mlabelF);
    m_contentLabel->setToolTip(tr("Description"));
    m_contentLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_contentLabel->setFixedWidth(78);
    conttelabellayout->addWidget(m_contentLabel);
    conttelabellayout->addStretch();
    m_textEdit = new DTextEdit(this);
    //设置对象名称和辅助显示名称
    m_textEdit->setObjectName("ScheduleTitleEdit");
    m_textEdit->setAccessibleName("ScheduleTitleEdit");
    m_textEdit->setFixedSize(319, 86);
    m_textEdit->setAcceptRichText(false);

    m_textEdit->setPlaceholderText(tr("New Event"));
    //设置关联控件，用于QTextEdit控件捕获MouseButtonPress等事件
    QWidget *mpContentWidget = m_textEdit->viewport();
    //设置事件过滤器
    m_textEdit->installEventFilter(this);
    mpContentWidget->installEventFilter(this);

    contentLabellayout->addLayout(conttelabellayout);
    contentLabellayout->addWidget(m_textEdit);
    maintlayout->addLayout(contentLabellayout);

    QHBoxLayout *alldayLabellayout = new QHBoxLayout;
    alldayLabellayout->setSpacing(0);
    alldayLabellayout->setMargin(0);
    m_adllDayLabel = new QLabel();
    m_adllDayLabel->setToolTip(tr("All Day"));
    DFontSizeManager::instance()->bind(m_adllDayLabel,DFontSizeManager::T6);
    QFontMetrics fontWidth_allDayLabel(mlabelF);
    QString str_allDayLabel = fontWidth_allDayLabel.elidedText(tr("All Day:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_adllDayLabel->setText(str_allDayLabel);
    m_adllDayLabel->setFont(mlabelF);
    m_adllDayLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_adllDayLabel->setFixedWidth(78);
    m_allDayCheckbox = new DCheckBox(this);
    //设置对象名称和辅助显示名称
    m_allDayCheckbox->setObjectName("AllDayCheckBox");
    m_allDayCheckbox->setAccessibleName("AllDayCheckBox");
    alldayLabellayout->addWidget(m_adllDayLabel);
    alldayLabellayout->addWidget(m_allDayCheckbox);
    maintlayout->addLayout(alldayLabellayout);

    QHBoxLayout *beginLabellayout = new QHBoxLayout;
    beginLabellayout->setSpacing(0);
    beginLabellayout->setMargin(0);
    m_beginTimeLabel = new QLabel();
    m_beginTimeLabel->setToolTip(tr("Starts"));
    DFontSizeManager::instance()->bind(m_beginTimeLabel,DFontSizeManager::T6);
    QFontMetrics fontWidth_beginTimeLabel(mlabelF);
    QString str_beginTimeLabel = fontWidth_beginTimeLabel.elidedText(tr("Starts:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_beginTimeLabel->setText(str_beginTimeLabel);
    m_beginTimeLabel->setFont(mlabelF);
    m_beginTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_beginTimeLabel->setFixedSize(78, 36);
    m_beginDateEdit = new QDateEdit(this);
    //设置对象名称和辅助显示名称
    m_beginDateEdit->setObjectName("ScheduleBeginDateEdit");
    m_beginDateEdit->setAccessibleName("ScheduleBeginDateEdit");
    m_beginDateEdit->setFixedSize(175, 36);

    m_beginTimeEdit = new CTimeEdit(this);
    //设置对象名称和辅助显示名称
    m_beginTimeEdit->setObjectName("ScheduleBeginTimeEdit");
    m_beginTimeEdit->setAccessibleName("ScheduleBeginTimeEdit");
    m_beginTimeEdit->setFixedSize(141, 36);
    m_beginDateEdit->setCalendarPopup(true);
    m_beginDateEdit->setDisplayFormat("yyyy-MM-dd");
    beginLabellayout->addWidget(m_beginTimeLabel);
    beginLabellayout->addWidget(m_beginDateEdit);
    beginLabellayout->addSpacing(8);
    beginLabellayout->addWidget(m_beginTimeEdit);
    beginLabellayout->addStretch();
    maintlayout->addLayout(beginLabellayout);

    QHBoxLayout *enQLabellayout = new QHBoxLayout;
    enQLabellayout->setSpacing(0);
    enQLabellayout->setMargin(0);
    m_endTimeLabel = new QLabel();
    m_endTimeLabel->setToolTip(tr("Ends"));
    DFontSizeManager::instance()->bind(m_endTimeLabel,DFontSizeManager::T6);
    QFontMetrics fontWidth_endTimeLabel(mlabelF);
    QString str_endTimeLabel = fontWidth_endTimeLabel.elidedText(tr("Ends:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_endTimeLabel->setText(str_endTimeLabel);
    m_endTimeLabel->setFont(mlabelF);
    m_endTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_endTimeLabel->setFixedSize(78, 36);
    m_endDateEdit = new QDateEdit(this);
    //设置对象名称和辅助显示名称
    m_endDateEdit->setObjectName("ScheduleEndDateEdit");
    m_endDateEdit->setAccessibleName("ScheduleEndDateEdit");
    m_endDateEdit->setFixedSize(175, 36);
    m_endTimeEdit = new CTimeEdit(this);
    //设置对象名称和辅助显示名称
    m_endTimeEdit->setObjectName("ScheduleEndTimeEdit");
    m_endTimeEdit->setAccessibleName("ScheduleEndTimeEdit");
    m_endTimeEdit->setFixedSize(141, 36);
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDisplayFormat("yyyy-MM-dd");

    enQLabellayout->addWidget(m_endTimeLabel);
    enQLabellayout->addWidget(m_endDateEdit);
    enQLabellayout->addSpacing(8);

    enQLabellayout->addWidget(m_endTimeEdit);
    enQLabellayout->addStretch();
    maintlayout->addLayout(enQLabellayout);

    QHBoxLayout *rminQLabellayout = new QHBoxLayout;
    rminQLabellayout->setSpacing(0);
    rminQLabellayout->setMargin(0);
    m_remindSetLabel = new QLabel();
    DFontSizeManager::instance()->bind(m_remindSetLabel,DFontSizeManager::T6);
    QFontMetrics fontWidth_remindSetLabel(mlabelF);
    QString str_remindSetLabel = fontWidth_remindSetLabel.elidedText(tr("Remind Me:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_remindSetLabel->setToolTip(tr("Remind Me"));
    m_remindSetLabel->setText(str_remindSetLabel);
    m_remindSetLabel->setFont(mlabelF);
    m_remindSetLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_remindSetLabel->setFixedWidth(78);

    m_rmindCombox = new DComboBox();
    //设置对象名称和辅助显示名称
    m_rmindCombox->setObjectName("RmindComboBox");
    m_rmindCombox->setAccessibleName("RmindComboBox");
    m_rmindCombox->setFixedSize(175, 36);
    m_rmindCombox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    rminQLabellayout->addWidget(m_remindSetLabel);
    rminQLabellayout->addWidget(m_rmindCombox);
    rminQLabellayout->addStretch();
    maintlayout->addLayout(rminQLabellayout);

    QHBoxLayout *repeatLabellayout = new QHBoxLayout;
    repeatLabellayout->setSpacing(0);
    repeatLabellayout->setMargin(0);
    m_beginrepeatLabel = new QLabel();
    m_beginrepeatLabel->setToolTip(tr("Repeat"));
    DFontSizeManager::instance()->bind(m_beginrepeatLabel,DFontSizeManager::T6);
    QFontMetrics fontWidth_beginRepeatLabel(mlabelF);
    QString str_beginRepeatLabel = fontWidth_beginRepeatLabel.elidedText(tr("Repeat:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_beginrepeatLabel->setText(str_beginRepeatLabel);
    m_beginrepeatLabel->setFont(mlabelF);
    m_beginrepeatLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_beginrepeatLabel->setFixedWidth(78);
    m_beginrepeatCombox = new DComboBox();
    //设置对象名称和辅助显示名称
    m_beginrepeatCombox->setObjectName("BeginRepeatComboBox");
    m_beginrepeatCombox->setAccessibleName("BeginRepeatComboBox");
    m_beginrepeatCombox->setFixedSize(175, 36);
    m_beginrepeatCombox->addItem(tr("Never"));
    m_beginrepeatCombox->addItem(tr("Daily"));
    m_beginrepeatCombox->addItem(tr("Weekdays"));
    m_beginrepeatCombox->addItem(tr("Weekly"));
    m_beginrepeatCombox->addItem(tr("Monthly"));
    m_beginrepeatCombox->addItem(tr("Yearly"));
    repeatLabellayout->addWidget(m_beginrepeatLabel);
    repeatLabellayout->addWidget(m_beginrepeatCombox);
    repeatLabellayout->addStretch();
    maintlayout->addLayout(repeatLabellayout);

    QHBoxLayout *endrepeatLabellayout = new QHBoxLayout;
    endrepeatLabellayout->setSpacing(0);
    endrepeatLabellayout->setMargin(0);
    m_endrepeatLabel = new QLabel();
    DFontSizeManager::instance()->bind(m_endrepeatLabel,DFontSizeManager::T6);
    QFontMetrics fontWidth_endrepeatLabel(mlabelF);
    QString str_endrepeatLabel = fontWidth_endrepeatLabel.elidedText(tr("End Repeat:"), Qt::ElideRight, DDECalendar::NewSchceduleLabelWidth);
    m_endrepeatLabel->setToolTip(tr("End Repeat"));
    m_endrepeatLabel->setText(str_endrepeatLabel);
    m_endrepeatLabel->setFont(mlabelF);
    m_endrepeatLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_endrepeatLabel->setFixedWidth(78);
    m_endrepeatCombox = new DComboBox();
    //设置对象名称和辅助显示名称
    m_endrepeatCombox->setObjectName("EndRepeatComboBox");
    m_endrepeatCombox->setAccessibleName("EndRepeatComboBox");
    m_endrepeatCombox->setFixedSize(175, 36);
    m_endrepeatCombox->addItem(tr("Never"));
    m_endrepeatCombox->addItem(tr("After"));
    m_endrepeatCombox->addItem(tr("On"));
    endrepeatLabellayout->addWidget(m_endrepeatLabel);
    endrepeatLabellayout->addWidget(m_endrepeatCombox);
    endrepeatLabellayout->addSpacing(8);

    QHBoxLayout *endrepeattimeslayout = new QHBoxLayout;
    endrepeattimeslayout->setSpacing(0);
    endrepeattimeslayout->setMargin(0);
    endrepeattimeslayout->setContentsMargins(10, 0, 0, 0);
    m_endrepeattimes = new DLineEdit(this);
    //设置对象名称和辅助显示名称
    m_endrepeattimes->setObjectName("EndRepeatTimeEidt");
    m_endrepeattimes->setAccessibleName("EndRepeatTimeEidt");
    m_endrepeattimes->setFixedSize(71, 36);
    m_endrepeattimes->setText(QString::number(10));
    m_endrepeattimes->setClearButtonEnabled(false);
    QRegExp rx("^[1-9]\\d{0,2}$");
    QValidator *validator = new QRegExpValidator(rx, this);
    m_endrepeattimes->lineEdit()->setValidator(validator);
    m_endrepeattimesLabel = new QLabel(tr("time(s)"));
    m_endrepeattimesLabel->setFont(mlabelF);
    m_endrepeattimesLabel->setFixedHeight(36);
    endrepeattimeslayout->addWidget(m_endrepeattimes);
    endrepeattimeslayout->addWidget(m_endrepeattimesLabel);
    m_endrepeattimesWidget = new DWidget;
    //设置对象名称和辅助显示名称
    m_endrepeattimesWidget->setObjectName("EndRepeatTimeWidget");
    m_endrepeattimesWidget->setAccessibleName("EndRepeatTimeWidget");
    m_endrepeattimesWidget->setLayout(endrepeattimeslayout);
    m_endrepeattimesWidget->setVisible(false);
    endrepeatLabellayout->addWidget(m_endrepeattimesWidget);

    m_endRepeatDate = new DDateEdit;
    //设置对象名称和辅助显示名称
    m_endRepeatDate->setObjectName("EndRepeatDateEdit");
    m_endRepeatDate->setAccessibleName("EndRepeatDateEdit");
    m_endRepeatDate->setCalendarPopup(true);
    m_endRepeatDate->setFixedSize(141, 36);
    m_endRepeatDate->setDate(QDate::currentDate());
    m_endRepeatDate->setDisplayFormat("yyyy-MM-dd");
    m_endRepeatDate->setCurrentSectionIndex(2);
    QFont enddatefont;
    enddatefont.setWeight(QFont::Medium);
    m_endRepeatDate->setFont(enddatefont);
    endrepeatLabellayout->addWidget(m_endRepeatDate);
    endrepeatLabellayout->addStretch();
    m_endRepeatDate->setVisible(false);
    m_endrepeatWidget = new DWidget;
    //设置对象名称和辅助显示名称
    m_endrepeatWidget->setObjectName("EndRepeatDateWidget");
    m_endrepeatWidget->setAccessibleName("EndRepeatDateWidget");
    m_endrepeatWidget->setLayout(endrepeatLabellayout);
    maintlayout->addWidget(m_endrepeatWidget);
    m_endrepeatWidget->setVisible(false);

    addButton(tr("Cancel"));
    addButton(tr("Save"), false, DDialog::ButtonRecommend);

    for (int i = 0; i < buttonCount(); i++) {
        QAbstractButton *button = getButton(i);
        button->setFixedSize(189, 36);
    }
    m_gwi = new DFrame(this);
    m_gwi->setFrameShape(QFrame::NoFrame);
    m_gwi->setLayout(maintlayout);
    DPalette anipa = m_gwi->palette();
    QColor color = "#F8F8F8";
    color.setAlphaF(0.0);
    anipa.setColor(DPalette::Background, color);
    addContent(m_gwi, Qt::AlignCenter);
    initDateEdit();

    if (m_type == 1)
        slotallDayStateChanged(0);
    setFocus();
}

void CSchceduleDlg::initConnection()
{
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this,
                     &CSchceduleDlg::setTheMe);
    connect(this, &DDialog::buttonClicked, this, &CSchceduleDlg::slotCancelBt);
    connect(this, &DDialog::buttonClicked, this, &CSchceduleDlg::slotOkBt);
    connect(m_textEdit, &DTextEdit::textChanged, this, &CSchceduleDlg::slotTextChange);
    connect(m_endrepeattimes,&DLineEdit::textChanged, this, &CSchceduleDlg::slotendrepeatTextchange);

    connect(m_allDayCheckbox, &DCheckBox::stateChanged, this,
            &CSchceduleDlg::slotallDayStateChanged);
    connect(m_beginrepeatCombox, QOverload<int>::of(&QComboBox::activated), this,
            &CSchceduleDlg::slotbRpeatactivated);
    connect(m_endrepeatCombox, QOverload<int>::of(&QComboBox::activated), this,
            &CSchceduleDlg::sloteRpeatactivated);
    connect(m_beginDateEdit, &DDateEdit::userDateChanged, this, &CSchceduleDlg::slotBDateEidtInfo);

    QShortcut *shortcut = new QShortcut(this);
    shortcut->setKey(QKeySequence(QLatin1String("ESC")));
    connect(shortcut, SIGNAL(activated()), this, SLOT(close()));
}

void CSchceduleDlg::initDateEdit()
{
    m_beginDateEdit->setMinimumDate(QDate(DDECalendar::QueryEarliestYear, 1, 1)); // 0天
    m_beginDateEdit->setMaximumDate(QDate(DDECalendar::QueryLatestYear, 12, 31));
    m_endDateEdit->setMinimumDate(QDate(DDECalendar::QueryEarliestYear, 1, 1)); // 0天
    m_endDateEdit->setMaximumDate(QDate(DDECalendar::QueryLatestYear, 12, 31));
    return;
}

void CSchceduleDlg::initRmindRpeatUI()
{
    if (m_scheduleDtailInfo.allday) {
        if (m_scheduleDtailInfo.remind) {
            if (m_scheduleDtailInfo.remindData.n == DDECalendar::OnStartDay) {
                m_rmindCombox->setCurrentIndex(1);
            } else if (m_scheduleDtailInfo.remindData.n == DDECalendar::OneDayBeforeWithDay) {
                m_rmindCombox->setCurrentIndex(2);
            } else if (m_scheduleDtailInfo.remindData.n == DDECalendar::TwoDayBeforeWithDay) {
                m_rmindCombox->setCurrentIndex(3);
            } else if (m_scheduleDtailInfo.remindData.n == DDECalendar::OneWeekBeforeWithDay) {
                m_rmindCombox->setCurrentIndex(4);
            }
        } else {
            m_rmindCombox->setCurrentIndex(0);
        }
    } else {
        if (m_scheduleDtailInfo.remind) {
            if (m_scheduleDtailInfo.remindData.n == DDECalendar::AtTimeOfEvent) {
                m_rmindCombox->setCurrentIndex(1);
            } else if (m_scheduleDtailInfo.remindData.n == DDECalendar::FifteenMinutesBefore) {
                m_rmindCombox->setCurrentIndex(2);
            } else if (m_scheduleDtailInfo.remindData.n == DDECalendar::ThirtyMinutesBefore) {
                m_rmindCombox->setCurrentIndex(3);
            } else if (m_scheduleDtailInfo.remindData.n == DDECalendar::OneHourBefore) {
                m_rmindCombox->setCurrentIndex(4);
            } else if (m_scheduleDtailInfo.remindData.n == DDECalendar::OneDayBeforeWithMinutes) {
                m_rmindCombox->setCurrentIndex(5);
            } else if (m_scheduleDtailInfo.remindData.n == DDECalendar::TwoDayBeforeWithMinutes) {
                m_rmindCombox->setCurrentIndex(6);
            } else if (m_scheduleDtailInfo.remindData.n == DDECalendar::OneWeekBeforeWithMinutes) {
                m_rmindCombox->setCurrentIndex(7);
            }
        } else {
            m_rmindCombox->setCurrentIndex(0);
        }
    }
    slotbRpeatactivated(m_scheduleDtailInfo.rpeat);
    m_beginrepeatCombox->setCurrentIndex(m_scheduleDtailInfo.rpeat);

    if (m_scheduleDtailInfo.rpeat != 0) {
        if (m_scheduleDtailInfo.enddata.type == 0) {
            m_endrepeatCombox->setCurrentIndex(0);
        } else if (m_scheduleDtailInfo.enddata.type == 1) {
            m_endrepeatCombox->setCurrentIndex(1);
            m_endrepeattimes->setText(QString::number(m_scheduleDtailInfo.enddata.tcount));
        } else if (m_scheduleDtailInfo.enddata.type == 2) {
            m_endrepeatCombox->setCurrentIndex(2);
            m_endRepeatDate->setDate(m_scheduleDtailInfo.enddata.date.date());
        }
        m_endrepeatWidget->show();
        sloteRpeatactivated(m_scheduleDtailInfo.enddata.type);
    } else {
        m_endrepeatWidget->hide();
    }
}

void CSchceduleDlg::setTheMe(const int type)
{
    //日程标题编辑框文字颜色
    QColor titleColor;
    if (type == 2) {
        titleColor = "#C0C6D4";
    } else {
        titleColor = "#414D68";
    }
    DPalette pa = m_textEdit->palette();
    //设置颜色
    pa.setColor(DPalette::Text,titleColor);
    m_textEdit->setPalette(pa);
}

void CSchceduleDlg::ChangeRecurInfo(QWidget *parent, const ScheduleDtailInfo &newinfo, const ScheduleDtailInfo &oldinfo, int m_themetype)
{
    Q_UNUSED(m_themetype);
    Q_UNUSED(parent);

    if (newinfo.RecurID == 0) {
        CSchceduleCtrlDlg msgBox;
        msgBox.setText(tr("You are changing a repeating event."));
        msgBox.setInformativeText(
            tr("Do you want to change only this occurrence of the event, or all "
               "occurrences?"));
        msgBox.addPushButton(tr("Cancel"));
        msgBox.addPushButton(tr("All"));
        msgBox.addsuggestButton(tr("Only This Event"));
        msgBox.exec();

        if (msgBox.clickButton() == 0) {
            return;
        } else if (msgBox.clickButton() == 1) {
            ScheduleDtailInfo scheduleDtailInfo = newinfo;
            if (scheduleDtailInfo.enddata.type ==1 &&scheduleDtailInfo.enddata.tcount<1) {
                scheduleDtailInfo.enddata.type =0;
            } else if (scheduleDtailInfo.enddata.type ==2 &&
                       scheduleDtailInfo.beginDateTime.daysTo(
                           scheduleDtailInfo.enddata.date)<0) {
                scheduleDtailInfo.enddata.type =0;
                scheduleDtailInfo.rpeat = 0;
            }
            CScheduleDataManage::getScheduleDataManage()
            ->getscheduleDataCtrl()
            ->updateScheduleInfo(scheduleDtailInfo);
        } else if (msgBox.clickButton() == 2) {
            ChangeOnlyInfo(newinfo,oldinfo);
        }
    } else {
        CSchceduleCtrlDlg msgBox;
        msgBox.setText(tr("You are changing a repeating event."));
        msgBox.setInformativeText(
            tr("Do you want to change only this occurrence of the event, or this and "
               "all future occurrences?"));
        msgBox.addPushButton(tr("Cancel"));
        msgBox.addPushButton(tr("All Future Events"));
        msgBox.addsuggestButton(tr("Only This Event"));
        msgBox.exec();

        if (msgBox.clickButton() == 0) {
            return;
        } else if (msgBox.clickButton() == 1) {
            ScheduleDtailInfo newschedule = newinfo;
            newschedule.RecurID = 0;
            newschedule.id = 0;
            if (newschedule.enddata.type ==1) {
                newschedule.enddata.tcount = qAbs(newinfo.enddata.tcount - newinfo.RecurID);
                if (newschedule.enddata.tcount <1) {
                    newschedule.enddata.type =0;
                    newschedule.rpeat = 0;
                }
            }
            CScheduleDataManage::getScheduleDataManage()
            ->getscheduleDataCtrl()
            ->addSchedule(newschedule);
            ScheduleDtailInfo updatescheduleData;
            CScheduleDataManage::getScheduleDataManage()
            ->getscheduleDataCtrl()
            ->getScheduleInfoById(oldinfo.id, updatescheduleData);
            if (updatescheduleData.enddata.type == 1) {
                updatescheduleData.enddata.tcount = newinfo.RecurID -1;
                if (updatescheduleData.enddata.tcount <1) {
                    updatescheduleData.enddata.type =0;
                    updatescheduleData.rpeat = 0;
                }
            } else {
                //如果结束类型为永不或结束于日期
                updatescheduleData.enddata.type = 2;
                updatescheduleData.enddata.date =
                    oldinfo.beginDateTime.addDays(-1);
            }

            CScheduleDataManage::getScheduleDataManage()
            ->getscheduleDataCtrl()
            ->updateScheduleInfo(updatescheduleData);
        } else if (msgBox.clickButton() == 2) {
            ChangeOnlyInfo(newinfo,oldinfo);
        }
    }
}

void CSchceduleDlg::ChangeOnlyInfo(const ScheduleDtailInfo &newinfo, const ScheduleDtailInfo &oldinfo)
{
    ScheduleDtailInfo newschedule = newinfo;
    newschedule.rpeat = 0;
    newschedule.RecurID = 0;
    newschedule.id = 0;
    newschedule.ignore.clear();
    CScheduleDataManage::getScheduleDataManage()
    ->getscheduleDataCtrl()
    ->addSchedule(newschedule);
    ScheduleDtailInfo updatescheduleData;
    CScheduleDataManage::getScheduleDataManage()
    ->getscheduleDataCtrl()
    ->getScheduleInfoById(oldinfo.id, updatescheduleData);
    updatescheduleData.ignore.append(oldinfo.beginDateTime);
    CScheduleDataManage::getScheduleDataManage()
    ->getscheduleDataCtrl()
    ->updateScheduleInfo(updatescheduleData);
}
