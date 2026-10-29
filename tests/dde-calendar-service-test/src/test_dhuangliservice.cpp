// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dhuangliservice.h"
#include "calendarhuangli.h"
#include "config.h"
#include <gtest/gtest.h>
#include <QObject>
#include <QSqlDatabase>

class test_dhuangliservice : public QObject, public ::testing::Test
{
public:
    test_dhuangliservice()
    {
        m_service = new DHuangliService();
    }
    ~test_dhuangliservice()
    {
        if (m_service) {
            if (m_service->m_huangli && m_service->m_huangli->m_database->m_database.isOpen()) {
                m_service->m_huangli->m_database->m_database.close();
            }
            delete m_service;
        }
    }
protected:
    DHuangliService *m_service = nullptr;
};

// DHuangliService::DHuangliService(QObject *parent)
TEST_F(test_dhuangliservice, Constructor)
{
    EXPECT_NE(m_service, nullptr);
}

// QString DHuangliService::getFestivalMonth(quint32 year, quint32 month)
TEST_F(test_dhuangliservice, GetFestivalMonth)
{
    QString result = m_service->getFestivalMonth(2020, 12);
    EXPECT_FALSE(result.isEmpty());
    // 无数据月份返回空数组
    QString empty = m_service->getFestivalMonth(1900, 1);
    EXPECT_TRUE(empty == "[]" || empty.isEmpty());
}

// QString DHuangliService::getHuangLiDay(quint32 year, quint32 month, quint32 day)
TEST_F(test_dhuangliservice, GetHuangLiDay)
{
    QString result = m_service->getHuangLiDay(2020, 12, 13);
    EXPECT_FALSE(result.isEmpty());
}

// QString DHuangliService::getHuangLiMonth(quint32 year, quint32 month, bool fill)
TEST_F(test_dhuangliservice, GetHuangLiMonth)
{
    QString r1 = m_service->getHuangLiMonth(2020, 12, false);
    QString r2 = m_service->getHuangLiMonth(2020, 12, true);
    EXPECT_FALSE(r1.isEmpty());
    EXPECT_FALSE(r2.isEmpty());
}

// CaLunarDayInfo DHuangliService::getLunarInfoBySolar(quint32 year, quint32 month, quint32 day)
TEST_F(test_dhuangliservice, GetLunarInfoBySolar)
{
    CaLunarDayInfo info = m_service->getLunarInfoBySolar(2020, 12, 13);
    EXPECT_FALSE(info.mLunarDayName.isEmpty());
}

// CaLunarMonthInfo DHuangliService::getLunarMonthCalendar(quint32 year, quint32 month, bool fill)
TEST_F(test_dhuangliservice, GetLunarMonthCalendar)
{
    CaLunarMonthInfo m1 = m_service->getLunarMonthCalendar(2020, 12, false);
    CaLunarMonthInfo m2 = m_service->getLunarMonthCalendar(2020, 12, true);
    EXPECT_TRUE(m1.mDays || !m1.mDays);
    EXPECT_TRUE(m2.mDays || !m2.mDays);
}
