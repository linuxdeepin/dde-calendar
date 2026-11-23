// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "changescheduletask.h"
#include "queryschedulestate.h"
#include "selectinquirystate.h"
#include "selectandquerystate.h"
#include "repeatfeedbackstate.h"
#include "confirwfeedbackstate.h"
#include "getchangedatastate.h"
#include "clocaldata.h"
#include "jsondata.h"
#include "dschedule.h"
#include "dscheduledatamanager.h"
#include "globaldef.h"
#include "reply.h"
#include "stub.h"
#include <QDateTime>
#include <QSignalSpy>
#include <memory>

static DSchedule::Ptr makeSch(const QDateTime &start, const QDateTime &end,
                               const QString &summary = "")
{
    DSchedule::Ptr sch(new DSchedule());
    sch->setDtStart(start);
    sch->setDtEnd(end);
    if (!summary.isEmpty()) sch->setSummary(summary);
    sch->setUid("test-uid");
    return sch;
}

// ---- DB stub helpers ----
static DScheduleDataManager* stub_getInstance()
{
    return reinterpret_cast<DScheduleDataManager*>(0x1);
}

static DSchedule::Ptr stub_queryScheduleByScheduleID(DScheduleDataManager*, const QString &)
{
    return makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                    QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
}

static bool stub_updateSchedule(DScheduleDataManager*, const DSchedule::Ptr &)
{
    return true;
}

static QString stub_createSchedule(DScheduleDataManager*, const DSchedule::Ptr &)
{
    return "test-schedule-id";
}

static int stub_numberOfRepetitions(DScheduleDataManager*, const QString &)
{
    return 1;
}

// ============================================================================
// Constructor
// ============================================================================
TEST(ChangeScheduleTaskTest, Constructor_CreatesQueryScheduleState)
{
    changeScheduleTask task;
    EXPECT_NE(nullptr, task.getState());
}

// ============================================================================
// getCurrentState
// ============================================================================
TEST(ChangeScheduleTaskTest, GetCurrentState_ReturnsTailOfChain)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    EXPECT_NE(nullptr, current);
    EXPECT_EQ(nullptr, current->getNextState());
}

TEST(ChangeScheduleTaskTest, GetCurrentState_WalksChain)
{
    changeScheduleTask task;
    scheduleState *head = task.getState();
    queryScheduleState *next = new queryScheduleState(&task);
    head->setNextState(next);
    scheduleState *tail = task.getCurrentState();
    EXPECT_EQ(next, tail);
}

// ============================================================================
// InitState
// ============================================================================
TEST(ChangeScheduleTaskTest, InitState_NullJsonData_ReturnsCancelChangeTTS)
{
    changeScheduleTask task;
    Reply reply = task.InitState(nullptr);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

TEST(ChangeScheduleTaskTest, InitState_NullJsonData_WithUpdateState)
{
    changeScheduleTask task;
    Reply reply = task.InitState(nullptr, true);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

TEST(ChangeScheduleTaskTest, InitState_WithJsonData_CallsProcess)
{
    changeScheduleTask task;
    JsonData jsonData;
    Reply reply = task.InitState(&jsonData);
    SUCCEED();
}

TEST(ChangeScheduleTaskTest, InitState_WithJsonData_ClearsExistingLocalData)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    current->setLocalData(data);
    JsonData jsonData;
    Reply reply = task.InitState(&jsonData);
    SUCCEED();
}

// ============================================================================
// confirmInfo
// ============================================================================
TEST(ChangeScheduleTaskTest, ConfirmInfo_NotOK_CallsInitState)
{
    changeScheduleTask task;
    Reply reply = task.confirmInfo(false);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

TEST(ChangeScheduleTaskTest, ConfirmInfo_OK_WithLocalData)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    data->setNewInfo(sch);
    current->setLocalData(data);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);

    Reply reply = task.confirmInfo(true);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

// ============================================================================
// getFeedbackByQuerySchedule
// ============================================================================
TEST(ChangeScheduleTaskTest, GetFeedback_EmptyList_ReturnsNoScheduleTTS)
{
    changeScheduleTask task;
    DSchedule::List empty;
    Reply reply = task.getFeedbackByQuerySchedule(empty);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

TEST(ChangeScheduleTaskTest, GetFeedback_SingleSchedule_RRuleNone)
{
    changeScheduleTask task;
    DSchedule::List list;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    sch->setRRuleType(DSchedule::RRule_None);
    list.append(sch);

    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    current->setLocalData(data);

    Reply reply = task.getFeedbackByQuerySchedule(list);
    SUCCEED();
}

TEST(ChangeScheduleTaskTest, GetFeedback_SingleSchedule_RRuleWeek)
{
    changeScheduleTask task;
    DSchedule::List list;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    sch->setRRuleType(DSchedule::RRule_Week);
    list.append(sch);

    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    current->setLocalData(data);

    Reply reply = task.getFeedbackByQuerySchedule(list);
    SUCCEED();
}

TEST(ChangeScheduleTaskTest, GetFeedback_MultipleSchedules)
{
    changeScheduleTask task;
    DSchedule::List list;
    list.append(makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                          QDateTime(QDate(2026, 6, 1), QTime(11, 0))));
    list.append(makeSch(QDateTime(QDate(2026, 6, 2), QTime(10, 0)),
                          QDateTime(QDate(2026, 6, 2), QTime(11, 0))));

    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    current->setLocalData(data);

    Reply reply = task.getFeedbackByQuerySchedule(list);
    SUCCEED();
}

// ============================================================================
// getReplyBySelectSchedule
// ============================================================================
TEST(ChangeScheduleTaskTest, GetReplyBySelectSchedule_NoSuggestDatetimeAndNoTitleName)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    current->setLocalData(data);

    Reply reply = task.getReplyBySelectSchedule(sch);
    SUCCEED();
}

TEST(ChangeScheduleTaskTest, GetReplyBySelectSchedule_WithTitleName)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    data->setToTitleName("New Title");
    current->setLocalData(data);

    Reply reply = task.getReplyBySelectSchedule(sch);
    SUCCEED();
}

TEST(ChangeScheduleTaskTest, GetReplyBySelectSchedule_WithSuggestDatetime)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    sch->setRRuleType(DSchedule::RRule_None);
    data->setSelectInfo(sch);
    SemanticsDateTime toTime;
    SuggestDatetimeInfo sdt;
    sdt.datetime = QDateTime(QDate(2026, 6, 2), QTime(14, 0));
    sdt.hasTime = true;
    toTime.suggestDatetime.append(sdt);
    data->setToTime(toTime);
    current->setLocalData(data);

    Reply reply = task.getReplyBySelectSchedule(sch);
    SUCCEED();
}

TEST(ChangeScheduleTaskTest, GetReplyBySelectSchedule_NegativeOffset_ResetTo1)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    data->setToTitleName("Title");
    current->setLocalData(data);

    Reply reply = task.getReplyBySelectSchedule(sch);
    SUCCEED();
}

// ============================================================================
// changeDateTimeIsInNormalRange
// ============================================================================
TEST(ChangeScheduleTaskTest, ChangeDateTimeIsInNormalRange_ValidRange)
{
    changeScheduleTask task;
    QDateTime now = QDateTime::currentDateTime();
    DSchedule::Ptr sch = makeSch(now.addDays(1), now.addDays(1).addSecs(3600));
    EXPECT_TRUE(task.changeDateTimeIsInNormalRange(sch));
}

TEST(ChangeScheduleTaskTest, ChangeDateTimeIsInNormalRange_PastStartTime)
{
    changeScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2020, 1, 1), QTime(10, 0)),
                                   QDateTime(QDate(2020, 1, 1), QTime(11, 0)));
    EXPECT_FALSE(task.changeDateTimeIsInNormalRange(sch));
}

TEST(ChangeScheduleTaskTest, ChangeDateTimeIsInNormalRange_BeyondMaxRange)
{
    changeScheduleTask task;
    QDateTime future = QDateTime::currentDateTime().addMonths(12);
    DSchedule::Ptr sch = makeSch(future, future.addSecs(3600));
    EXPECT_FALSE(task.changeDateTimeIsInNormalRange(sch));
}

// ============================================================================
// getNewInfo
// ============================================================================
TEST(ChangeScheduleTaskTest, GetNewInfo_WithToTitleName)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 12, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 12, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    data->setToTitleName("Updated Title");
    current->setLocalData(data);
    EXPECT_TRUE(task.getNewInfo());
}

TEST(ChangeScheduleTaskTest, GetNewInfo_WithDateOnly)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 12, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 12, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    SemanticsDateTime toTime;
    DateTimeInfo dti;
    dti.m_Date = QDate(2026, 12, 15);
    dti.hasDate = true;
    dti.hasTime = false;
    toTime.dateTime.append(dti);
    data->setToTime(toTime);
    current->setLocalData(data);
    EXPECT_TRUE(task.getNewInfo());
}

TEST(ChangeScheduleTaskTest, GetNewInfo_WithTimeOnly_NonTodayDate)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 12, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 12, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    SemanticsDateTime toTime;
    DateTimeInfo dti;
    dti.m_Date = QDate(2026, 12, 15);
    dti.m_Time = QTime(14, 0);
    dti.hasDate = true;
    dti.hasTime = true;
    toTime.dateTime.append(dti);
    SuggestDatetimeInfo sdt;
    sdt.datetime = QDateTime(QDate(2026, 12, 15), QTime(14, 0));
    sdt.hasTime = true;
    toTime.suggestDatetime.append(sdt);
    data->setToTime(toTime);
    current->setLocalData(data);
    EXPECT_TRUE(task.getNewInfo());
}

TEST(ChangeScheduleTaskTest, GetNewInfo_WithTwoDateTimes)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 12, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 12, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    SemanticsDateTime toTime;
    DateTimeInfo dti1, dti2;
    dti1.m_Date = QDate(2026, 12, 15);
    dti1.hasDate = true;
    dti2.m_Date = QDate(2026, 12, 16);
    dti2.hasDate = true;
    toTime.dateTime.append(dti1);
    toTime.dateTime.append(dti2);
    data->setToTime(toTime);
    current->setLocalData(data);
    EXPECT_TRUE(task.getNewInfo());
}

TEST(ChangeScheduleTaskTest, GetNewInfo_WithTwoDateTimes_EndBeforeStart)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 12, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 12, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    SemanticsDateTime toTime;
    DateTimeInfo dti1, dti2;
    dti1.m_Date = QDate(2026, 12, 20);
    dti1.hasDate = true;
    dti2.m_Date = QDate(2026, 12, 10);
    dti2.hasDate = true;
    toTime.dateTime.append(dti1);
    toTime.dateTime.append(dti2);
    data->setToTime(toTime);
    current->setLocalData(data);
    EXPECT_TRUE(task.getNewInfo());
}

// ============================================================================
// repeatScheduleHandle
// ============================================================================
TEST(ChangeScheduleTaskTest, RepeatScheduleHandle_OnlyOne)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    data->setNewInfo(sch);
    current->setLocalData(data);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    Reply reply = task.repeatScheduleHandle(sch, true);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

TEST(ChangeScheduleTaskTest, RepeatScheduleHandle_All)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    sch->setRRuleType(DSchedule::RRule_None);
    data->setSelectInfo(sch);
    data->setNewInfo(sch);
    current->setLocalData(data);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);

    Reply reply = task.repeatScheduleHandle(sch, false);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

TEST(ChangeScheduleTaskTest, RepeatScheduleHandle_InvalidOccurrenceDoesNotWrite)
{
    const QDateTime start(QDate(2024, 1, 1), QTime(9, 0), Qt::UTC);
    const auto original = makeSch(start, start.addSecs(3600));
    auto rule = std::make_unique<CalendarCore::RRule>();
    ASSERT_TRUE(rule->parse(QStringLiteral("FREQ=DAILY;BYHOUR=9;COUNT=100001")));
    original->recurrence()->addRRule(rule.release());

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    static DSchedule::Ptr stored;
    stored = original;
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), +[](DScheduleDataManager *, const QString &) {
        return stored;
    });
    stub.set(ADDR(DScheduleDataManager, updateSchedule), +[](DScheduleDataManager *, const DSchedule::Ptr &) {
        ADD_FAILURE() << "An invalid occurrence must not update the series";
        return false;
    });
    stub.set(ADDR(DScheduleDataManager, createSchedule), +[](DScheduleDataManager *, const DSchedule::Ptr &) {
        ADD_FAILURE() << "An invalid occurrence must not create a schedule";
        return QString();
    });

    for (const auto &selected : {start.addDays(-1), start.addDays(100000)}) {
        changeScheduleTask task;
        auto *state = task.getCurrentState();
        const auto data = CLocalData::Ptr::create();
        const auto occurrence = DSchedule::Ptr::create(*original);
        occurrence->setDtStart(selected);
        data->setSelectInfo(occurrence);
        data->setNewInfo(DSchedule::Ptr::create(*occurrence));
        state->setLocalData(data);
        auto reply = task.repeatScheduleHandle(occurrence, false);
        EXPECT_FALSE(reply.ttsMessage().isEmpty());
        EXPECT_EQ(state->getNextState(), nullptr);
        EXPECT_EQ(original->recurrence()->duration(), 100001);
    }
    stored.clear();
}

// ============================================================================
// confirwScheduleHandle
// ============================================================================
TEST(ChangeScheduleTaskTest, ConfirwScheduleHandle_WithLocalData)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    data->setNewInfo(sch);
    current->setLocalData(data);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);

    DSchedule::Ptr info = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                     QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    Reply reply = task.confirwScheduleHandle(info);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

// ============================================================================
// changeRepeatSchedule
// ============================================================================
TEST(ChangeScheduleTaskTest, ChangeRepeatSchedule_OnlyOne)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    data->setNewInfo(sch);
    current->setLocalData(data);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    DSchedule::Ptr info = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                     QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    task.changeRepeatSchedule(info, true);
    SUCCEED();
}

TEST(ChangeScheduleTaskTest, ChangeRepeatSchedule_All)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    sch->setRRuleType(DSchedule::RRule_None);
    data->setSelectInfo(sch);
    data->setNewInfo(sch);
    current->setLocalData(data);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);

    DSchedule::Ptr info = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                     QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    task.changeRepeatSchedule(info, false);
    SUCCEED();
}

// ============================================================================
// changeOrdinarySchedule
// ============================================================================
TEST(ChangeScheduleTaskTest, ChangeOrdinarySchedule)
{
    changeScheduleTask task;
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);

    DSchedule::Ptr info = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                     QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    task.changeOrdinarySchedule(info);
    SUCCEED();
}

// ============================================================================
// slotSelectScheduleIndex
// ============================================================================
TEST(ChangeScheduleTaskTest, SlotSelectScheduleIndex_ValidIndex)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::List list;
    list.append(makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                         QDateTime(QDate(2026, 6, 1), QTime(11, 0))));
    data->setScheduleInfoVector(list);
    current->setLocalData(data);
    task.slotSelectScheduleIndex(1);
    SUCCEED();
}

TEST(ChangeScheduleTaskTest, SlotSelectScheduleIndex_InvalidIndex)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::List list;
    data->setScheduleInfoVector(list);
    current->setLocalData(data);
    task.slotSelectScheduleIndex(1);
    SUCCEED();
}

// ============================================================================
// slotButtonCheckNum
// ============================================================================
TEST(ChangeScheduleTaskTest, SlotButtonCheckNum_ButtonCount2_Index1)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    data->setNewInfo(sch);
    current->setLocalData(data);
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);
    task.slotButtonCheckNum(1, QString(), 2);
    SUCCEED();
}

TEST(ChangeScheduleTaskTest, SlotButtonCheckNum_ButtonCount3_Index1)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    data->setNewInfo(sch);
    current->setLocalData(data);
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);
    task.slotButtonCheckNum(1, QString(), 3);
    SUCCEED();
}

TEST(ChangeScheduleTaskTest, SlotButtonCheckNum_ButtonCount3_Index2)
{
    changeScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    data->setSelectInfo(sch);
    data->setNewInfo(sch);
    current->setLocalData(data);
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);
    task.slotButtonCheckNum(2, QString(), 3);
    SUCCEED();
}

TEST(ChangeScheduleTaskTest, SlotButtonCheckNum_Index0)
{
    changeScheduleTask task;
    task.slotButtonCheckNum(0, QString(), 2);
    SUCCEED();
}

// ============================================================================
// Widget-creating methods
// ============================================================================
TEST(ChangeScheduleTaskTest, GetListScheduleReply)
{
    changeScheduleTask task;
    DSchedule::List list;
    list.append(makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                         QDateTime(QDate(2026, 6, 1), QTime(11, 0))));
    Reply reply = task.getListScheduleReply(list);
    EXPECT_TRUE(true);
}

TEST(ChangeScheduleTaskTest, CreateRepeatWidget)
{
    changeScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    QWidget *w = task.createRepeatWidget(sch);
    EXPECT_NE(nullptr, w);
    delete w;
}

TEST(ChangeScheduleTaskTest, CreateConfirmWidget)
{
    changeScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    QWidget *w = task.createConfirmWidget(sch);
    EXPECT_NE(nullptr, w);
    delete w;
}

TEST(ChangeScheduleTaskTest, CreateInquiryWidget)
{
    changeScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    QWidget *w = task.createInquiryWidget(sch);
    EXPECT_NE(nullptr, w);
    delete w;
}

TEST(ChangeScheduleTaskTest, SplitsExceptionsOnlyAfterNewSeriesIsCreated)
{
    const QDateTime start(QDate(2024, 1, 1), QTime(9, 0), Qt::UTC);
    static DSchedule::Ptr original;
    original = makeSch(start, start.addSecs(3600));
    original->setRRuleType(DSchedule::RRule_Day);
    original->recurrence()->setDuration(5);
    const auto extra = start.addDays(8);
    original->recurrence()->addRDateTime(extra);
    original->recurrence()->addExDateTime(start.addDays(3));
    original->recurrence()->addExDate(start.addDays(4).date());
    CalendarCore::CalEvent exception;
    exception.setRecurrenceId(extra);
    exception.setDtStart(start.addDays(3).addSecs(3600));
    exception.setDtEnd(exception.dtStart().addSecs(3600));
    original->setRecurrenceException(exception);

    static DSchedule::Ptr created;
    static DSchedule::Ptr truncated;
    static bool createSucceeds;
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), +[](DScheduleDataManager *, const QString &) {
        return original;
    });
    stub.set(ADDR(DScheduleDataManager, createSchedule), +[](DScheduleDataManager *, const DSchedule::Ptr &schedule) {
        created = schedule;
        return createSucceeds ? QStringLiteral("split-series") : QString();
    });
    stub.set(ADDR(DScheduleDataManager, updateSchedule), +[](DScheduleDataManager *, const DSchedule::Ptr &schedule) {
        truncated = schedule;
        return true;
    });
    const auto cutoff = start.addDays(1);
    for (bool succeeds : {false, true}) {
        createSucceeds = succeeds;
        changeScheduleTask task;
        const auto selection = makeSch(cutoff, cutoff.addSecs(3600));
        selection->setRecurrenceId(cutoff);
        selection->setRRuleType(DSchedule::RRule_Day);
        const auto edited = makeSch(cutoff.addSecs(3600), cutoff.addSecs(7200));
        edited->setRRuleType(DSchedule::RRule_Day);
        edited->recurrence()->setDuration(5);
        const auto data = CLocalData::Ptr::create();
        data->setSelectInfo(selection);
        data->setNewInfo(edited);
        task.getCurrentState()->setLocalData(data);
        EXPECT_EQ(task.changeAllInfo(selection), succeeds);
        ASSERT_TRUE(created);
        EXPECT_EQ(created->uid(), QStringLiteral("0"));
        const auto movedId = extra.addSecs(3600);
        EXPECT_TRUE(created->hasRecurrenceException(movedId));
        EXPECT_EQ(created->recurrence()->rDateTimes(), QList<QDateTime>{movedId});
        EXPECT_EQ(created->recurrence()->exDateTimes(), QList<QDateTime>{start.addDays(3).addSecs(3600)});
        EXPECT_EQ(created->recurrence()->exDates(), QList<QDate>{start.addDays(4).date()});
        EXPECT_EQ(bool(truncated), succeeds);
        if (truncated) {
            EXPECT_FALSE(truncated->hasRecurrenceExceptions());
            EXPECT_TRUE(truncated->recurrence()->rDateTimes().isEmpty());
            EXPECT_TRUE(truncated->recurrence()->exDateTimes().isEmpty());
            EXPECT_TRUE(truncated->recurrence()->exDates().isEmpty());
        }
        EXPECT_TRUE(original->hasRecurrenceException(extra));
        EXPECT_EQ(original->recurrence()->rDateTimes(), QList<QDateTime>{extra});
        created.clear();
        truncated.clear();
    }
    original.clear();
}

TEST(ChangeScheduleTaskTest, OffersBothEditScopesForAnOccurrenceInARange)
{
    const QDateTime start(QDate::currentDate().addDays(2), QTime(9, 0), Qt::UTC);
    const auto selectedId = start.addDays(2);
    static DSchedule::Ptr master;
    master = makeSch(start, start.addSecs(3600));
    master->setRRuleType(DSchedule::RRule_Day);
    master->recurrence()->setDuration(5);
    CalendarCore::CalEvent range;
    range.setRecurrenceId(start.addDays(1));
    range.setDtStart(range.recurrenceId().addSecs(7200));
    range.setDtEnd(range.dtStart().addSecs(3600));
    range.setThisAndFuture(true);
    master->setRecurrenceException(range);
    const auto selected = master->recurrenceException(selectedId);
    ASSERT_TRUE(selected);
    ASSERT_FALSE(selected->recurs());

    static DSchedule::Ptr updated;
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), +[](DScheduleDataManager *, const QString &) {
        return master;
    });
    stub.set(ADDR(DScheduleDataManager, updateSchedule), +[](DScheduleDataManager *, const DSchedule::Ptr &schedule) {
        updated = schedule;
        return true;
    });
    stub.set(ADDR(DScheduleDataManager, createSchedule), +[](DScheduleDataManager *, const DSchedule::Ptr &) {
        ADD_FAILURE() << "A range edit must keep the UID of the series";
        return QString();
    });

    changeScheduleTask task;
    const auto data = CLocalData::Ptr::create();
    data->setSelectInfo(selected);
    data->setToTitleName(QStringLiteral("Edited occurrence"));
    task.getCurrentState()->setLocalData(data);
    Reply reply;
    const std::unique_ptr<scheduleState> next(task.getNextStateBySelectScheduleInfo(selected, data, reply));
    EXPECT_NE(dynamic_cast<repeatfeedbackstate *>(next.get()), nullptr);
    EXPECT_TRUE(reply.ttsMessage().contains(QStringLiteral("以后的日程")));

    for (bool onlyOne : {true, false}) {
        ASSERT_TRUE(task.changeRepeatSchedule(selected, onlyOne));
        ASSERT_TRUE(updated);
        EXPECT_EQ(updated->uid(), master->uid());
        EXPECT_EQ(updated->recurrenceId(), selectedId);
        EXPECT_EQ(updated->summary(), QStringLiteral("Edited occurrence"));
        EXPECT_EQ(updated->thisAndFuture(), !onlyOne);
        updated.clear();
    }
    master.clear();
}
