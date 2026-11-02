// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_csystemdtimercontrol.h"
#include "../third-party_stub/stub.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QProcess>
#include <QCoreApplication>

static QString g_execDirectResult;
static bool g_execDirectCalled = false;

QString stub_execDirectCommand(CSystemdTimerControl *, const QString &program, const QStringList &args)
{
    Q_UNUSED(program)
    Q_UNUSED(args)
    g_execDirectCalled = true;
    return g_execDirectResult;
}

test_csystemdtimercontrol::test_csystemdtimercontrol()
{
    m_timerControl = new CSystemdTimerControl();
}

test_csystemdtimercontrol::~test_csystemdtimercontrol()
{
    delete m_timerControl;
}

// --- Constructor / Destructor ---

TEST_F(test_csystemdtimercontrol, Constructor)
{
    EXPECT_NE(nullptr, m_timerControl);
    EXPECT_FALSE(m_timerControl->m_systemdPath.isEmpty());
}

TEST_F(test_csystemdtimercontrol, Destructor)
{
    CSystemdTimerControl *obj = new CSystemdTimerControl();
    delete obj;
    SUCCEED();
}

// --- createPath ---

TEST_F(test_csystemdtimercontrol, CreatePath)
{
    m_timerControl->m_systemdPath.clear();
    m_timerControl->createPath();
    EXPECT_FALSE(m_timerControl->m_systemdPath.isEmpty());
    EXPECT_TRUE(m_timerControl->m_systemdPath.endsWith("/systemd/user/"));
    QDir dir(m_timerControl->m_systemdPath);
    EXPECT_TRUE(dir.exists());
}

// --- buildingConfiggure ---

TEST_F(test_csystemdtimercontrol, BuildingConfiggure_EmptyVector)
{
    QVector<SystemDInfo> empty;
    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->buildingConfiggure(empty);
    EXPECT_FALSE(g_execDirectCalled);
}

TEST_F(test_csystemdtimercontrol, BuildingConfiggure_NonEmpty)
{
    QVector<SystemDInfo> infos;
    SystemDInfo info;
    info.accountID = "account1234567890";
    info.alarmID = "alarm1234567890";
    info.laterCount = 1;
    info.triggerTimer = QDateTime::currentDateTime().addSecs(3600);
    infos.append(info);

    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->buildingConfiggure(infos);

    EXPECT_TRUE(g_execDirectCalled);
    QString expectedName = QString("calendar-remind-%1-%2-%3")
        .arg(info.accountID.mid(0, 8))
        .arg(info.alarmID.mid(0, 8))
        .arg(info.laterCount);
    QFileInfo svcFile(m_timerControl->m_systemdPath + expectedName + ".service");
    EXPECT_TRUE(svcFile.exists());
    QFileInfo timerFile(m_timerControl->m_systemdPath + expectedName + ".timer");
    EXPECT_TRUE(timerFile.exists());
}

// --- stopSystemdTimerByJobInfos ---

TEST_F(test_csystemdtimercontrol, StopSystemdTimerByJobInfos)
{
    QVector<SystemDInfo> infos;
    SystemDInfo info;
    info.accountID = "acct1234567890";
    info.alarmID = "alrm1234567890";
    info.laterCount = 2;
    infos.append(info);

    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->stopSystemdTimerByJobInfos(infos);
    EXPECT_TRUE(g_execDirectCalled);
}

TEST_F(test_csystemdtimercontrol, StopSystemdTimerByJobInfos_Empty)
{
    QVector<SystemDInfo> empty;
    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->stopSystemdTimerByJobInfos(empty);
    EXPECT_FALSE(g_execDirectCalled);
}

// --- stopSystemdTimerByJobInfo ---

TEST_F(test_csystemdtimercontrol, StopSystemdTimerByJobInfo)
{
    SystemDInfo info;
    info.accountID = "acct1234567890";
    info.alarmID = "alrm1234567890";
    info.laterCount = 3;

    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->stopSystemdTimerByJobInfo(info);
    EXPECT_TRUE(g_execDirectCalled);
}

// --- startSystemdTimer ---

TEST_F(test_csystemdtimercontrol, StartSystemdTimer_Empty)
{
    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->startSystemdTimer(QStringList());
    EXPECT_FALSE(g_execDirectCalled);
}

TEST_F(test_csystemdtimercontrol, StartSystemdTimer_NonEmpty)
{
    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->startSystemdTimer(QStringList() << "test-timer-1" << "test-timer-2");
    EXPECT_TRUE(g_execDirectCalled);
}

// --- stopSystemdTimer ---

TEST_F(test_csystemdtimercontrol, StopSystemdTimer_Empty)
{
    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->stopSystemdTimer(QStringList());
    EXPECT_FALSE(g_execDirectCalled);
}

TEST_F(test_csystemdtimercontrol, StopSystemdTimer_NonEmpty)
{
    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->stopSystemdTimer(QStringList() << "test-timer-1" << "test-timer-2");
    EXPECT_TRUE(g_execDirectCalled);
}

// --- removeFile ---

TEST_F(test_csystemdtimercontrol, RemoveFile)
{
    QString tempPath = m_timerControl->m_systemdPath + "test_removefile_tmp.txt";
    {
        QFile f(tempPath);
        f.open(QIODevice::WriteOnly);
        f.write("test");
        f.close();
    }
    EXPECT_TRUE(QFile::exists(tempPath));
    m_timerControl->removeFile(QStringList() << tempPath);
    EXPECT_FALSE(QFile::exists(tempPath));
}

TEST_F(test_csystemdtimercontrol, RemoveFile_NonExistent)
{
    QString nonExistent = m_timerControl->m_systemdPath + "nonexistent_file_12345.txt";
    m_timerControl->removeFile(QStringList() << nonExistent);
    SUCCEED();
}

// --- stopAllRemindSystemdTimer ---

TEST_F(test_csystemdtimercontrol, StopAllRemindSystemdTimer)
{
    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->stopAllRemindSystemdTimer("account1234567890");
    EXPECT_TRUE(g_execDirectCalled);
}

// --- removeRemindFile ---

TEST_F(test_csystemdtimercontrol, RemoveRemindFile_WithMatchingFiles)
{
    QString prefix = "calendar-remind-" + QString("account1234567890").mid(0, 8);
    QString file1 = m_timerControl->m_systemdPath + prefix + "-test1.timer";
    QString file2 = m_timerControl->m_systemdPath + prefix + "-test2.service";
    {
        QFile f1(file1);
        f1.open(QIODevice::WriteOnly);
        f1.write("test");
        f1.close();
        QFile f2(file2);
        f2.open(QIODevice::WriteOnly);
        f2.write("test");
        f2.close();
    }
    EXPECT_TRUE(QFile::exists(file1));
    EXPECT_TRUE(QFile::exists(file2));

    m_timerControl->removeRemindFile("account1234567890");

    EXPECT_FALSE(QFile::exists(file1));
    EXPECT_FALSE(QFile::exists(file2));
}

TEST_F(test_csystemdtimercontrol, RemoveRemindFile_NoMatchingFiles)
{
    m_timerControl->removeRemindFile("nonexistent_account_9999");
    SUCCEED();
}

// --- startCalendarServiceSystemdTimer ---

TEST_F(test_csystemdtimercontrol, StartCalendarServiceSystemdTimer)
{
    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);

    QString svcPath = m_timerControl->m_systemdPath + "com.dde.calendarserver.calendar.service";
    QString timerPath = m_timerControl->m_systemdPath + "com.dde.calendarserver.calendar.timer";
    {
        QFile f(svcPath); f.open(QIODevice::WriteOnly); f.close();
        QFile f2(timerPath); f2.open(QIODevice::WriteOnly); f2.close();
    }

    QString wantsTimer = m_timerControl->m_systemdPath + "timers.target.wants/com.dde.calendarserver.calendar.timer";
    QFile::remove(wantsTimer);

    m_timerControl->startCalendarServiceSystemdTimer();

    EXPECT_TRUE(g_execDirectCalled);
    EXPECT_FALSE(QFile::exists(svcPath));
    EXPECT_FALSE(QFile::exists(timerPath));
}

// --- startDownloadTask ---

TEST_F(test_csystemdtimercontrol, StartDownloadTask)
{
    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    QString accountID = "dlaccount1234567";
    m_timerControl->startDownloadTask(accountID, 30);
    EXPECT_TRUE(g_execDirectCalled);

    QString serviceName = accountID + "_calendar";
    QFileInfo svcFile(m_timerControl->m_systemdPath + serviceName + ".service");
    EXPECT_TRUE(svcFile.exists());
    QFileInfo timerFile(m_timerControl->m_systemdPath + serviceName + ".timer");
    EXPECT_TRUE(timerFile.exists());
}

// --- stopDownloadTask ---

TEST_F(test_csystemdtimercontrol, StopDownloadTask)
{
    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    QString accountID = "dlaccount1234567";
    m_timerControl->stopDownloadTask(accountID);
    EXPECT_TRUE(g_execDirectCalled);
}

// --- startUploadTask ---

TEST_F(test_csystemdtimercontrol, StartUploadTask_NotActive)
{
    Stub stub;
    g_execDirectResult = "inactive";
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->startUploadTask(15);
    EXPECT_TRUE(g_execDirectCalled);
}

TEST_F(test_csystemdtimercontrol, StartUploadTask_AlreadyActive)
{
    Stub stub;
    g_execDirectResult = "active";
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->startUploadTask(15);
    EXPECT_TRUE(g_execDirectCalled);
}

// --- stopUploadTask ---

TEST_F(test_csystemdtimercontrol, StopUploadTask)
{
    Stub stub;
    g_execDirectCalled = false;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    m_timerControl->stopUploadTask();
    EXPECT_TRUE(g_execDirectCalled);
}

// --- createFile ---

TEST_F(test_csystemdtimercontrol, CreateFile)
{
    QString filePath = m_timerControl->m_systemdPath + "test_createfile_tmp.txt";
    QString content = "test content line1\ntest content line2";
    m_timerControl->createFile(filePath, content);

    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly | QIODevice::Text));
    QString readContent = QString::fromUtf8(f.readAll());
    f.close();
    EXPECT_EQ(content, readContent);
    QFile::remove(filePath);
}

// --- loadTemplateFile ---

TEST_F(test_csystemdtimercontrol, LoadTemplateFile_NonExistentResource)
{
    QString result = m_timerControl->loadTemplateFile("nonexistent_template_file.txt");
    EXPECT_TRUE(result.isEmpty());
}

// --- createServiceFromTemplate ---

TEST_F(test_csystemdtimercontrol, CreateServiceFromTemplate)
{
    Stub stub;
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    QString name = "test_svc_template";
    QString command = "echo hello";
    m_timerControl->createServiceFromTemplate(name, command);

    QFile f(m_timerControl->m_systemdPath + name + ".service");
    ASSERT_TRUE(f.exists());
    ASSERT_TRUE(f.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(f.readAll());
    f.close();
    EXPECT_TRUE(content.contains(command));
    QFile::remove(m_timerControl->m_systemdPath + name + ".service");
}

// --- createTimerFromTemplate ---

TEST_F(test_csystemdtimercontrol, CreateTimerFromTemplate)
{
    QString name = "test_timer_template";
    QDateTime trigger = QDateTime::currentDateTime().addSecs(1800);
    m_timerControl->createTimerFromTemplate(name, trigger);

    QFile f(m_timerControl->m_systemdPath + name + ".timer");
    ASSERT_TRUE(f.exists());
    ASSERT_TRUE(f.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(f.readAll());
    f.close();
    EXPECT_TRUE(content.contains(trigger.toString("yyyy-MM-dd hh:mm:ss")));
    QFile::remove(m_timerControl->m_systemdPath + name + ".timer");
}

// --- createPeriodicTimerFromTemplate ---

TEST_F(test_csystemdtimercontrol, CreatePeriodicTimerFromTemplate)
{
    QString name = "test_periodic_timer";
    int interval = 30;
    m_timerControl->createPeriodicTimerFromTemplate(name, interval);

    QFile f(m_timerControl->m_systemdPath + name + ".timer");
    ASSERT_TRUE(f.exists());
    ASSERT_TRUE(f.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(f.readAll());
    f.close();
    EXPECT_TRUE(content.contains(QString("OnUnitInactiveSec=%1m").arg(interval)));
    QFile::remove(m_timerControl->m_systemdPath + name + ".timer");
}

// --- execSystemdCommand ---

TEST_F(test_csystemdtimercontrol, ExecSystemdCommand)
{
    Stub stub;
    g_execDirectResult = "ok";
    stub.set(ADDR(CSystemdTimerControl, execDirectCommand), stub_execDirectCommand);
    QString result = m_timerControl->execSystemdCommand(QStringList() << "status" << "test.timer");
    EXPECT_EQ(QString("ok"), result);
}

// --- execDirectCommand ---

TEST_F(test_csystemdtimercontrol, ExecDirectCommand_RealProcess)
{
    QString result = m_timerControl->execDirectCommand("echo", QStringList() << "hello_world_test");
    EXPECT_TRUE(result.contains("hello_world_test"));
}

TEST_F(test_csystemdtimercontrol, ExecDirectCommand_NonExistentProgram)
{
    QString result = m_timerControl->execDirectCommand("nonexistent_program_xyz_123", QStringList());
    EXPECT_TRUE(result.isEmpty());
}
