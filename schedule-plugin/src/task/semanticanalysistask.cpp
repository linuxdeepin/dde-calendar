// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "semanticanalysistask.h"
#include "commondef.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "../data/createjsondata.h"
#include "../data/queryjsondata.h"
#include "../data/canceljsondata.h"
#include "../data/changejsondata.h"
#include "../globaldef.h"

semanticAnalysisTask::semanticAnalysisTask(QObject *parent)
    : QObject(parent)
{
}

semanticAnalysisTask::~semanticAnalysisTask()
{
    deleteJsonData();
}

bool semanticAnalysisTask::resolveTaskJson(const QString &semantic)
{
    setIntent("");
    deleteJsonData();

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(semantic.toUtf8(), &jsonError);

    if (!doc.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        qCDebug(PluginLogger) << "JSON parsed successfully";
        auto rootObject = doc.object();
        if (!(rootObject.contains("intent")
              && rootObject["intent"].isObject())) {
            qCWarning(PluginLogger) << "Invalid JSON structure - missing or invalid intent object";
            return false;
        }

        auto intentObject = rootObject["intent"].toObject();
        if (intentObject.empty()) {
            qCWarning(PluginLogger) << "Empty intent object";
            return false;
        }

        if (intentObject.contains("semantic") && intentObject["semantic"].isArray()) {
            qCDebug(PluginLogger) << "Processing semantic array";
            auto semanticObjArr = intentObject["semantic"].toArray();
            for (int i = 0; i < semanticObjArr.size(); ++i) {
                auto semanticObj = semanticObjArr[i].toObject();
                if (semanticObj.contains("intent") && semanticObj["intent"].isString()) {
                    setIntent(semanticObj["intent"].toString());
                }
                m_JsonData = createJsonDataFactory(Intent());
                if (m_JsonData != nullptr) {
                    qCDebug(PluginLogger) << "Created JSON data handler for intent:" << Intent();
                    m_JsonData->JosnResolve(semanticObj);
                }
            }
        }

        if (intentObject.contains("voice_answer") && intentObject["voice_answer"].isArray()) {
            qCDebug(PluginLogger) << "Processing voice answer array";
            auto voiceAnsObjArr = intentObject["voice_answer"].toArray();
            for (int i = 0; i < voiceAnsObjArr.size(); ++i) {
                auto voiceAnsObj = voiceAnsObjArr[i].toObject();
                if (voiceAnsObj.contains("type") && voiceAnsObj["type"].isString()) {
                    if (voiceAnsObj["type"] != "TTS") {
                        continue;
                    }
                    if (voiceAnsObj.contains("content") && voiceAnsObj["content"].isString()) {
                        //语音播报的文本
                        if (m_JsonData != nullptr) {
                            m_JsonData->setSuggestMsg(voiceAnsObj["content"].toString());
                        }
                        break;
                    }
                }
            }
        }

        if (intentObject.contains("shouldEndSession") && intentObject["shouldEndSession"].isBool()) {
            setShouldEndSession(intentObject["shouldEndSession"].toBool());
        } else {
            qCDebug(PluginLogger) << "No shouldEndSession specified, defaulting to true";
            setShouldEndSession(true);
        }
    } else {
        qCWarning(PluginLogger) << "JSON parse error:" << jsonError.errorString();
        return false;
    }
    return true;
}

QString semanticAnalysisTask::Intent() const
{
    return m_Intent;
}

void semanticAnalysisTask::setIntent(const QString &Intent)
{
    m_Intent = Intent;
}

JsonData *semanticAnalysisTask::getJsonData() const
{
    return m_JsonData;
}

void semanticAnalysisTask::deleteJsonData()
{
    if (m_JsonData != nullptr) {
        qCDebug(PluginLogger) << "Deleting JSON data handler";
        delete m_JsonData;
        m_JsonData = nullptr;
    }
}

JsonData *semanticAnalysisTask::createJsonDataFactory(const QString &Intent)
{
    qCDebug(PluginLogger) << "Creating JSON data handler for intent:" << Intent;
    JsonData *data = nullptr;
    if (Intent == JSON_CREATE) {
        //创建
        qCDebug(PluginLogger) << "Creating CreateJsonData handler";
        data = new CreateJsonData();
    } else if (Intent == JSON_VIEW) {
        //查询
        qCDebug(PluginLogger) << "Creating QueryJsonData handler";
        data = new QueryJsonData();
    } else if (Intent == JSON_CANCEL) {
        //取消
        qCDebug(PluginLogger) << "Creating cancelJsonData handler";
        data = new cancelJsonData();
    } else if (Intent == JSON_CHANGE) {
        //改变
        qCDebug(PluginLogger) << "Creating changejsondata handler";
        data = new changejsondata();
    } else {
        qCWarning(PluginLogger) << "Unknown intent type:" << Intent;
    }
    return data;
}

void semanticAnalysisTask::setShouldEndSession(bool isEnd)
{
    if (m_JsonData != nullptr) {
        qCDebug(PluginLogger) << "Setting shouldEndSession to:" << isEnd;
        m_JsonData->setShouldEndSession(isEnd);
    }
}

void semanticAnalysisTask::setJsonData(JsonData *JsonData)
{
    m_JsonData = JsonData;
}
