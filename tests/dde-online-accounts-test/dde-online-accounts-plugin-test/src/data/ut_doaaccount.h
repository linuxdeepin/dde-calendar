/*
* Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     wangyou <wangyou@uniontech.com>
*
* Maintainer: wangyou <wangyou@uniontech.com>
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
#ifndef UT_DOAACCOUNT_H
#define UT_DOAACCOUNT_H

#include "gtest/gtest.h"
#include "doaaccount.h"

class ut_doaaccount : public ::testing::Test
{
public:
    ut_doaaccount();
    void SetUp() override;
    void TearDown() override;
    bool stub_resutlMsg();
    bool stub_resultFalse();
protected:
    DOAAccount *doaAccount = nullptr;

};

#endif // UT_DOAACCOUNT_H