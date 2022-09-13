// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduledatainfo.h"
#include "utils.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimeZone>
#include <QDebug>

ScheduleDataInfo::ScheduleDataInfo()
    : m_ScheduleID(0)
    , m_ScheduleBeginDateTime()
    , m_ScheduleEndDateTime()
    , m_IgnoreVectorTime()
    , m_ScheduleTitleName("")
    , m_ScheduleDescription("")
    , m_ScheduleIsAllDay(true)
    , m_ScheduleType(1)
    , m_ScheduleRecurID(0)
    , m_ScheduleRRule()
{

}

/**
 * @brief ScheduleID    获取日程ID
 * @return      日程ID
 */
int ScheduleDataInfo::getID() const
{
    return m_ScheduleID;
}

/**
 * @brief setScheduleID             设置日程ID
 * @param scheduleID                日程id
 */
void ScheduleDataInfo::setID(int scheduleID)
{
    m_ScheduleID = scheduleID;
}

/**
 * @brief ScheduleBeginDateTime     获取日程开始时间
 * @return                          日程开始时间
 */
QDateTime &ScheduleDataInfo::getBeginDateTime()
{
    return m_ScheduleBeginDateTime;
}

QDateTime ScheduleDataInfo::getBeginDateTime() const
{
    return m_ScheduleBeginDateTime;
}

/**
 * @brief setScheduleBeginDateTime      设置日程开始时间
 * @param ScheduleBeginDateTime         日程开始时间
 */
void ScheduleDataInfo::setBeginDateTime(const QDateTime &beginDateTime)
{
    m_ScheduleBeginDateTime = beginDateTime;
}

/**
 * @brief ScheduleEndDateTime       获取日程结束时间
 * @return                          日程结束时间
 */
QDateTime &ScheduleDataInfo::getEndDateTime()
{
    return m_ScheduleEndDateTime;
}

QDateTime ScheduleDataInfo::getEndDateTime() const
{
    return m_ScheduleEndDateTime;
}

/**
 * @brief setScheduleEndDateTime    设置日程结束时间
 * @param ScheduleEndDateTime       日程结束时间
 */
void ScheduleDataInfo::setEndDateTime(const QDateTime &endDateTime)
{
    m_ScheduleEndDateTime = endDateTime;
}

QVector<QDateTime> &ScheduleDataInfo::getIgnoreTime()
{
    return m_IgnoreVectorTime;
}

/**
 * @brief IgnoreVectorTime          获取日程重复忽略时间集
 * @return                          日程重复忽略时间集
 */
QVector<QDateTime> ScheduleDataInfo::getIgnoreTime() const
{
    return m_IgnoreVectorTime;
}

/**
 * @brief setIgnoreVectorTime       设置日程重复忽略时间集
 * @param IgnoreVectorTime          日程重复忽略时间集
 */
void ScheduleDataInfo::setIgnoreTime(const QVector<QDateTime> &ignoreVectorTime)
{
    m_IgnoreVectorTime = ignoreVectorTime;
}

/**
 * @brief ScheduleTitleName         获取日程标题
 * @return                          日程标题
 */
QString ScheduleDataInfo::getTitleName() const
{
    return m_ScheduleTitleName;
}

/**
 * @brief setScheduleTitleName      设置日程标题
 * @param ScheduleTitleName         日程标题
 */
void ScheduleDataInfo::setTitleName(const QString &titleName)
{
    m_ScheduleTitleName = titleName;
}

/**
 * @brief ScheduleDescription       获取日程描述信息
 * @return                          日程描述信息
 */
QString ScheduleDataInfo::getDescription() const
{
    return m_ScheduleDescription;
}

/**
 * @brief setScheduleDescription    设置日程描述信息
 * @param ScheduleDescription       日程描述信息
 */
void ScheduleDataInfo::setDescription(const QString &description)
{
    m_ScheduleDescription = description;
}

/**
 * @brief ScheduleIsAllDay      获取日程是否为全天
 * @return      是否为全天
 */
bool ScheduleDataInfo::getAllDay() const
{
    return m_ScheduleIsAllDay;
}

/**
 * @brief setScheduleIsAllDay   设置日程是否为全天
 * @param ScheduleIsAllDay      是否为全天
 */
void ScheduleDataInfo::setAllDay(const bool isAllDay)
{
    m_ScheduleIsAllDay = isAllDay;
}

/**
 * @brief ScheduleType  获取日程类型
 * @return      日程类型
 */
int ScheduleDataInfo::getType() const
{
    return m_ScheduleType;
}

/**
 * @brief setScheduleType   设置日程类型
 * @param ScheduleType      日程类型
 */
void ScheduleDataInfo::setType(int scheduleType)
{
    m_ScheduleType = scheduleType;
}

/**
 * @brief ScheduleRecurID   获取日程重复id类型
 * @return                  重复id类型
 */
int ScheduleDataInfo::getRecurID() const
{
    return m_ScheduleRecurID;
}

/**
 * @brief setRecurID            设置日程重复id类型
 * @param ScheduleRecurID       重复id类型
 */
void ScheduleDataInfo::setRecurID(int recurID)
{
    m_ScheduleRecurID = recurID;
}

/**
 * @brief setScheduleRemindData     设置日程提醒规则
 * @param remindData                提醒规则
 */
void ScheduleDataInfo::setRemindData(const RemindData &remindData)
{
    m_ScheduleRemind = remindData;
}

/**
 * @brief getScheduleRemindData     获取提醒规则
 * @return
 */
RemindData ScheduleDataInfo::getRemindData() const
{
    return m_ScheduleRemind;
}

/**
 * @brief ScheduleRemind    设置日程提醒规则
 * @return                  提醒规则
 */
QString ScheduleDataInfo::getScheduleRemind() const
{
    return RemindData::RemindDataToRemindString(m_ScheduleRemind, this->getAllDay());
}

/**
 * @brief setScheduleRemind     设置日程提醒规则
 * @param ScheduleRemind        提醒规则
 */
void ScheduleDataInfo::setScheduleRemind(const QString &scheduleRemind)
{
    m_ScheduleRemind = RemindData::RemindStringToRemindData(scheduleRemind);
}

/**
 * @brief ScheduleRRule     获取日程重复规则
 * @return                  日程重复规则
 */
QString ScheduleDataInfo::getScheduleRRule() const
{
    return m_ScheduleRRule.RuleDataToRuleString(m_ScheduleRRule);;
}

/**
 * @brief setScheduleRRule  设置日程重复规则
 * @param ScheduleRRule 日程重复规则
 */
void ScheduleDataInfo::setScheduleRRule(const QString &scheduleRRule)
{
    m_ScheduleRRule = RepetitionRule::RuleStringToRuleData(scheduleRRule);
}

void ScheduleDataInfo::registerMetaType()
{
    qRegisterMetaType<ScheduleDataInfo>("ScheduleDataInfo");
}

bool ScheduleDataInfo::getIsLunar() const
{
    return m_isLunar;
}

void ScheduleDataInfo::setIsLunar(bool isLunar)
{
    m_isLunar = isLunar;
}

/**
 * @brief setRepetitionRule     设置重复规则
 * @param rule
 */
void ScheduleDataInfo::setRepetitionRule(const RepetitionRule &rule)
{
    m_ScheduleRRule = rule;
}

/**
 * @brief ScheduleDataInfo::getRepetitionRule       获取重复规则
 * @return
 */
RepetitionRule &ScheduleDataInfo::getRepetitionRule()
{
    return m_ScheduleRRule;
}

/**
 * @brief getRepetitionRule         获取重复规则
 * @return
 */
RepetitionRule ScheduleDataInfo::getRepetitionRule() const
{
    return m_ScheduleRRule;
}

/**
 * @brief ScheduleDataInfo::isValid 日程是否有效
 * @return
 */
bool ScheduleDataInfo::isValid() const
{
    if (!getBeginDateTime().isValid()) {
        return  false;
    }
    if (!getEndDateTime().isValid()) {
        return  false;
    }
    if (getType() < 1) {
        return  false;
    }
    return true;
}

/**
 * @brief ScheduleDataInfo::setIsMoveInfo       设置是否为拖拽移动日程
 * @param isMoveInfo
 */
void ScheduleDataInfo::setIsMoveInfo(const bool &isMoveInfo)
{
    m_moveInfo = isMoveInfo;
}

/**
 * @brief ScheduleDataInfo::getIsMoveInfo       获取是否为拖拽移动日程
 * @return
 */
bool ScheduleDataInfo::getIsMoveInfo() const
{
    return  m_moveInfo;
}

/**
 * @brief ScheduleToJson        日程数据转换为json形式的字符串
 * @param scheduleJsonData      日程数据
 * @return          json形式的字符串
 */
QString ScheduleDataInfo::ScheduleToJsonStr(const ScheduleDataInfo &scheduleJsonData)
{
    QJsonObject _scheduleJsonObject;
    _scheduleJsonObject.insert("ID", scheduleJsonData.getID());
    _scheduleJsonObject.insert("AllDay", scheduleJsonData.getAllDay());
    _scheduleJsonObject.insert("Remind", scheduleJsonData.getScheduleRemind());
    _scheduleJsonObject.insert("RRule", scheduleJsonData.getScheduleRRule());
    _scheduleJsonObject.insert("Title", scheduleJsonData.getTitleName());
    _scheduleJsonObject.insert("Description", scheduleJsonData.getDescription());
    _scheduleJsonObject.insert("Type", scheduleJsonData.getType());
    _scheduleJsonObject.insert("Start", DateTimeToStringDate(scheduleJsonData.getBeginDateTime()));
    _scheduleJsonObject.insert("End", DateTimeToStringDate(scheduleJsonData.getEndDateTime()));
    _scheduleJsonObject.insert("RecurID", scheduleJsonData.getRecurID());
    QJsonArray _ignoreJsonArray;
    for (int i = 0; i < scheduleJsonData.getIgnoreTime().count(); i++) {
        _ignoreJsonArray.append(DateTimeToStringDate(scheduleJsonData.getIgnoreTime().at(i)));
    }
    _scheduleJsonObject.insert("Ignore", _ignoreJsonArray);
    _scheduleJsonObject.insert("IsLunar", scheduleJsonData.getIsLunar());
    // 构建 JSON 文档
    QJsonDocument _scheduleJsonDocument;
    _scheduleJsonDocument.setObject(_scheduleJsonObject);
    QByteArray _scheduleByteArray = _scheduleJsonDocument.toJson(QJsonDocument::Compact);
    QString _resultJsonStr(_scheduleByteArray);
    return _resultJsonStr;
}

/**
 * @brief JsonToSchedule            Json对象转换到日程数据
 * @param ScheduleJsonObject        json对象
 * @return      日程数据
 */
ScheduleDataInfo ScheduleDataInfo::JsonToSchedule(const QJsonObject &scheduleJsonObject)
{
    ScheduleDataInfo _resultSchedule;
    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    //日程id
    if (scheduleJsonObject.contains("ID")) {
        _resultSchedule.setID(scheduleJsonObject.value("ID").toInt());
    }
    //日程是否为全天
    if (scheduleJsonObject.contains("AllDay")) {
        _resultSchedule.setAllDay(scheduleJsonObject.value("AllDay").toBool());
    }
    //日程提醒规则
    if (scheduleJsonObject.contains("Remind")) {
        _resultSchedule.setScheduleRemind(scheduleJsonObject.value("Remind").toString());
    }
    //日程标题
    if (scheduleJsonObject.contains("Title")) {
        _resultSchedule.setTitleName(scheduleJsonObject.value("Title").toString());
    }
    //日程描述
    if (scheduleJsonObject.contains("Description")) {
        _resultSchedule.setDescription(scheduleJsonObject.value("Description").toString());
    }
    //日程类型
    if (scheduleJsonObject.contains("Type")) {
        _resultSchedule.setType(scheduleJsonObject.value("Type").toInt());
    }
    //日程开始时间
    if (scheduleJsonObject.contains("Start")) {
        _resultSchedule.setBeginDateTime(StringDateToDateTime(scheduleJsonObject.value("Start").toString()));
    }
    //日程结束时间
    if (scheduleJsonObject.contains("End")) {
        _resultSchedule.setEndDateTime(StringDateToDateTime(scheduleJsonObject.value("End").toString()));
    }
    //日程重复ID
    if (scheduleJsonObject.contains("RecurID")) {
        _resultSchedule.setRecurID(scheduleJsonObject.value("RecurID").toInt());
    }
    //日程重复规则
    if (scheduleJsonObject.contains("RRule")) {
        _resultSchedule.setScheduleRRule(scheduleJsonObject.value("RRule").toString());
    }
    //重复日程忽略日期集
    if (scheduleJsonObject.contains("Ignore")) {
        QJsonArray subArray = scheduleJsonObject.value("Ignore").toArray();
        QVector<QDateTime> _ignoreDateVector;
        for (int i = 0; i < subArray.size(); i++) {
            QString subObj = subArray.at(i).toString();
            _ignoreDateVector.append(StringDateToDateTime(subObj));
        }
        _resultSchedule.setIgnoreTime(_ignoreDateVector);
    }
    //是否为农历日程
    if (scheduleJsonObject.contains("IsLunar")) {
        _resultSchedule.setIsLunar(scheduleJsonObject.value("IsLunar").toBool());
    }
    return _resultSchedule;
}

/**
 * @brief ScheduleDataInfo::JsonStrToSchedule   将json转换为日程
 * @param jsonStr                               json数据
 * @return
 */
ScheduleDataInfo ScheduleDataInfo::JsonStrToSchedule(const QString &jsonStr)
{
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        qWarning() << json_error.errorString();
    }
    QJsonObject ssubObj = jsonDoc.object();
    return ScheduleDataInfo::JsonToSchedule(ssubObj);
}

/**
 * @brief StringDateToDateTime      string类型的datetime数据转换为QDateTime
 * @param dateStr                   string类型的datetime数据
 * @return                          QDateTime
 */
QDateTime ScheduleDataInfo::StringDateToDateTime(const QString &dateStr)
{
    return QDateTime::fromString(dateStr, Qt::ISODate);
}

/**
 * @brief DateTimeToStringDate      QDateTime转换为string类型的datetime数据
 * @param dateTime                  需要转换的时间
 * @return                          string类型的datetime数据
 */
QString ScheduleDataInfo::DateTimeToStringDate(const QDateTime &dateTime)
{
    QTime _offsetTime = QTime(0, 0).addSecs(dateTime.timeZone().offsetFromUtc(dateTime));
    return QString("%1+%2").arg(dateTime.toString("yyyy-MM-ddThh:mm:ss")).arg(_offsetTime.toString("hh:mm"));
}

/**
 * @brief JsonArrayDataToScheduleMapData        将JsonArray数据转换为QMap存储Schedule数据
 * @param jsonArrData                           JsonArray数据
 * @return                                      QMap存储Schedule数据
 */
QMap<QDate, QVector<ScheduleDataInfo> > ScheduleDataInfo::JsonArrayDataToScheduleMapData(const QJsonArray &jsonArrData)
{
    QMap<QDate, QVector<ScheduleDataInfo> > _resultMap;
    QJsonObject _jsonObjScheduleVector;
    QVector<ScheduleDataInfo> _scheduleVector;
    QDate _date;
    for (int i = 0; i < jsonArrData.size(); i++) {
        _jsonObjScheduleVector = jsonArrData.at(i).toObject();
        _scheduleVector.clear();
        //因为是预先定义好的JSON数据格式，所以这里可以这样读取
        if (_jsonObjScheduleVector.contains("Date")) {
            _date = QDate::fromString(_jsonObjScheduleVector.value("Date").toString(), "yyyy-MM-dd");
        }
        if (_jsonObjScheduleVector.contains("Jobs")) {
            QJsonArray subArray = _jsonObjScheduleVector.value("Jobs").toArray();
            for (int j = 0; j < subArray.size(); j++) {
                QJsonObject _jsonObjectSchedule = subArray.at(j).toObject();
                _scheduleVector.append(JsonToSchedule(_jsonObjectSchedule));
            }
        }
        _resultMap[_date] = _scheduleVector;
    }
    return _resultMap;
}

QMap<QDate, QVector<ScheduleDataInfo> > ScheduleDataInfo::StrJsonToRangeInfo(const QString &jsonStr)
{
    QMap<QDate, QVector<ScheduleDataInfo> >_resultInfo {};
    QVector<ScheduleDataInfo>  _scheduleVector;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &json_error));
    if (json_error.error != QJsonParseError::NoError) {
        qWarning() << json_error.errorString();
        return _resultInfo;
    }
    QJsonArray rootarry = jsonDoc.array();
    _resultInfo = JsonArrayDataToScheduleMapData(rootarry);
    return _resultInfo;
}

/**
 * @brief ScheduleDataInfo::operator == 判断日程是否相等
 * @param info
 * @return
 */
bool ScheduleDataInfo::operator ==(const ScheduleDataInfo &info) const
{
    if (info.getType() == 4) {
        //由于后端每次启动设置的节日日程id不一定相同，所以不比较日程id
        return this->getTitleName() == info.getTitleName() &&
               this->getBeginDateTime() == info.getBeginDateTime();
    } else {
        return this->getID() == info.getID() && this->getRecurID() == info.getRecurID() &&
               this->getTitleName() == info.getTitleName() /*&& this->getEndDateTime() == info.getEndDateTime()
               && this->getBeginDateTime() == info.getBeginDateTime()*/;
    }
}

/**
 * @brief ScheduleDataInfo::operator <      对比日程大小
 * @param info
 * @return
 */
bool ScheduleDataInfo::operator <(const ScheduleDataInfo &info) const
{
    if (getBeginDateTime().date() != getEndDateTime().date() &&
            info.getBeginDateTime().date() == info.getEndDateTime().date()) {
        return true;
    } else if (getBeginDateTime().date() == getEndDateTime().date() &&
               info.getBeginDateTime().date() != info.getEndDateTime().date()) {
        return false;
    } else if (getBeginDateTime().date() != getEndDateTime().date() &&
               info.getBeginDateTime().date() != info.getEndDateTime().date()) {
        if (getBeginDateTime().date() == info.getBeginDateTime().date()) {
            return getBeginDateTime().daysTo(getEndDateTime()) > info.getBeginDateTime().daysTo(info.getEndDateTime());
        }
        return getBeginDateTime().date() < info.getBeginDateTime().date();
    } else {
        if (getType() == 4) return true;
        if (info.getType() == 4) return false;
        if (getBeginDateTime() == info.getBeginDateTime()) {
            if (getTitleName() == info.getTitleName()) {
                return getID() < info.getID();
            }
            return getTitleName() < info.getTitleName();
        } else {
            return getBeginDateTime() < info.getBeginDateTime();
        }
    }
}

/**
 * @brief operator <<           日程信息调试打印
 * @param debug
 * @param scheduleJsonData
 * @return
 */
QDebug operator<<(QDebug debug, const ScheduleDataInfo &scheduleJsonData)
{
    debug << "("
          << QString("ID:") << scheduleJsonData.getID() << QString(",")
          << QString("IsAllDay:") << scheduleJsonData.getAllDay() << QString(",")
          << QString("TitleName:") << scheduleJsonData.getTitleName() << QString(",")
          << QString("BeginDateTime:") << scheduleJsonData.getBeginDateTime() << QString(",")
          << QString("EndDateTime:") << scheduleJsonData.getEndDateTime() << QString(",")
          << QString("Description:") << scheduleJsonData.getDescription() << QString(",")
          << QString("Type:") << scheduleJsonData.getType() << QString(",")
          << QString("RRule:") << scheduleJsonData.getScheduleRRule() << QString(",")
          << QString("Remind:") << scheduleJsonData.getScheduleRemind() << QString(",")
          << QString("RecurID:") << scheduleJsonData.getRecurID()
          << QString("IsLunar:") << scheduleJsonData.getIsLunar()
          << ")";
    return debug;
}
/**
 * @brief JobTypeInfo           构造函数
 */
JobTypeInfo::JobTypeInfo(int typeNo, QString typeName, int colorTypeNo, QString colorHex, int authority)
    : iJobTypeNo(typeNo)
    , strJobTypeName(typeName)
    , m_ColorInfo(colorTypeNo, colorHex)
    , iAuthority(authority)
{
}

JobTypeInfo::JobTypeInfo(int typeNo, QString typeName, const JobTypeColorInfo &colorInfo)
    : iJobTypeNo(typeNo)
    , strJobTypeName(typeName)
    , m_ColorInfo(colorInfo)
    , iAuthority(colorInfo.getAuthority())
{
}

/**
 * @brief JobTypeInfo           构造函数
 */
JobTypeInfo::JobTypeInfo(const JobTypeInfo &_other)
{
    iJobTypeNo = _other.getJobTypeNo();
    strJobTypeName = _other.getJobTypeName();
    m_ColorInfo = _other.getColorInfo();
    iAuthority = _other.getAuthority();
}

JobTypeColorInfo::JobTypeColorInfo(int typeNo, QString colorHex, int authority)
    : iTypeNo(typeNo)
    , strColorHex(colorHex)
    , iAuthority(authority)
{

}

/**
 * @brief JsonStrToTypeInfo           从json串解析出日程类型列表
 * @param jsonStr                     json串
 * @param QList<JobTypeInfo>          日程类型列表
 */
bool JobTypeInfo::jsonStrToJobTypeInfoList(const QString &strJson, QList<JobTypeInfo> &lstJobType)
{
    QJsonArray ja;
    Utils::StringToObject(strJson, ja);

    lstJobType.clear();
    QJsonObject _jsonObj;
    for (int i = 0; i < ja.size(); i++) {
        _jsonObj = ja.at(i).toObject();
        JobTypeInfo _jobtype;
        //因为是预先定义好的JSON数据格式，所以这里可以这样读取
        //日程类型编号
        if (_jsonObj.contains("JobTypeNo")) {
            _jobtype.setJobTypeNo(_jsonObj.value("JobTypeNo").toInt());
        }
        //日程类型名称
        if (_jsonObj.contains("JobTypeName")) {
            _jobtype.setJobTypeName(_jsonObj.value("JobTypeName").toString());
        }
        //日程类型颜色编号
        if (_jsonObj.contains("ColorTypeNo")) {
            _jobtype.setColorTypeNo(_jsonObj.value("ColorTypeNo").toInt());
        }
        //日程类型颜色16进制编码
        if (_jsonObj.contains("ColorHex")) {
            _jobtype.setColorHex(_jsonObj.value("ColorHex").toString());
        }
        //日程类型权限
        if (_jsonObj.contains("Authority")) {
            _jobtype.setAuthority(_jsonObj.value("Authority").toInt());
        }

        lstJobType.push_back(_jobtype);
    }

    return true;
}

/**
 * @brief JsonStrToJobTypeInfo        从json串解析出一条日程类型记录
 * @param jsonStr                     json串
 * @param lstJobType                  一条日程类型记录
 */
bool JobTypeInfo::jsonStrToJobTypeInfo(const QString &strJson, JobTypeInfo &jobType)
{
    QJsonArray ja;
    Utils::StringToObject(strJson, ja);
    if (ja.size() < 1) {
        return false;
    }

    QJsonObject _jsonObj = ja.at(0).toObject();
    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    //日程类型编号
    if (_jsonObj.contains("JobTypeNo")) {
        jobType.setJobTypeNo(_jsonObj.value("JobTypeNo").toInt());
    }
    //日程类型名称
    if (_jsonObj.contains("JobTypeName")) {
        jobType.setJobTypeName(_jsonObj.value("JobTypeName").toString());
    }
    //日程类型颜色编号
    if (_jsonObj.contains("ColorTypeNo")) {
        jobType.setColorTypeNo(_jsonObj.value("ColorTypeNo").toInt());
    }
    //日程类型颜色16进制编码
    if (_jsonObj.contains("ColorHex")) {
        jobType.setColorHex(_jsonObj.value("ColorHex").toString());
    }
    //日程类型权限
    if (_jsonObj.contains("Authority")) {
        jobType.setAuthority(_jsonObj.value("Authority").toInt());
    }
    return true;
}

//将一条日程记录转换为json
bool JobTypeInfo::jobTypeInfoToJsonStr(const JobTypeInfo &jobType, QString &strJson)
{
    QJsonArray jsonArray;
    QJsonDocument doc;
    QJsonObject obj;
    obj.insert("JobTypeNo",   jobType.getJobTypeNo());
    obj.insert("JobTypeName", jobType.getJobTypeName());
    obj.insert("ColorTypeNo", jobType.getColorTypeNo());
    obj.insert("ColorHex",    jobType.getColorHex());
    obj.insert("Authority",   jobType.getAuthority());
    jsonArray.append(obj);
    doc.setArray(jsonArray);
    strJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    return true;
}

/**
* @brief jobTypeInfoListToJosnString  日程列表转Json串
* param  lstJobType                   JobType日程类型信息列表
* param  strJson                      json格式的日程类型信息
* return bool                         返回操作结果
*/
bool JobTypeInfo::jobTypeInfoListToJosnString(const QList<JobTypeInfo> &lstJobType, QString &strJson)
{
    QJsonArray jsonArray;
    QJsonDocument doc;
    foreach (JobTypeInfo _jobType, lstJobType) {
        QJsonObject obj;
        obj.insert("JobTypeNo",   _jobType.getJobTypeNo());
        obj.insert("JobTypeName", _jobType.getJobTypeName());
        obj.insert("ColorTypeNo", _jobType.getColorTypeNo());
        obj.insert("ColorHex",    _jobType.getColorHex());
        obj.insert("Authority",   _jobType.getAuthority());
        jsonArray.append(obj);
    }
    doc.setArray(jsonArray);
    strJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    return true;
}


/**
 * @brief JsonStrToTypeInfo           从json串解析出颜色列表
 * @param jsonStr                     json串
 * @param QList<JobTypeInfo>          颜色列表
 */
bool JobTypeInfo::jsonStrToColorTypeInfoList(const QString &strJson, QList<JobTypeColorInfo> &lstColorType)
{
    QJsonArray ja;
    Utils::StringToObject(strJson, ja);

    lstColorType.clear();
    QJsonObject _jsonObj;
    for (int i = 0; i < ja.size(); i++) {
        _jsonObj = ja.at(i).toObject();
        JobTypeColorInfo _colortype;
        //因为是预先定义好的JSON数据格式，所以这里可以这样读取
        //颜色类型编号
        if (_jsonObj.contains("ColorTypeNo")) {
            _colortype.setTypeNo(_jsonObj.value("ColorTypeNo").toInt());
        }
        //颜色16进制编码
        if (_jsonObj.contains("ColorHex")) {
            _colortype.setColorHex(_jsonObj.value("ColorHex").toString());
        }
        //颜色类型权限
        if (_jsonObj.contains("Authority")) {
            _colortype.setAuthority(_jsonObj.value("Authority").toInt());
        }

        lstColorType.push_back(_colortype);
    }

    return true;
}

/**
 * @brief JsonStrToTypeInfo           从json串解析出颜色信息
 * @param jsonStr                     json串
 * @param colorTypeInfo               颜色信息
 */
bool JobTypeInfo::jsonStrToColorTypeInfo(const QString &strJson, JobTypeColorInfo &colorTypeInfo)
{
    QJsonArray ja;
    Utils::StringToObject(strJson, ja);
    if (ja.size() < 1) {
        return false;
    }

    QJsonObject _jsonObj = ja.at(0).toObject();
    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    //颜色类型编号
    if (_jsonObj.contains("ColorTypeNo")) {
        colorTypeInfo.setTypeNo(_jsonObj.value("ColorTypeNo").toInt());
    }
    //颜色16进制编码
    if (_jsonObj.contains("ColorHex")) {
        colorTypeInfo.setColorHex(_jsonObj.value("ColorHex").toString());
    }
    //颜色类型权限
    if (_jsonObj.contains("Authority")) {
        colorTypeInfo.setAuthority(_jsonObj.value("Authority").toInt());
    }

    return true;
}

/**
 * @brief colorTypeInfoToJsonStr      将一条颜色记录转换为json
 * @param jsonStr                     json串
 * @param colorTypeInfo               颜色信息
 */
bool JobTypeInfo::colorTypeInfoToJsonStr(const JobTypeColorInfo &colorType, QString &strJson)
{
    QJsonArray jsonArray;
    QJsonDocument doc;
    QJsonObject obj;
    obj.insert("ColorTypeNo", colorType.getTypeNo());
    obj.insert("ColorHex",    colorType.getColorHex());
    obj.insert("Authority",   colorType.getAuthority());
    jsonArray.append(obj);
    doc.setArray(jsonArray);
    strJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    return true;
}
/**
 * @brief colorTypeToJosnString  颜色列表转Json串
 * param  lstColorType           JobType日程类型信息列表
 * param  strJson                json格式的日程类型信息
 * return bool                   返回操作结果
 */
bool JobTypeInfo::colorTypeInfoListToJosnString(const QList<JobTypeColorInfo> &lstColorType, QString &strJson)
{
    QJsonArray jsonArray;
    QJsonDocument doc;
    foreach (JobTypeColorInfo var, lstColorType) {
        QJsonObject obj;
        obj.insert("ColorTypeNo", var.getTypeNo());
        obj.insert("ColorHex",    var.getColorHex());
        obj.insert("Authority",   var.getAuthority());
        jsonArray.append(obj);
    }
    doc.setArray(jsonArray);
    strJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    return true;
}

JobTypeColorInfo JobTypeInfo::getColorInfo() const
{
    return m_ColorInfo;
}

JobTypeColorInfo &JobTypeInfo::getColorInfo()
{
    return m_ColorInfo;
}

void JobTypeInfo::setColorInfo(const JobTypeColorInfo &ColorInfo)
{
    m_ColorInfo = ColorInfo;
}
