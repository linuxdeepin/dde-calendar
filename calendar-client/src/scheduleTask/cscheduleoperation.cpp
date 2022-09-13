// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cscheduleoperation.h"

#include "cscheduledbus.h"
#include "schedulectrldlg.h"
#include "configsettings.h"
#include "dcalendarddialog.h"
#include "cdynamicicon.h"

CScheduleOperation::CScheduleOperation(QWidget *parent)
    : QObject(parent)
    , m_DBusManager(CScheduleDBus::getInstance())
    , m_widget(parent)
{

}

/**
 * @brief CScheduleOperation::createSchedule        创建日程
 * @param scheduleInfo
 */
bool CScheduleOperation::createSchedule(const ScheduleDataInfo &scheduleInfo)
{
    //如果为农历且重复类型为每年
    if (scheduleInfo.getIsLunar() && RepetitionRule::RRule_EVEYEAR == scheduleInfo.getRepetitionRule().getRuleId()) {
        lunarMessageDialogShow(scheduleInfo);
    }
    return m_DBusManager->CreateJob(scheduleInfo);
}

/**
 * @brief CScheduleOperation::changeSchedule        修改日程
 * @param newInfo
 * @param oldInfo
 */
bool CScheduleOperation::changeSchedule(const ScheduleDataInfo &newInfo, const ScheduleDataInfo &oldInfo)
{
    bool _result {false};
    if (newInfo.getRepetitionRule().getRuleId() == 0 && newInfo.getRepetitionRule().getRuleId() == oldInfo.getRepetitionRule().getRuleId()) {
        //如果为普通日程且没有修改重复类型则更新日程
        _result = m_DBusManager->UpdateJob(newInfo);
    } else {
        //如果切换了全天状态则提醒是否修改全部
        if (newInfo.getAllDay() != oldInfo.getAllDay()) {
            CScheduleCtrlDlg msgBox(m_widget);
            msgBox.setText(
                tr("All occurrences of a repeating event must have the same all-day status."));
            msgBox.setInformativeText(tr("Do you want to change all occurrences?"));
            msgBox.addPushButton(tr("Cancel", "button"), true);
            msgBox.addWaringButton(tr("Change All"), true);
            msgBox.exec();

            if (msgBox.clickButton() == 0) {
                _result = false;
            } else if (msgBox.clickButton() == 1) {
                //更新日程
                showLunarMessageDialog(newInfo, oldInfo);
                _result =  m_DBusManager->UpdateJob(newInfo);
            }
        } else if (oldInfo.getRepetitionRule().getRuleId() != newInfo.getRepetitionRule().getRuleId()) {
            //修改重复规则
            CScheduleCtrlDlg msgBox(m_widget);
            msgBox.setText(tr("You are changing the repeating rule of this event."));
            msgBox.setInformativeText(tr("Do you want to change all occurrences?"));
            msgBox.addPushButton(tr("Cancel", "button"), true);
            msgBox.addWaringButton(tr("Change All"), true);
            msgBox.exec();
            if (msgBox.clickButton() == 0) {
                _result = false;
            } else if (msgBox.clickButton() == 1) {
                //更新日程
                showLunarMessageDialog(newInfo, oldInfo);
                _result = m_DBusManager->UpdateJob(newInfo);
            }
        } else {
            _result = changeRecurInfo(newInfo, oldInfo);
        }
    }
    return  _result;
}

/**
 * @brief CScheduleOperation::deleteSchedule        删除日程
 * @param scheduleInfo
 */
bool CScheduleOperation::deleteSchedule(const ScheduleDataInfo &scheduleInfo)
{
    bool _restuleBool {false};
    //如果为普通日程
    if (scheduleInfo.getRepetitionRule().getRuleId() == 0) {
        CScheduleCtrlDlg msgBox(m_widget);
        msgBox.setText(tr("You are deleting an event."));
        msgBox.setInformativeText(tr("Are you sure you want to delete this event?"));
        msgBox.addPushButton(tr("Cancel", "button"), true);
        msgBox.addWaringButton(tr("Delete", "button"), true);
        msgBox.exec();
        if (msgBox.clickButton() == 0) {
            return false;
        } else if (msgBox.clickButton() == 1) {
            m_DBusManager->DeleteJob(scheduleInfo.getID());
            _restuleBool = true;
        }
    } else {
        //如果为重复日程的第一个
        if (scheduleInfo.getRecurID() == 0) {
            CScheduleCtrlDlg msgBox(m_widget);
            msgBox.setText(tr("You are deleting an event."));
            msgBox.setInformativeText(tr("Do you want to delete all occurrences of this event, or only the selected occurrence?"));
            msgBox.addPushButton(tr("Cancel", "button"));
            msgBox.addPushButton(tr("Delete All"));
            msgBox.addWaringButton(tr("Delete Only This Event"));
            msgBox.exec();
            if (msgBox.clickButton() == 0) {
                return false;
            } else if (msgBox.clickButton() == 1) {
                //删除所有日程
                m_DBusManager->DeleteJob(scheduleInfo.getID());
                _restuleBool = true;
            } else if (msgBox.clickButton() == 2) {
                //仅删除此日程
                ScheduleDataInfo newschedule;
                m_DBusManager->GetJob(scheduleInfo.getID(), newschedule);
                newschedule.getIgnoreTime().append(scheduleInfo.getBeginDateTime());
                m_DBusManager->UpdateJob(newschedule);
                _restuleBool = true;
            }
        } else {
            CScheduleCtrlDlg msgBox(m_widget);
            msgBox.setText(tr("You are deleting an event."));
            msgBox.setInformativeText(tr("Do you want to delete this and all future occurrences of this event, or only the selected occurrence?"));
            msgBox.addPushButton(tr("Cancel", "button"));
            msgBox.addPushButton(tr("Delete All Future Events"));
            msgBox.addWaringButton(tr("Delete Only This Event"));
            msgBox.exec();

            if (msgBox.clickButton() == 0) {
                return false;
            } else if (msgBox.clickButton() == 1) {
                //删除选中日程及之后的日程
                ScheduleDataInfo newschedule;
                //获取原始日程信息
                m_DBusManager->GetJob(scheduleInfo.getID(), newschedule);
                //修改重复规则
                changeRepetitionRule(newschedule, scheduleInfo);
                //如果修改后的日程为普通日程且忽略列表内包含日程开始时间则删除该日程
                if (newschedule.getRepetitionRule().getRuleId() == RepetitionRule::RRule_NONE && newschedule.getIgnoreTime().contains(newschedule.getBeginDateTime())) {
                    //删除日程
                    m_DBusManager->DeleteJob(newschedule.getID());
                } else {
                    //更新日程
                    m_DBusManager->UpdateJob(newschedule);
                }

                _restuleBool = true;
            } else if (msgBox.clickButton() == 2) {
                ScheduleDataInfo newschedule;
                m_DBusManager->GetJob(scheduleInfo.getID(), newschedule);
                newschedule.getIgnoreTime().append(scheduleInfo.getBeginDateTime());
                m_DBusManager->UpdateJob(newschedule);
                _restuleBool = true;
            }
        }
    }
    return  _restuleBool;
}

QString CScheduleOperation::queryScheduleStr(const QString &key, const QDateTime &startTime, const QDateTime &endTime)
{
    QString _resultStr;
    m_DBusManager->QueryJobs(key, startTime, endTime, _resultStr);
    return _resultStr;
}

bool CScheduleOperation::queryScheduleInfo(const QString &key, const QDateTime &startTime, const QDateTime &endTime, QMap<QDate, QVector<ScheduleDataInfo> > &info)
{
    return m_DBusManager->QueryJobs(key, startTime, endTime, info);
}

/**
 * @brief CScheduleOperation::deleteOnlyInfo        删除日程 仅删除此日程 不弹框提醒
 * @param scheduleInfo
 */
void CScheduleOperation::deleteOnlyInfo(const ScheduleDataInfo &scheduleInfo)
{
    //如果为纪念日或节日则不处理
    if (scheduleInfo.getType() == 4)
        return;
    //如果为普通日程则删除
    if (scheduleInfo.getRepetitionRule().getRuleId() == RepetitionRule::RRule_NONE) {
        m_DBusManager->DeleteJob(scheduleInfo.getID());
    } else {
        //仅删除此日程
        ScheduleDataInfo newschedule;
        m_DBusManager->GetJob(scheduleInfo.getID(), newschedule);
        newschedule.getIgnoreTime().append(scheduleInfo.getBeginDateTime());
        m_DBusManager->UpdateJob(newschedule);
    }
}

/**
 * @brief CScheduleOperation::ChangeRecurInfo       修改重复日程
 * @param newinfo
 * @param oldinfo
 */
bool CScheduleOperation::changeRecurInfo(const ScheduleDataInfo &newinfo, const ScheduleDataInfo &oldinfo)
{
    bool _result{false};
    //如果为重复类型第一个
    if (newinfo.getRecurID() == 0) {
        CScheduleCtrlDlg msgBox(m_widget);
        msgBox.setText(tr("You are changing a repeating event."));
        msgBox.setInformativeText(
            tr("Do you want to change only this occurrence of the event, or all "
               "occurrences?"));
        msgBox.addPushButton(tr("Cancel", "button"));
        msgBox.addPushButton(tr("All"));
        msgBox.addsuggestButton(tr("Only This Event"));
        msgBox.exec();

        if (msgBox.clickButton() == 0) {
            _result = false;
        } else if (msgBox.clickButton() == 1) {
            //修改所有日程
            ScheduleDataInfo _scheduleDataInfo = newinfo;
            RepetitionRule _rule = _scheduleDataInfo.getRepetitionRule();
            //如果此重复日程只有它一个则将修改为普通日程
            if ((_rule.getRuleType() == 1 && _rule.getEndCount() < 1) ||
                    (_rule.getRuleType() == 2 && _scheduleDataInfo.getBeginDateTime().daysTo(_rule.getEndDate()) < 0)) {
                _rule.setRuleId(RepetitionRule::RRuleID::RRule_NONE);
                _rule.setRuleType(RepetitionRule::RRuleEndType::RRuleType_NEVER);
            }
            _scheduleDataInfo.setRepetitionRule(_rule);
            //TODO 清空忽略日程
            _scheduleDataInfo.getIgnoreTime().clear();
            //更新日程
            showLunarMessageDialog(_scheduleDataInfo, oldinfo);
            _result = m_DBusManager->UpdateJob(_scheduleDataInfo);
        } else if (msgBox.clickButton() == 2) {
            //仅修改此日程
            _result = changeOnlyInfo(newinfo, oldinfo);
        }
    } else {
        CScheduleCtrlDlg msgBox(m_widget);
        msgBox.setText(tr("You are changing a repeating event."));
        msgBox.setInformativeText(
            tr("Do you want to change only this occurrence of the event, or this and "
               "all future occurrences?"));
        msgBox.addPushButton(tr("Cancel", "button"));
        msgBox.addPushButton(tr("All Future Events"));
        msgBox.addsuggestButton(tr("Only This Event"));
        msgBox.exec();

        if (msgBox.clickButton() == 0) {
            _result = false;
        } else if (msgBox.clickButton() == 1) {
            // 根据id获取日程并修改
            ScheduleDataInfo updatescheduleData;
            // 获取原始数据
            m_DBusManager->GetJob(oldinfo.getID(), updatescheduleData);
            //修改重复规则
            changeRepetitionRule(updatescheduleData, newinfo);
            //如果修改后的日程为普通日程且忽略列表内包含日程开始时间则删除该日程
            if (updatescheduleData.getRepetitionRule().getRuleId() == RepetitionRule::RRule_NONE && updatescheduleData.getIgnoreTime().contains(updatescheduleData.getBeginDateTime())) {
                //删除日程
                m_DBusManager->DeleteJob(updatescheduleData.getID());
            } else {
                //更新日程
                m_DBusManager->UpdateJob(updatescheduleData);
            }
            //创建日程
            ScheduleDataInfo newschedule = newinfo;
            //获取重复规则
            RepetitionRule _rule = newschedule.getRepetitionRule();
            if (_rule.getRuleType() == 1) {
                //更新重复规则
                _rule.setEndCount(qAbs(_rule.getEndCount() - newschedule.getRecurID()));
                if (_rule.getEndCount() < 1) {
                    _rule.setRuleId(RepetitionRule::RRuleID::RRule_NONE);
                    _rule.setRuleType(RepetitionRule::RRuleEndType::RRuleType_NEVER);
                }
            }
            newschedule.setRecurID(0);
            newschedule.setID(0);
            newschedule.setRepetitionRule(_rule);
            //创建新日程
            //如果为农历且重复类型为每年
            if (newschedule.getIsLunar() && RepetitionRule::RRule_EVEYEAR == newschedule.getRepetitionRule().getRuleId()) {
                lunarMessageDialogShow(newschedule);
            }
            _result = m_DBusManager->CreateJob(newschedule);
        } else if (msgBox.clickButton() == 2) {
            _result = changeOnlyInfo(newinfo, oldinfo);
        }
    }
    return _result;
}

/**
 * @brief CScheduleOperation::ChangeOnlyInfo        修改重复日程,仅修改此日程
 * @param newinfo
 * @param oldinfo
 */
bool CScheduleOperation::changeOnlyInfo(const ScheduleDataInfo &newinfo, const ScheduleDataInfo &oldinfo)
{
    ScheduleDataInfo newschedule = newinfo;
    newschedule.getRepetitionRule().clear();
    newschedule.setRecurID(0);
    newschedule.setID(0);
    newschedule.getIgnoreTime().clear();
    //创建日程
    m_DBusManager->CreateJob(newschedule);
    ScheduleDataInfo updatescheduleData;
    //获取原始信息
    m_DBusManager->GetJob(oldinfo.getID(), updatescheduleData);
    updatescheduleData.getIgnoreTime().append(oldinfo.getBeginDateTime());
    //更新原始信息
    return m_DBusManager->UpdateJob(updatescheduleData);
}

/**
 * @brief CScheduleOperation::changeRepetitionRule      修改日程重复规则
 * @param newinfo
 * @param oldinfo
 */
void CScheduleOperation::changeRepetitionRule(ScheduleDataInfo &newinfo, const ScheduleDataInfo &oldinfo)
{
    switch (newinfo.getRepetitionRule().getRuleType()) {
    case RepetitionRule::RRuleType_FREQ: {
        //如果为结束与次数则修改结束次数
        newinfo.getRepetitionRule().setEndCount(oldinfo.getRecurID() - 1);
        //结束次数为0表示不重复，设置为普通日程
        if (newinfo.getRepetitionRule().getEndCount() < 1) {
            newinfo.getRepetitionRule().setRuleId(RepetitionRule::RRuleID::RRule_NONE);
            newinfo.getRepetitionRule().setRuleType(RepetitionRule::RRuleEndType::RRuleType_NEVER);
        }
        break;
    }
    default: {
        //如果该日程结束类型为永不和结束于日期则修改结束日期
        newinfo.getRepetitionRule().setRuleType(RepetitionRule::RRuleType_DATE);
        //设置结束日期，默认为0点
        QDateTime endDate(QDate(oldinfo.getBeginDateTime().date().addDays(-1)), QTime());
        newinfo.getRepetitionRule().setEndDate(endDate);
        break;
    }
    }
}

/**
 * @brief CScheduleOperation::createJobType      创建日程类型
 * @param newinfo
 * @param oldinfo
 */
bool CScheduleOperation::createJobType(JobTypeInfo &jobTypeInfo)//新增时，颜色可能是：自定义/默认类型。以“自定义颜色编码默认为0”来区分.
{
    //创建日程
    QString strJson = "";

    int colorTypeNo = jobTypeInfo.getColorInfo().getTypeNo();

    //以“自定义颜色编码默认为0”来区分.
    if (0 == colorTypeNo) {
        colorTypeNo = JobTypeInfoManager::instance()->getNextColorTypeNo();
        jobTypeInfo.getColorInfo().setTypeNo(colorTypeNo);
        //保存新选择的颜色值
        CConfigSettings::getInstance()->setOption("LastUserColor", jobTypeInfo.getColorInfo().getColorHex());
    }
    //保存选择的颜色编号,只记录系统默认颜色的编号
    if (jobTypeInfo.getColorInfo().getTypeNo() < 10)
        CConfigSettings::getInstance()->setOption("LastSysColorTypeNo", jobTypeInfo.getColorInfo().getTypeNo());

    if (0 == jobTypeInfo.getJobTypeNo()) {
        jobTypeInfo.setJobTypeNo(JobTypeInfoManager::instance()->getNextTypeNo());
        jobTypeInfo.getColorInfo().setTypeNo(colorTypeNo);
    }
    jobTypeInfo.setAuthority(7);//自定义日程类型默认权限为7

    JobTypeInfo::jobTypeInfoToJsonStr(jobTypeInfo, strJson);
    return m_DBusManager->AddJobType(strJson);// no:10,hex:#123
}

/**
 * @brief CScheduleOperation::updateJobType      修改日程类型
 * @param oldJobTypeInfo
 * @param newJobTypeInfo
 * 只能更新名称和颜色
 * 颜色可能是：自定义-自定义、自定义-默认类型、默认类型-默认类型
 */
bool CScheduleOperation::updateJobType(JobTypeInfo &oldJobTypeInfo, JobTypeInfo &newJobTypeInfo)
{
    //如果oldJobTypeInfo中typeno为0，则是新增
    if (0 == oldJobTypeInfo.getJobTypeNo()) {
        return createJobType(newJobTypeInfo);
    }
    bool bRet = true;
    //如果修改的日程类型没有改变则不处理
    if (oldJobTypeInfo == newJobTypeInfo) {
        return bRet;
    }

    //更新日程类型
    newJobTypeInfo.setJobTypeNo(oldJobTypeInfo.getJobTypeNo());
    //以“自定义颜色编码默认为0”来区分.
    if (0 == newJobTypeInfo.getColorTypeNo()) {
        //配置新颜色编号
        if (oldJobTypeInfo.getColorTypeNo() > 9) {
            newJobTypeInfo.getColorInfo().setTypeNo(oldJobTypeInfo.getColorTypeNo());
        } else {
            newJobTypeInfo.getColorInfo().setTypeNo(JobTypeInfoManager::instance()->getNextColorTypeNo());
        }
        //保存新选择的颜色值
        CConfigSettings::getInstance()->setOption("LastUserColor", newJobTypeInfo.getColorInfo().getColorHex());
    }

    bRet = updateJobType(newJobTypeInfo);
    //如果更新成功，且是系统默认颜色，缓存编号，只记录系统默认颜色的编号
    if (bRet && newJobTypeInfo.getColorInfo().getTypeNo() < 10) {
        CConfigSettings::getInstance()->setOption("LastSysColorTypeNo", newJobTypeInfo.getColorInfo().getTypeNo());
    }
    return bRet;
}
/**
 * @brief CScheduleOperation::updateJobType      修改日程类型
 * @param jobTypeInfo
 * 只能更新名称和颜色编号
 */
bool CScheduleOperation::updateJobType(const JobTypeInfo &jobTypeInfo)
{
    //修改日程
    QString strJson = "";
    JobTypeInfo::jobTypeInfoToJsonStr(jobTypeInfo, strJson);
    return m_DBusManager->UpdateJobType(strJson);
}

void CScheduleOperation::lunarMessageDialogShow(const ScheduleDataInfo &newinfo)
{
    //如果该日程为闰月日程，因为对应的闰月需要间隔好多年，所以添加对应的提示信息
    CaHuangLiDayInfo huangLiInfo;
    CScheduleDBus::getInstance()->GetHuangLiDay(newinfo.getBeginDateTime().date(), huangLiInfo);
    if (huangLiInfo.mLunarMonthName.contains("闰")) {
        DCalendarDDialog prompt(m_widget);
        prompt.setIcon(QIcon(CDynamicIcon::getInstance()->getPixmap()));
        prompt.setDisplayPosition(DAbstractDialog::Center);
        prompt.setMessage(tr("You have selected a leap month, and will be reminded according to the rules of the lunar calendar."));
        prompt.addButton(tr("OK", "button"), true, DDialog::ButtonNormal);
        if (m_widget) {
            //获取父窗口的中心坐标
            const QPoint global = m_widget->mapToGlobal(m_widget->rect().center());
            //相对父窗口居中显示
            prompt.move(global.x() - prompt.width() / 2, global.y() - prompt.height() / 2);
        }
        prompt.exec();
    }
}

void CScheduleOperation::showLunarMessageDialog(const ScheduleDataInfo &newinfo, const ScheduleDataInfo &oldinfo)
{
    //在阴历每年重复情况下如果修改了开始时间或重复规则
    if (newinfo.getIsLunar() && RepetitionRule::RRule_EVEYEAR == newinfo.getRepetitionRule().getRuleId()) {
        if (oldinfo.getBeginDateTime().date() != newinfo.getBeginDateTime().date()
            || oldinfo.getRepetitionRule().getRuleId() != newinfo.getRepetitionRule().getRuleId()
            || oldinfo.getIsLunar() != newinfo.getIsLunar()) {
            //判断是否为闰月
            lunarMessageDialogShow(newinfo);
        }
    }
}

/**
 * @brief CScheduleOperation::getJobTypeList      获取日程类型列表
 * @param lstJobTypeInfo
 * @return 操作结果
 */
bool CScheduleOperation::getJobTypeList(QList<JobTypeInfo> &lstJobTypeInfo)
{
    QString strJson;
    if (!m_DBusManager->GetJobTypeList(strJson)) {
        return false;
    }
    JobTypeInfo::jsonStrToJobTypeInfoList(strJson, lstJobTypeInfo);
    return true;
}

/**
 * @brief CScheduleOperation::deleteJobType      删除日程类型
 * @param iJobTypeNo
 * @return 操作结果
 */
bool CScheduleOperation::deleteJobType(const int iJobTypeNo)
{
    //删除日程类型
    return m_DBusManager->DeleteJobType(iJobTypeNo);
}
/**
 * @brief CScheduleOperation::isJobTypeUsed      获取日程类型是否被使用
 * @param iJobTypeNo
 * @return 操作结果
 */
bool CScheduleOperation::isJobTypeUsed(const int iJobTypeNo)
{
    //获取日程类型是否被使用
    return m_DBusManager->isJobTypeUsed(iJobTypeNo);
}

/**
 * @brief CScheduleOperation::getColorTypeList      获取颜色类型列表
 * @param lstColorTypeInfo
 * @return 操作结果
 */
bool CScheduleOperation::getColorTypeList(QList<JobTypeColorInfo> &lstColorTypeInfo)
{
    QString strJson;
    if (!m_DBusManager->GetJobTypeColorList(strJson)) {
        return false;
    }
    return JobTypeInfo::jsonStrToColorTypeInfoList(strJson, lstColorTypeInfo);
}
