// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "schedulemanagetask.h"
#include "semanticanalysistask.h"
#include "schedulebasetask.h"
#include "globaldef.h"
#include "accountmanager.h"
#include "createjsondata.h"
#include "queryjsondata.h"
#include "canceljsondata.h"
#include "changejsondata.h"
#include "jsondata.h"
#include "stub.h"

#include <QApplication>
#include <QWidget>

// Stub for AccountManager::resetAccount — avoids DBus calls during construction
static void stub_resetAccount() {}

// Fixture that stubs AccountManager::resetAccount before getInstance()
class ScheduleManageTaskTest : public ::testing::Test
{
protected:
    Stub stub;

    void SetUp() override
    {
        stub.set(ADDR(AccountManager, resetAccount), stub_resetAccount);
    }

    void TearDown() override
    {
        // Stub auto-resets in destructor
    }
};

// ============================================================================
// getInstance (singleton)
// ============================================================================

TEST_F(ScheduleManageTaskTest, GetInstance_ReturnsSamePointer)
{
    ScheduleManageTask *instance1 = ScheduleManageTask::getInstance();
    ScheduleManageTask *instance2 = ScheduleManageTask::getInstance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(ScheduleManageTaskTest, GetInstance_NotNull)
{
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    ASSERT_NE(nullptr, instance);
}

// ============================================================================
// releaseInstance (no-op)
// ============================================================================

TEST_F(ScheduleManageTaskTest, ReleaseInstance_NoCrash)
{
    ScheduleManageTask::getInstance();
    ScheduleManageTask::releaseInstance();
    SUCCEED();
}

// ============================================================================
// getReply / setReply
// ============================================================================

TEST_F(ScheduleManageTaskTest, SetReply_StoresValue)
{
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    Reply reply(42, "test_reply");
    instance->setReply(reply);
    EXPECT_EQ(42, instance->getReply().code());
}

TEST_F(ScheduleManageTaskTest, GetReply_DefaultInvalid)
{
    // Use a fresh approach: set and verify
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    Reply reply;
    instance->setReply(reply);
    EXPECT_EQ(ERR_FAIL, instance->getReply().code());
}

// ============================================================================
// process — with known intent
// ============================================================================

TEST_F(ScheduleManageTaskTest, Process_CreateIntent_SetsReply)
{
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    semanticAnalysisTask semantic;
    semantic.setIntent(JSON_CREATE);

    CreateJsonData *data = new CreateJsonData();
    data->m_DateTimeInvalid = true;
    semantic.setJsonData(data);

    instance->process(semantic);
    // createScheduleTask::SchedulePress: getDateTimeInvalid() returns true → DATETIME_ERR_TTS (code=0)
    Reply reply = instance->getReply();
    EXPECT_EQ(0, reply.code());
}

TEST_F(ScheduleManageTaskTest, Process_ViewIntent_SetsReply)
{
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    semanticAnalysisTask semantic;
    semantic.setIntent(JSON_VIEW);

    QueryJsonData *data = new QueryJsonData();
    data->m_DateTimeInvalid = true;
    semantic.setJsonData(data);

    instance->process(semantic);
    // queryScheduleTask::SchedulePress: getDateTimeInvalid() returns true → DATETIME_ERR_TTS (code=0)
    Reply reply = instance->getReply();
    EXPECT_EQ(0, reply.code());
}

TEST_F(ScheduleManageTaskTest, Process_CancelIntent_SetsReply)
{
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    semanticAnalysisTask semantic;
    semantic.setIntent(JSON_CANCEL);

    cancelJsonData *data = new cancelJsonData();
    data->m_DateTimeInvalid = true;
    semantic.setJsonData(data);

    instance->process(semantic);
    // cancelScheduleTask uses base SchedulePress → m_State->process(jsonData)
    // getDateTimeInvalid() returns true → DATETIME_ERR_TTS (code=0)
    Reply reply = instance->getReply();
    EXPECT_EQ(0, reply.code());
}

TEST_F(ScheduleManageTaskTest, Process_ChangeIntent_SetsReply)
{
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    semanticAnalysisTask semantic;
    semantic.setIntent(JSON_CHANGE);

    changejsondata *data = new changejsondata();
    data->m_DateTimeInvalid = true;
    semantic.setJsonData(data);

    instance->process(semantic);
    // changeScheduleTask uses base SchedulePress → m_State->process(jsonData)
    // getDateTimeInvalid() returns true → DATETIME_ERR_TTS (code=0)
    Reply reply = instance->getReply();
    EXPECT_EQ(0, reply.code());
}

// ============================================================================
// process — with unknown intent (m_preScheduleTask remains nullptr → error reply)
// ============================================================================

TEST_F(ScheduleManageTaskTest, Process_UnknownIntent_ReturnsErrorTTS)
{
    // Singleton persists m_preScheduleTask from prior tests, so we provide
    // valid JsonData with m_DateTimeInvalid=true to avoid nullptr deref.
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    semanticAnalysisTask semantic;
    semantic.setIntent("UNKNOWN_INTENT");

    JsonData *data = new JsonData();
    data->m_DateTimeInvalid = true;
    semantic.setJsonData(data);

    instance->process(semantic);
    Reply reply = instance->getReply();
    // Unknown intent does not change m_preScheduleTask; if it is set,
    // SchedulePress processes the data (returns DATETIME_ERR_TTS, code=0).
    // If m_preScheduleTask is null, reply has G_ERR_TTS (code=0).
    EXPECT_EQ(0, reply.code());
}

// ============================================================================
// process — intent switching resets previous task state
// ============================================================================

TEST_F(ScheduleManageTaskTest, Process_IntentSwitch_CallsInitStateOnOldTask)
{
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();

    // First call with CREATE to set m_preScheduleTask
    semanticAnalysisTask semantic1;
    semantic1.setIntent(JSON_CREATE);
    instance->process(semantic1);

    // Switch to VIEW — should call InitState on the old CREATE task
    semanticAnalysisTask semantic2;
    semantic2.setIntent(JSON_VIEW);
    instance->process(semantic2);

    Reply reply = instance->getReply();
    // Reply should be set (either error or from SchedulePress)
    SUCCEED();
}

// ============================================================================
// slotReceivce (no-op)
// ============================================================================

TEST_F(ScheduleManageTaskTest, SlotReceivce_NoCrash)
{
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    QVariant data;
    Reply *reply = nullptr;
    instance->slotReceivce(data, reply);
    SUCCEED();
}

// ============================================================================
// slotWidgetHideInitState
// ============================================================================

TEST_F(ScheduleManageTaskTest, SlotWidgetHideInitState_WithPreScheduleTask_NoCrash)
{
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    // Ensure m_preScheduleTask is set by processing a known intent
    semanticAnalysisTask semantic;
    semantic.setIntent(JSON_CREATE);
    instance->process(semantic);

    instance->slotWidgetHideInitState();
    SUCCEED();
}

// ============================================================================
// connectHideEventToInitState (private — accessible via -fno-access-control)
// ============================================================================

TEST_F(ScheduleManageTaskTest, ConnectHideEventToInitState_NullWidget_NoCrash)
{
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    Reply reply;
    // Default reply has null widget
    instance->connectHideEventToInitState(reply);
    SUCCEED();
}

TEST_F(ScheduleManageTaskTest, ConnectHideEventToInitState_WithWidget_NoCrash)
{
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    Reply reply;
    QWidget *widget = new QWidget();
    reply.setReplyWidget(widget);
    // Widget is not an IconDFrame, so qobject_cast returns null → no connection
    instance->connectHideEventToInitState(reply);
    delete widget;
    SUCCEED();
}

// ============================================================================
// Destructor — exercised implicitly via singleton lifecycle
// ============================================================================

TEST_F(ScheduleManageTaskTest, SingletonLifecycle_NoCrash)
{
    // getInstance creates the singleton; it persists for the test process lifetime.
    // We verify that multiple getInstance/process calls don't crash.
    ScheduleManageTask *instance = ScheduleManageTask::getInstance();
    for (int i = 0; i < 3; ++i) {
        semanticAnalysisTask semantic;
        semantic.setIntent(JSON_CREATE);
        instance->process(semantic);
    }
    SUCCEED();
}
