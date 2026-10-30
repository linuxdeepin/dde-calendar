// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dreminddata.h"
#include "gtest/gtest.h"
#include <QDateTime>
#include <QString>

class test_dreminddata : public ::testing::Test
{
protected:
    void SetUp() override
    {
        data = new DRemindData();
    }
    void TearDown() override
    {
        delete data;
    }
    DRemindData *data = nullptr;
};

// DRemindData::DRemindData()
TEST_F(test_dreminddata, Constructor_DefaultValues)
{
    // Arrange
    DRemindData *d = new DRemindData();

    // Act — values set by constructor

    // Assert
    EXPECT_EQ(d->alarmID(), QString(""));
    EXPECT_EQ(d->accountID(), QString(""));
    EXPECT_EQ(d->scheduleID(), QString(""));
    EXPECT_EQ(d->remindCount(), 0);
    EXPECT_EQ(d->notifyid(), -1);

    delete d;
}

// QString DRemindData::accountID() const
TEST_F(test_dreminddata, AccountID_Default)
{
    // Arrange
    // data already constructed with default ""

    // Act
    QString result = data->accountID();

    // Assert
    EXPECT_EQ(result, QString(""));
}

// void DRemindData::setAccountID(const QString &accountID)
TEST_F(test_dreminddata, SetAccountID)
{
    // Arrange
    QString expected = "test-account-123";

    // Act
    data->setAccountID(expected);

    // Assert
    EXPECT_EQ(data->accountID(), expected);
}

// QString DRemindData::scheduleID() const
TEST_F(test_dreminddata, ScheduleID_Default)
{
    // Arrange
    // default value ""

    // Act
    QString result = data->scheduleID();

    // Assert
    EXPECT_EQ(result, QString(""));
}

// void DRemindData::setScheduleID(const QString &scheduleID)
TEST_F(test_dreminddata, SetScheduleID)
{
    // Arrange
    QString expected = "sched-456";

    // Act
    data->setScheduleID(expected);

    // Assert
    EXPECT_EQ(data->scheduleID(), expected);
}

// QDateTime DRemindData::recurrenceId() const
TEST_F(test_dreminddata, RecurrenceId_Default)
{
    // Arrange
    // default-constructed QDateTime is invalid

    // Act
    QDateTime result = data->recurrenceId();

    // Assert
    EXPECT_FALSE(result.isValid());
}

// void DRemindData::setRecurrenceId(const QDateTime &recurrenceId)
TEST_F(test_dreminddata, SetRecurrenceId)
{
    // Arrange
    QDateTime expected = QDateTime::fromString("2025-06-15T10:30:00", Qt::ISODate);

    // Act
    data->setRecurrenceId(expected);

    // Assert
    EXPECT_EQ(data->recurrenceId(), expected);
}

// int DRemindData::remindCount() const
TEST_F(test_dreminddata, RemindCount_Default)
{
    // Arrange
    // default value 0

    // Act
    int result = data->remindCount();

    // Assert
    EXPECT_EQ(result, 0);
}

// void DRemindData::setRemindCount(int remindCount)
TEST_F(test_dreminddata, SetRemindCount)
{
    // Arrange
    int expected = 5;

    // Act
    data->setRemindCount(expected);

    // Assert
    EXPECT_EQ(data->remindCount(), expected);
}

// int DRemindData::notifyid() const
TEST_F(test_dreminddata, Notifyid_Default)
{
    // Arrange
    // default value -1

    // Act
    int result = data->notifyid();

    // Assert
    EXPECT_EQ(result, -1);
}

// void DRemindData::setNotifyid(int notifyid)
TEST_F(test_dreminddata, SetNotifyid)
{
    // Arrange
    int expected = 42;

    // Act
    data->setNotifyid(expected);

    // Assert
    EXPECT_EQ(data->notifyid(), expected);
}

// QDateTime DRemindData::dtRemind() const
TEST_F(test_dreminddata, DtRemind_Default)
{
    // Arrange
    // default-constructed QDateTime is invalid

    // Act
    QDateTime result = data->dtRemind();

    // Assert
    EXPECT_FALSE(result.isValid());
}

// void DRemindData::setDtRemind(const QDateTime &dtRemind)
TEST_F(test_dreminddata, SetDtRemind)
{
    // Arrange
    QDateTime expected = QDateTime::fromString("2025-06-15T08:00:00", Qt::ISODate);

    // Act
    data->setDtRemind(expected);

    // Assert
    EXPECT_EQ(data->dtRemind(), expected);
}

// QDateTime DRemindData::dtStart() const
TEST_F(test_dreminddata, DtStart_Default)
{
    // Arrange
    // default-constructed QDateTime is invalid

    // Act
    QDateTime result = data->dtStart();

    // Assert
    EXPECT_FALSE(result.isValid());
}

// void DRemindData::setDtStart(const QDateTime &dtStart)
TEST_F(test_dreminddata, SetDtStart)
{
    // Arrange
    QDateTime expected = QDateTime::fromString("2025-06-15T09:00:00", Qt::ISODate);

    // Act
    data->setDtStart(expected);

    // Assert
    EXPECT_EQ(data->dtStart(), expected);
}

// QDateTime DRemindData::dtEnd() const
TEST_F(test_dreminddata, DtEnd_Default)
{
    // Arrange
    // default-constructed QDateTime is invalid

    // Act
    QDateTime result = data->dtEnd();

    // Assert
    EXPECT_FALSE(result.isValid());
}

// void DRemindData::setDtEnd(const QDateTime &dtEnd)
TEST_F(test_dreminddata, SetDtEnd)
{
    // Arrange
    QDateTime expected = QDateTime::fromString("2025-06-15T11:00:00", Qt::ISODate);

    // Act
    data->setDtEnd(expected);

    // Assert
    EXPECT_EQ(data->dtEnd(), expected);
}

// QString DRemindData::alarmID() const
TEST_F(test_dreminddata, AlarmID_Default)
{
    // Arrange
    // default value ""

    // Act
    QString result = data->alarmID();

    // Assert
    EXPECT_EQ(result, QString(""));
}

// void DRemindData::setAlarmID(const QString &alarmID)
TEST_F(test_dreminddata, SetAlarmID)
{
    // Arrange
    QString expected = "alarm-789";

    // Act
    data->setAlarmID(expected);

    // Assert
    EXPECT_EQ(data->alarmID(), expected);
}

// void DRemindData::updateRemindTimeByCount()
TEST_F(test_dreminddata, UpdateRemindTimeByCount_DefaultCount)
{
    // Arrange — default remindCount is 0
    // duration = (10 + ((0-1)*5)) * 60000 = 5 * 60000 = 300000 ms (5 min)

    // Act
    data->updateRemindTimeByCount();

    // Assert — dtRemind should be set to ~now + 5 min
    QDateTime now = QDateTime::currentDateTime();
    QDateTime remind = data->dtRemind();
    EXPECT_TRUE(remind.isValid());
    qint64 diffMs = now.msecsTo(remind);
    EXPECT_NEAR(diffMs, 300000, 5000);
}

TEST_F(test_dreminddata, UpdateRemindTimeByCount_NormalCount)
{
    // Arrange — count=3, duration = (10 + (2*5)) * 60000 = 20 * 60000 = 1200000 ms (20 min)
    data->setRemindCount(3);

    // Act
    data->updateRemindTimeByCount();

    // Assert
    QDateTime now = QDateTime::currentDateTime();
    QDateTime remind = data->dtRemind();
    EXPECT_TRUE(remind.isValid());
    qint64 diffMs = now.msecsTo(remind);
    EXPECT_NEAR(diffMs, 1200000, 5000);
}

TEST_F(test_dreminddata, UpdateRemindTimeByCount_CappedAtHour)
{
    // Arrange — count=11, duration = (10 + (10*5)) * 60000 = 60 * 60000 = 3600000 ms = 1 hour
    data->setRemindCount(11);

    // Act
    data->updateRemindTimeByCount();

    // Assert — should be capped at exactly 1 hour
    QDateTime now = QDateTime::currentDateTime();
    QDateTime remind = data->dtRemind();
    EXPECT_TRUE(remind.isValid());
    qint64 diffMs = now.msecsTo(remind);
    EXPECT_NEAR(diffMs, 3600000, 5000);
}

TEST_F(test_dreminddata, UpdateRemindTimeByCount_ExceedsHourCapped)
{
    // Arrange — count=20, would be (10 + (19*5)) * 60000 = 105 * 60000 = 6300000 > 3600000
    data->setRemindCount(20);

    // Act
    data->updateRemindTimeByCount();

    // Assert — should be capped at 1 hour
    QDateTime now = QDateTime::currentDateTime();
    QDateTime remind = data->dtRemind();
    EXPECT_TRUE(remind.isValid());
    qint64 diffMs = now.msecsTo(remind);
    EXPECT_NEAR(diffMs, 3600000, 5000);
}

// void DRemindData::updateRemindTimeByMesc(qint64 duration)
TEST_F(test_dreminddata, UpdateRemindTimeByMesc_PositiveDuration)
{
    // Arrange
    qint64 duration = 600000; // 10 minutes

    // Act
    data->updateRemindTimeByMesc(duration);

    // Assert
    QDateTime now = QDateTime::currentDateTime();
    QDateTime remind = data->dtRemind();
    EXPECT_TRUE(remind.isValid());
    qint64 diffMs = now.msecsTo(remind);
    EXPECT_NEAR(diffMs, 600000, 5000);
}

TEST_F(test_dreminddata, UpdateRemindTimeByMesc_ZeroDuration)
{
    // Arrange
    qint64 duration = 0;

    // Act
    data->updateRemindTimeByMesc(duration);

    // Assert — should be approximately now
    QDateTime now = QDateTime::currentDateTime();
    QDateTime remind = data->dtRemind();
    EXPECT_TRUE(remind.isValid());
    qint64 diffMs = now.msecsTo(remind);
    EXPECT_NEAR(diffMs, 0, 5000);
}

TEST_F(test_dreminddata, UpdateRemindTimeByMesc_NegativeDuration)
{
    // Arrange
    qint64 duration = -300000; // -5 minutes (past)

    // Act
    data->updateRemindTimeByMesc(duration);

    // Assert
    QDateTime now = QDateTime::currentDateTime();
    QDateTime remind = data->dtRemind();
    EXPECT_TRUE(remind.isValid());
    qint64 diffMs = now.msecsTo(remind);
    EXPECT_NEAR(diffMs, -300000, 5000);
}

// QDateTime DRemindData::getRemindTimeByMesc(qint64 duration) — private, accessible via -fno-access-control
TEST_F(test_dreminddata, GetRemindTimeByMesc_PositiveDuration)
{
    // Arrange
    qint64 duration = 1800000; // 30 min

    // Act
    QDateTime result = data->getRemindTimeByMesc(duration);

    // Assert
    QDateTime now = QDateTime::currentDateTime();
    EXPECT_TRUE(result.isValid());
    qint64 diffMs = now.msecsTo(result);
    EXPECT_NEAR(diffMs, 1800000, 5000);
}

TEST_F(test_dreminddata, GetRemindTimeByMesc_ZeroDuration)
{
    // Arrange
    qint64 duration = 0;

    // Act
    QDateTime result = data->getRemindTimeByMesc(duration);

    // Assert
    QDateTime now = QDateTime::currentDateTime();
    EXPECT_TRUE(result.isValid());
    qint64 diffMs = now.msecsTo(result);
    EXPECT_NEAR(diffMs, 0, 5000);
}
