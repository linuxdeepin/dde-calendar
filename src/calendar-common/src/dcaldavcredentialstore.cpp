// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavcredentialstore.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusVariant>
#include <QMap>
#include <QPointer>
#include <QRegularExpression>
#include <QSemaphore>
#include <QThread>
#include <QTimer>
#include <QVariantMap>

using DCalDavSecretAttributes = QMap<QString, QString>;
Q_DECLARE_METATYPE(DCalDavSecretAttributes)

namespace {

const char *const SecretServiceName = "org.freedesktop.secrets";
const char *const SecretServicePath = "/org/freedesktop/secrets";
const char *const DefaultCollectionPath = "/org/freedesktop/secrets/aliases/default";
const char *const SecretServiceInterface = "org.freedesktop.Secret.Service";
const char *const SecretCollectionInterface = "org.freedesktop.Secret.Collection";
const char *const SecretItemInterface = "org.freedesktop.Secret.Item";
const char *const SecretPromptInterface = "org.freedesktop.Secret.Prompt";
constexpr int kSecretServiceTimeoutMs = 10000;
constexpr int kSecretPromptTimeoutMs = 30000;
const char *const CalendarApplicationAttribute = "org.deepin.dde-calendar";

enum class OperationType {
    Store,
    Read,
    Delete,
};

void setError(QString *errorMessage, const QString &message)
{
    if (errorMessage != nullptr) {
        *errorMessage = message;
    }
}

bool isRootPath(const QDBusObjectPath &path)
{
    return path.path().isEmpty() || path.path() == QStringLiteral("/");
}

class SecretServiceOperation final : public QObject
{
    Q_OBJECT
public:
    using Callback = std::function<void(bool, const QString &, const QString &)>;

    SecretServiceOperation(OperationType type, const QString &label, const QString &password,
                           const QString &itemPath, Callback callback, QObject *parent = nullptr)
        : QObject(parent)
        , m_type(type)
        , m_label(label)
        , m_password(password)
        , m_itemPath(itemPath)
        , m_bus(QDBusConnection::sessionBus())
        , m_callback(std::move(callback))
    {
    }

    void cancelForContext()
    {
        if (m_finished) {
            return;
        }
        m_contextDestroyed = true;
        m_callback = Callback();
        m_password.clear();
    }

    void start()
    {
        if (m_started) {
            return;
        }
        m_started = true;
        if (m_contextDestroyed) {
            finish(false, QString(), QStringLiteral("Secret Service operation was cancelled."));
            return;
        }
        if (m_type == OperationType::Delete) {
            deleteItem();
            return;
        }
        if (m_type == OperationType::Store && (m_label.trimmed().isEmpty() || m_password.isEmpty())) {
            finish(false, QString(), QStringLiteral("CalDAV credential label or password is empty."));
            return;
        }

        m_bus = QDBusConnection::sessionBus();
        if (!m_bus.isConnected()) {
            finish(false, QString(), QStringLiteral("Session D-Bus is unavailable."));
            return;
        }
        openSession();
    }

private:
    void openSession()
    {
        QDBusInterface service(QLatin1String(SecretServiceName),
                               QLatin1String(SecretServicePath),
                               QLatin1String(SecretServiceInterface), m_bus);
        service.setTimeout(kSecretServiceTimeoutMs);
        if (!service.isValid()) {
            finish(false, QString(), QStringLiteral("Secret Service is unavailable."));
            return;
        }
        watch(service.asyncCall(QStringLiteral("OpenSession"), QStringLiteral("plain"),
                                QVariant::fromValue(QDBusVariant(QString()))),
              [this](const QDBusMessage &reply) {
                  if (reply.arguments().size() < 2) {
                      finish(false, QString(), QStringLiteral("Secret Service session could not be opened."));
                      return;
                  }
                  m_sessionPath = reply.arguments().at(1).value<QDBusObjectPath>();
                  if (m_sessionPath.path().isEmpty()) {
                      finish(false, QString(), QStringLiteral("Secret Service returned an invalid session."));
                      return;
                  }
                  if (m_contextDestroyed) {
                      closeSessionAndFinish(false, QString(),
                                            QStringLiteral("Secret Service operation was cancelled."));
                      return;
                  }
                  if (m_type == OperationType::Read) {
                      readItem();
                  } else {
                      createItem();
                  }
              });
    }

    void createItem()
    {
        qDBusRegisterMetaType<DCalDavSecretAttributes>();
        QVariantMap properties;
        properties.insert(QStringLiteral("org.freedesktop.Secret.Item.Label"), m_label);
        DCalDavSecretAttributes attributes;
        attributes.insert(QStringLiteral("application"), QLatin1String(CalendarApplicationAttribute));
        properties.insert(QStringLiteral("org.freedesktop.Secret.Item.Attributes"),
                          QVariant::fromValue(attributes));

        QDBusArgument secret;
        secret.beginStructure();
        secret << m_sessionPath << QByteArray() << m_password.toUtf8() << QStringLiteral("text/plain");
        secret.endStructure();

        QDBusInterface collection(QLatin1String(SecretServiceName),
                                  QLatin1String(DefaultCollectionPath),
                                  QLatin1String(SecretCollectionInterface), m_bus);
        collection.setTimeout(kSecretServiceTimeoutMs);
        watch(collection.asyncCall(QStringLiteral("CreateItem"), properties,
                                   QVariant::fromValue(secret), false),
              [this](const QDBusMessage &reply) {
                  if (reply.arguments().size() < 2) {
                      closeSessionAndFinish(false, QString(),
                                            QStringLiteral("Secret Service item could not be created."));
                      return;
                  }
                  m_itemPath = reply.arguments().at(0).value<QDBusObjectPath>().path();
                  if (m_itemPath.isEmpty() || m_itemPath == QStringLiteral("/")) {
                      closeSessionAndFinish(false, QString(),
                                            QStringLiteral("Secret Service returned an invalid item."));
                      return;
                  }
                  if (m_contextDestroyed) {
                      deleteItem();
                      return;
                  }
                  prompt(reply.arguments().at(1).value<QDBusObjectPath>());
              });
    }

    void readItem()
    {
        QDBusInterface item(QLatin1String(SecretServiceName), m_itemPath,
                            QLatin1String(SecretItemInterface), m_bus);
        item.setTimeout(kSecretServiceTimeoutMs);
        if (!item.isValid()) {
            closeSessionAndFinish(false, QString(), QStringLiteral("Secret Service item is unavailable."));
            return;
        }
        watch(item.asyncCall(QStringLiteral("GetSecret"), QVariant::fromValue(m_sessionPath)),
              [this](const QDBusMessage &reply) {
                  if (reply.arguments().isEmpty()) {
                      closeSessionAndFinish(false, QString(),
                                            QStringLiteral("Secret Service item could not be read."));
                      return;
                  }
                  const QDBusArgument secret = reply.arguments().first().value<QDBusArgument>();
                  QDBusObjectPath returnedSession;
                  QByteArray parameters;
                  QByteArray value;
                  QString contentType;
                  secret.beginStructure();
                  secret >> returnedSession >> parameters >> value >> contentType;
                  secret.endStructure();
                  if (returnedSession != m_sessionPath || value.isNull()) {
                      closeSessionAndFinish(false, QString(),
                                            QStringLiteral("Secret Service item could not be read."));
                      return;
                  }
                  m_result = QString::fromUtf8(value);
                  closeSessionAndFinish(true, m_result, QString());
              });
    }

    void deleteItem()
    {
        m_bus = QDBusConnection::sessionBus();
        if (!m_bus.isConnected()) {
            finish(false, QString(), QStringLiteral("Session D-Bus is unavailable."));
            return;
        }
        QDBusInterface item(QLatin1String(SecretServiceName), m_itemPath,
                            QLatin1String(SecretItemInterface), m_bus);
        item.setTimeout(kSecretServiceTimeoutMs);
        if (!item.isValid()) {
            finish(false, QString(), QStringLiteral("Secret Service item is unavailable."));
            return;
        }
        watch(item.asyncCall(QStringLiteral("Delete")), [this](const QDBusMessage &reply) {
            if (reply.arguments().isEmpty()) {
                finish(false, QString(), QStringLiteral("Secret Service item could not be deleted."));
                return;
            }
            prompt(reply.arguments().first().value<QDBusObjectPath>());
        });
    }

    void prompt(const QDBusObjectPath &promptPath)
    {
        if (isRootPath(promptPath)) {
            closeSessionAndFinish(true, m_type == OperationType::Store
                                         ? QStringLiteral("secret-service:") + m_itemPath : m_result,
                                  QString());
            return;
        }
        m_promptCompleted = false;
        if (!m_bus.connect(QLatin1String(SecretServiceName), promptPath.path(),
                           QLatin1String(SecretPromptInterface), QStringLiteral("Completed"),
                           this, SLOT(promptCompleted(bool)))) {
            closeSessionAndFinish(false, QString(),
                                  QStringLiteral("Secret Service prompt could not be monitored."));
            return;
        }
        m_promptPath = promptPath;
        if (m_promptTimer == nullptr) {
            m_promptTimer = new QTimer(this);
            m_promptTimer->setSingleShot(true);
            connect(m_promptTimer, &QTimer::timeout, this, [this]() {
                disconnectPrompt();
                closeSessionAndFinish(false, QString(),
                                      QStringLiteral("Secret Service prompt timed out."));
            });
        }
        m_promptTimer->start(kSecretPromptTimeoutMs);

        QDBusInterface promptInterface(QLatin1String(SecretServiceName), promptPath.path(),
                                       QLatin1String(SecretPromptInterface), m_bus);
        promptInterface.setTimeout(kSecretServiceTimeoutMs);
        watch(promptInterface.asyncCall(QStringLiteral("Prompt"), QString()),
              [this](const QDBusMessage &) {
                  // The result is delivered by SecretPrompt.Completed.
              },
              [this](const QString &errorMessage) {
                  if (m_promptCompleted || m_finished) {
                      return;
                  }
                  closeSessionAndFinish(false, QString(), errorMessage);
              });
    }

private slots:
    void promptCompleted(bool dismissed)
    {
        m_promptCompleted = true;
        disconnectPrompt();
        closeSessionAndFinish(!dismissed,
                              m_type == OperationType::Store
                                  ? QStringLiteral("secret-service:") + m_itemPath : m_result,
                              dismissed ? QStringLiteral("Secret Service prompt was dismissed.")
                                        : QString());
    }

private:
    void watch(const QDBusPendingCall &call,
               const std::function<void(const QDBusMessage &)> &handler,
               const std::function<void(const QString &)> &errorHandler = {})
    {
        auto *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                [this, watcher, handler, errorHandler]() {
                    const QDBusMessage reply = watcher->reply();
                    watcher->deleteLater();
                    if (reply.type() == QDBusMessage::ErrorMessage) {
                        if (errorHandler) {
                            errorHandler(reply.errorMessage());
                        } else if (!m_finished) {
                            closeSessionAndFinish(false, QString(), reply.errorMessage());
                        }
                        return;
                    }
                    if (!m_finished) {
                        handler(reply);
                    }
                });
    }

    void disconnectPrompt()
    {
        if (m_promptPath.path().isEmpty()) {
            return;
        }
        if (m_promptTimer != nullptr) {
            m_promptTimer->stop();
        }
        m_bus.disconnect(QLatin1String(SecretServiceName), m_promptPath.path(),
                         QLatin1String(SecretPromptInterface), QStringLiteral("Completed"),
                         this, SLOT(promptCompleted(bool)));
        m_promptPath = QDBusObjectPath();
    }

    void closeSessionAndFinish(bool success, const QString &value, const QString &error)
    {
        if (m_finished) {
            return;
        }
        // Secret Service does not expose a CloseSession method. Sessions are
        // released when the D-Bus connection is closed, so do not turn a
        // successful credential operation into a failure during cleanup.
        m_sessionPath = QDBusObjectPath();
        finish(success, value, error);
    }

    void finish(bool success, const QString &value, const QString &error)
    {
        if (m_finished) {
            return;
        }
        if (m_contextDestroyed && m_type == OperationType::Store && !m_cleanupStarted
            && !m_itemPath.isEmpty() && m_itemPath != QStringLiteral("/")) {
            m_cleanupStarted = true;
            deleteItem();
            return;
        }
        m_finished = true;
        disconnectPrompt();
        m_password.clear();
        const Callback callback = std::move(m_callback);
        // Queue destruction before invoking external code. The callback may destroy
        // the context that owns this operation.
        deleteLater();
        if (callback) {
            callback(success, value, error);
        }
    }

    OperationType m_type;
    QString m_label;
    QString m_password;
    QString m_itemPath;
    QString m_result;
    QDBusConnection m_bus;
    QDBusObjectPath m_sessionPath;
    QDBusObjectPath m_promptPath;
    QTimer *m_promptTimer = nullptr;
    Callback m_callback;
    bool m_started = false;
    bool m_finished = false;
    bool m_promptCompleted = false;
    bool m_contextDestroyed = false;
    bool m_cleanupStarted = false;
};

SecretServiceOperation *createOperation(OperationType type, const QString &label,
                                         const QString &password, const QString &itemPath,
                                         SecretServiceOperation::Callback callback, QObject *context)
{
    auto *operation = new SecretServiceOperation(type, label, password, itemPath,
                                                   std::move(callback));
    if (context != nullptr) {
        QObject::connect(context, &QObject::destroyed, operation,
                         &SecretServiceOperation::cancelForContext);
    }
    return operation;
}

template<typename Callback>
SecretServiceOperation::Callback wrapValueCallback(Callback callback)
{
    return [callback](bool success, const QString &value,
                       const QString &error) {
        if (callback) {
            callback(success, value, error);
        }
    };
}

SecretServiceOperation::Callback wrapDeleteCallback(
    const DCalDavCredentialStore::DeleteCallback &callback)
{
    return [callback](bool success, const QString &, const QString &error) {
        if (callback) {
            callback(success, error);
        }
    };
}

template<typename Start>
bool runSynchronously(Start &&start, QString &value, QString *errorMessage)
{
    QThread thread;
    QSemaphore completed;
    bool success = false;
    auto *operation = start([&](bool operationSuccess, const QString &operationValue,
                                const QString &operationError) {
        success = operationSuccess;
        value = operationValue;
        setError(errorMessage, operationError);
        completed.release();
        thread.quit();
    });
    QPointer<SecretServiceOperation> operationGuard(operation);
    operation->moveToThread(&thread);
    QObject::connect(&thread, &QThread::started, operation, &SecretServiceOperation::start,
                     Qt::QueuedConnection);
    thread.start();
    completed.acquire();
    thread.wait();
    if (operationGuard != nullptr) {
        delete operationGuard.data();
    }
    return success;
}

} // namespace

bool DCalDavCredentialStore::storePassword(const QString &label, const QString &password,
                                           QString &credentialRef, QString *errorMessage)
{
    credentialRef.clear();
    QString value;
    const bool success = runSynchronously(
        [&](const SecretServiceOperation::Callback &callback) {
            return new SecretServiceOperation(OperationType::Store, label, password, QString(),
                                              callback);
        }, value, errorMessage);
    if (success) {
        credentialRef = value;
    }
    return success;
}

bool DCalDavCredentialStore::readPassword(const QString &credentialRef, QString &password,
                                          QString *errorMessage)
{
    password.clear();
    QString itemPath;
    if (!parseItemPath(credentialRef, itemPath)) {
        setError(errorMessage, QStringLiteral("Invalid Secret Service credential reference."));
        return false;
    }
    return runSynchronously(
        [&](const SecretServiceOperation::Callback &callback) {
            return new SecretServiceOperation(OperationType::Read, QString(), QString(), itemPath,
                                              callback);
        }, password, errorMessage);
}

bool DCalDavCredentialStore::deletePassword(const QString &credentialRef, QString *errorMessage)
{
    QString itemPath;
    if (!parseItemPath(credentialRef, itemPath)) {
        setError(errorMessage, QStringLiteral("Invalid Secret Service credential reference."));
        return false;
    }
    QString ignored;
    return runSynchronously(
        [&](const SecretServiceOperation::Callback &callback) {
            return new SecretServiceOperation(OperationType::Delete, QString(), QString(), itemPath,
                                              callback);
        }, ignored, errorMessage);
}

void DCalDavCredentialStore::storePasswordAsync(const QString &label, const QString &password,
                                                const StoreCallback &callback, QObject *context)
{
    auto *operation = createOperation(OperationType::Store, label, password, QString(),
                                      wrapValueCallback(callback), context);
    QTimer::singleShot(0, operation, &SecretServiceOperation::start);
}

void DCalDavCredentialStore::readPasswordAsync(const QString &credentialRef,
                                               const ReadCallback &callback, QObject *context)
{
    QString itemPath;
    if (!parseItemPath(credentialRef, itemPath)) {
        if (callback) {
            callback(false, QString(), QStringLiteral("Invalid Secret Service credential reference."));
        }
        return;
    }
    auto *operation = createOperation(
        OperationType::Read, QString(), QString(), itemPath, wrapValueCallback(callback), context);
    QTimer::singleShot(0, operation, &SecretServiceOperation::start);
}

void DCalDavCredentialStore::deletePasswordAsync(const QString &credentialRef,
                                                 const DeleteCallback &callback, QObject *context)
{
    QString itemPath;
    if (!parseItemPath(credentialRef, itemPath)) {
        if (callback) {
            callback(false, QStringLiteral("Invalid Secret Service credential reference."));
        }
        return;
    }
    auto *operation = createOperation(
        OperationType::Delete, QString(), QString(), itemPath, wrapDeleteCallback(callback), context);
    QTimer::singleShot(0, operation, &SecretServiceOperation::start);
}

bool DCalDavCredentialStore::parseItemPath(const QString &credentialRef, QString &itemPath)
{
    itemPath.clear();
    const QString prefix = QStringLiteral("secret-service:");
    if (!credentialRef.startsWith(prefix)) {
        return false;
    }

    itemPath = credentialRef.mid(prefix.size());
    static const QRegularExpression objectPathPattern(QStringLiteral("^(/(?:[A-Za-z0-9_]+))+$"));
    if (!objectPathPattern.match(itemPath).hasMatch() || itemPath.size() > 512) {
        itemPath.clear();
        return false;
    }
    return true;
}

#include "dcaldavcredentialstore.moc"
