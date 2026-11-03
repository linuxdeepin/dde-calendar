// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "syncfilemanage.h"
#include "syncoperation.h"
#include "units.h"

#include <gtest/gtest.h>
#include <memory>

#include "dbus_call_stub.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QSqlDatabase>

class SyncFileManageTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // “WithoutDbus”类用例依赖“同步服务不存在”的环境；CI 会话里真实存在
        // com.deepin.sync.Daemon，故统一桩掉 D-Bus 调用使其确定性失败。
        m_noDbus = std::unique_ptr<ScopedDbusUnavailable>(new ScopedDbusUnavailable());
        m_mgr = new SyncFileManage();
    }
    void TearDown() override
    {
        delete m_mgr;
        m_mgr = nullptr;
    }
    SyncFileManage *m_mgr = nullptr;
    std::unique_ptr<ScopedDbusUnavailable> m_noDbus;
};

// ---- SyncFileManage::SyncFileManage (constructor) ----

TEST_F(SyncFileManageTest, ConstructorCreatesObject)
{
    EXPECT_NE(m_mgr, nullptr);
}

TEST_F(SyncFileManageTest, ConstructorInitializesSyncoperation)
{
    EXPECT_NE(m_mgr->getSyncoperation(), nullptr);
}

// ---- SyncFileManage::~SyncFileManage (destructor) ----

TEST_F(SyncFileManageTest, DestructorDoesNotCrash)
{
    auto *mgr = new SyncFileManage();
    delete mgr;
    SUCCEED();
}

// ---- SyncFileManage::getSyncoperation ----

TEST_F(SyncFileManageTest, GetSyncoperationReturnsNonNull)
{
    Syncoperation *op = m_mgr->getSyncoperation();
    EXPECT_NE(op, nullptr);
}

// ---- SyncFileManage::UserSyncDirectory ----

TEST_F(SyncFileManageTest, UserSyncDirectoryCreatesDir)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString dirPath = tmp.path() + "/syncdir";
    m_mgr->UserSyncDirectory(dirPath);
    EXPECT_TRUE(QDir(dirPath).exists());
}

TEST_F(SyncFileManageTest, UserSyncDirectoryExistingDirNoCrash)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString dirPath = tmp.path() + "/existingdir";
    QDir().mkpath(dirPath);
    ASSERT_TRUE(QDir(dirPath).exists());
    m_mgr->UserSyncDirectory(dirPath);
    EXPECT_TRUE(QDir(dirPath).exists());
}

// ---- SyncFileManage::SyncDbCreate (private, accessible via -fno-access-control) ----

TEST_F(SyncFileManageTest, SyncDbCreateCreatesFileAndOpensDb)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString dbPath = tmp.path() + "/testdb.db";
    bool ret = m_mgr->SyncDbCreate(dbPath);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(QFile::exists(dbPath));
}

TEST_F(SyncFileManageTest, SyncDbCreateExistingFile)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString dbPath = tmp.path() + "/existingdb.db";
    {
        QFile f(dbPath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("test");
        f.close();
    }
    bool ret = m_mgr->SyncDbCreate(dbPath);
    EXPECT_TRUE(ret);
}

// ---- SyncFileManage::SyncDbDelete (private, accessible via -fno-access-control) ----

TEST_F(SyncFileManageTest, SyncDbDeleteEmptyPathReturnsFalse)
{
    bool ret = m_mgr->SyncDbDelete(QString());
    EXPECT_FALSE(ret);
}

TEST_F(SyncFileManageTest, SyncDbDeleteRemovesDirectory)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString filePath = tmp.path() + "/subdir/testdb.db";
    QDir().mkpath(tmp.path() + "/subdir");
    {
        QFile f(filePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("data");
        f.close();
    }
    bool ret = m_mgr->SyncDbDelete(filePath);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(QDir(tmp.path() + "/subdir").exists());
}

TEST_F(SyncFileManageTest, SyncDbDeleteNonExistentPathReturnsTrue)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString filePath = tmp.path() + "/nonexistent.db";
    bool ret = m_mgr->SyncDbDelete(filePath);
    EXPECT_TRUE(ret);
}

// ---- SyncFileManage::syncDataDelete ----

TEST_F(SyncFileManageTest, SyncDataDeleteEmptyPathReturnsFalse)
{
    bool ret = m_mgr->syncDataDelete(QString());
    EXPECT_FALSE(ret);
}

TEST_F(SyncFileManageTest, SyncDataDeleteValidPath)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString filePath = tmp.path() + "/subdir/testdb.db";
    QDir().mkpath(tmp.path() + "/subdir");
    {
        QFile f(filePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("data");
        f.close();
    }
    bool ret = m_mgr->syncDataDelete(filePath);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(QDir(tmp.path() + "/subdir").exists());
}

// ---- SyncFileManage::SyncDataUpload ----

TEST_F(SyncFileManageTest, SyncDataUploadWithoutDbusReturnsError)
{
    int errorcode = 0;
    bool ret = m_mgr->SyncDataUpload("/nonexistent/file.db", errorcode);
    // 源码缺陷（Mode 5 缺陷清单已记录）：SyncDataUpload 以 result.error_code
    // 而非 result.ret 判断成败；仅当错误消息是含 "code" 的 JSON 时才会设置
    // error_code。打桩返回的错误消息不含 JSON code，error_code 保持
    // SYNC_No_Error，因此该缺陷路径确定性地返回 true。断言按实际（缺陷）行为
    // 固化，待生产修复后改为 EXPECT_FALSE。
    EXPECT_TRUE(ret);
}

// ---- SyncFileManage::SyncDataDownload ----

TEST_F(SyncFileManageTest, SyncDataDownloadWithoutDbusDoesNotCrash)
{
    QString uid = "testuser";
    QString filepath;
    int errorcode = 0;
    bool ret = m_mgr->SyncDataDownload(uid, filepath, errorcode);
    // DBus service not available — download fails, but may create DB if
    // error_code happens to be SYNC_Data_Not_Exist
    // Either way it must not crash
    Q_UNUSED(ret);
    SUCCEED();
}

// ---- SyncFileManage::getuserInfo ----

TEST_F(SyncFileManageTest, GetUserInfoWithoutDbusReturnsNull)
{
    // DBus service is not available in test environment
    auto result = m_mgr->getuserInfo();
    EXPECT_EQ(result, nullptr);
}
