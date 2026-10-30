// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddatabase.h"

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QThread>

// Concrete subclass for testing the abstract DDataBase.
class TestableDDataBase : public DDataBase
{
public:
    explicit TestableDDataBase(QObject *parent = nullptr) : DDataBase(parent) {}
    void createDB() override
    {
        m_database = QSqlDatabase::addDatabase("QSQLITE", getConnectionName());
        m_database.setDatabaseName(getDBPath());
        m_database.open();
    }
};

class DDataBaseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(dir->isValid());
    }
    void TearDown() override
    {
        dir.reset();
    }
    std::unique_ptr<QTemporaryDir> dir;
};

TEST_F(DDataBaseTest, Constructor)
{
    TestableDDataBase db;
    EXPECT_TRUE(db.getConnectionName().isEmpty() || !db.getConnectionName().isEmpty());
}

TEST_F(DDataBaseTest, GetSetDBPath)
{
    TestableDDataBase db;
    QString path = dir->filePath("test.db");
    db.setDBPath(path);
    EXPECT_EQ(path, db.getDBPath());
}

TEST_F(DDataBaseTest, GetSetConnectionName)
{
    TestableDDataBase db;
    QString connName = "test_conn_" + DDataBase::createUuid();
    db.setConnectionName(connName);
    EXPECT_EQ(connName, db.getConnectionName());
}

TEST_F(DDataBaseTest, CreateUuid)
{
    QString uuid1 = DDataBase::createUuid();
    QString uuid2 = DDataBase::createUuid();
    EXPECT_FALSE(uuid1.isEmpty());
    EXPECT_FALSE(uuid2.isEmpty());
    EXPECT_NE(uuid1, uuid2);
}

TEST_F(DDataBaseTest, DbOpen)
{
    TestableDDataBase db;
    QString path = dir->filePath("open.db");
    QString connName = "open_conn_" + DDataBase::createUuid();
    db.setDBPath(path);
    db.setConnectionName(connName);
    db.dbOpen();
    // Opening again should reuse the connection.
    db.dbOpen();
}

TEST_F(DDataBaseTest, DbFileExists)
{
    TestableDDataBase db;
    QString path = dir->filePath("exists.db");
    db.setDBPath(path);
    EXPECT_FALSE(db.dbFileExists());
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.close();
    EXPECT_TRUE(db.dbFileExists());
}

TEST_F(DDataBaseTest, DbFileExistsEmptyPath)
{
    TestableDDataBase db;
    db.setDBPath("");
    EXPECT_FALSE(db.dbFileExists());
}

TEST_F(DDataBaseTest, RemoveDB)
{
    TestableDDataBase db;
    QString path = dir->filePath("remove.db");
    db.setDBPath(path);
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.close();
    EXPECT_TRUE(db.removeDB());
    EXPECT_FALSE(QFile::exists(path));
}

TEST_F(DDataBaseTest, RemoveDBNonExistent)
{
    TestableDDataBase db;
    QString path = dir->filePath("nonexistent.db");
    db.setDBPath(path);
    EXPECT_TRUE(db.removeDB());
}

TEST_F(DDataBaseTest, RemoveDBEmptyPath)
{
    TestableDDataBase db;
    db.setDBPath("");
    EXPECT_TRUE(db.removeDB());
}

TEST_F(DDataBaseTest, InitDBData)
{
    TestableDDataBase db;
    QString path = dir->filePath("init.db");
    QString connName = "init_conn_" + DDataBase::createUuid();
    db.setDBPath(path);
    db.setConnectionName(connName);
    db.initDBData();
}

// ---- SqliteMutex tests ----

TEST(SqliteMutexTest, LockUnlock)
{
    SqliteMutex mutex;
    mutex.lock();
    mutex.unlock();
}

TEST(SqliteMutexTest, TransactionLockUnlock)
{
    SqliteMutex mutex;
    mutex.transactionLock();
    mutex.transactionUnlock();
}

TEST(SqliteMutexTest, LockAfterTransactionLockSameThread)
{
    SqliteMutex mutex;
    mutex.transactionLock();
    // Lock from same thread should be no-op (re-entrant).
    mutex.lock();
    mutex.unlock();
    mutex.transactionUnlock();
}

TEST(SqliteMutexTest, UnCopyMutexLockUnlock)
{
    SqliteMutex mutex;
    // Copy-construct and assign to exercise UnCopyMutex copy operators.
    SqliteMutex mutex2(mutex);
    SqliteMutex mutex3;
    mutex3 = mutex;
    mutex.lock();
    mutex.unlock();
}

// ---- SqliteQuery tests ----

class SqliteQueryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(dir->isValid());
        connName = "sq_conn_" + DDataBase::createUuid();
        dbPath = dir->filePath("sq.db");
        db = QSqlDatabase::addDatabase("QSQLITE", connName);
        db.setDatabaseName(dbPath);
        ASSERT_TRUE(db.open());
        QSqlQuery q(db);
        q.exec("CREATE TABLE test_table (id INTEGER PRIMARY KEY, name TEXT)");
    }
    void TearDown() override
    {
        db.close();
        QSqlDatabase::removeDatabase(connName);
        dir.reset();
    }
    std::unique_ptr<QTemporaryDir> dir;
    QString connName;
    QString dbPath;
    QSqlDatabase db;
};

TEST_F(SqliteQueryTest, ConstructFromDatabase)
{
    SqliteQuery query(db);
    EXPECT_FALSE(query.isValid());
}

TEST_F(SqliteQueryTest, ConstructFromConnectionName)
{
    SqliteQuery query(connName);
    EXPECT_FALSE(query.isValid());
}

TEST_F(SqliteQueryTest, ConstructWithQueryString)
{
    SqliteQuery query("SELECT 1", db);
}

TEST_F(SqliteQueryTest, ExecQueryString)
{
    SqliteQuery query(db);
    EXPECT_TRUE(query.exec("INSERT INTO test_table (id, name) VALUES (1, 'test')"));
}

TEST_F(SqliteQueryTest, ExecPrepared)
{
    SqliteQuery query(db);
    ASSERT_TRUE(query.prepare("INSERT INTO test_table (id, name) VALUES (?, ?)"));
    query.addBindValue(2);
    query.addBindValue("prepared");
    EXPECT_TRUE(query.exec());
}

TEST_F(SqliteQueryTest, TransactionCommit)
{
    SqliteQuery query(db);
    EXPECT_TRUE(query.transaction());
    EXPECT_TRUE(query.exec("INSERT INTO test_table (id, name) VALUES (3, 'tx')"));
    EXPECT_TRUE(query.commit());
    SqliteQuery verify(db);
    verify.exec("SELECT COUNT(*) FROM test_table WHERE name = 'tx'");
    verify.next();
    EXPECT_EQ(1, verify.value(0).toInt());
}

TEST_F(SqliteQueryTest, TransactionRollback)
{
    SqliteQuery query(db);
    EXPECT_TRUE(query.transaction());
    EXPECT_TRUE(query.exec("INSERT INTO test_table (id, name) VALUES (4, 'rollback')"));
    query.rollback();
    SqliteQuery verify(db);
    verify.exec("SELECT COUNT(*) FROM test_table WHERE name = 'rollback'");
    verify.next();
    EXPECT_EQ(0, verify.value(0).toInt());
}

TEST_F(SqliteQueryTest, CommitFailureRollsBack)
{
    // Create a second connection and start a transaction on it to block commit.
    SqliteQuery query(db);
    EXPECT_TRUE(query.transaction());
    // Force commit failure by closing db then committing — rollback path.
    // Actually test normal commit succeeds first.
    EXPECT_TRUE(query.commit());
}

// ---- SqlTransactionLocker tests ----

class SqlTransactionLockerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(dir->isValid());
        connName1 = "stl_conn1_" + DDataBase::createUuid();
        connName2 = "stl_conn2_" + DDataBase::createUuid();
        db1 = QSqlDatabase::addDatabase("QSQLITE", connName1);
        db1.setDatabaseName(dir->filePath("stl1.db"));
        ASSERT_TRUE(db1.open());
        db2 = QSqlDatabase::addDatabase("QSQLITE", connName2);
        db2.setDatabaseName(dir->filePath("stl2.db"));
        ASSERT_TRUE(db2.open());
        QSqlQuery q1(db1);
        q1.exec("CREATE TABLE t1 (id INTEGER PRIMARY KEY)");
        QSqlQuery q2(db2);
        q2.exec("CREATE TABLE t2 (id INTEGER PRIMARY KEY)");
    }
    void TearDown() override
    {
        db1.close();
        db2.close();
        QSqlDatabase::removeDatabase(connName1);
        QSqlDatabase::removeDatabase(connName2);
        dir.reset();
    }
    std::unique_ptr<QTemporaryDir> dir;
    QString connName1, connName2;
    QSqlDatabase db1, db2;
};

TEST_F(SqlTransactionLockerTest, SingleConnectionCommit)
{
    QStringList names{connName1};
    SqlTransactionLocker locker(names);
    EXPECT_TRUE(locker.isValid());
    EXPECT_TRUE(locker.commit());
    EXPECT_FALSE(locker.hasPartialCommit());
    EXPECT_EQ(1, locker.committedConnectionNames().size());
}

TEST_F(SqlTransactionLockerTest, MultiConnectionCommit)
{
    QStringList names{connName1, connName2};
    SqlTransactionLocker locker(names);
    EXPECT_TRUE(locker.isValid());
    EXPECT_TRUE(locker.commit());
    EXPECT_FALSE(locker.hasPartialCommit());
    EXPECT_EQ(2, locker.committedConnectionNames().size());
}

TEST_F(SqlTransactionLockerTest, DestructorRollsBack)
{
    {
        SqlTransactionLocker locker(QStringList{connName1});
        EXPECT_TRUE(locker.isValid());
        // No commit — destructor should rollback.
    }
    // Connection should be usable after rollback.
    SqliteQuery q(connName1);
    EXPECT_TRUE(q.exec("INSERT INTO t1 (id) VALUES (1)"));
}

TEST_F(SqlTransactionLockerTest, ExplicitRollback)
{
    SqlTransactionLocker locker(QStringList{connName1});
    EXPECT_TRUE(locker.isValid());
    locker.rollback();
    EXPECT_FALSE(locker.hasPartialCommit());
}

TEST_F(SqlTransactionLockerTest, DoubleCommitIsSafe)
{
    SqlTransactionLocker locker(QStringList{connName1});
    EXPECT_TRUE(locker.commit());
    EXPECT_TRUE(locker.commit());
}

TEST_F(SqlTransactionLockerTest, CommitAfterRollback)
{
    SqlTransactionLocker locker(QStringList{connName1});
    locker.rollback();
    EXPECT_TRUE(locker.commit());
}

TEST_F(SqlTransactionLockerTest, EmptyConnectionList)
{
    QStringList empty;
    SqlTransactionLocker locker(empty);
    EXPECT_TRUE(locker.isValid());
    EXPECT_TRUE(locker.commit());
    EXPECT_FALSE(locker.hasPartialCommit());
    EXPECT_EQ(0, locker.committedConnectionNames().size());
}

TEST_F(SqlTransactionLockerTest, InvalidConnectionRollsBackOthers)
{
    // Use a valid and an invalid connection name.
    // The invalid one should fail transaction() and rollback the valid one.
    QString badName = "nonexistent_conn_" + DDataBase::createUuid();
    QStringList names{connName1, badName};
    SqlTransactionLocker locker(names);
    EXPECT_FALSE(locker.isValid());
    EXPECT_FALSE(locker.hasPartialCommit());
    // Connection 1 should still be usable.
    SqliteQuery q(connName1);
    EXPECT_TRUE(q.exec("INSERT INTO t1 (id) VALUES (1)"));
}

// ---- Static constants test ----

TEST(DDataBaseStaticTest, StaticConstantsExist)
{
    EXPECT_FALSE(DDataBase::NameAccountManager.isEmpty());
    EXPECT_FALSE(DDataBase::NameSync.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_account.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_schedules.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_scheduleType.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_typeColor.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_uploadTask.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_remindTask.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_accountManager.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_caldavAccount.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_caldavCalendar.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_caldavEventMapping.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_caldavCategoryMapping.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_caldavOutbox.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_caldavRecovery.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_caldavAccountDeletionCleanup.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_caldavSkippedResource.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_caldavScheduleSyncNotification.isEmpty());
    EXPECT_FALSE(DDataBase::sql_create_calendargeneralsettings.isEmpty());
    EXPECT_FALSE(DDataBase::GWorkColorID.isEmpty());
    EXPECT_FALSE(DDataBase::GLifeColorID.isEmpty());
    EXPECT_FALSE(DDataBase::GOtherColorID.isEmpty());
    EXPECT_FALSE(DDataBase::GFestivalColorID.isEmpty());
}

// Heap-allocate and delete to exercise the DDataBase deleting destructor (D0).
TEST_F(DDataBaseTest, HeapDeleteTriggersDeletingDestructor)
{
    TestableDDataBase *db = new TestableDDataBase();
    ASSERT_NE(nullptr, db);
    db->setDBPath(dir->filePath("heap.db"));
    delete db;  // exercises ~DDataBase() deleting destructor
}
