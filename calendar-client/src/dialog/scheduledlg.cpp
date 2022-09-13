// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduledlg.h"
#include "schedulectrldlg.h"
#include "scheduledatamanage.h"
#include "timeedit.h"
#include "constants.h"
#include "cscheduleoperation.h"
#include "cdynamicicon.h"
#include "configsettings.h"

#include <DHiDPIHelper>
#include <DFontSizeManager>
#include <DRadioButton>
#include <DLabel>

#include <QHBoxLayout>
#include <QIcon>
#include <QShortcut>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QBitmap>

const int  dialog_width = 468;      //对话框宽度

DGUI_USE_NAMESPACE
CScheduleDlg::CScheduleDlg(int type, QWidget *parent, const bool isAllDay)
    : DCalendarDDialog(parent)
    , m_createAllDay(isAllDay)
{
    setContentsMargins(0, 0, 0, 0);
    m_type = type;
    initUI();
    initConnection();
    setTabFouseOrder();
    initColor();

    if (type == 1) {
        m_titleLabel->setText(tr("New Event"));
        m_beginDateEdit->setDate(QDate::currentDate());
        int hours = QTime::currentTime().hour();
        int minnutes = QTime::currentTime().minute() % DDECalendar::QuarterOfAnhourWithMinute;

        if (minnutes != 0) {
            minnutes = QTime::currentTime().minute() / DDECalendar::QuarterOfAnhourWithMinute *
                       DDECalendar::QuarterOfAnhourWithMinute + DDECalendar::QuarterOfAnhourWithMinute;
        }
        m_beginTimeEdit->setTime(QTime(hours, minnutes));
        m_endDateEdit->setDate(QDate::currentDate());
        m_endTimeEdit->setTime(QTime(hours, minnutes).addSecs(3600));
    } else {
        m_titleLabel->setText(tr("Edit Event"));
    }
    setFixedSize(dialog_width, 524);
    //焦点设置到输入框
    m_textEdit->setFocus();
}

CScheduleDlg::~CScheduleDlg()
{
}

void CScheduleDlg::setData(const ScheduleDataInfo &info)
{
    m_ScheduleDataInfo = info;
    m_typeComBox->setCurrentJobTypeNo(info.getType());
    if (m_type == 1) {
        //如果为新建则设置为提示信息
        m_textEdit->setPlaceholderText(info.getTitleName());
    } else {
        //如果为编辑则显示
        m_textEdit->setPlainText(info.getTitleName());
        //光标移动到文末
        m_textEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    }

    m_beginDateEdit->setDate(info.getBeginDateTime().date());
    m_beginTimeEdit->setTime(info.getBeginDateTime().time());
    m_endDateEdit->setDate(info.getEndDateTime().date());
    m_endTimeEdit->setTime(info.getEndDateTime().time());
    m_allDayCheckbox->setChecked(info.getAllDay());
    m_endRepeatDate->setMinimumDate(info.getBeginDateTime().date());

    m_currentDate = info.getBeginDateTime();
    m_EndDate = info.getEndDateTime();

    updateEndTimeListAndTimeDiff(m_currentDate, m_EndDate);
    slotallDayStateChanged(info.getAllDay());
    //根据是否为农历更新重复选项
    updateRepeatCombox(info.getIsLunar());
    initRmindRpeatUI();
    setShowState(info.getIsLunar());
}

void CScheduleDlg::setDate(const QDateTime &date)
{
    m_currentDate = date;
    int hours = date.time().hour();
    int minnutes = 0;
    //取15的整数倍
    minnutes = date.time().minute() / DDECalendar::QuarterOfAnhourWithMinute *
               DDECalendar::QuarterOfAnhourWithMinute;
    //如果有余数则添加15分钟
    minnutes = minnutes + (date.time().minute() % DDECalendar::QuarterOfAnhourWithMinute == 0 ? 0 : 15);
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
    m_endTimeEdit->setTime(m_EndDate.time());
    m_endRepeatDate->setMinimumDate(date.date());
    updateEndTimeListAndTimeDiff(m_currentDate, m_EndDate);
    setShowState(false);
}

void CScheduleDlg::setAllDay(bool flag)
{
    m_allDayCheckbox->setChecked(flag);
}

/**
 * @brief CScheduleDlg::clickOkBtn      对话框确定按钮处理
 * @return              返回true表示隐藏对话框  false表示不隐藏对话框
 */
bool CScheduleDlg::clickOkBtn()
{
    ScheduleDataInfo _newSchedule = m_ScheduleDataInfo;
    QDateTime beginDateTime, endDateTime;
    beginDateTime.setDate(m_beginDateEdit->date());
    beginDateTime.setTime(m_beginTimeEdit->getTime());
    endDateTime.setDate(m_endDateEdit->date());
    endDateTime.setTime(m_endTimeEdit->getTime());

    //设置是否为农历日程
    switch (m_calendarCategoryRadioGroup->checkedId()) {
    case 1:
        //农历日程
        _newSchedule.setIsLunar(true);
        break;
    default:
        //公历日程
        _newSchedule.setIsLunar(false);
        break;
    }

    if (m_textEdit->toPlainText().isEmpty()) {
        _newSchedule.setTitleName(m_textEdit->placeholderText());
    } else {
        _newSchedule.setTitleName(m_textEdit->toPlainText());
    }

    if (_newSchedule.getTitleName().isEmpty()) {
        return false;
    }
    //如果类型选项不为负数则设置日程类型
    if (m_typeComBox->isEditable()) {
        JobTypeInfo jobType(0, m_typeComBox->lineEdit()->text(), m_colorSeletorWideget->getSelectedColorInfo());
        //创建日程类型
        if (CScheduleOperation().createJobType(jobType)) {
            _newSchedule.setType(jobType.getJobTypeNo());
        }
    } else if (m_typeComBox->currentIndex() >= 0)
        _newSchedule.setType(m_typeComBox->getCurrentJobTypeNo());

    if (beginDateTime > endDateTime) {
        DCalendarDDialog *prompt = new DCalendarDDialog(this);
        prompt->setIcon(QIcon(":/resources/icon/warning.svg"));
        prompt->setMessage(tr("End time must be greater than start time"));
        prompt->addButton(tr("OK", "button"), true, DDialog::ButtonNormal);
        prompt->exec();
        return false;
    }
    if (m_type == 1)
        _newSchedule.setID(0) ;
    _newSchedule.setAllDay(m_allDayCheckbox->isChecked());

    RemindData _remindData;
    if (_newSchedule.getAllDay()) {
        _remindData.setRemindTime(QTime(9, 0));
        switch (m_rmindCombox->currentIndex()) {
        case 1:
            _remindData.setRemindNum(DDECalendar::OnStartDay);
            break;
        case 2:
            _remindData.setRemindNum(DDECalendar::OneDayBeforeWithDay);
            break;
        case 3:
            _remindData.setRemindNum(DDECalendar::TwoDayBeforeWithDay);
            break;
        case 4:
            _remindData.setRemindNum(DDECalendar::OneWeekBeforeWithDay);
            break;
        default:
            break;
        }
    } else {
        switch (m_rmindCombox->currentIndex()) {
        case 1:
            _remindData.setRemindNum(DDECalendar::AtTimeOfEvent);
            break;
        case 2:
            _remindData.setRemindNum(DDECalendar::FifteenMinutesBefore);
            break;
        case 3:
            _remindData.setRemindNum(DDECalendar::ThirtyMinutesBefore);
            break;
        case 4:
            _remindData.setRemindNum(DDECalendar::OneHourBefore);
            break;
        case 5:
            _remindData.setRemindNum(DDECalendar::OneDayBeforeWithMinutes);
            break;
        case 6:
            _remindData.setRemindNum(DDECalendar::TwoDayBeforeWithMinutes);
            break;
        case 7:
            _remindData.setRemindNum(DDECalendar::OneWeekBeforeWithMinutes);
            break;
        default:
            break;
        }
    }
    _newSchedule.setRemindData(_remindData);

    RepetitionRule _repetitionRule;
    //根据是否为农历日程，设置对应的重复规则
    RepetitionRule::RRuleID ruleID;
    if (_newSchedule.getIsLunar()) {
        switch (m_beginrepeatCombox->currentIndex()) {
        case 1:
            //每月
            ruleID = RepetitionRule::RRule_EVEMONTH;
            break;
        case 2: {
            //每年
            ruleID = RepetitionRule::RRule_EVEYEAR;
        } break;
        default:
            //默认不重复
            ruleID = RepetitionRule::RRule_NONE;
            break;
        }

    } else {
        ruleID = static_cast<RepetitionRule::RRuleID>(m_beginrepeatCombox->currentIndex());
    }
    _repetitionRule.setRuleId(ruleID);
    if (_repetitionRule.getRuleId() > 0) {
        _repetitionRule.setRuleType(static_cast<RepetitionRule::RRuleEndType>
                                    (m_endrepeatCombox->currentIndex()));
        if (m_endrepeatCombox->currentIndex() == 1) {
            if (m_endrepeattimes->text().isEmpty()) {
                return false;
            }
            _repetitionRule.setEndCount(m_endrepeattimes->text().toInt());
        } else if (m_endrepeatCombox->currentIndex() == 2) {
            QDateTime endrpeattime = beginDateTime;
            endrpeattime.setDate(m_endRepeatDate->date());

            if (beginDateTime > endrpeattime) {
                return false;
            }
            _repetitionRule.setEndDate(endrpeattime);
        }
    }
    _newSchedule.setRepetitionRule(_repetitionRule);
    _newSchedule.setBeginDateTime(beginDateTime);
    _newSchedule.setEndDateTime(endDateTime);
    CScheduleOperation _scheduleOperation(this);

    if (m_type == 1) {
        //创建日程
        _scheduleOperation.createSchedule(_newSchedule);

    } else if (m_type == 0) {
        //修改日程,根据返回的参数判断是否关闭对话框
        return _scheduleOperation.changeSchedule(_newSchedule, m_ScheduleDataInfo);
    }
    return true;
}

void CScheduleDlg::updateEndTimeListAndTimeDiff(const QDateTime &begin, const QDateTime &end)
{
    //更新是否超过一天标识
    updateIsOneMoreDay(begin, end);
    m_timeDiff = begin.msecsTo(end);
    updateEndTimeList(begin.time(), !m_isMoreThenOneDay);
    m_endTimeEdit->setTime(end.time());
    m_endTimeEdit->setCurrentText(m_endTimeEdit->lineEdit()->text());
}

void CScheduleDlg::updateEndTimeList(const QTime &begin, bool isShowTimeInterval)
{
    m_endTimeEdit->setMineTime(begin);
    m_endTimeEdit->updateListItem(isShowTimeInterval);
}

void CScheduleDlg::slotBeginTimeChange()
{
    if (m_currentDate.time() == m_beginTimeEdit->getTime())
        return;
    //根据联动修改结束时间下拉列表内容和结束时间
    m_currentDate.setTime(m_beginTimeEdit->getTime());
    m_EndDate = m_currentDate.addMSecs(m_timeDiff);
    //开始时间改变需要更新是否超过一天标识，根据该标识判断结束时间下拉列表是否展示（xxx:xxx hours）
    //由于结束时间（endTimeEdit）自己不参加联动，所以该标识需要在开始时间中更新
    updateIsOneMoreDay(m_currentDate, m_EndDate);
    updateEndTimeList(m_currentDate.time(), !m_isMoreThenOneDay);
    m_endTimeEdit->setTime(m_EndDate.time());
    m_endDateEdit->setDate(m_EndDate.date());
}

void CScheduleDlg::slotEndTimeChange()
{
    if (m_EndDate.time() == m_endTimeEdit->getTime()) {
        return;
    }
    m_EndDate.setTime(m_endTimeEdit->getTime());
    //根据结束时间修改联动时间和结束日期
    //如果时间没有超过一天
    if (!m_isMoreThenOneDay) {
        //如果时间小于一天则需要根据结束时间（time）是否大于开始时间判断计算结束日期
        QTime endTime = m_endTimeEdit->getTime();
        if (endTime < m_currentDate.time()) {
            m_EndDate.setDate(m_currentDate.date().addDays(1));
        } else {
            m_EndDate.setDate(m_currentDate.date());
        }
        m_endDateEdit->setDate(m_EndDate.date());
    }
    m_timeDiff = m_currentDate.msecsTo(m_EndDate);
}

void CScheduleDlg::slotEndDateChange(const QDate &date)
{
    if (m_EndDate.date() == date)
        return;
    //修改联动时间和结束时间下拉列表内容
    m_EndDate.setDate(date);
    // 如果开始时间晚于结束时间，则将结束时间修改为开始时间
    if (m_currentDate.msecsTo(m_EndDate) < 0) {
        m_EndDate = m_currentDate;
    }
    updateEndTimeListAndTimeDiff(m_currentDate, m_EndDate);
}

void CScheduleDlg::slotBtClick(int buttonIndex, const QString &buttonName)
{
    Q_UNUSED(buttonName)
    //是否隐藏对话框
    switch (buttonIndex) {
    case 0: {
        //取消
        close();
        break;
    }
    case 1: {
        //确定
        //自动化测试会出现短时间内按钮click2次的情况。添加第一次触发后将保存按钮置灰的设置。
        //若保存按钮不启用则不处理
        if (getButton(1)->isEnabled()) {
            m_setAccept = clickOkBtn();
            //若新建或编辑成功则将保存按钮置灰
            getButton(1)->setEnabled(!m_setAccept);
            //如果确定创建(修改)成功则关闭对话框
            if (m_setAccept) {
                close();
            } else {
                setFocusProxy(getButton(1));
            }
        }
        break;
    }
    default:
        break;
    }
}

void CScheduleDlg::slotTextChange()
{
    QString textContent = m_textEdit->toPlainText();
    int length = textContent.count();
    QString tStitlename = textContent;

    int maxLength = 256; // 最大字符数
    //去除回车字符
    if (tStitlename.contains("\n")) {
        //设置纯文本显示原始内容
        tStitlename.replace("\n", "");
        m_textEdit->setPlainText(tStitlename);
        //将焦点移动到最后
        m_textEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
        //因设置text会再次触发textchange信号,在此直接return
        return;
    }
    //如果长度大于最大长度则显示原来的字符
    if (length > maxLength) {
        m_textEdit->setPlainText(m_context);
        //将焦点移动到最后
        m_textEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
        return;
    }

    m_context = m_textEdit->toPlainText();
}

void CScheduleDlg::slotendrepeatTextchange()
{
    setOkBtnEnabled();
}

void CScheduleDlg::slotBDateEidtInfo(const QDate &date)
{
    m_endRepeatDate->setMinimumDate(date);
    m_endDateEdit->setMinimumDate(date);
    m_currentDate.setDate(m_beginDateEdit->date());
    m_EndDate.setDate(m_endDateEdit->date());

    if (m_EndDate < m_currentDate) {
        m_endTimeEdit->setTime(m_beginTimeEdit->getTime().addSecs(3600));
        m_EndDate.setTime(m_endTimeEdit->getTime());
    }
    //修改联动时间和结束时间下拉列表内容
    updateEndTimeListAndTimeDiff(m_currentDate, m_EndDate);
}

void CScheduleDlg::slotallDayStateChanged(int state)
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
            m_beginDateEdit->setDate(m_ScheduleDataInfo.getBeginDateTime().date());
            m_beginTimeEdit->setTime(m_ScheduleDataInfo.getBeginDateTime().time());
            m_endDateEdit->setDate(m_ScheduleDataInfo.getEndDateTime().date());
            m_endTimeEdit->setTime(m_ScheduleDataInfo.getEndDateTime().time());
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
            m_beginDateEdit->setDate(m_ScheduleDataInfo.getBeginDateTime().date());
            m_beginTimeEdit->setTime(QTime(0, 0));
            m_endDateEdit->setDate(m_ScheduleDataInfo.getEndDateTime().date());
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

void CScheduleDlg::slotbRpeatactivated(int index)
{
    if (index > 0) {
        m_endrepeatWidget->setVisible(true);
    } else {
        m_endrepeatWidget->setVisible(false);
    }
    setOkBtnEnabled();
    resize();
}

void CScheduleDlg::sloteRpeatactivated(int index)
{
    if (index == 0) {
        m_endrepeattimesWidget->setVisible(false);
        m_endRepeatDate->setVisible(false);
    } else if (index == 1) {
        m_endrepeattimesWidget->setVisible(true);
        m_endRepeatDate->setVisible(false);
        QFont mlabelF;
        mlabelF.setWeight(QFont::Medium);
        QFontMetrics fontWidth_endrepeattimesLabel(mlabelF);
        QString endrepeattimesStr = fontWidth_endrepeattimesLabel.elidedText(tr("time(s)"), Qt::ElideRight,
                                                                             m_endrepeattimesLabel->width());
        m_endrepeattimesLabel->setText(endrepeattimesStr);
    } else {
        m_endrepeattimesWidget->setVisible(false);
        m_endRepeatDate->setVisible(true);
    }
    setOkBtnEnabled();
}

void CScheduleDlg::slotJobComboBoxEditingFinished()
{
    if (m_typeComBox->lineEdit()->text().isEmpty()) {
        //名称为空
        m_typeComBox->showAlertMessage(tr("Enter a name please"));
        m_typeComBox->setAlert(true);
    }
}

void CScheduleDlg::slotTypeRpeatactivated(int index)
{
    Q_UNUSED(index);
    if (m_typeComBox->isEditable()) {
        m_typeComBox->setIconSize(QSize(0, 0));
        m_colorSeletorWideget->show();
    } else {
        //若下拉选择隐藏提醒消息
        m_typeComBox->hideAlertMessage();
        m_typeComBox->setIconSize(QSize(16, 16));
        m_typeEditStatus = false;
        m_colorSeletorWideget->hide();
    }
    //设置保存按钮状态
    setOkBtnEnabled();
    resize();
}

void CScheduleDlg::slotRadioBtnClicked(int btnId)
{
    //与上一次选项一致不做重置处理
    if (m_prevCheckRadioID == btnId) {
        return;
    }
    m_prevCheckRadioID = btnId;
    bool jobIsLunar = RadioLunarId == btnId;
    setShowState(jobIsLunar);
    //更新重复规则下拉显示和保存按钮的显示状态
    updateRepeatCombox(jobIsLunar);
}

void CScheduleDlg::slotBtnAddItemClicked()
{
    m_colorSeletorWideget->show();
    m_typeEditStatus = true;
    //添加日程类型的时候需要判断保存按钮是否可用
    setOkBtnEnabled();
    resize();
}

void CScheduleDlg::slotTypeEditTextChanged(const QString &text)
{
    if (!m_typeEditStatus) {
        return;
    }
    QString tStitlename = text;
    //去除回车字符
    if (tStitlename.contains("\n")) {
        //设置纯文本显示原始内容
        tStitlename.replace("\n", "");
        m_typeComBox->setEditText(tStitlename);
        return;
    }
    //最大限制20个字符，超出后过滤掉
    if (tStitlename.length() > 20) {
        m_typeComBox->setEditText(m_TypeContext);
        return;
    } else {
        m_TypeContext = tStitlename;
    }
    //如果内容不为空且去除空格内容为空表示为全空格
    if (!tStitlename.isEmpty() && tStitlename.trimmed().isEmpty()) {
        //名称为全空格，返回
        m_typeComBox->showAlertMessage(tr("The name can not only contain whitespaces"));
        m_typeComBox->setAlert(true);
    } else if (JobTypeInfoManager::instance()->isJobTypeNameUsed(tStitlename)) {
        //重名，返回
        m_typeComBox->showAlertMessage(tr("The name already exists"));
        m_typeComBox->setAlert(true);
    } else {
        //如果日程类型编辑框存在焦点（没有编辑结束）且有警告则取消警告和提示信息
        if (m_typeComBox->hasFocus() && m_typeComBox->isAlert()) {
            m_typeComBox->hideAlertMessage();
            m_typeComBox->setAlert(false);
        }
    }
    setOkBtnEnabled();
    m_TypeContext = tStitlename;
}

bool CScheduleDlg::eventFilter(QObject *obj, QEvent *pEvent)
{
    if (obj == m_textEdit) {
        if (pEvent->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(pEvent);
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                return true;
            }
            if (keyEvent->key() == Qt::Key_Tab) {
                //如果为tab则切换到全天checkbox
                m_allDayCheckbox->setFocus(Qt::TabFocusReason);
                return true;
            }
        }
    }
    return DCalendarDDialog::eventFilter(obj, pEvent);
}

void CScheduleDlg::showEvent(QShowEvent *event)
{
    DDialog::showEvent(event);
    //更新窗口大小
    resize();
}

void CScheduleDlg::closeEvent(QCloseEvent *event)
{
    DDialog::closeEvent(event);
    //如果为true 这设置返回值为Accepted 否则设置为Rejected
    if (m_setAccept) {
        accept();
    } else {
        reject();
    }
}

void CScheduleDlg::changeEvent(QEvent *event)
{
    Q_UNUSED(event);
    QFont mlabelF;
    mlabelF.setWeight(QFont::Medium);

    QFontMetrics fontWidth_typeLabel(mlabelF);
    QString str_typelabel = fontWidth_typeLabel.elidedText(tr("Type:"), Qt::ElideRight,
                                                           DDECalendar::NewScheduleLabelWidth);
    m_typeLabel->setText(str_typelabel);

    QFontMetrics fontWidth_contentlabel(mlabelF);
    QString str_contentlabel = fontWidth_contentlabel.elidedText(tr("Description:"), Qt::ElideRight,
                                                                 DDECalendar::NewScheduleLabelWidth);
    m_contentLabel->setText(str_contentlabel);

    QFontMetrics fontWidth_allDayLabel(mlabelF);
    QString str_allDayLabel = fontWidth_allDayLabel.elidedText(tr("All Day:"), Qt::ElideRight,
                                                               DDECalendar::NewScheduleLabelWidth);
    m_adllDayLabel->setText(str_allDayLabel);

    QFontMetrics fontWidth_beginTimeLabel(mlabelF);
    QString str_beginTimeLabel = fontWidth_beginTimeLabel.elidedText(tr("Starts:"), Qt::ElideRight,
                                                                     DDECalendar::NewScheduleLabelWidth);
    m_beginTimeLabel->setText(str_beginTimeLabel);

    QFontMetrics fontWidth_endTimeLabel(mlabelF);
    QString str_endTimeLabel = fontWidth_endTimeLabel.elidedText(tr("Ends:"), Qt::ElideRight,
                                                                 DDECalendar::NewScheduleLabelWidth);
    m_endTimeLabel->setText(str_endTimeLabel);

    QFontMetrics fontWidth_remindSetLabel(mlabelF);
    QString str_remindSetLabel = fontWidth_remindSetLabel.elidedText(tr("Remind Me:"), Qt::ElideRight,
                                                                     DDECalendar::NewScheduleLabelWidth);
    m_remindSetLabel->setText(str_remindSetLabel);

    QFontMetrics fontWidth_beginRepeatLabel(mlabelF);
    QString str_beginRepeatLabel = fontWidth_beginRepeatLabel.elidedText(tr("Repeat:"), Qt::ElideRight,
                                                                         DDECalendar::NewScheduleLabelWidth);
    m_beginrepeatLabel->setText(str_beginRepeatLabel);

    QFontMetrics fontWidth_endrepeatLabel(mlabelF);
    QString str_endrepeatLabel = fontWidth_endrepeatLabel.elidedText(tr("End Repeat:"), Qt::ElideRight,
                                                                     DDECalendar::NewScheduleLabelWidth);
    m_endrepeatLabel->setText(str_endrepeatLabel);

    if (m_endrepeattimesWidget->isVisible()) {
        //如果结束与次数显示,则根据label大小设置显示内容
        QFontMetrics fontWidth_endrepeattimesLabel(mlabelF);
        QString endrepeattimesStr = fontWidth_endrepeattimesLabel.elidedText(tr("time(s)"), Qt::ElideRight,
                                                                             m_endrepeattimesLabel->width());
        m_endrepeattimesLabel->setText(endrepeattimesStr);
    }
}

/**
 * @brief CScheduleDlg::updateDateTimeFormat 更新日期显示格式
 */
void CScheduleDlg::updateDateTimeFormat()
{
    m_beginDateEdit->setDisplayFormat(m_dateFormat);
    m_endDateEdit->setDisplayFormat(m_dateFormat);
    m_endRepeatDate->setDisplayFormat(m_dateFormat);
}

void CScheduleDlg::initUI()
{
    const int label_Fixed_Width = 78;
    const int item_Fixed_Height = 36;

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
    //类型
    {
        //使用网格布局
        QGridLayout *typelayout = new QGridLayout;
        typelayout->setSpacing(0);
        typelayout->setMargin(0);
        m_typeLabel = new QLabel();
        m_typeLabel->setToolTip(tr("Type"));
        DFontSizeManager::instance()->bind(m_typeLabel, DFontSizeManager::T6);
        QFontMetrics fontWidth_typeLabel(mlabelF);
        QString str_typelabel = fontWidth_typeLabel.elidedText(tr("Type:"), Qt::ElideRight,
                                                               DDECalendar::NewScheduleLabelWidth);
        m_typeLabel->setText(str_typelabel);
        m_typeLabel->setFont(mlabelF);
        m_typeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_typeLabel->setFixedSize(label_Fixed_Width, item_Fixed_Height);
        //m_typeComBox = new DComboBox(this);
        m_typeComBox = new JobTypeComboBox(this);
        //设置对象名称和辅助显示名称
        m_typeComBox->setObjectName("ScheduleTypeCombobox");
        m_typeComBox->setAccessibleName("ScheduleTypeCombobox");
        m_typeComBox->setFixedSize(350, item_Fixed_Height);
        initJobTypeComboBox();//todo

        m_colorSeletorWideget = new ColorSeletorWidget();
        m_colorSeletorWideget->hide();

        typelayout->addWidget(m_typeLabel, 0, 0);
        typelayout->addWidget(m_typeComBox, 0, 1);
        typelayout->addWidget(m_colorSeletorWideget, 1, 1);
        //添加垂直间隔
        typelayout->setVerticalSpacing(5);
        maintlayout->addLayout(typelayout);
    }

    //内容
    {
        QHBoxLayout *contentLabellayout = new QHBoxLayout;
        contentLabellayout->setSpacing(0);
        contentLabellayout->setMargin(0);

        m_contentLabel = new QLabel(this);
        DFontSizeManager::instance()->bind(m_contentLabel, DFontSizeManager::T6);
        QFontMetrics fontWidth_contentlabel(mlabelF);
        QString str_contentlabel = fontWidth_contentlabel.elidedText(tr("Description:"), Qt::ElideRight,
                                                                     DDECalendar::NewScheduleLabelWidth);
        m_contentLabel->setText(str_contentlabel);
        m_contentLabel->setFont(mlabelF);
        m_contentLabel->setToolTip(tr("Description"));
        m_contentLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_contentLabel->setFixedSize(label_Fixed_Width, item_Fixed_Height);

        m_textEdit = new DTextEdit(this);
        //设置对象名称和辅助显示名称
        m_textEdit->setObjectName("ScheduleTitleEdit");
        m_textEdit->setAccessibleName("ScheduleTitleEdit");
        m_textEdit->setFixedSize(350, 86);
        m_textEdit->setAcceptRichText(false);

        m_textEdit->setPlaceholderText(tr("New Event"));
        //设置关联控件，用于QTextEdit控件捕获MouseButtonPress等事件
        QWidget *mpContentWidget = m_textEdit->viewport();
        //设置事件过滤器
        m_textEdit->installEventFilter(this);
        mpContentWidget->installEventFilter(this);

        contentLabellayout->addWidget(m_contentLabel, 0, Qt::AlignTop);
        contentLabellayout->addWidget(m_textEdit);
        contentLabellayout->addStretch();
        maintlayout->addLayout(contentLabellayout);
    }

    //全天
    {
        QHBoxLayout *alldayLabellayout = new QHBoxLayout;
        alldayLabellayout->setSpacing(0);
        alldayLabellayout->setMargin(0);
        m_adllDayLabel = new QLabel(this);
        m_adllDayLabel->setToolTip(tr("All Day"));
        DFontSizeManager::instance()->bind(m_adllDayLabel, DFontSizeManager::T6);
        QFontMetrics fontWidth_allDayLabel(mlabelF);
        QString str_allDayLabel = fontWidth_allDayLabel.elidedText(tr("All Day:"), Qt::ElideRight,
                                                                   DDECalendar::NewScheduleLabelWidth);
        m_adllDayLabel->setText(str_allDayLabel);
        m_adllDayLabel->setFont(mlabelF);
        m_adllDayLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_adllDayLabel->setFixedSize(label_Fixed_Width, item_Fixed_Height);
        m_allDayCheckbox = new DCheckBox(this);
        //设置对象名称和辅助显示名称
        m_allDayCheckbox->setObjectName("AllDayCheckBox");
        m_allDayCheckbox->setAccessibleName("AllDayCheckBox");
        alldayLabellayout->addWidget(m_adllDayLabel);
        alldayLabellayout->addWidget(m_allDayCheckbox);
        maintlayout->addLayout(alldayLabellayout);
    }

    //时间
    {
        DLabel *tLabel = new DLabel(tr("Time:"));
        tLabel->setToolTip(tr("Time"));
        DFontSizeManager::instance()->bind(tLabel, DFontSizeManager::T6);
        tLabel->setElideMode(Qt::ElideRight);
        tLabel->setFont(mlabelF);
        tLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        tLabel->setFixedSize(DDECalendar::NewScheduleLabelWidth, item_Fixed_Height);

        m_solarRadioBtn = new DRadioButton(tr("Solar"));
        m_lunarRadioBtn = new DRadioButton(tr("Lunar"));
        m_solarRadioBtn->setMinimumWidth(72);
        m_lunarRadioBtn->setMinimumWidth(72);

        m_calendarCategoryRadioGroup = new QButtonGroup(this);
        m_calendarCategoryRadioGroup->setExclusive(true);
        m_calendarCategoryRadioGroup->addButton(m_solarRadioBtn, RadioSolarId);
        m_calendarCategoryRadioGroup->addButton(m_lunarRadioBtn, RadioLunarId);

        QHBoxLayout *tLayout = new QHBoxLayout;
        tLayout->setSpacing(8);
        tLayout->setMargin(0);
        tLayout->addWidget(tLabel);
        tLayout->addWidget(m_solarRadioBtn);
        tLayout->addWidget(m_lunarRadioBtn);
        tLayout->addStretch(1);

        maintlayout->addLayout(tLayout);
    }

    //开始时间
    {
        QHBoxLayout *beginLabellayout = new QHBoxLayout;
        beginLabellayout->setSpacing(0);
        beginLabellayout->setMargin(0);
        m_beginTimeLabel = new QLabel(this);
        m_beginTimeLabel->setToolTip(tr("Starts"));
        DFontSizeManager::instance()->bind(m_beginTimeLabel, DFontSizeManager::T6);
        QFontMetrics fontWidth_beginTimeLabel(mlabelF);
        QString str_beginTimeLabel = fontWidth_beginTimeLabel.elidedText(tr("Starts:"), Qt::ElideRight,
                                                                         DDECalendar::NewScheduleLabelWidth);
        m_beginTimeLabel->setText(str_beginTimeLabel);
        m_beginTimeLabel->setFont(mlabelF);
        m_beginTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_beginTimeLabel->setFixedSize(label_Fixed_Width, item_Fixed_Height);
        m_beginDateEdit = new CDateEdit(this);
        m_beginDateEdit->setDisplayFormat(m_dateFormat);
        //设置对象名称和辅助显示名称
        m_beginDateEdit->setObjectName("ScheduleBeginDateEdit");
        m_beginDateEdit->setAccessibleName("ScheduleBeginDateEdit");
        m_beginDateEdit->setFixedSize(200, item_Fixed_Height);
        m_beginDateEdit->setCalendarPopup(true);

        m_beginTimeEdit = new CTimeEdit(this);
        //设置对象名称和辅助显示名称
        m_beginTimeEdit->setObjectName("ScheduleBeginTimeEdit");
        m_beginTimeEdit->setAccessibleName("ScheduleBeginTimeEdit");
        m_beginTimeEdit->setFixedSize(140, item_Fixed_Height);

        beginLabellayout->addWidget(m_beginTimeLabel);
        beginLabellayout->addWidget(m_beginDateEdit);
        beginLabellayout->addSpacing(8);
        beginLabellayout->addWidget(m_beginTimeEdit);
        beginLabellayout->addStretch();
        maintlayout->addLayout(beginLabellayout);
    }

    //结束时间
    {
        QHBoxLayout *enQLabellayout = new QHBoxLayout;
        enQLabellayout->setSpacing(0);
        enQLabellayout->setMargin(0);
        m_endTimeLabel = new QLabel(this);
        m_endTimeLabel->setToolTip(tr("Ends"));
        DFontSizeManager::instance()->bind(m_endTimeLabel, DFontSizeManager::T6);
        QFontMetrics fontWidth_endTimeLabel(mlabelF);
        QString str_endTimeLabel = fontWidth_endTimeLabel.elidedText(tr("Ends:"), Qt::ElideRight,
                                                                     DDECalendar::NewScheduleLabelWidth);
        m_endTimeLabel->setText(str_endTimeLabel);
        m_endTimeLabel->setFont(mlabelF);
        m_endTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_endTimeLabel->setFixedSize(label_Fixed_Width, item_Fixed_Height);
        m_endDateEdit = new CDateEdit(this);
        m_endDateEdit->setDisplayFormat(m_dateFormat);
        //设置对象名称和辅助显示名称
        m_endDateEdit->setObjectName("ScheduleEndDateEdit");
        m_endDateEdit->setAccessibleName("ScheduleEndDateEdit");
        m_endDateEdit->setFixedSize(200, item_Fixed_Height);
        m_endDateEdit->setCalendarPopup(true);

        m_endTimeEdit = new CTimeEdit(this);
        //设置对象名称和辅助显示名称
        m_endTimeEdit->setObjectName("ScheduleEndTimeEdit");
        m_endTimeEdit->setAccessibleName("ScheduleEndTimeEdit");
        m_endTimeEdit->setFixedSize(140, item_Fixed_Height);

        enQLabellayout->addWidget(m_endTimeLabel);
        enQLabellayout->addWidget(m_endDateEdit);
        enQLabellayout->addSpacing(8);

        enQLabellayout->addWidget(m_endTimeEdit);
        enQLabellayout->addStretch();
        maintlayout->addLayout(enQLabellayout);
    }

    //提醒
    {
        QHBoxLayout *rminQLabellayout = new QHBoxLayout;
        rminQLabellayout->setSpacing(0);
        rminQLabellayout->setMargin(0);
        m_remindSetLabel = new QLabel();
        DFontSizeManager::instance()->bind(m_remindSetLabel, DFontSizeManager::T6);
        QFontMetrics fontWidth_remindSetLabel(mlabelF);
        QString str_remindSetLabel = fontWidth_remindSetLabel.elidedText(tr("Remind Me:"), Qt::ElideRight,
                                                                         DDECalendar::NewScheduleLabelWidth);
        m_remindSetLabel->setToolTip(tr("Remind Me"));
        m_remindSetLabel->setText(str_remindSetLabel);
        m_remindSetLabel->setFont(mlabelF);
        m_remindSetLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_remindSetLabel->setFixedSize(label_Fixed_Width, item_Fixed_Height);

        m_rmindCombox = new DComboBox(this);
        //设置对象名称和辅助显示名称
        m_rmindCombox->setObjectName("RmindComboBox");
        m_rmindCombox->setAccessibleName("RmindComboBox");
        m_rmindCombox->setFixedSize(200, item_Fixed_Height);
        m_rmindCombox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        rminQLabellayout->addWidget(m_remindSetLabel);
        rminQLabellayout->addWidget(m_rmindCombox);
        rminQLabellayout->addStretch();
        maintlayout->addLayout(rminQLabellayout);
    }

    //重复
    {
        QHBoxLayout *repeatLabellayout = new QHBoxLayout;
        repeatLabellayout->setSpacing(0);
        repeatLabellayout->setMargin(0);
        m_beginrepeatLabel = new QLabel();
        m_beginrepeatLabel->setToolTip(tr("Repeat"));
        DFontSizeManager::instance()->bind(m_beginrepeatLabel, DFontSizeManager::T6);
        QFontMetrics fontWidth_beginRepeatLabel(mlabelF);
        QString str_beginRepeatLabel = fontWidth_beginRepeatLabel.elidedText(tr("Repeat:"), Qt::ElideRight,
                                                                             DDECalendar::NewScheduleLabelWidth);
        m_beginrepeatLabel->setText(str_beginRepeatLabel);
        m_beginrepeatLabel->setFont(mlabelF);
        m_beginrepeatLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_beginrepeatLabel->setFixedSize(label_Fixed_Width, item_Fixed_Height);
        m_beginrepeatCombox = new DComboBox(this);
        //设置对象名称和辅助显示名称
        m_beginrepeatCombox->setObjectName("BeginRepeatComboBox");
        m_beginrepeatCombox->setAccessibleName("BeginRepeatComboBox");
        m_beginrepeatCombox->setFixedSize(200, item_Fixed_Height);
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
    }

    //结束重复
    {
        QHBoxLayout *endrepeatLabellayout = new QHBoxLayout;
        endrepeatLabellayout->setSpacing(0);
        endrepeatLabellayout->setMargin(0);
        m_endrepeatLabel = new QLabel();
        DFontSizeManager::instance()->bind(m_endrepeatLabel, DFontSizeManager::T6);
        QFontMetrics fontWidth_endrepeatLabel(mlabelF);
        QString str_endrepeatLabel = fontWidth_endrepeatLabel.elidedText(tr("End Repeat:"), Qt::ElideRight,
                                                                         DDECalendar::NewScheduleLabelWidth);
        m_endrepeatLabel->setToolTip(tr("End Repeat"));
        m_endrepeatLabel->setText(str_endrepeatLabel);
        m_endrepeatLabel->setFont(mlabelF);
        m_endrepeatLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_endrepeatLabel->setFixedSize(label_Fixed_Width, item_Fixed_Height);
        m_endrepeatCombox = new DComboBox(this);
        //设置对象名称和辅助显示名称
        m_endrepeatCombox->setObjectName("EndRepeatComboBox");
        m_endrepeatCombox->setAccessibleName("EndRepeatComboBox");
        m_endrepeatCombox->setFixedSize(200, item_Fixed_Height);
        m_endrepeatCombox->addItem(tr("Never"));
        m_endrepeatCombox->addItem(tr("After"));
        m_endrepeatCombox->addItem(tr("On"));
        endrepeatLabellayout->addWidget(m_endrepeatLabel);
        endrepeatLabellayout->addWidget(m_endrepeatCombox);
        endrepeatLabellayout->addSpacing(8);

        QHBoxLayout *endrepeattimeslayout = new QHBoxLayout;
        endrepeattimeslayout->setSpacing(0);
        endrepeattimeslayout->setMargin(0);
        endrepeattimeslayout->setContentsMargins(0, 0, 0, 0);
        m_endrepeattimes = new DLineEdit(this);
        //设置对象名称和辅助显示名称
        m_endrepeattimes->setObjectName("EndRepeatTimeEidt");
        m_endrepeattimes->setAccessibleName("EndRepeatTimeEidt");
        m_endrepeattimes->setFixedSize(71, item_Fixed_Height);
        m_endrepeattimes->setText(QString::number(10));
        m_endrepeattimes->setClearButtonEnabled(false);
        QRegExp rx("^[1-9]\\d{0,2}$");
        QValidator *validator = new QRegExpValidator(rx, this);
        m_endrepeattimes->lineEdit()->setValidator(validator);
        m_endrepeattimesLabel = new QLabel(tr("time(s)"));
        m_endrepeattimesLabel->setToolTip(tr("time(s)"));
        m_endrepeattimesLabel->setFont(mlabelF);
        m_endrepeattimesLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_endrepeattimesLabel->setFixedHeight(item_Fixed_Height);
        endrepeattimeslayout->addWidget(m_endrepeattimes);
        endrepeattimeslayout->addWidget(m_endrepeattimesLabel);
        m_endrepeattimesWidget = new DWidget;
        //设置对象名称和辅助显示名称
        m_endrepeattimesWidget->setObjectName("EndRepeatTimeWidget");
        m_endrepeattimesWidget->setAccessibleName("EndRepeatTimeWidget");
        m_endrepeattimesWidget->setLayout(endrepeattimeslayout);
        m_endrepeattimesWidget->setVisible(false);
        m_endrepeattimesWidget->setFixedSize(140, item_Fixed_Height);
        endrepeatLabellayout->addWidget(m_endrepeattimesWidget);

        m_endRepeatDate = new CDateEdit;
        //设置对象名称和辅助显示名称
        m_endRepeatDate->setObjectName("EndRepeatDateEdit");
        m_endRepeatDate->setAccessibleName("EndRepeatDateEdit");
        m_endRepeatDate->setCalendarPopup(true);
        m_endRepeatDate->setFixedSize(140, item_Fixed_Height);
        m_endRepeatDate->setDate(QDate::currentDate());
        m_endRepeatDate->setDisplayFormat(m_dateFormat);
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
    }

    m_gwi = new DFrame(this);
    m_gwi->setFrameShape(QFrame::NoFrame);
    m_gwi->setLayout(maintlayout);
    addContent(m_gwi, Qt::AlignCenter);
    initDateEdit();
    if (m_type == 1)
        slotallDayStateChanged(0);
    //添加按钮
    addButton(tr("Cancel", "button"));
    addButton(tr("Save", "button"), false, DDialog::ButtonRecommend);
    for (int i = 0; i < buttonCount(); i++) {
        QAbstractButton *button = getButton(i);
        button->setFixedSize(189, 36);
    }
}

void CScheduleDlg::initConnection()
{
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this,
                     &CScheduleDlg::setTheMe);
    connect(this, &DDialog::buttonClicked, this, &CScheduleDlg::slotBtClick);
    connect(m_textEdit, &DTextEdit::textChanged, this, &CScheduleDlg::slotTextChange);
    connect(m_endrepeattimes, &DLineEdit::textChanged, this, &CScheduleDlg::slotendrepeatTextchange);

    connect(m_allDayCheckbox, &DCheckBox::stateChanged, this,
            &CScheduleDlg::slotallDayStateChanged);
    connect(m_beginrepeatCombox, QOverload<int>::of(&QComboBox::activated), this,
            &CScheduleDlg::slotbRpeatactivated);
    connect(m_endrepeatCombox, QOverload<int>::of(&QComboBox::activated), this,
            &CScheduleDlg::sloteRpeatactivated);
    connect(m_typeComBox, QOverload<int>::of(&QComboBox::activated), this,
            &CScheduleDlg::slotTypeRpeatactivated);
    connect(m_beginDateEdit, &DDateEdit::userDateChanged, this, &CScheduleDlg::slotBDateEidtInfo);
    QShortcut *shortcut = new QShortcut(this);
    shortcut->setKey(QKeySequence(QLatin1String("ESC")));
    connect(shortcut, SIGNAL(activated()), this, SLOT(close()));

    connect(m_beginTimeEdit, &CTimeEdit::signaleditingFinished, this,
            &CScheduleDlg::slotBeginTimeChange);
    connect(m_endTimeEdit, &CTimeEdit::signaleditingFinished, this, &CScheduleDlg::slotEndTimeChange);
    connect(m_endDateEdit, &QDateEdit::userDateChanged, this, &CScheduleDlg::slotEndDateChange);
    connect(m_calendarCategoryRadioGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &CScheduleDlg::slotRadioBtnClicked);
    connect(m_typeComBox, &JobTypeComboBox::signalAddTypeBtnClicked, this, &CScheduleDlg::slotBtnAddItemClicked);
    connect(m_typeComBox, &JobTypeComboBox::editTextChanged, this, &CScheduleDlg::slotTypeEditTextChanged);
    connect(m_typeComBox, &JobTypeComboBox::editingFinished, this, &CScheduleDlg::slotJobComboBoxEditingFinished);
}

void CScheduleDlg::initDateEdit()
{
    m_beginDateEdit->setMinimumDate(QDate(DDECalendar::QueryEarliestYear, 1, 1)); // 0天
    m_beginDateEdit->setMaximumDate(QDate(DDECalendar::QueryLatestYear, 12, 31));
    m_endDateEdit->setMinimumDate(QDate(DDECalendar::QueryEarliestYear, 1, 1)); // 0天
    m_endDateEdit->setMaximumDate(QDate(DDECalendar::QueryLatestYear, 12, 31));
    return;
}
void CScheduleDlg::initJobTypeComboBox()
{
    m_typeComBox->updateJobType();
}

void CScheduleDlg::initRmindRpeatUI()
{
    if (m_ScheduleDataInfo.getAllDay()) {
        if (m_ScheduleDataInfo.getRemindData().getRemindNum() > -1) {
            if (m_ScheduleDataInfo.getRemindData().getRemindNum() == DDECalendar::OnStartDay) {
                m_rmindCombox->setCurrentIndex(1);
            } else if (m_ScheduleDataInfo.getRemindData().getRemindNum() == DDECalendar::OneDayBeforeWithDay) {
                m_rmindCombox->setCurrentIndex(2);
            } else if (m_ScheduleDataInfo.getRemindData().getRemindNum() == DDECalendar::TwoDayBeforeWithDay) {
                m_rmindCombox->setCurrentIndex(3);
            } else if (m_ScheduleDataInfo.getRemindData().getRemindNum() == DDECalendar::OneWeekBeforeWithDay) {
                m_rmindCombox->setCurrentIndex(4);
            }
        } else {
            m_rmindCombox->setCurrentIndex(0);
        }
    } else {
        if (m_ScheduleDataInfo.getRemindData().getRemindNum() > -1) {
            if (m_ScheduleDataInfo.getRemindData().getRemindNum() == DDECalendar::AtTimeOfEvent) {
                m_rmindCombox->setCurrentIndex(1);
            } else if (m_ScheduleDataInfo.getRemindData().getRemindNum() == DDECalendar::FifteenMinutesBefore) {
                m_rmindCombox->setCurrentIndex(2);
            } else if (m_ScheduleDataInfo.getRemindData().getRemindNum() == DDECalendar::ThirtyMinutesBefore) {
                m_rmindCombox->setCurrentIndex(3);
            } else if (m_ScheduleDataInfo.getRemindData().getRemindNum() == DDECalendar::OneHourBefore) {
                m_rmindCombox->setCurrentIndex(4);
            } else if (m_ScheduleDataInfo.getRemindData().getRemindNum() ==
                       DDECalendar::OneDayBeforeWithMinutes) {
                m_rmindCombox->setCurrentIndex(5);
            } else if (m_ScheduleDataInfo.getRemindData().getRemindNum() ==
                       DDECalendar::TwoDayBeforeWithMinutes) {
                m_rmindCombox->setCurrentIndex(6);
            } else if (m_ScheduleDataInfo.getRemindData().getRemindNum() ==
                       DDECalendar::OneWeekBeforeWithMinutes) {
                m_rmindCombox->setCurrentIndex(7);
            }
        } else {
            m_rmindCombox->setCurrentIndex(0);
        }
    }
    slotbRpeatactivated(m_ScheduleDataInfo.getRepetitionRule().getRuleId());
    RepetitionRule::RRuleID ruleID = m_ScheduleDataInfo.getRepetitionRule().getRuleId();
    if (m_ScheduleDataInfo.getIsLunar()) {
        switch (ruleID) {
        case RepetitionRule::RRule_EVEYEAR:
            m_beginrepeatCombox->setCurrentIndex(2);
            break;
        case RepetitionRule::RRule_EVEMONTH:
            m_beginrepeatCombox->setCurrentIndex(1);
            break;
        default:
            m_beginrepeatCombox->setCurrentIndex(0);
            break;
        }
    } else {
        m_beginrepeatCombox->setCurrentIndex(ruleID);
    }

    if (m_ScheduleDataInfo.getRepetitionRule().getRuleId() != 0) {
        if (m_ScheduleDataInfo.getRepetitionRule().getRuleType() == 0) {
            m_endrepeatCombox->setCurrentIndex(0);
        } else if (m_ScheduleDataInfo.getRepetitionRule().getRuleType() == 1) {
            m_endrepeatCombox->setCurrentIndex(1);
            m_endrepeattimes->setText(QString::number(m_ScheduleDataInfo.getRepetitionRule().getEndCount()));
        } else if (m_ScheduleDataInfo.getRepetitionRule().getRuleType() == 2) {
            m_endrepeatCombox->setCurrentIndex(2);
            m_endRepeatDate->setDate(m_ScheduleDataInfo.getRepetitionRule().getEndDate().date());
        }
        m_endrepeatWidget->show();
        sloteRpeatactivated(m_ScheduleDataInfo.getRepetitionRule().getRuleType());
    } else {
        m_endrepeatWidget->hide();
    }
}

void CScheduleDlg::setTheMe(const int type)
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
    pa.setColor(DPalette::Text, titleColor);
    m_textEdit->setPalette(pa);
}

/**
 * @brief CScheduleDlg::setTabFouseOrder
 *      设置tab顺序
 */
void CScheduleDlg::setTabFouseOrder()
{
    setTabOrder(m_typeComBox, m_textEdit);
    setTabOrder(m_textEdit, m_allDayCheckbox);
    setTabOrder(m_allDayCheckbox, m_solarRadioBtn);
    setTabOrder(m_solarRadioBtn, m_lunarRadioBtn);
    setTabOrder(m_lunarRadioBtn, m_beginDateEdit);
    setTabOrder(m_beginDateEdit, m_beginTimeEdit);
//    setTabOrder(m_beginTimeEdit, m_endDateEdit);
    setTabOrder(m_endDateEdit, m_endTimeEdit);
//    setTabOrder(m_endTimeEdit, m_rmindCombox);
    setTabOrder(m_rmindCombox, m_beginrepeatCombox);
    setTabOrder(m_beginrepeatCombox, m_endrepeatCombox);
    //结束于次数，设置tab顺序
    if (m_ScheduleDataInfo.getRepetitionRule().getRuleType() ==
            RepetitionRule::RRuleEndType::RRuleType_FREQ)
        setTabOrder(m_endrepeatCombox, m_endrepeattimes);
    //结束于日期，设置tab顺序
    if (m_ScheduleDataInfo.getRepetitionRule().getRuleType() ==
            RepetitionRule::RRuleEndType::RRuleType_DATE)
        setTabOrder(m_endrepeatCombox, m_endRepeatDate);
}

void CScheduleDlg::updateIsOneMoreDay(const QDateTime &begin, const QDateTime &end)
{
    // 一天毫秒数
    static qint64 oneDayMses = 24 * 60 * 60 * 1000;
    m_isMoreThenOneDay = begin.msecsTo(end) >= oneDayMses;
}

void CScheduleDlg::updateRepeatCombox(bool isLunar)
{
    if (nullptr == m_beginrepeatCombox) {
        return;
    }
    m_beginrepeatCombox->clear();
    if (isLunar) {
        m_beginrepeatCombox->addItem(tr("Never"));
        m_beginrepeatCombox->addItem(tr("Monthly"));
        m_beginrepeatCombox->addItem(tr("Yearly"));
    } else {
        m_beginrepeatCombox->addItem(tr("Never"));
        m_beginrepeatCombox->addItem(tr("Daily"));
        m_beginrepeatCombox->addItem(tr("Weekdays"));
        m_beginrepeatCombox->addItem(tr("Weekly"));
        m_beginrepeatCombox->addItem(tr("Monthly"));
        m_beginrepeatCombox->addItem(tr("Yearly"));
    }
    //重置重复规则
    m_beginrepeatCombox->setCurrentIndex(0);    //默认选择第一个
    slotbRpeatactivated(0);     //更新“结束重复”状态
}

/**
 * @brief CScheduleDlg::isShowLunar
 * 系统语言检查，根据语言类型更改控件状态和显示状态，规则如下
 * 中文环境下所有功能正常显示
 */
bool CScheduleDlg::isShowLunar()
{
    return QLocale::system().name().startsWith("zh_");
}

/**
 * @brief CScheduleDlg::setShowState
 * @param jobIsLunar
 * 非中文环境下若新建日程则不农历按钮置灰不可选
 * 非中文环境下若编辑农历日程则仅公历按钮可选，其他的都置灰,当选择公历后,控件都可使用
 */
void CScheduleDlg::setShowState(bool jobIsLunar)
{
    //如果不显示农历
    if (isShowLunar()) {
        m_lunarRadioBtn->setEnabled(true);
        m_beginDateEdit->setLunarCalendarStatus(jobIsLunar);
        m_endDateEdit->setLunarCalendarStatus(jobIsLunar);
        m_endRepeatDate->setLunarCalendarStatus(jobIsLunar);
    } else {
        //在不显示农历环境下,取消农历显示
        m_beginDateEdit->setLunarCalendarStatus(false);
        m_endDateEdit->setLunarCalendarStatus(false);
        m_endRepeatDate->setLunarCalendarStatus(false);
        m_lunarRadioBtn->setEnabled(false);
        //在不显示农历情况下，需要根据编辑日程是否为农历日程来判断显示状态
        setWidgetEnabled(!jobIsLunar);
        //只有在不显示农历情况下才会根据是否显示农历来设置按钮状态
        //在不显示农历情况下是无法切换到农历的，所以只有在切换公历和编辑日程时才会设置
        getButton(1)->setEnabled(!jobIsLunar);
    }

    if (jobIsLunar) {
        //为农历日程
        m_lunarRadioBtn->setChecked(true);
    } else {
        //公历日程
        m_solarRadioBtn->setChecked(true);
    }
}

void CScheduleDlg::setWidgetEnabled(bool isEnabled)
{
    m_typeComBox->setEnabled(isEnabled);
    m_textEdit->setEnabled(isEnabled);
    m_allDayCheckbox->setEnabled(isEnabled);
    m_beginDateEdit->setEnabled(isEnabled);
    m_beginTimeEdit->setEnabled(isEnabled);
    m_endDateEdit->setEnabled(isEnabled);
    m_endTimeEdit->setEnabled(isEnabled);
    m_rmindCombox->setEnabled(isEnabled);
    m_beginrepeatCombox->setEnabled(isEnabled);
    m_endrepeatCombox->setEnabled(isEnabled);
    m_endrepeattimes->setEnabled(isEnabled);
    m_endRepeatDate->setEnabled(isEnabled);
}

void CScheduleDlg::initColor()
{
    //将用户上一次选择的自定义颜色添加进去
    QString colorName = CConfigSettings::getInstance()->value("LastUserColor", "").toString();
    if (!colorName.isEmpty()) {
        m_colorSeletorWideget->setUserColor(JobTypeColorInfo(0, colorName, 7));
    }

    //选中上一次选中的颜色
    int colorId = CConfigSettings::getInstance()->value("LastSysColorTypeNo", -1).toInt();
    if (colorId > 0) {
        m_colorSeletorWideget->setSelectedColorById(colorId);
    }
}

void CScheduleDlg::resize()
{
    int h = 0;
    if (m_endrepeatWidget->isVisible()) {
        h += 36 + 10;
    }

    if (m_colorSeletorWideget->isVisible()) {
        h += 18 + 5;
    }
    //524: 默认界面高度, h: 新增控件高度
    setFixedSize(dialog_width, 524 + h);
}

void CScheduleDlg::setOkBtnEnabled()
{
    QAbstractButton *m_OkBt = getButton(1);

    //根据类型输入框的内容判断保存按钮是否有效
    if (m_OkBt != nullptr && m_typeComBox->lineEdit() != nullptr) {
        const QString &typeStr = m_typeComBox->lineEdit()->text();
        if (typeStr.isEmpty() || typeStr.trimmed().isEmpty() || JobTypeInfoManager::instance()->isJobTypeNameUsed(typeStr)) {
            m_OkBt->setEnabled(false);
            //若内容无效直接退出，不判断结束次数是否为空
            return;
        } else {
            m_OkBt->setEnabled(true);
        }
    }
    //如果结束与次数为空，则保存按钮置灰
    if (m_beginrepeatCombox->currentIndex() > 0 && m_endrepeatCombox->currentIndex() == 1) {
        m_OkBt->setEnabled(!m_endrepeattimes->text().isEmpty());
    } else {
        //日期  //永不
        m_OkBt->setEnabled(true);
    }
}
