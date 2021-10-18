/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     hejinghai <hejinghai@uniontech.com>
*
* Maintainer: hejinghai <hejinghai@uniontech.com>
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
#include "ut_huanglidatabase.h"
#include "../third-party_stub/stub.h"
#include "config.h"
#include "sqldatabase_stub.h"

ut_huanglidatabase::ut_huanglidatabase()
{
}

bool stub_OpenHuangliDatabase(void *obj, const QString &dbpath)
{
    Q_UNUSED(dbpath);
    HuangLiDataBase *o = reinterpret_cast<HuangLiDataBase *>(obj);
    QString name;
    {
        name = QSqlDatabase::database().connectionName();
    }
    o->m_database.removeDatabase(name);
    o->m_database = QSqlDatabase::addDatabase("QSQLITE");
    o->m_database.setDatabaseName(HL_DATABASE_DIR);

    return o->m_database.open();
}

void ut_huanglidatabase::SetUp()
{
    Stub stub;
    stub.set(ADDR(HuangLiDataBase, OpenHuangliDatabase), stub_OpenHuangliDatabase);
    hlDb = new HuangLiDataBase();
}

void ut_huanglidatabase::TearDown()
{
    if (hlDb->m_database.isOpen()) {
        hlDb->m_database.close();
    }
    delete hlDb;
    hlDb = nullptr;
}

//QString HuangLiDataBase::QueryFestivalList(quint32 year, quint8 month)
TEST_F(ut_huanglidatabase, QueryFestivalList)
{
    quint32 year = 2020;
    quint8 month = 10;
    const QString strFes = "[{\"description\":\"10月1日至10月8日放假8天，9月27日，10月10日上班\",\"id\":\"2020100110\",\"list\":"
                           "[{\"date\":\"2020-10-1\",\"status\":1},"
                           "{\"date\":\"2020-10-2\",\"status\":1},"
                           "{\"date\":\"2020-10-3\",\"status\":1},"
                           "{\"date\":\"2020-10-4\",\"status\":1},"
                           "{\"date\":\"2020-10-5\",\"status\":1},"
                           "{\"date\":\"2020-10-6\",\"status\":1},"
                           "{\"date\":\"2020-10-7\",\"status\":1},"
                           "{\"date\":\"2020-10-8\",\"status\":1},"
                           "{\"date\":\"2020-9-27\",\"status\":2},"
                           "{\"date\":\"2020-10-10\",\"status\":2}],"
                           "\"month\":10,\"name\":\"中秋节\",\"rest\":"
                           "\"10月9日至10月10日请假2天，与周末连休可拼11天长假。\"}]";
    QString getFes = hlDb->QueryFestivalList(year, month);
    EXPECT_EQ(strFes, getFes);
}

//QList<stHuangLi> HuangLiDataBase::QueryHuangLiByDays(const QList<stDay> &days)
TEST_F(ut_huanglidatabase, QueryHuangLiByDays)
{
    QList<stDay> days;
    stDay day1, day2;
    day1.Year = day2.Year = 2020;
    day1.Month = day2.Month = 10;
    day1.Day = 1;
    day2.Day = 2;
    days << day1 << day2;

    QList<stHuangLi> getHuangli = hlDb->QueryHuangLiByDays(days);

    stHuangLi hl1, hl2;
    hl1 = getHuangli.at(0);
    hl2 = getHuangli.at(1);
    QString hl2Suit = "开光.解除.起基.动土.拆卸.上梁.立碑.修坟.安葬.破土.启攒.移柩.";
    QString hl2Avoid = "嫁娶.出行.安床.作灶.祭祀.入宅.移徙.出火.进人口.置产.";
    EXPECT_EQ(20201001, hl1.ID);
    EXPECT_EQ(20201002, hl2.ID);
    EXPECT_EQ(hl2Suit, hl2.Suit);
    EXPECT_EQ(hl2Avoid, hl2.Avoid);
}