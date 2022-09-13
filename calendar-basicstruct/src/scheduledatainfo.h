// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULEJSONDATA_H
#define SCHEDULEJSONDATA_H

#include "repetitionrule.h"
#include "reminddata.h"
#include "utils.h"

#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QMap>
#include <QJsonObject>


class QDebug;
/**
 * @brief The ScheduleJsonData class
 * 日程数据结构与Json对应
 */
class ScheduleDataInfo
{
public:
    explicit ScheduleDataInfo();
    //获取日程ID
    int getID() const;
    //设置日程ID
    void setID(int scheduleID);
    //获取开始时间
    QDateTime &getBeginDateTime();
    QDateTime getBeginDateTime() const;
    //设置开始时间
    void setBeginDateTime(const QDateTime &beginDateTime);
    //获取结束时间
    QDateTime &getEndDateTime();
    QDateTime getEndDateTime() const;
    //设置结束时间
    void setEndDateTime(const QDateTime &endDateTime);
    //获取日程忽略时间集
    QVector<QDateTime> &getIgnoreTime();
    QVector<QDateTime> getIgnoreTime() const;
    //获取日程忽略时间集
    void setIgnoreTime(const QVector<QDateTime> &ignoreVectorTime);
    //获取日程标题
    QString getTitleName() const;
    //设置日程标题
    void setTitleName(const QString &titleName);
    //获取日程描述信息
    QString getDescription() const;
    //设置日程描述信息
    void setDescription(const QString &description);
    //获取日程是否为全天
    bool getAllDay() const;
    //设置日程是否为全天
    void setAllDay(const bool isAllDay);
    //获取日程类型
    int getType() const;
    //设置日程类型
    void setType(int scheduleType);
    //获取日程重复id类型
    int getRecurID() const;
    //设置日程重复id类型
    void setRecurID(int recurID);
    //设置日程提醒规则
    void setRemindData(const RemindData &remindData);
    //获取提醒规则
    RemindData getRemindData() const;
    //设置重复规则
    void setRepetitionRule(const RepetitionRule &rule);
    //获取重复规则
    RepetitionRule &getRepetitionRule();
    //获取重复规则
    RepetitionRule getRepetitionRule() const;
    //该日程是否有效
    bool isValid() const;
    //设置是否为拖拽移动日程
    void setIsMoveInfo(const bool &isMoveInfo);
    //获取是否为拖拽移动日程
    bool getIsMoveInfo()const;
    //日程数据转换为json形式的字符串
    static QString ScheduleToJsonStr(const ScheduleDataInfo &scheduleJsonData);
    //Json对象转换到日程数据
    static ScheduleDataInfo JsonToSchedule(const QJsonObject &scheduleJsonObject);
    //将json转换为日程
    static ScheduleDataInfo JsonStrToSchedule(const QString &jsonStr);
    //string类型的datetime数据转换为QDateTime
    static QDateTime StringDateToDateTime(const QString &dateStr);
    //QDateTime转换为string类型的datetime数据
    static QString DateTimeToStringDate(const QDateTime &dateTime);
    //将JsonArray数据转换为QMap存储Schedule数据
    static QMap<QDate, QVector<ScheduleDataInfo> > JsonArrayDataToScheduleMapData(const QJsonArray &jsonArrData);
    static QMap<QDate, QVector<ScheduleDataInfo> > StrJsonToRangeInfo(const QString &jsonStr);
    //判断日程是否相等
    bool operator ==(const ScheduleDataInfo &info) const;
    //判断日程大小
    bool operator <(const ScheduleDataInfo &info) const;
    /**
     * @brief getIsLunar        返回是否为农历日程
     * @return
     */
    bool getIsLunar() const;

    /**
     * @brief setIsLunar        设置是否为农历日程
     * @param isLunar
     */
    void setIsLunar(bool isLunar);

private:
    //获取日程提醒规则
    QString getScheduleRemind() const;
    //设置日程提醒规则
    void setScheduleRemind(const QString &scheduleRemind);
    //获取日程重复规则
    QString getScheduleRRule() const;
    //设置日程重复规则
    void setScheduleRRule(const QString &scheduleRRule);
public:
    static void registerMetaType();
    //日程信息调试打印
    friend QDebug operator<<(QDebug debug, const ScheduleDataInfo &scheduleJsonData);
private:
    /**
     * @brief m_ScheduleID  日程ID
     */
    int         m_ScheduleID;
    /**
     * @brief m_ScheduleBeginDateTime   日常开始时间
     */
    QDateTime       m_ScheduleBeginDateTime;
    /**
     * @brief m_ScheduleEndDateTime 日程结束时间
     */
    QDateTime       m_ScheduleEndDateTime;
    /**
     * @brief m_Ignore_VectorTime   重复日程忽略时间集
     */
    QVector<QDateTime>      m_IgnoreVectorTime;
    /**
     * @brief m_ScheduleTitleName   日程标题
     */
    QString m_ScheduleTitleName;
    /**
     * @brief m_ScheduleDescription 日程描述
     */
    QString m_ScheduleDescription;
    /**
     *@brief m_ScheduleIsAllDay     是否为全天日程
     * true  表示全天
     * false    表示非全天
     */
    bool        m_ScheduleIsAllDay;  //1全天
    /**
     * @brief m_ScheduleType    日程类型
     * 1 工作
     * 2 生活
     * 3 其他
     * 4 节日信息
     */
    int m_ScheduleType; //1工作 2 生活 3其他
    /**
     * @brief m_ScheduleRecurID     日程重复id类型
     * 0 原始数据
     * >0   克隆数据
     */
    int m_ScheduleRecurID;
    /**
     * @brief m_ScheduleRemind      提醒规则
     */
    RemindData      m_ScheduleRemind;
    /**
     * 日程重复规则
     * 具体规则可参考资料          https://www.rfc-editor.org/rfc/rfc5545.html#page-38
     *                          3.3.10.  Recurrence Rule
     * @brief m_Rule         重复规则
     */
    RepetitionRule  m_ScheduleRRule;

    bool m_isLunar = false; //是否为农历日程
    //客户端使用判断是否为拖拽移动日程
    bool            m_moveInfo{false};
};
Q_DECLARE_METATYPE(ScheduleDataInfo);

class JobTypeColorInfo
{
    /*
     *功能：
     * 1.保存日程颜色信息，包括TypeNo、ColorHex、Authority
     * 2.提供查、增、删、改接口
    */
public:
    explicit JobTypeColorInfo(int typeNo = 0, QString colorHex = "", int authority = 0);

    void setTypeNo(int typeNo)
    {
        iTypeNo = typeNo;
        return;
    }
    int getTypeNo() const
    {
        return iTypeNo;
    }
    void setColorHex(QString colorHex)
    {
        strColorHex = colorHex;
        return;
    }
    QString getColorHex() const
    {
        return strColorHex;
    }
    void setAuthority(int authority)
    {
        iAuthority = authority;
        return;
    }
    int getAuthority() const
    {
        return iAuthority;//系统默认颜色设置权限为1，用户自定义为7.1:2:4分别对应——展示:改:删除
    }

    //颜色是否为系统默认颜色
    bool isSysColorInfo() const
    {
        return (iTypeNo > 0 && iTypeNo < 10);
    }

    JobTypeColorInfo &operator=(const JobTypeColorInfo *info)
    {
        iTypeNo = info->getTypeNo();
        strColorHex = info->getColorHex();
        iAuthority = info->getAuthority();

        return *this;
    }
    bool operator==(const JobTypeColorInfo &info)
    {
        return iTypeNo == info.iTypeNo && strColorHex == info.strColorHex
               && iAuthority == info.iAuthority;
    }
    bool operator!=(const JobTypeColorInfo &info)
    {
        return !(*this == info);
    }

private:
    int iTypeNo;
    QString strColorHex;
    int iAuthority;
};

class JobTypeInfo
{
    /*功能：
     * 1.保存日程类型信息，包括JobTypeNo、JobTypeName、ColorTypeNo、ColorHex、Authority
     * 2.提供查、增、删、改接口
    */
public:
    JobTypeInfo(int typeNo = 0, QString typeName = "", int colorTypeNo = 0, QString colorHex = "", int authority = 0);
    JobTypeInfo(int typeNo, QString typeName, const JobTypeColorInfo &colorInfo = JobTypeColorInfo());
    JobTypeInfo(const JobTypeInfo &);
    JobTypeInfo &operator=(const JobTypeInfo *info)
    {
        iJobTypeNo = info->getJobTypeNo();
        strJobTypeName = info->getJobTypeName();
        m_ColorInfo = info->m_ColorInfo;
        iAuthority = info->getAuthority();

        return *this;
    }

    bool operator==(const JobTypeInfo &info)
    {
        return this->iJobTypeNo == info.iJobTypeNo
               && this->strJobTypeName == info.strJobTypeName
               && this->m_ColorInfo == info.m_ColorInfo
               && this->iAuthority == info.iAuthority;
    }

    bool operator!=(const JobTypeInfo &info)
    {
        return !(*this == info);
    }

    void setJobTypeNo(int typeNo)
    {
        iJobTypeNo = typeNo;
        return;
    }
    int getJobTypeNo() const
    {
        return iJobTypeNo;
    }

    void setJobTypeName(QString typeName)
    {
        strJobTypeName = typeName;
        return;
    }
    QString getJobTypeName() const
    {
        return strJobTypeName;
    }

    //设置颜色编码，默认为0。新建日程类型时，如果是选择自定义颜色，请不设置，或设置为0。
    void setColorTypeNo(int typeNo)
    {
        m_ColorInfo.setTypeNo(typeNo);
    }
    int getColorTypeNo() const
    {
        return m_ColorInfo.getTypeNo();
    }

    void setColorHex(const QString &colorHex)
    {
        m_ColorInfo.setColorHex(colorHex);
    }
    QString getColorHex() const
    {
        return m_ColorInfo.getColorHex();
    }

    void setAuthority(int authority)
    {
        iAuthority = authority;
        return;
    }
    int getAuthority() const
    {
        return iAuthority;//系统默认日程类型设置权限为1，用户自定义为7.1:2:4分别对应——展示:改:删除
    }

    //将json转换为日程列表
    static bool jsonStrToJobTypeInfoList(const QString &strJson, QList<JobTypeInfo> &lstJobType);

    //将json转换为一条日程记录
    static bool jsonStrToJobTypeInfo(const QString &strJson, JobTypeInfo &jobType);

    //将一条日程记录转换为json
    static bool jobTypeInfoToJsonStr(const JobTypeInfo &jobType, QString &strJson);

    //日程列表转Json串
    static bool jobTypeInfoListToJosnString(const QList<JobTypeInfo> &lstJobType, QString &strJson);

    //将json转换为颜色列表
    static bool jsonStrToColorTypeInfoList(const QString &strJson, QList<JobTypeColorInfo> &lstJobType);

    //将json转换为一条颜色记录
    static bool jsonStrToColorTypeInfo(const QString &strJson, JobTypeColorInfo &colorType);

    //将一条颜色记录转换为json
    static bool colorTypeInfoToJsonStr(const JobTypeColorInfo &colorType, QString &strJson);
    /**
     * @brief colorTypeToJosnString  颜色列表转Json串
     * param  lstColorType           JobType日程类型信息列表
     * param  strJson              json格式的日程类型信息
     * return bool                 返回操作结果
     */
    static bool colorTypeInfoListToJosnString(const QList<JobTypeColorInfo> &lstColorType, QString &strJson);

    JobTypeColorInfo getColorInfo() const;
    JobTypeColorInfo &getColorInfo();
    void setColorInfo(const JobTypeColorInfo &ColorInfo);

private:
    int iJobTypeNo;         //日程类型编号
    QString strJobTypeName; //日程类型名称
    JobTypeColorInfo m_ColorInfo; //日程颜色信息
    int iAuthority;         //权限
};

#endif // SCHEDULEJSONDATA_H
