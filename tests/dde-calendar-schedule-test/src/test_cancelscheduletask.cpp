// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "cancelscheduletask.h"
#include "queryschedulestate.h"
#include "selectinquirystate.h"
#include "repeatfeedbackstate.h"
#include "confirwfeedbackstate.h"
#include "clocaldata.h"
#include "jsondata.h"
#include "dschedule.h"
#include "dscheduledatamanager.h"
#include "globaldef.h"
#include "reply.h"
#include "stub.h"
#include <QDateTime>
#include <QSignalSpy>

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

static void stub_deleteScheduleByScheduleID(DScheduleDataManager*, const QString &)
{
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

// ============================================================================
// Constructor
// ============================================================================
TEST(CancelScheduleTaskTest, Constructor_CreatesQueryScheduleState)
{
    cancelScheduleTask task;
    EXPECT_NE(nullptr, task.getState());
}

// ============================================================================
// getCurrentState
// ============================================================================
TEST(CancelScheduleTaskTest, GetCurrentState_ReturnsTailOfChain)
{
    cancelScheduleTask task;
    scheduleState *current = task.getCurrentState();
    EXPECT_NE(nullptr, current);
    EXPECT_EQ(nullptr, current->getNextState());
}

TEST(CancelScheduleTaskTest, GetCurrentState_WalksChain)
{
    cancelScheduleTask task;
    scheduleState *head = task.getState();
    queryScheduleState *next = new queryScheduleState(&task);
    head->setNextState(next);
    scheduleState *tail = task.getCurrentState();
    EXPECT_EQ(next, tail);
}

// ============================================================================
// InitState
// ============================================================================
TEST(CancelScheduleTaskTest, InitState_NullJsonData_ReturnsCancelDeletionTTS)
{
    cancelScheduleTask task;
    Reply reply = task.InitState(nullptr);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

TEST(CancelScheduleTaskTest, InitState_NullJsonData_WithUpdateState)
{
    cancelScheduleTask task;
    Reply reply = task.InitState(nullptr, true);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

TEST(CancelScheduleTaskTest, InitState_WithJsonData_CallsProcess)
{
    cancelScheduleTask task;
    JsonData jsonData;
    Reply reply = task.InitState(&jsonData);
    EXPECT_TRUE(true);
}

// ============================================================================
// confirmInfo
// ============================================================================
TEST(CancelScheduleTaskTest, ConfirmInfo_NotOK_CallsInitState)
{
    cancelScheduleTask task;
    Reply reply = task.confirmInfo(false);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

TEST(CancelScheduleTaskTest, ConfirmInfo_OK_WithLocalData)
{
    cancelScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    data->setSelectInfo(makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                  QDateTime(QDate(2026, 6, 1), QTime(11, 0))));
    current->setLocalData(data);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, deleteScheduleByScheduleID), stub_deleteScheduleByScheduleID);

    Reply reply = task.confirmInfo(true);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

// ============================================================================
// getFeedbackByQuerySchedule
// ============================================================================
TEST(CancelScheduleTaskTest, GetFeedback_EmptyList_ReturnsNoScheduleTTS)
{
    cancelScheduleTask task;
    DSchedule::List empty;
    Reply reply = task.getFeedbackByQuerySchedule(empty);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

TEST(CancelScheduleTaskTest, GetFeedback_SingleSchedule_RRuleNone)
{
    cancelScheduleTask task;
    DSchedule::List list;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    sch->setRRuleType(DSchedule::RRule_None);
    list.append(sch);
    Reply reply = task.getFeedbackByQuerySchedule(list);
    EXPECT_TRUE(true);
}

TEST(CancelScheduleTaskTest, GetFeedback_SingleSchedule_Recurring)
{
    cancelScheduleTask task;
    DSchedule::List list;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    sch->setRRuleType(DSchedule::RRule_Day);
    list.append(sch);
    Reply reply = task.getFeedbackByQuerySchedule(list);
    EXPECT_TRUE(true);
}

TEST(CancelScheduleTaskTest, GetFeedback_MultipleSchedules)
{
    cancelScheduleTask task;
    DSchedule::List list;
    list.append(makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                         QDateTime(QDate(2026, 6, 1), QTime(11, 0))));
    list.append(makeSch(QDateTime(QDate(2026, 6, 2), QTime(10, 0)),
                         QDateTime(QDate(2026, 6, 2), QTime(11, 0))));
    Reply reply = task.getFeedbackByQuerySchedule(list);
    EXPECT_TRUE(true);
}

// ============================================================================
// getReplyBySelectSchedule
// ============================================================================
TEST(CancelScheduleTaskTest, GetReplyBySelectSchedule_RRuleNone)
{
    cancelScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    sch->setRRuleType(DSchedule::RRule_None);
    Reply reply = task.getReplyBySelectSchedule(sch);
    EXPECT_TRUE(true);
}

TEST(CancelScheduleTaskTest, GetReplyBySelectSchedule_Recurring)
{
    cancelScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    sch->setRRuleType(DSchedule::RRule_Week);
    Reply reply = task.getReplyBySelectSchedule(sch);
    EXPECT_TRUE(true);
}

// ============================================================================
// repeatScheduleHandle
// ============================================================================
TEST(CancelScheduleTaskTest, RepeatScheduleHandle_OnlyOne)
{
    cancelScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);
    Reply reply = task.repeatScheduleHandle(sch, true);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

TEST(CancelScheduleTaskTest, RepeatScheduleHandle_All_InvalidRecurrenceId)
{
    cancelScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, deleteScheduleByScheduleID), stub_deleteScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);
    Reply reply = task.repeatScheduleHandle(sch, false);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

// ============================================================================
// confirwScheduleHandle
// ============================================================================
TEST(CancelScheduleTaskTest, ConfirwScheduleHandle)
{
    cancelScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, deleteScheduleByScheduleID), stub_deleteScheduleByScheduleID);
    Reply reply = task.confirwScheduleHandle(sch);
    EXPECT_FALSE(reply.ttsMessage().isEmpty());
}

// ============================================================================
// deleteRepeatSchedule
// ============================================================================
TEST(CancelScheduleTaskTest, DeleteRepeatSchedule_OnlyOne)
{
    cancelScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);
    task.deleteRepeatSchedule(sch, true);
    SUCCEED();
}

TEST(CancelScheduleTaskTest, DeleteRepeatSchedule_All_InvalidRecurrenceId)
{
    cancelScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, deleteScheduleByScheduleID), stub_deleteScheduleByScheduleID);
    task.deleteRepeatSchedule(sch, false);
    SUCCEED();
}

TEST(CancelScheduleTaskTest, DeleteRepeatSchedule_All_ValidRecurrenceId)
{
    cancelScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);
    task.deleteRepeatSchedule(sch, false);
    SUCCEED();
}

// ============================================================================
// deleteOrdinarySchedule
// ============================================================================
TEST(CancelScheduleTaskTest, DeleteOrdinarySchedule)
{
    cancelScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, deleteScheduleByScheduleID), stub_deleteScheduleByScheduleID);
    task.deleteOrdinarySchedule(sch);
    SUCCEED();
}

// ============================================================================
// slotButtonCheckNum
// ============================================================================
TEST(CancelScheduleTaskTest, SlotButtonCheckNum_ButtonCount2_Index1)
{
    cancelScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    data->setSelectInfo(makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                  QDateTime(QDate(2026, 6, 1), QTime(11, 0))));
    current->setLocalData(data);
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, deleteScheduleByScheduleID), stub_deleteScheduleByScheduleID);
    task.slotButtonCheckNum(1, QString(), 2);
    SUCCEED();
}

TEST(CancelScheduleTaskTest, SlotButtonCheckNum_ButtonCount3_Index1)
{
    cancelScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    data->setSelectInfo(makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                  QDateTime(QDate(2026, 6, 1), QTime(11, 0))));
    current->setLocalData(data);
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);
    stub.set(ADDR(DScheduleDataManager, deleteScheduleByScheduleID), stub_deleteScheduleByScheduleID);
    task.slotButtonCheckNum(1, QString(), 3);
    SUCCEED();
}

TEST(CancelScheduleTaskTest, SlotButtonCheckNum_ButtonCount3_Index2)
{
    cancelScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    data->setSelectInfo(makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                  QDateTime(QDate(2026, 6, 1), QTime(11, 0))));
    current->setLocalData(data);
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    stub.set(ADDR(DScheduleDataManager, updateSchedule), stub_updateSchedule);
    stub.set(ADDR(DScheduleDataManager, deleteScheduleByScheduleID), stub_deleteScheduleByScheduleID);
    task.slotButtonCheckNum(2, QString(), 3);
    SUCCEED();
}

TEST(CancelScheduleTaskTest, SlotButtonCheckNum_Index0)
{
    cancelScheduleTask task;
    task.slotButtonCheckNum(0, QString(), 2);
    SUCCEED();
}

// ============================================================================
// slotSelectScheduleIndex
// ============================================================================
TEST(CancelScheduleTaskTest, SlotSelectScheduleIndex_ValidIndex)
{
    cancelScheduleTask task;
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

TEST(CancelScheduleTaskTest, SlotSelectScheduleIndex_InvalidIndex)
{
    cancelScheduleTask task;
    scheduleState *current = task.getCurrentState();
    CLocalData::Ptr data(new CLocalData());
    DSchedule::List list;
    data->setScheduleInfoVector(list);
    current->setLocalData(data);
    task.slotSelectScheduleIndex(1);
    SUCCEED();
}

// ============================================================================
// Widget-creating methods (need QApplication)
// ============================================================================
TEST(CancelScheduleTaskTest, GetRepeatReply)
{
    cancelScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    Reply reply = task.getRepeatReply(sch);
    EXPECT_TRUE(true);
}

TEST(CancelScheduleTaskTest, GetConfirwScheduleReply)
{
    cancelScheduleTask task;
    DSchedule::Ptr sch = makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                                   QDateTime(QDate(2026, 6, 1), QTime(11, 0)));
    Reply reply = task.getConfirwScheduleReply(sch);
    EXPECT_TRUE(true);
}

TEST(CancelScheduleTaskTest, GetListScheduleReply)
{
    cancelScheduleTask task;
    DSchedule::List list;
    list.append(makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                         QDateTime(QDate(2026, 6, 1), QTime(11, 0))));
    Reply reply = task.getListScheduleReply(list);
    EXPECT_TRUE(true);
}

TEST(CancelScheduleTaskTest, CreateRepeatWidget)
{
    cancelScheduleTask task;
    DSchedule::Ptr info(new DSchedule());
    info->setSummary("test");
    QWidget *w1 = task.createRepeatWidget(info);
    EXPECT_NE(w1, nullptr);
    delete w1;
    SUCCEED();
}

TEST(CancelScheduleTaskTest, CreateConfirmWidget)
{
    cancelScheduleTask task;
    DSchedule::Ptr info2(new DSchedule());
    info2->setSummary("test2");
    QWidget *w2 = task.createConfirmWidget(info2);
    EXPECT_NE(w2, nullptr);
    delete w2;
    SUCCEED();
}
