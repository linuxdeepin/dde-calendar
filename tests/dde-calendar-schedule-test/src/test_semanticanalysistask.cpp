// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "semanticanalysistask.h"
#include "createjsondata.h"
#include "queryjsondata.h"
#include "canceljsondata.h"
#include "changejsondata.h"
#include "globaldef.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// Helper to build a semantic JSON string
static QString makeSemanticJson(const QString &intent,
                                const QString &ttsContent = QString(),
                                bool hasShouldEndSession = false,
                                bool shouldEndSession = true)
{
    QJsonObject root;
    QJsonObject intentObj;

    if (!intent.isEmpty()) {
        QJsonArray semanticArr;
        QJsonObject semanticItem;
        semanticItem["intent"] = intent;
        semanticArr.append(semanticItem);
        intentObj["semantic"] = semanticArr;
    }

    if (!ttsContent.isEmpty()) {
        QJsonArray voiceAnswerArr;
        QJsonObject voiceItem;
        voiceItem["type"] = "TTS";
        voiceItem["content"] = ttsContent;
        voiceAnswerArr.append(voiceItem);
        intentObj["voice_answer"] = voiceAnswerArr;
    }

    if (hasShouldEndSession) {
        intentObj["shouldEndSession"] = shouldEndSession;
    }

    root["intent"] = intentObj;
    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

TEST(SemanticAnalysisTaskTest, Constructor_NoCrash)
{
    semanticAnalysisTask task;
    EXPECT_EQ(QString(), task.Intent());
    EXPECT_EQ(nullptr, task.getJsonData());
}

TEST(SemanticAnalysisTaskTest, Destructor_CleansUpJsonData)
{
    semanticAnalysisTask *task = new semanticAnalysisTask();
    JsonData *data = new CreateJsonData();
    task->setJsonData(data);
    // Destructor should call deleteJsonData which deletes m_JsonData
    delete task;
    SUCCEED();
}

TEST(SemanticAnalysisTaskTest, Destructor_WithNullJsonData_NoCrash)
{
    semanticAnalysisTask *task = new semanticAnalysisTask();
    delete task;
    SUCCEED();
}

// ============================================================================
// Intent / setIntent
// ============================================================================

TEST(SemanticAnalysisTaskTest, SetIntent_StoresValue)
{
    semanticAnalysisTask task;
    task.setIntent(JSON_CREATE);
    EXPECT_EQ(QString(JSON_CREATE), task.Intent());
}

TEST(SemanticAnalysisTaskTest, Intent_DefaultEmpty)
{
    semanticAnalysisTask task;
    EXPECT_EQ(QString(), task.Intent());
}

// ============================================================================
// getJsonData / setJsonData
// ============================================================================

TEST(SemanticAnalysisTaskTest, SetJsonData_StoresValue)
{
    semanticAnalysisTask task;
    JsonData *data = new CreateJsonData();
    task.setJsonData(data);
    EXPECT_EQ(data, task.getJsonData());
    // Clean up — destructor will delete it
}

TEST(SemanticAnalysisTaskTest, GetJsonData_DefaultNull)
{
    semanticAnalysisTask task;
    EXPECT_EQ(nullptr, task.getJsonData());
}

// ============================================================================
// resolveTaskJson — invalid JSON
// ============================================================================

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_InvalidJson_ReturnsFalse)
{
    semanticAnalysisTask task;
    EXPECT_FALSE(task.resolveTaskJson("not a json"));
}

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_EmptyJson_ReturnsFalse)
{
    semanticAnalysisTask task;
    // Empty JSON object — no "intent" key → returns false
    EXPECT_FALSE(task.resolveTaskJson("{}"));
}

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_IntentNotObject_ReturnsFalse)
{
    semanticAnalysisTask task;
    QJsonObject root;
    root["intent"] = "not_an_object";
    QString jsonStr = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
    EXPECT_FALSE(task.resolveTaskJson(jsonStr));
}

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_EmptyIntentObject_ReturnsFalse)
{
    semanticAnalysisTask task;
    QJsonObject root;
    QJsonObject emptyIntent;
    root["intent"] = emptyIntent;
    QString jsonStr = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
    EXPECT_FALSE(task.resolveTaskJson(jsonStr));
}

// ============================================================================
// resolveTaskJson — valid JSON with various intents
// ============================================================================

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_CreateIntent_SetsIntentAndJsonData)
{
    semanticAnalysisTask task;
    QString json = makeSemanticJson(JSON_CREATE, "hello", true, true);
    EXPECT_TRUE(task.resolveTaskJson(json));
    EXPECT_EQ(QString(JSON_CREATE), task.Intent());
    ASSERT_NE(nullptr, task.getJsonData());
    // createJsonDataFactory returns CreateJsonData for JSON_CREATE
    EXPECT_NE(nullptr, dynamic_cast<CreateJsonData *>(task.getJsonData()));
}

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_ViewIntent_CreatesQueryJsonData)
{
    semanticAnalysisTask task;
    QString json = makeSemanticJson(JSON_VIEW);
    EXPECT_TRUE(task.resolveTaskJson(json));
    EXPECT_EQ(QString(JSON_VIEW), task.Intent());
    ASSERT_NE(nullptr, task.getJsonData());
    EXPECT_NE(nullptr, dynamic_cast<QueryJsonData *>(task.getJsonData()));
}

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_CancelIntent_CreatesCancelJsonData)
{
    semanticAnalysisTask task;
    QString json = makeSemanticJson(JSON_CANCEL);
    EXPECT_TRUE(task.resolveTaskJson(json));
    EXPECT_EQ(QString(JSON_CANCEL), task.Intent());
    ASSERT_NE(nullptr, task.getJsonData());
    EXPECT_NE(nullptr, dynamic_cast<cancelJsonData *>(task.getJsonData()));
}

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_ChangeIntent_CreatesChangeJsonData)
{
    semanticAnalysisTask task;
    QString json = makeSemanticJson(JSON_CHANGE);
    EXPECT_TRUE(task.resolveTaskJson(json));
    EXPECT_EQ(QString(JSON_CHANGE), task.Intent());
    ASSERT_NE(nullptr, task.getJsonData());
    EXPECT_NE(nullptr, dynamic_cast<changejsondata *>(task.getJsonData()));
}

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_UnknownIntent_NoJsonData)
{
    semanticAnalysisTask task;
    QString json = makeSemanticJson("UNKNOWN");
    EXPECT_TRUE(task.resolveTaskJson(json));
    EXPECT_EQ(QString("UNKNOWN"), task.Intent());
    EXPECT_EQ(nullptr, task.getJsonData());
}

// ============================================================================
// resolveTaskJson — voice_answer processing
// ============================================================================

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_WithTTSContent_SetsSuggestMsg)
{
    semanticAnalysisTask task;
    QString json = makeSemanticJson(JSON_CREATE, "test_tts_content", true, true);
    EXPECT_TRUE(task.resolveTaskJson(json));
    ASSERT_NE(nullptr, task.getJsonData());
    EXPECT_EQ(QString("test_tts_content"), task.getJsonData()->SuggestMsg());
}

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_NonTTSVoiceAnswer_Skipped)
{
    semanticAnalysisTask task;
    QJsonObject root;
    QJsonObject intentObj;
    QJsonArray semanticArr;
    QJsonObject semanticItem;
    semanticItem["intent"] = JSON_CREATE;
    semanticArr.append(semanticItem);
    intentObj["semantic"] = semanticArr;

    QJsonArray voiceAnswerArr;
    QJsonObject voiceItem;
    voiceItem["type"] = "NOT_TTS";
    voiceItem["content"] = "should_be_skipped";
    voiceAnswerArr.append(voiceItem);
    intentObj["voice_answer"] = voiceAnswerArr;

    root["intent"] = intentObj;
    QString json = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));

    EXPECT_TRUE(task.resolveTaskJson(json));
    ASSERT_NE(nullptr, task.getJsonData());
    // SuggestMsg should be empty default since non-TTS was skipped
    EXPECT_EQ(QString(""), task.getJsonData()->SuggestMsg());
}

// ============================================================================
// resolveTaskJson — shouldEndSession
// ============================================================================

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_ShouldEndSessionTrue)
{
    semanticAnalysisTask task;
    QString json = makeSemanticJson(JSON_CREATE, QString(), true, true);
    EXPECT_TRUE(task.resolveTaskJson(json));
    ASSERT_NE(nullptr, task.getJsonData());
    EXPECT_TRUE(task.getJsonData()->ShouldEndSession());
}

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_ShouldEndSessionFalse)
{
    semanticAnalysisTask task;
    QString json = makeSemanticJson(JSON_CREATE, QString(), true, false);
    EXPECT_TRUE(task.resolveTaskJson(json));
    ASSERT_NE(nullptr, task.getJsonData());
    EXPECT_FALSE(task.getJsonData()->ShouldEndSession());
}

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_NoShouldEndSession_DefaultsTrue)
{
    semanticAnalysisTask task;
    QString json = makeSemanticJson(JSON_CREATE, QString(), false);
    EXPECT_TRUE(task.resolveTaskJson(json));
    ASSERT_NE(nullptr, task.getJsonData());
    EXPECT_TRUE(task.getJsonData()->ShouldEndSession());
}

// ============================================================================
// resolveTaskJson — resets state on each call
// ============================================================================

TEST(SemanticAnalysisTaskTest, ResolveTaskJson_ResetsIntentOnNewCall)
{
    semanticAnalysisTask task;
    QString json1 = makeSemanticJson(JSON_CREATE);
    EXPECT_TRUE(task.resolveTaskJson(json1));
    EXPECT_EQ(QString(JSON_CREATE), task.Intent());

    QString json2 = makeSemanticJson(JSON_VIEW);
    EXPECT_TRUE(task.resolveTaskJson(json2));
    EXPECT_EQ(QString(JSON_VIEW), task.Intent());
}

// ============================================================================
// deleteJsonData (private — accessible via -fno-access-control)
// ============================================================================

TEST(SemanticAnalysisTaskTest, DeleteJsonData_WithValidData_DeletesAndNulls)
{
    semanticAnalysisTask task;
    JsonData *data = new CreateJsonData();
    task.setJsonData(data);
    task.deleteJsonData();
    EXPECT_EQ(nullptr, task.getJsonData());
}

TEST(SemanticAnalysisTaskTest, DeleteJsonData_WithNullData_NoCrash)
{
    semanticAnalysisTask task;
    task.deleteJsonData();
    EXPECT_EQ(nullptr, task.getJsonData());
}

TEST(SemanticAnalysisTaskTest, DeleteJsonData_CalledTwice_NoCrash)
{
    semanticAnalysisTask task;
    JsonData *data = new CreateJsonData();
    task.setJsonData(data);
    task.deleteJsonData();
    task.deleteJsonData();
    EXPECT_EQ(nullptr, task.getJsonData());
}

// ============================================================================
// createJsonDataFactory (private — accessible via -fno-access-control)
// ============================================================================

TEST(SemanticAnalysisTaskTest, CreateJsonDataFactory_Create_ReturnsCreateJsonData)
{
    semanticAnalysisTask task;
    JsonData *data = task.createJsonDataFactory(JSON_CREATE);
    ASSERT_NE(nullptr, data);
    EXPECT_NE(nullptr, dynamic_cast<CreateJsonData *>(data));
    delete data;
}

TEST(SemanticAnalysisTaskTest, CreateJsonDataFactory_View_ReturnsQueryJsonData)
{
    semanticAnalysisTask task;
    JsonData *data = task.createJsonDataFactory(JSON_VIEW);
    ASSERT_NE(nullptr, data);
    EXPECT_NE(nullptr, dynamic_cast<QueryJsonData *>(data));
    delete data;
}

TEST(SemanticAnalysisTaskTest, CreateJsonDataFactory_Cancel_ReturnsCancelJsonData)
{
    semanticAnalysisTask task;
    JsonData *data = task.createJsonDataFactory(JSON_CANCEL);
    ASSERT_NE(nullptr, data);
    EXPECT_NE(nullptr, dynamic_cast<cancelJsonData *>(data));
    delete data;
}

TEST(SemanticAnalysisTaskTest, CreateJsonDataFactory_Change_ReturnsChangeJsonData)
{
    semanticAnalysisTask task;
    JsonData *data = task.createJsonDataFactory(JSON_CHANGE);
    ASSERT_NE(nullptr, data);
    EXPECT_NE(nullptr, dynamic_cast<changejsondata *>(data));
    delete data;
}

TEST(SemanticAnalysisTaskTest, CreateJsonDataFactory_UnknownIntent_ReturnsNull)
{
    semanticAnalysisTask task;
    JsonData *data = task.createJsonDataFactory("UNKNOWN");
    EXPECT_EQ(nullptr, data);
}

// ============================================================================
// setShouldEndSession (private — accessible via -fno-access-control)
// ============================================================================

TEST(SemanticAnalysisTaskTest, SetShouldEndSession_WithJsonData_SetsValue)
{
    semanticAnalysisTask task;
    JsonData *data = new CreateJsonData();
    task.setJsonData(data);
    task.setShouldEndSession(false);
    EXPECT_FALSE(data->ShouldEndSession());
    task.setShouldEndSession(true);
    EXPECT_TRUE(data->ShouldEndSession());
}

TEST(SemanticAnalysisTaskTest, SetShouldEndSession_WithNullJsonData_NoCrash)
{
    semanticAnalysisTask task;
    task.setShouldEndSession(true);
    SUCCEED();
}
