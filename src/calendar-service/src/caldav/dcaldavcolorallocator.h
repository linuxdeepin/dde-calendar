// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVCOLORALLOCATOR_H
#define DCALDAVCOLORALLOCATOR_H

#include <QString>

class DAccountDataBase;

class DCalDavColorAllocator
{
public:
    static QString nextColor(DAccountDataBase *database);
};

#endif // DCALDAVCOLORALLOCATOR_H
