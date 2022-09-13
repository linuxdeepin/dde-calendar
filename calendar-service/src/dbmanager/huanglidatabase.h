// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HUANGLIDATABASE_H
#define HUANGLIDATABASE_H
#include "src/lunardatastruct.h"
#include "src/dbusdatastruct.h"
#include "lunarandfestival.h"

#include <QObject>
#include <QSqlDatabase>

class HuangLiDataBase : public QObject
{
    Q_OBJECT
public:
    explicit HuangLiDataBase(QObject *parent = nullptr);
    QString QueryFestivalList(quint32 year, quint8 month);
    QList<stHuangLi> QueryHuangLiByDays(const QList<stDay> &days);

signals:

public slots:

private:
    QSqlDatabase m_database;
    QString dbpath;

    bool OpenHuangliDatabase(const QString &dbpath);
};

#endif // HUANGLIDATABASE_H
