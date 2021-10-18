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
#ifndef UT_SCHEDULERDATABASE_H
#define UT_SCHEDULERDATABASE_H

#include "dbmanager/schedulerdatabase.h"
#include "gtest/gtest.h"
#include <QObject>

class ut_schedulerdatabase : public ::testing::Test
{
public:
    ut_schedulerdatabase();
    void SetUp() override;

    void TearDown() override;

protected:
    SchedulerDatabase *sDb = nullptr;
};

#endif // UT_SCHEDULERDATABASE_H