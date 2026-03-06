// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LUNARMANAGER_H
#define LUNARMANAGER_H

#include "dbushuanglirequest.h"
#include "huangliData/dbusdatastruct.h"
#include "huangliData/lunardatastruct.h"
#include <QObject>

class LunarManager : public QObject
{
    Q_OBJECT
public:
    explicit LunarManager(QObject *parent = nullptr);

    static LunarManager* getInstace();

    //按月获取节假日信息
    bool getFestivalMonth(quint32 year, quint32 month, FestivalInfo& festivalInfo);
    //按月获取节假日信息
    bool getFestivalMonth(const QDate &date, FestivalInfo& festivalInfo);
    //按天获取黄历信息
    bool getHuangLiDay(quint32 year, quint32 month, quint32 day, CaHuangLiDayInfo &info);
    //按天获取农历信息
    bool getHuangLiDay(const QDate &date, CaHuangLiDayInfo &out);
    //按月获取黄历信息
    bool getHuangLiMonth(quint32 year, quint32 month, CaHuangLiMonthInfo &info, bool fill = false);
    //按月获取黄历信息
    bool getHuangLiMonth(const QDate &date, CaHuangLiMonthInfo &info, bool fill = false);

    //获取当天的农历月日期和日日期名
    QString getHuangLiShortName(const QDate &date);
    //查询农历信息
    void queryLunarInfo(const QDate &startDate, const QDate &stopDate);
    //查询节假日信息
    void queryFestivalInfo(const QDate &startDate, const QDate &stopDate);
    //获取节假日日期信息
    QMap<QDate, int> getFestivalInfoDateMap(const QDate &startDate, const QDate &stopDate);
    //获取当天农历数据
    CaHuangLiDayInfo getHuangLiDay(const QDate &date);
    //获取一定时间范围内的农历数据
    QMap<QDate, CaHuangLiDayInfo> getHuangLiDayMap(const QDate &startDate, const QDate &stopDate);
    //异步获取当天农历数据
    void getHuangLiDayAsync(const QDate &date);
    //确保农历数据已加载（带缓存，避免重复查询）
    void ensureLunarDataLoaded(const QDate &startDate, const QDate &endDate);

signals:
    //异步获取农历数据完成信号
    void huangLiDayReady(const QDate &date, const CaHuangLiDayInfo &info);
    //农历信息查询完成信号
    void lunarInfoReady();
    //节假日信息查询完成信号
    void festivalInfoReady();

public slots:

private:
    DbusHuangLiRequest* m_dbusRequest = nullptr;    //dbus请求实例
    QMap<QDate, CaHuangLiDayInfo> m_lunarInfoMap;   //缓存的农历数据
    QMap<QDate, int> m_festivalDateMap;     //缓存的节假日数据
    QList<QPair<QDate, QDate>> m_queriedRanges;  //已查询的日期范围缓存

};
#define gLunarManager LunarManager::getInstace()
#endif // LUNARMANAGER_H
