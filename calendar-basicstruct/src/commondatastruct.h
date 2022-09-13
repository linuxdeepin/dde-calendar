// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMMONDATASTRUCT_H
#define COMMONDATASTRUCT_H
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QMetaType>

enum jobtypes {
    jobTypeWork = 1,
    jobTypeLife,
    jobTypeOther,
    JobTypeFestival,
};

typedef struct JobTypeJSON {
    uint ID;
    QString Name;
    QString Color;
} stJobTypeJSON;

//关于名字的国际化暂时不做处理，如果后期确认需要国际化的很少则通过client做适配来处理
static QVector<stJobTypeJSON> globalPredefinedTypes {
    {
        jobTypeWork,
        "Work",
        "#FF0000" // red
    },
    {
        jobTypeLife,
        "Life",
        "#00FF00" // green
    },
    {
        jobTypeOther,
        "Other",
        "#800080" // purple
    },
    {
        JobTypeFestival,
        "Festival",
        "#FFFF00" // yellow
    }};

typedef struct _tagJob {
    _tagJob()
        : ID {0}
        , Type {0}
        , RecurID {0}
        , RemindLaterCount {0}
        , IsLunar {false}
    {
    }
    qint64 ID;
    qint64 Type;
    qint64 RecurID;
    QString Title {};
    QString Description {};
    bool AllDay {false};
    QDateTime Start;
    QDateTime End;
    QString RRule {};//重复规则，如：工作日起床闹钟
    QString Remind {};//提前提醒，如：提前几天或几分钟
    QString Ignore {};//如：工作日起床闹钟，除了周五
    QString Title_pinyin {};
    QDateTime RemidTime; //提醒时间
    qint32 RemindLaterCount; //执行稍后提醒次数
    bool IsLunar;
} Job;

typedef struct JobArr {
    QDate date;
    QList<Job> jobs {};
    QList<Job> extends {};
} stJobArr;

typedef struct JobTime {
    QDateTime start;
    QDateTime end;
    qint64 recurID = 0;
} stJobTime;

enum RepeatType {
    RepeatNone = 0,
    RepeatDaily,
    RepeatWorkDay,
    RepeatWeekly,
    RepeatMonthly,
    RepeatYearly,
};

enum RepeatOverCondition {
    RepeatOverNever = 0,
    RepeatOverCount,
    RepeatOverUntil,
};

typedef struct RRuleOptions {
    //初始化
    RepeatType rpeat {RepeatType::RepeatNone}; //重复规则 0 无  1 每天 2 每个工作日 3 每周 4每月 5每年
    RepeatOverCondition type {RepeatOverCondition::RepeatOverNever}; //结束重复 0 永不 1  多少次结束  2 结束日期
    int tcount {}; //多少次结束只有当type=1才会生效
    QDateTime overdate; //type=2时才有效
} stRRuleOptions;

Q_DECLARE_METATYPE(Job)
#endif // COMMONDATASTRUCT_H
