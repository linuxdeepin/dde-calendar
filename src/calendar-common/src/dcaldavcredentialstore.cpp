// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavcredentialstore.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusVariant>
#include <QEventLoop>
#include <QMap>
#include <QVariant>
#include <QVariantMap>
#include <QRegularExpression>
#include <QTimer>

typedef QMap<QString, QString> DCalDavSecretAttributes;
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

class SecretPromptWaiter final : public QObject
{
    Q_OBJECT
public:
    explicit SecretPromptWaiter(QEventLoop *eventLoop)
        : m_eventLoop(eventLoop)
    {
    }

    bool completed() const
    {
        return m_completed;
    }

    bool dismissed() const
    {
        return m_dismissed;
    }

public slots:
    void slotCompleted(bool dismissed)
    {
        m_dismissed = dismissed;
        m_completed = true;
        if (m_eventLoop != nullptr) {
            m_eventLoop->quit();
        }
    }

private:
    QEventLoop *m_eventLoop = nullptr;
    bool m_completed = false;
    bool m_dismissed = false;
};

bool promptAndWait(QDBusConnection bus, const QDBusObjectPath &promptPath,
                  QString *errorMessage)
{
    if (isRootPath(promptPath)) {
        return true;
    }
    if (!bus.isConnected() || promptPath.path().isEmpty()) {
        setError(errorMessage, QStringLiteral("Secret Service prompt is unavailable."));
        return false;
    }

    QDBusInterface prompt(QLatin1String(SecretServiceName), promptPath.path(),
                          QLatin1String(SecretPromptInterface), bus);
    prompt.setTimeout(kSecretServiceTimeoutMs);
    if (!prompt.isValid()) {
        setError(errorMessage, QStringLiteral("Secret Service prompt is unavailable."));
        return false;
    }

    QEventLoop eventLoop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    SecretPromptWaiter waiter(&eventLoop);
    if (!bus.connect(QLatin1String(SecretServiceName), promptPath.path(),
                     QLatin1String(SecretPromptInterface), QStringLiteral("Completed"),
                     &waiter, SLOT(slotCompleted(bool)))) {
        setError(errorMessage, QStringLiteral("Secret Service prompt could not be monitored."));
        return false;
    }

    const QDBusMessage reply = prompt.call(QStringLiteral("Prompt"), QString());
    if (reply.type() == QDBusMessage::ErrorMessage) {
        bus.disconnect(QLatin1String(SecretServiceName), promptPath.path(),
                      QLatin1String(SecretPromptInterface), QStringLiteral("Completed"),
                      &waiter, SLOT(slotCompleted(bool)));
        setError(errorMessage, QStringLiteral("Secret Service prompt could not be shown."));
        return false;
    }

    QObject::connect(&timeoutTimer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    timeoutTimer.start(kSecretPromptTimeoutMs);
    if (!waiter.completed()) {
        eventLoop.exec();
    }
    bus.disconnect(QLatin1String(SecretServiceName), promptPath.path(),
                   QLatin1String(SecretPromptInterface), QStringLiteral("Completed"),
                   &waiter, SLOT(slotCompleted(bool)));

    if (!waiter.completed()) {
        setError(errorMessage, QStringLiteral("Secret Service prompt timed out."));
        return false;
    }
    if (waiter.dismissed()) {
        setError(errorMessage, QStringLiteral("Secret Service prompt was dismissed."));
        return false;
    }
    return true;
}

bool openSession(const QDBusConnection &bus, QDBusObjectPath &sessionPath, QString *errorMessage)
{
    if (!bus.isConnected()) {
        setError(errorMessage, QStringLiteral("Session D-Bus is unavailable."));
        return false;
    }

    QDBusInterface service(QLatin1String(SecretServiceName),
                           QLatin1String(SecretServicePath),
                           QLatin1String(SecretServiceInterface), bus);
    service.setTimeout(kSecretServiceTimeoutMs);
    if (!service.isValid()) {
        setError(errorMessage, QStringLiteral("Secret Service is unavailable."));
        return false;
    }

    const QDBusMessage reply = service.call(
        QStringLiteral("OpenSession"), QStringLiteral("plain"),
        QVariant::fromValue(QDBusVariant(QString())));
    if (reply.type() == QDBusMessage::ErrorMessage || reply.arguments().size() < 2) {
        setError(errorMessage, QStringLiteral("Secret Service session could not be opened."));
        return false;
    }

    sessionPath = reply.arguments().at(1).value<QDBusObjectPath>();
    if (sessionPath.path().isEmpty()) {
        setError(errorMessage, QStringLiteral("Secret Service returned an invalid session."));
        return false;
    }
    return true;
}

void closeSession(const QDBusConnection &bus, const QDBusObjectPath &sessionPath)
{
    if (sessionPath.path().isEmpty()) {
        return;
    }
    QDBusInterface service(QLatin1String(SecretServiceName),
                           QLatin1String(SecretServicePath),
                           QLatin1String(SecretServiceInterface), bus);
    if (service.isValid()) {
        service.setTimeout(kSecretServiceTimeoutMs);
        service.call(QStringLiteral("CloseSession"), QVariant::fromValue(sessionPath));
    }
}

} // namespace

bool DCalDavCredentialStore::storePassword(const QString &label, const QString &password,
                                           QString &credentialRef, QString *errorMessage)
{
    credentialRef.clear();
    if (label.trimmed().isEmpty() || password.isEmpty()) {
        setError(errorMessage, QStringLiteral("CalDAV credential label or password is empty."));
        return false;
    }

    const QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusObjectPath sessionPath;
    if (!openSession(bus, sessionPath, errorMessage)) {
        return false;
    }

    qDBusRegisterMetaType<DCalDavSecretAttributes>();
    QVariantMap properties;
    properties.insert(QStringLiteral("org.freedesktop.Secret.Item.Label"), label);
    DCalDavSecretAttributes attributes;
    attributes.insert(QStringLiteral("application"), QLatin1String(CalendarApplicationAttribute));
    properties.insert(QStringLiteral("org.freedesktop.Secret.Item.Attributes"),
                      QVariant::fromValue(attributes));

    QDBusArgument secret;
    secret.beginStructure();
    secret << sessionPath << QByteArray() << password.toUtf8() << QStringLiteral("text/plain");
    secret.endStructure();

    QDBusInterface collection(QLatin1String(SecretServiceName),
                              QLatin1String(DefaultCollectionPath),
                              QLatin1String(SecretCollectionInterface), bus);
    collection.setTimeout(kSecretServiceTimeoutMs);
    const QDBusMessage reply = collection.call(
        QStringLiteral("CreateItem"), properties, QVariant::fromValue(secret), false);
    if (reply.type() == QDBusMessage::ErrorMessage || reply.arguments().size() < 2) {
        closeSession(bus, sessionPath);
        setError(errorMessage, QStringLiteral("Secret Service item could not be created."));
        return false;
    }

    const QDBusObjectPath itemPath = reply.arguments().at(0).value<QDBusObjectPath>();
    const QDBusObjectPath promptPath = reply.arguments().at(1).value<QDBusObjectPath>();
    if (!promptAndWait(bus, promptPath, errorMessage)) {
        closeSession(bus, sessionPath);
        return false;
    }
    closeSession(bus, sessionPath);
    if (itemPath.path().isEmpty() || itemPath.path() == QStringLiteral("/")) {
        setError(errorMessage, QStringLiteral("Secret Service returned an invalid item."));
        return false;
    }

    credentialRef = QStringLiteral("secret-service:") + itemPath.path();
    return true;
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

    const QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusObjectPath sessionPath;
    if (!openSession(bus, sessionPath, errorMessage)) {
        return false;
    }

    bool success = false;
    QDBusInterface item(QLatin1String(SecretServiceName), itemPath,
                        QLatin1String(SecretItemInterface), bus);
    item.setTimeout(kSecretServiceTimeoutMs);
    if (item.isValid()) {
        const QDBusMessage reply = item.call(
            QStringLiteral("GetSecret"), QVariant::fromValue(sessionPath));
        if (reply.type() != QDBusMessage::ErrorMessage && !reply.arguments().isEmpty()) {
            const QDBusArgument secret = reply.arguments().first().value<QDBusArgument>();
            QDBusObjectPath returnedSession;
            QByteArray parameters;
            QByteArray value;
            QString contentType;
            secret.beginStructure();
            secret >> returnedSession >> parameters >> value >> contentType;
            secret.endStructure();
            if (returnedSession == sessionPath && !value.isNull()) {
                password = QString::fromUtf8(value);
                success = true;
            }
        }
    }
    closeSession(bus, sessionPath);

    if (!success) {
        setError(errorMessage, QStringLiteral("Secret Service item could not be read."));
    }
    return success;
}

bool DCalDavCredentialStore::deletePassword(const QString &credentialRef, QString *errorMessage)
{
    QString itemPath;
    if (!parseItemPath(credentialRef, itemPath)) {
        setError(errorMessage, QStringLiteral("Invalid Secret Service credential reference."));
        return false;
    }

    const QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        setError(errorMessage, QStringLiteral("Session D-Bus is unavailable."));
        return false;
    }

    QDBusInterface item(QLatin1String(SecretServiceName), itemPath,
                        QLatin1String(SecretItemInterface), bus);
    item.setTimeout(kSecretServiceTimeoutMs);
    if (!item.isValid()) {
        setError(errorMessage, QStringLiteral("Secret Service item is unavailable."));
        return false;
    }

    const QDBusMessage reply = item.call(QStringLiteral("Delete"));
    if (reply.type() == QDBusMessage::ErrorMessage || reply.arguments().isEmpty()) {
        setError(errorMessage, QStringLiteral("Secret Service item could not be deleted."));
        return false;
    }

    const QDBusObjectPath promptPath = reply.arguments().first().value<QDBusObjectPath>();
    return promptAndWait(bus, promptPath, errorMessage);
}

bool DCalDavCredentialStore::parseItemPath(const QString &credentialRef, QString &itemPath)
{
    itemPath.clear();
    const QString prefix = QStringLiteral("secret-service:");
    if (!credentialRef.startsWith(prefix)) {
        return false;
    }

    itemPath = credentialRef.mid(prefix.size());
    static const QRegularExpression objectPathPattern(
        QStringLiteral("^(/(?:[A-Za-z0-9_]+))+$"));
    if (!objectPathPattern.match(itemPath).hasMatch() || itemPath.size() > 512) {
        itemPath.clear();
        return false;
    }
    return true;
}

#include "dcaldavcredentialstore.moc"
