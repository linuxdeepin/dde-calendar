// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavtransport.h"
#include "dcaldavutils.h"
#include "dcaldavxmlreader.h"
#include "mock_caldav_server.h"

#include <QAuthenticator>
#include <QCryptographicHash>
#include <QEventLoop>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>

#include <gtest/gtest.h>

namespace {

bool createMockTlsCredentials(QByteArray &certificate, QByteArray &privateKey)
{
    const QString opensslPath = QStandardPaths::findExecutable(QStringLiteral("openssl"));
    if (opensslPath.isEmpty()) {
        return false;
    }

    QTemporaryDir directory;
    if (!directory.isValid()) {
        return false;
    }

    const QString certificatePath = directory.filePath(QStringLiteral("certificate.pem"));
    const QString keyPath = directory.filePath(QStringLiteral("key.pem"));

    QProcess process;
    process.start(opensslPath, {
        QStringLiteral("req"), QStringLiteral("-x509"),
        QStringLiteral("-newkey"), QStringLiteral("rsa:2048"),
        QStringLiteral("-keyout"), keyPath,
        QStringLiteral("-out"), certificatePath,
        QStringLiteral("-days"), QStringLiteral("1"),
        QStringLiteral("-nodes"),
        QStringLiteral("-subj"), QStringLiteral("/CN=localhost"),
    });
    if (!process.waitForFinished(10000)) {
        return false;
    }

    QFile certFile(certificatePath);
    if (!certFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    certificate = certFile.readAll();

    QFile keyFile(keyPath);
    if (!keyFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    privateKey = keyFile.readAll();
    return !certificate.isEmpty() && !privateKey.isEmpty();
}

void trustMockCertificate(const QByteArray &certificate)
{
    QSslConfiguration configuration = QSslConfiguration::defaultConfiguration();
    configuration.addCaCertificate(QSslCertificate(certificate, QSsl::Pem));
    QSslConfiguration::setDefaultConfiguration(configuration);
}

} // namespace

// ---------------------------------------------------------------------------
// DCalDavTransport::normalizeUrl — edge cases
// ---------------------------------------------------------------------------

TEST(CalDavTransportExtras, NormalizesEmptyUrlReturnsInvalid)
{
    EXPECT_FALSE(DCalDavTransport::normalizeUrl(QUrl()).isValid());
    EXPECT_FALSE(DCalDavTransport::normalizeUrl(QUrl(QStringLiteral("  "))).isValid());
}

TEST(CalDavTransportExtras, NormalizesSchemeRelativeUrl)
{
    EXPECT_EQ(QStringLiteral("https://example.com/path"),
              DCalDavTransport::normalizeUrl(QUrl(QStringLiteral("//example.com/path"))).toString());
}

TEST(CalDavTransportExtras, NormalizesHttpSchemeRejected)
{
    EXPECT_FALSE(DCalDavTransport::normalizeUrl(QUrl(QStringLiteral("http://example.com"))).isValid());
}

TEST(CalDavTransportExtras, NormalizesUrlWithFragmentStripped)
{
    const QUrl result = DCalDavTransport::normalizeUrl(QUrl(QStringLiteral("https://example.com/path#section")));
    EXPECT_EQ(QStringLiteral("https://example.com/path"), result.toString());
    EXPECT_TRUE(result.fragment().isEmpty());
}

TEST(CalDavTransportExtras, NormalizesUrlWithPasswordRejected)
{
    EXPECT_FALSE(DCalDavTransport::normalizeUrl(QUrl(QStringLiteral("https://user:pass@example.com"))).isValid());
}

TEST(CalDavTransportExtras, NormalizesUrlWithEmptyHostRejected)
{
    EXPECT_FALSE(DCalDavTransport::normalizeUrl(QUrl(QStringLiteral("https://"))).isValid());
}

TEST(CalDavTransportExtras, NormalizesUrlLowercasesScheme)
{
    const QUrl result = DCalDavTransport::normalizeUrl(QUrl(QStringLiteral("HTTPS://example.com/path")));
    EXPECT_EQ(QStringLiteral("https"), result.scheme());
}

TEST(CalDavTransportExtras, NormalizesUrlWithExplicitPort)
{
    const QUrl result = DCalDavTransport::normalizeUrl(QUrl(QStringLiteral("https://example.com:8443/path")));
    EXPECT_EQ(8443, result.port());
    EXPECT_EQ(QStringLiteral("https://example.com:8443/path"), result.toString());
}

// ---------------------------------------------------------------------------
// DCalDavTransport::isSecureUrl
// ---------------------------------------------------------------------------

TEST(CalDavTransportExtras, IsSecureUrlAcceptsHttps)
{
    EXPECT_TRUE(DCalDavTransport::isSecureUrl(QUrl(QStringLiteral("https://example.com/path"))));
}

TEST(CalDavTransportExtras, IsSecureUrlRejectsHttp)
{
    EXPECT_FALSE(DCalDavTransport::isSecureUrl(QUrl(QStringLiteral("http://example.com"))));
}

TEST(CalDavTransportExtras, IsSecureUrlRejectsEmpty)
{
    EXPECT_FALSE(DCalDavTransport::isSecureUrl(QUrl()));
}

// ---------------------------------------------------------------------------
// DCalDavTransport::isSameOrigin — port edge cases
// ---------------------------------------------------------------------------

TEST(CalDavTransportExtras, IsSameOriginWithExplicitPort443)
{
    EXPECT_TRUE(DCalDavTransport::isSameOrigin(
        QUrl(QStringLiteral("https://example.com:443/a")),
        QUrl(QStringLiteral("https://example.com/b"))));
}

TEST(CalDavTransportExtras, IsSameOriginDifferentPorts)
{
    EXPECT_FALSE(DCalDavTransport::isSameOrigin(
        QUrl(QStringLiteral("https://example.com:8443/a")),
        QUrl(QStringLiteral("https://example.com:9443/b"))));
}

TEST(CalDavTransportExtras, IsSameOriginDifferentHosts)
{
    EXPECT_FALSE(DCalDavTransport::isSameOrigin(
        QUrl(QStringLiteral("https://a.example.com")),
        QUrl(QStringLiteral("https://b.example.com"))));
}

// ---------------------------------------------------------------------------
// DCalDavTransport::urlForLog
// ---------------------------------------------------------------------------

TEST(CalDavTransportExtras, UrlForLogReturnsHashedHost)
{
    const QString log = DCalDavTransport::urlForLog(QUrl(QStringLiteral("https://example.com/path")));
    EXPECT_TRUE(log.startsWith(QStringLiteral("caldav-host:")));

    const QByteArray expectedHash = QCryptographicHash::hash(
        QByteArrayLiteral("example.com"), QCryptographicHash::Sha256).toHex().left(12);
    EXPECT_EQ(QStringLiteral("caldav-host:") + QString::fromLatin1(expectedHash), log);
}

TEST(CalDavTransportExtras, UrlForLogInvalidUrl)
{
    EXPECT_EQ(QStringLiteral("<invalid-url>"), DCalDavTransport::urlForLog(QUrl()));
    EXPECT_EQ(QStringLiteral("<invalid-url>"), DCalDavTransport::urlForLog(QUrl(QStringLiteral("http://example.com"))));
}

TEST(CalDavTransportExtras, UrlForLogConsistentForSameHost)
{
    const QString log1 = DCalDavTransport::urlForLog(QUrl(QStringLiteral("https://example.com/path1")));
    const QString log2 = DCalDavTransport::urlForLog(QUrl(QStringLiteral("https://example.com/path2")));
    EXPECT_EQ(log1, log2);
}

TEST(CalDavTransportExtras, UrlForLogDifferentForDifferentHost)
{
    const QString log1 = DCalDavTransport::urlForLog(QUrl(QStringLiteral("https://host1.com")));
    const QString log2 = DCalDavTransport::urlForLog(QUrl(QStringLiteral("https://host2.com")));
    EXPECT_NE(log1, log2);
}

// ---------------------------------------------------------------------------
// DCalDavTransport::classifyError — exhaustive branches
// ---------------------------------------------------------------------------

TEST(CalDavTransportExtras, ClassifyErrorResponseTooLargeHasPriority)
{
    EXPECT_EQ(DCalDavTransport::ResponseTooLarge,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 0, true, true, true));
}

TEST(CalDavTransportExtras, ClassifyErrorCertificateInvalidPriority)
{
    EXPECT_EQ(DCalDavTransport::CertificateInvalid,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 0, true, true, false));
}

TEST(CalDavTransportExtras, ClassifyErrorTimedOut)
{
    EXPECT_EQ(DCalDavTransport::RequestTimedOut,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 0, false, true, false));
}

TEST(CalDavTransportExtras, ClassifyError403)
{
    EXPECT_EQ(DCalDavTransport::PermissionDenied,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 403, false, false, false));
}

TEST(CalDavTransportExtras, ClassifyError500Family)
{
    EXPECT_EQ(DCalDavTransport::ServerUnavailable,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 500, false, false, false));
    EXPECT_EQ(DCalDavTransport::ServerUnavailable,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 502, false, false, false));
    EXPECT_EQ(DCalDavTransport::ServerUnavailable,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 503, false, false, false));
    EXPECT_EQ(DCalDavTransport::ServerUnavailable,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 504, false, false, false));
}

TEST(CalDavTransportExtras, ClassifyErrorHttpStatusPositiveReturnsNoError)
{
    EXPECT_EQ(DCalDavTransport::NoError,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 200, false, false, false));
    EXPECT_EQ(DCalDavTransport::NoError,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 404, false, false, false));
    EXPECT_EQ(DCalDavTransport::NoError,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 409, false, false, false));
}

TEST(CalDavTransportExtras, ClassifyErrorConnectionRefused)
{
    EXPECT_EQ(DCalDavTransport::NetworkUnavailable,
              DCalDavTransport::classifyError(QNetworkReply::ConnectionRefusedError, 0, false, false, false));
}

TEST(CalDavTransportExtras, ClassifyErrorTemporaryNetworkFailure)
{
    EXPECT_EQ(DCalDavTransport::NetworkUnavailable,
              DCalDavTransport::classifyError(QNetworkReply::TemporaryNetworkFailureError, 0, false, false, false));
}

TEST(CalDavTransportExtras, ClassifyErrorNetworkSessionFailed)
{
    EXPECT_EQ(DCalDavTransport::NetworkUnavailable,
              DCalDavTransport::classifyError(QNetworkReply::NetworkSessionFailedError, 0, false, false, false));
}

TEST(CalDavTransportExtras, ClassifyErrorGenericNetworkError)
{
    EXPECT_EQ(DCalDavTransport::NetworkError,
              DCalDavTransport::classifyError(QNetworkReply::TimeoutError, 0, false, false, false));
    EXPECT_EQ(DCalDavTransport::NetworkError,
              DCalDavTransport::classifyError(QNetworkReply::SslHandshakeFailedError, 0, false, false, false));
}

// ---------------------------------------------------------------------------
// DCalDavTransport::cancel
// ---------------------------------------------------------------------------

TEST(CalDavTransportExtras, CancelWithNoActiveRepliesDoesNotCrash)
{
    DCalDavTransport transport;
    transport.cancel();
}

TEST(CalDavTransportExtras, CancelAbortsActiveSend)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseDelay(10000);

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/calendars/user/event.ics")));
    request.method = "GET";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");
    request.timeoutMilliseconds = 30000;

    bool callbackCalled = false;
    transport.send(request, [&](const DCalDavTransport::Response &) {
        callbackCalled = true;
    });

    QTimer::singleShot(100, &transport, [&]() { transport.cancel(); });

    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();

    // cancel() disconnects all reply signals and aborts, so the callback
    // is intentionally NOT invoked — the request is simply discarded.
    EXPECT_FALSE(callbackCalled);
}

// ---------------------------------------------------------------------------
// DCalDavTransport::send — invalid URL rejected
// ---------------------------------------------------------------------------

TEST(CalDavTransportExtras, SendInvalidUrlReturnsError)
{
    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = QUrl(QStringLiteral("http://example.com"));
    request.method = "GET";

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &transportResponse) {
        response = transportResponse;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(3000, &loop, &QEventLoop::quit);
    if (!callbackCalled) {
        loop.exec();
    }
    ASSERT_TRUE(callbackCalled);
    EXPECT_NE(DCalDavTransport::NoError, response.error);
}

// ---------------------------------------------------------------------------
// DCalDavUtils::transportErrorText
// ---------------------------------------------------------------------------

TEST(CalDavUtilsExtras, TransportErrorTextCertificateInvalid)
{
    DCalDavTransport::Response response;
    response.error = DCalDavTransport::CertificateInvalid;
    EXPECT_EQ(QStringLiteral("The server certificate is invalid."), DCalDavUtils::transportErrorText(response));
}

TEST(CalDavUtilsExtras, TransportErrorTextAuthenticationFailed)
{
    DCalDavTransport::Response response;
    response.error = DCalDavTransport::AuthenticationFailed;
    EXPECT_EQ(QStringLiteral("Incorrect username or password. Please try again."), DCalDavUtils::transportErrorText(response));
}

TEST(CalDavUtilsExtras, TransportErrorTextNetworkUnavailable)
{
    DCalDavTransport::Response response;
    response.error = DCalDavTransport::NetworkUnavailable;
    EXPECT_EQ(QStringLiteral("Unable to connect to the server. Please check your network connection and server address."),
              DCalDavUtils::transportErrorText(response));
}

TEST(CalDavUtilsExtras, TransportErrorTextRequestTimedOut)
{
    DCalDavTransport::Response response;
    response.error = DCalDavTransport::RequestTimedOut;
    EXPECT_EQ(QStringLiteral("The server request timed out."), DCalDavUtils::transportErrorText(response));
}

TEST(CalDavUtilsExtras, TransportErrorTextPermissionDenied)
{
    DCalDavTransport::Response response;
    response.error = DCalDavTransport::PermissionDenied;
    EXPECT_EQ(QStringLiteral("The server denied access."), DCalDavUtils::transportErrorText(response));
}

TEST(CalDavUtilsExtras, TransportErrorTextDefaultFallback)
{
    DCalDavTransport::Response response;
    response.error = DCalDavTransport::RateLimited;
    response.httpStatus = 429;
    const QString text = DCalDavUtils::transportErrorText(response);
    EXPECT_TRUE(text.contains(QStringLiteral("429"))) << text.toStdString();
    EXPECT_TRUE(text.contains(QString::number(static_cast<int>(DCalDavTransport::RateLimited)))) << text.toStdString();
}

TEST(CalDavUtilsExtras, TransportErrorTextDefaultNoError)
{
    DCalDavTransport::Response response;
    response.error = DCalDavTransport::NoError;
    response.httpStatus = 200;
    const QString text = DCalDavUtils::transportErrorText(response);
    EXPECT_TRUE(text.contains(QStringLiteral("200"))) << text.toStdString();
}

// ---------------------------------------------------------------------------
// DCalDavUtils::eventUrl
// ---------------------------------------------------------------------------

TEST(CalDavUtilsExtras, EventUrlWithTrailingSlash)
{
    DCalDavCalendarInfo calendar;
    calendar.href = QStringLiteral("https://example.com/calendars/user/work/");
    const QUrl url = DCalDavUtils::eventUrl(calendar, QStringLiteral("event-123"));
    EXPECT_EQ(QStringLiteral("/calendars/user/work/event-123.ics"), url.path());
    EXPECT_EQ(QStringLiteral("https"), url.scheme());
    EXPECT_EQ(QStringLiteral("example.com"), url.host());
}

TEST(CalDavUtilsExtras, EventUrlWithoutTrailingSlash)
{
    DCalDavCalendarInfo calendar;
    calendar.href = QStringLiteral("https://example.com/calendars/user/work");
    const QUrl url = DCalDavUtils::eventUrl(calendar, QStringLiteral("event-123"));
    EXPECT_EQ(QStringLiteral("/calendars/user/work/event-123.ics"), url.path());
}

TEST(CalDavUtilsExtras, EventUrlPercentEncodesUid)
{
    DCalDavCalendarInfo calendar;
    calendar.href = QStringLiteral("https://example.com/cal/");
    const QUrl url = DCalDavUtils::eventUrl(calendar, QStringLiteral("uid with spaces"));
    EXPECT_TRUE(url.path().endsWith(QStringLiteral(".ics")));
    EXPECT_FALSE(url.path().contains(QLatin1Char(' ')));
}

TEST(CalDavUtilsExtras, EventUrlPreservesSpecialCharacters)
{
    DCalDavCalendarInfo calendar;
    calendar.href = QStringLiteral("https://example.com/cal/");
    const QUrl url = DCalDavUtils::eventUrl(calendar, QStringLiteral("uid-@-test"));
    EXPECT_TRUE(url.path().endsWith(QStringLiteral(".ics")));
}

// ---------------------------------------------------------------------------
// DCalDavXmlStreamReader — readNext (indirect), readNextStartElement, skipCurrentElement
// ---------------------------------------------------------------------------

TEST(CalDavXmlReaderExtras, ReadNextStartElementAdvancesToFirstElement)
{
    DCalDavXmlStreamReader reader(QByteArrayLiteral("<root><child/></root>"));
    EXPECT_TRUE(reader.readNextStartElement());
    EXPECT_EQ(QStringLiteral("root"), reader.name());
    EXPECT_TRUE(reader.readNextStartElement());
    EXPECT_EQ(QStringLiteral("child"), reader.name());
}

TEST(CalDavXmlReaderExtras, ReadNextStartElementReturnsFalseAtEnd)
{
    DCalDavXmlStreamReader reader(QByteArrayLiteral("<root/>"));
    ASSERT_TRUE(reader.readNextStartElement());
    // After root's StartElement, readNextStartElement encounters EndElement and returns false
    EXPECT_FALSE(reader.readNextStartElement());
}

TEST(CalDavXmlReaderExtras, SkipCurrentElementSkipsChildren)
{
    DCalDavXmlStreamReader reader(
        QByteArrayLiteral("<root><child><grandchild/></child><sibling/></root>"));
    ASSERT_TRUE(reader.readNextStartElement());
    EXPECT_EQ(QStringLiteral("root"), reader.name());
    ASSERT_TRUE(reader.readNextStartElement());
    EXPECT_EQ(QStringLiteral("child"), reader.name());
    reader.skipCurrentElement();
    EXPECT_TRUE(reader.readNextStartElement());
    EXPECT_EQ(QStringLiteral("sibling"), reader.name());
}

TEST(CalDavXmlReaderExtras, SkipCurrentElementAtRootLevel)
{
    DCalDavXmlStreamReader reader(QByteArrayLiteral("<root><a><child/></a><b/></root>"));
    ASSERT_TRUE(reader.readNextStartElement());
    EXPECT_EQ(QStringLiteral("root"), reader.name());
    ASSERT_TRUE(reader.readNextStartElement());
    EXPECT_EQ(QStringLiteral("a"), reader.name());
    reader.skipCurrentElement();
    EXPECT_TRUE(reader.readNextStartElement());
    EXPECT_EQ(QStringLiteral("b"), reader.name());
}

// ---------------------------------------------------------------------------
// DCalDavXmlStreamReader — readElementText with ErrorOnUnexpectedElement
// ---------------------------------------------------------------------------

TEST(CalDavXmlReaderExtras, ReadElementTextErrorOnUnexpectedElement)
{
    DCalDavXmlStreamReader reader(
        QByteArrayLiteral("<root>before<child>inside</child>after</root>"));
    ASSERT_TRUE(reader.readNextStartElement());
    const QString text = reader.readElementText(QXmlStreamReader::ErrorOnUnexpectedElement);
    EXPECT_EQ(QStringLiteral("before"), text);
    EXPECT_TRUE(reader.hasError());
}

TEST(CalDavXmlReaderExtras, ReadElementTextErrorOnUnexpectedElementRaisesError)
{
    DCalDavXmlStreamReader reader(
        QByteArrayLiteral("<root>text<nested/></root>"));
    ASSERT_TRUE(reader.readNextStartElement());
    reader.readElementText(QXmlStreamReader::ErrorOnUnexpectedElement);
    EXPECT_TRUE(reader.hasError());
    EXPECT_FALSE(reader.errorString().isEmpty());
}

TEST(CalDavXmlReaderExtras, ReadElementTextSkipChildElements)
{
    DCalDavXmlStreamReader reader(
        QByteArrayLiteral("<root>before<child>inside</child>after</root>"));
    ASSERT_TRUE(reader.readNextStartElement());
    const QString text = reader.readElementText(QXmlStreamReader::SkipChildElements);
    EXPECT_EQ(QStringLiteral("beforeafter"), text);
    EXPECT_FALSE(reader.hasError());
}

TEST(CalDavXmlReaderExtras, ReadElementTextEmptyElement)
{
    DCalDavXmlStreamReader reader(QByteArrayLiteral("<root/>"));
    ASSERT_TRUE(reader.readNextStartElement());
    const QString text = reader.readElementText(QXmlStreamReader::SkipChildElements);
    EXPECT_TRUE(text.isEmpty());
    EXPECT_FALSE(reader.hasError());
}

TEST(CalDavXmlReaderExtras, ReadElementTextEntityReference)
{
    DCalDavXmlStreamReader reader(QByteArrayLiteral("<root>a&amp;b</root>"));
    ASSERT_TRUE(reader.readNextStartElement());
    const QString text = reader.readElementText(QXmlStreamReader::SkipChildElements);
    EXPECT_EQ(QStringLiteral("a&b"), text);
}

// ---------------------------------------------------------------------------
// DCalDavXmlStreamReader — name, atEnd, hasError, errorString, depthExceeded
// ---------------------------------------------------------------------------

TEST(CalDavXmlReaderExtras, NameReturnsCurrentElementName)
{
    DCalDavXmlStreamReader reader(QByteArrayLiteral("<foo/>"));
    ASSERT_TRUE(reader.readNextStartElement());
    EXPECT_EQ(QStringLiteral("foo"), reader.name());
}

TEST(CalDavXmlReaderExtras, AtEndReturnsTrueAfterParsingComplete)
{
    DCalDavXmlStreamReader reader(QByteArrayLiteral("<root/>"));
    ASSERT_TRUE(reader.readNextStartElement());
    EXPECT_FALSE(reader.readNextStartElement());
    // Advance past EndElement to reach EndDocument
    while (!reader.atEnd()) {
        reader.readNext();
    }
    EXPECT_TRUE(reader.atEnd());
}

TEST(CalDavXmlReaderExtras, HasErrorReturnsFalseForValidXml)
{
    DCalDavXmlStreamReader reader(QByteArrayLiteral("<root/>"));
    ASSERT_TRUE(reader.readNextStartElement());
    EXPECT_FALSE(reader.hasError());
    EXPECT_TRUE(reader.errorString().isEmpty() || reader.errorString() == QStringLiteral("No error"));
}

TEST(CalDavXmlReaderExtras, HasErrorReturnsTrueForMalformedXml)
{
    DCalDavXmlStreamReader reader(QByteArrayLiteral("<root><unclosed>"));
    ASSERT_TRUE(reader.readNextStartElement());
    ASSERT_TRUE(reader.readNextStartElement());
    EXPECT_FALSE(reader.readNextStartElement());
    EXPECT_TRUE(reader.hasError());
}

TEST(CalDavXmlReaderExtras, ErrorStringReturnsMessageForMalformedXml)
{
    DCalDavXmlStreamReader reader(QByteArrayLiteral("<root><unclosed>"));
    ASSERT_TRUE(reader.readNextStartElement());
    ASSERT_TRUE(reader.readNextStartElement());
    EXPECT_FALSE(reader.readNextStartElement());
    EXPECT_FALSE(reader.errorString().isEmpty());
}

TEST(CalDavXmlReaderExtras, DepthExceededReturnsFalseInitially)
{
    DCalDavXmlStreamReader reader(QByteArrayLiteral("<root/>"));
    EXPECT_FALSE(reader.depthExceeded());
}

TEST(CalDavXmlReaderExtras, DepthExceededReturnsTrueWhenMaximumExceeded)
{
    QByteArray xml;
    for (int i = 0; i < 70; ++i) {
        xml.append("<a>");
    }
    for (int i = 0; i < 70; ++i) {
        xml.append("</a>");
    }
    DCalDavXmlStreamReader reader(xml);
    while (reader.readNextStartElement()) {
    }
    EXPECT_TRUE(reader.depthExceeded());
}

// ---------------------------------------------------------------------------
// DCalDavXmlReader::parseDiscovery — edge cases
// ---------------------------------------------------------------------------

TEST(CalDavXmlReaderExtras, ParseDiscoveryRejectsTooLargeXml)
{
    const QByteArray xml(5 * 1024 * 1024, ' ');
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    EXPECT_FALSE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage));
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryRejectsTooManyCollections)
{
    QByteArray xml("<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">");
    for (int i = 0; i < 260; ++i) {
        xml.append("<d:response><d:href>/cal/");
        xml.append(QByteArray::number(i));
        xml.append("/</d:href><d:propstat><d:prop>"
                   "<d:resourcetype><d:collection/><c:calendar/></d:resourcetype>"
                   "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat></d:response>");
    }
    xml.append("</d:multistatus>");

    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    EXPECT_FALSE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage));
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryRejectsNonMultistatusRoot)
{
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    EXPECT_FALSE(DCalDavXmlReader::parseDiscovery("<foo/>", result, &errorMessage));
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryNullErrorMessage)
{
    DCalDavXmlReader::DiscoveryResult result;
    EXPECT_FALSE(DCalDavXmlReader::parseDiscovery("<foo/>", result, nullptr));
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryEmptyXml)
{
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    EXPECT_FALSE(DCalDavXmlReader::parseDiscovery(QByteArray(), result, &errorMessage));
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(CalDavXmlReaderExtras, ParseDiscoverySkipNonResponseElements)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\">"
        "<d:unknown><d:nested/></d:unknown>"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat><d:prop>"
        "<d:displayname>My Calendar</d:displayname>"
        "<d:resourcetype><d:collection/></d:resourcetype>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    // No calendar resourcetype → not a calendar collection
    EXPECT_EQ(0, result.calendarCollections.size());
    // Non-calendar response with displayName → principal display name fallback
    EXPECT_EQ(QStringLiteral("My Calendar"), result.principalDisplayName);
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryPropstatNon200SkipsProperties)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat><d:prop>"
        "<d:displayname>Should be skipped</d:displayname>"
        "<d:resourcetype><d:collection/><c:calendar/></d:resourcetype>"
        "</d:prop><d:status>HTTP/1.1 403 Forbidden</d:status></d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    EXPECT_EQ(0, result.calendarCollections.size());
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryReadPrivilegesWrite)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat><d:prop>"
        "<d:resourcetype><d:collection/><c:calendar/></d:resourcetype>"
        "<d:current-user-privilege-set>"
        "<d:privilege><d:write/></d:privilege>"
        "</d:current-user-privilege-set>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    ASSERT_EQ(1, result.calendarCollections.size());
    EXPECT_EQ(DCalDavXmlReader::WritePrivilege, result.calendarCollections.first().privileges);
    EXPECT_TRUE(result.calendarCollections.first().privilegesKnown);
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryReadPrivilegesWriteContentAndProperties)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat><d:prop>"
        "<d:resourcetype><d:collection/><c:calendar/></d:resourcetype>"
        "<d:current-user-privilege-set>"
        "<d:privilege><d:write-content/></d:privilege>"
        "<d:privilege><d:write-properties/></d:privilege>"
        "</d:current-user-privilege-set>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    ASSERT_EQ(1, result.calendarCollections.size());
    EXPECT_EQ(DCalDavXmlReader::WritePrivilege, result.calendarCollections.first().privileges);
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryPrivilegeSetWithUnknownPrivilege)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat><d:prop>"
        "<d:resourcetype><d:collection/><c:calendar/></d:resourcetype>"
        "<d:current-user-privilege-set>"
        "<d:privilege><d:unknown-privilege/></d:privilege>"
        "</d:current-user-privilege-set>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    ASSERT_EQ(1, result.calendarCollections.size());
    EXPECT_EQ(DCalDavXmlReader::NoPrivilege, result.calendarCollections.first().privileges);
    EXPECT_TRUE(result.calendarCollections.first().privilegesKnown);
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryMultipleResponsesMergeProperties)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>/principals/</d:href>"
        "<d:propstat><d:prop>"
        "<d:current-user-principal><d:href>/principals/user/</d:href></d:current-user-principal>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response>"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat><d:prop>"
        "<c:calendar-home-set><d:href>/calendars/</d:href></c:calendar-home-set>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    EXPECT_EQ(QStringLiteral("/principals/user/"), result.currentUserPrincipalHref);
    EXPECT_EQ(QStringLiteral("/calendars/"), result.calendarHomeSetHref);
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryOversizedHrefSilentlyDropped)
{
    QByteArray xml("<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat><d:prop>"
        "<d:resourcetype><d:collection/><c:calendar/></d:resourcetype>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response>"
        "<d:response><d:href>");
    xml.append(QByteArray(5000, 'x'));
    xml.append("</d:href><d:propstat><d:prop>"
        "<d:resourcetype><d:collection/><c:calendar/></d:resourcetype>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat></d:response></d:multistatus>");

    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    // Oversized hrefs are silently dropped at the response level, not treated as a parse error
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    EXPECT_EQ(1, result.calendarCollections.size());
    EXPECT_EQ(QStringLiteral("/cal/"), result.calendarCollections.first().href);
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryOversizedColorRejected)
{
    QByteArray xml("<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\" xmlns:apple=\"http://apple.com/ns/ical/\">"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat><d:prop>"
        "<d:resourcetype><d:collection/><c:calendar/></d:resourcetype>"
        "<apple:calendar-color>");
    xml.append(QByteArray(5000, 'x'));
    xml.append("</apple:calendar-color>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat></d:response></d:multistatus>");

    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    EXPECT_FALSE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage));
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryPrincipalDisplayNameFallback)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\">"
        "<d:response><d:href>/principals/</d:href>"
        "<d:propstat><d:prop>"
        "<d:displayname>My Principal</d:displayname>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    EXPECT_EQ(QStringLiteral("My Principal"), result.principalDisplayName);
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryOversizedPrincipalDisplayNameRejected)
{
    QByteArray xml("<d:multistatus xmlns:d=\"DAV:\">"
        "<d:response><d:href>/principals/</d:href>"
        "<d:propstat><d:prop>"
        "<d:displayname>");
    xml.append(QByteArray(5000, 'x'));
    xml.append("</d:displayname>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat></d:response></d:multistatus>");

    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    EXPECT_FALSE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage));
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryHttpStatusCodeParsing)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat><d:prop>"
        "<d:resourcetype><d:collection/><c:calendar/></d:resourcetype>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    ASSERT_EQ(1, result.calendarCollections.size());
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryMalformedHttpStatusDoesNotMerge)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat><d:prop>"
        "<d:displayname>Should be skipped</d:displayname>"
        "<d:resourcetype><d:collection/><c:calendar/></d:resourcetype>"
        "</d:prop><d:status>garbage</d:status></d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    EXPECT_EQ(0, result.calendarCollections.size());
}

TEST(CalDavXmlReaderExtras, ParseDiscoveryCalendarWithOversizedHrefNotAdded)
{
    QByteArray xml("<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>");
    xml.append(QByteArray(5000, 'x'));
    xml.append("</d:href><d:propstat><d:prop>"
        "<d:resourcetype><d:collection/><c:calendar/></d:resourcetype>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat></d:response></d:multistatus>");

    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    EXPECT_EQ(0, result.calendarCollections.size());
}

// ===========================================================================
// Gap-closing tests for dcaldavxmlreader.cpp
// ===========================================================================

// --- readElementText: Invalid token path (line 70-71) ---

TEST(CalDavXmlReaderGaps, ReadElementTextInvalidTokenReturnsText)
{
    DCalDavXmlStreamReader reader(QByteArray("<root>hello<"));
    ASSERT_TRUE(reader.readNextStartElement());
    const QString text = reader.readElementText(QXmlStreamReader::SkipChildElements);
    EXPECT_EQ(QStringLiteral("hello"), text);
    EXPECT_TRUE(reader.hasError());
}

// --- readElementText: end-of-loop return (line 73-74) ---
// Calling readElementText when no element is open causes the while loop to
// exhaust naturally and return at the fallback (line 73-74).

TEST(CalDavXmlReaderGaps, ReadElementTextEndOfLoopFallbackReturn)
{
    DCalDavXmlStreamReader reader(QByteArray("<root>text</root>"));
    ASSERT_TRUE(reader.readNextStartElement());
    ASSERT_EQ(QStringLiteral("root"), reader.name());
    const QString text = reader.readElementText(QXmlStreamReader::SkipChildElements);
    EXPECT_EQ(QStringLiteral("text"), text);
    // Reader is now past </root>; calling readElementText again hits the
    // fallback return because no element is open and the document ends.
    const QString fallback = reader.readElementText(QXmlStreamReader::SkipChildElements);
    EXPECT_TRUE(fallback.isEmpty());
}

// --- readPrivilegeSet: non-privilege element skip (lines 135-137) ---

TEST(CalDavXmlReaderGaps, ParseDiscoveryNonPrivilegeElementInPrivilegeSet)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\">"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat><d:prop>"
        "<d:current-user-privilege-set>"
        "<d:unexpected-child/>"
        "<d:privilege><d:read/></d:privilege>"
        "</d:current-user-privilege-set>"
        "<d:resourcetype><c:calendar xmlns:c=\"urn:ietf:params:xml:ns:caldav\"/></d:resourcetype>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response></d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    ASSERT_EQ(1, result.calendarCollections.size());
    EXPECT_TRUE(result.calendarCollections.first().privileges & DCalDavXmlReader::ReadPrivilege);
}

// --- readProperty: non-href child in current-user-principal (line 177-178) ---

TEST(CalDavXmlReaderGaps, ParseDiscoveryNonHrefChildInCurrentUserPrincipal)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\">"
        "<d:response><d:href>/principals/</d:href>"
        "<d:propstat><d:prop>"
        "<d:current-user-principal>"
        "<d:displayname>Should be skipped</d:displayname>"
        "<d:href>/principals/users/me/</d:href>"
        "</d:current-user-principal>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response></d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    EXPECT_EQ(QStringLiteral("/principals/users/me/"), result.currentUserPrincipalHref);
}

// --- readProperty: non-href child in calendar-home-set (line 186-187) ---

TEST(CalDavXmlReaderGaps, ParseDiscoveryNonHrefChildInCalendarHomeSet)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>/principals/</d:href>"
        "<d:propstat><d:prop>"
        "<c:calendar-home-set>"
        "<d:displayname>Should be skipped</d:displayname>"
        "<d:href>/calendars/</d:href>"
        "</c:calendar-home-set>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response></d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    EXPECT_EQ(QStringLiteral("/calendars/"), result.calendarHomeSetHref);
}

// --- readProperty: unknown property skip (line 199-200) ---

TEST(CalDavXmlReaderGaps, ParseDiscoveryUnknownPropertySkipped)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat><d:prop>"
        "<d:creationdate>2026-01-01T00:00:00Z</d:creationdate>"
        "<d:resourcetype><c:calendar/></d:resourcetype>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response></d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    ASSERT_EQ(1, result.calendarCollections.size());
}

// --- readPropertyStatus: non-prop/status element skip (line 241-242) ---

TEST(CalDavXmlReaderGaps, ParseDiscoveryNonPropStatusElementInPropstat)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>/cal/</d:href>"
        "<d:propstat>"
        "<d:responsedescription>Extra info</d:responsedescription>"
        "<d:prop><d:resourcetype><c:calendar/></d:resourcetype></d:prop>"
        "<d:status>HTTP/1.1 200 OK</d:status>"
        "</d:propstat>"
        "</d:response></d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    ASSERT_EQ(1, result.calendarCollections.size());
}

// --- readResponse: non-href/propstat element skip (line 259-260) ---

TEST(CalDavXmlReaderGaps, ParseDiscoveryNonHrefPropstatElementInResponse)
{
    const QByteArray xml =
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response>"
        "<d:status>HTTP/1.1 200 OK</d:status>"
        "<d:href>/cal/</d:href>"
        "<d:propstat><d:prop><d:resourcetype><c:calendar/></d:resourcetype></d:prop>"
        "<d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response></d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage)) << errorMessage.toStdString();
    ASSERT_EQ(1, result.calendarCollections.size());
}

// ===========================================================================
// Gap-closing tests for dcaldavtransport.cpp
// ===========================================================================

// --- cancel(): null reply in m_activeReplies (line 138) ---

TEST(CalDavTransportGaps, CancelWithNullReplyInActiveRepliesDoesNotCrash)
{
    DCalDavTransport transport;
    transport.m_activeReplies.insert(nullptr);
    transport.cancel();
}

// --- send(): empty callback early return (lines 167-169) ---

TEST(CalDavTransportGaps, SendWithEmptyCallbackDoesNothing)
{
    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = QUrl(QStringLiteral("https://example.com"));
    request.method = "GET";
    transport.send(request, DCalDavTransport::Callback());
    SUCCEED();
}

// --- send(): successful network round-trip (finished handler, readyRead) ---

TEST(CalDavTransportGaps, SendSuccessfulRequest)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/calendars/")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(DCalDavTransport::NoError, response.error);
    EXPECT_EQ(207, response.httpStatus);
    EXPECT_FALSE(response.body.isEmpty());
}

// --- send(): timeout (timer.timeout lambda, lines 234-237) ---

TEST(CalDavTransportGaps, SendTimeoutAbortsRequest)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseDelay(10000);

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/calendars/")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");
    request.timeoutMilliseconds = 200;

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(DCalDavTransport::RequestTimedOut, response.error);
}

// --- send(): SSL errors (sslErrors lambda, lines 230-233) ---

TEST(CalDavTransportGaps, SendSslErrorsSetsCertificateInvalid)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    // Deliberately do NOT trust this certificate — SSL errors will fire.

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/calendars/")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    // The exact error depends on whether sslErrors or the raw connection
    // failure dominates, but either way the request must fail.
    EXPECT_NE(DCalDavTransport::NoError, response.error);
}

// --- send(): response too large (readyRead lambda, lines 225-227) ---

TEST(CalDavTransportGaps, SendResponseTooLargeAbortsRequest)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/calendars/")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");
    request.maximumResponseBytes = 10;

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(DCalDavTransport::ResponseTooLarge, response.error);
}

// --- send(): authentication challenge (authenticationRequired lambda, lines 208-216) ---

TEST(CalDavTransportGaps, SendAuthenticationChallengeInvokesLambda)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatusForTarget("/calendars/", 401);

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/calendars/")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(401, response.httpStatus);
    EXPECT_EQ(DCalDavTransport::AuthenticationFailed, response.error);
}

// --- send(): redirect followed (lines 260-286) ---

TEST(CalDavTransportGaps, SendRedirectFollowedSuccessfully)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatusForTarget("/redirect-me", 302);
    server.setRedirectLocation("/calendars/");

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/redirect-me")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(DCalDavTransport::NoError, response.error);
    EXPECT_EQ(207, response.httpStatus);
}

// --- send(): redirect without Location header (lines 288-289) ---

TEST(CalDavTransportGaps, SendRedirectWithoutLocationReturnsNetworkError)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatusForTarget("/no-location", 302);
    // No redirect location set — Location header will be absent.

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/no-location")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(DCalDavTransport::NetworkError, response.error);
}

// --- send(): redirect to http:// URL returns InvalidUrl (normalizeUrl rejects non-https) ---

TEST(CalDavTransportGaps, SendRedirectToHttpUrlReturnsInvalidUrl)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatusForTarget("/redirect-insecure", 302);
    const QByteArray insecureLocation = "http://localhost:" +
        QByteArray::number(server.serverPort()) + "/calendars/";
    server.setRedirectLocation(insecureLocation);

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/redirect-insecure")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(DCalDavTransport::InvalidUrl, response.error);
}

// --- send(): redirect to invalid URL (lines 266-269) ---

TEST(CalDavTransportGaps, SendRedirectToInvalidUrlReturnsInvalidUrl)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatusForTarget("/redirect-invalid", 302);
    server.setRedirectLocation("http:///");

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/redirect-invalid")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(DCalDavTransport::InvalidUrl, response.error);
}

// --- send(): redirect exceeds maximum (lines 276-280) ---

TEST(CalDavTransportGaps, SendRedirectExceedsMaximumReturnsNetworkError)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatusForTarget("/redirect-loop", 302);
    server.setRedirectLocation("/redirect-loop");

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/redirect-loop")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");
    request.timeoutMilliseconds = 10000;

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(15000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(DCalDavTransport::NetworkError, response.error);
}

// --- classifyError: HTTP 403 → PermissionDenied ---

TEST(CalDavTransportGaps, SendReturnsPermissionDeniedFor403)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatusForTarget("/forbidden", 403);

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/forbidden")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(403, response.httpStatus);
    EXPECT_EQ(DCalDavTransport::PermissionDenied, response.error);
}

// --- classifyError: HTTP 429 → RateLimited ---

TEST(CalDavTransportGaps, SendReturnsRateLimitedFor429)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatusForTarget("/rate-limited", 429);

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/rate-limited")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(429, response.httpStatus);
    EXPECT_EQ(DCalDavTransport::RateLimited, response.error);
    EXPECT_FALSE(response.retryAfter.isEmpty());
}

// --- classifyError: HTTP 503 → ServerUnavailable ---

TEST(CalDavTransportGaps, SendReturnsServerUnavailableFor503)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatusForTarget("/server-error", 503);

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/server-error")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(503, response.httpStatus);
    EXPECT_EQ(DCalDavTransport::ServerUnavailable, response.error);
}

// --- send(): request with contentType and custom headers ---

TEST(CalDavTransportGaps, SendWithContentTypeAndCustomHeaders)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/calendars/")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");
    request.contentType = "application/xml; charset=utf-8";
    request.headers["X-Custom-Header"] = "test-value";
    request.body = "<?xml version=\"1.0\"?>"
                   "<d:propfind xmlns:d=\"DAV:\">"
                   "<d:prop><d:displayname/></d:prop>"
                   "</d:propfind>";

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &resp) {
        response = resp;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(DCalDavTransport::NoError, response.error);
}

// --- cancel(): aborts an active reply ---

TEST(CalDavTransportGaps, CancelAbortsActiveReply)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseDelay(5000);

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/calendars/")));
    request.method = "PROPFIND";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("pass");
    request.timeoutMilliseconds = 30000;

    transport.send(request, [](const DCalDavTransport::Response &) {});

    QEventLoop loop;
    QTimer::singleShot(200, &loop, [&]() {
        transport.cancel();
        EXPECT_TRUE(transport.m_activeReplies.isEmpty());
        EXPECT_TRUE(transport.m_authenticationConnections.isEmpty());
        loop.quit();
    });
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    loop.exec();
}

// --- normalizeUrl: scheme-less URL gets https prefix ---

TEST(CalDavTransportGaps, NormalizeUrlAddsHttpsScheme)
{
    QUrl result = DCalDavTransport::normalizeUrl(QUrl("//localhost/calendars/"));
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(QStringLiteral("https"), result.scheme());
}

// --- normalizeUrl: empty scheme-less path gets https:// prefix ---

TEST(CalDavTransportGaps, NormalizeUrlAddsHttpsSchemeForPlainHost)
{
    QUrl result = DCalDavTransport::normalizeUrl(QUrl("localhost"));
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(QStringLiteral("https"), result.scheme());
    EXPECT_EQ(QStringLiteral("localhost"), result.host());
}

// --- urlForLog: invalid URL returns placeholder ---

TEST(CalDavTransportGaps, UrlForLogReturnsPlaceholderForInvalidUrl)
{
    QString result = DCalDavTransport::urlForLog(QUrl());
    EXPECT_EQ(QStringLiteral("<invalid-url>"), result);
}

// --- isSecureUrl: http URL returns false ---

TEST(CalDavTransportGaps, IsSecureUrlReturnsFalseForHttp)
{
    EXPECT_FALSE(DCalDavTransport::isSecureUrl(QUrl("http://localhost/")));
}

// --- isSecureUrl: https URL returns true ---

TEST(CalDavTransportGaps, IsSecureUrlReturnsTrueForHttps)
{
    EXPECT_TRUE(DCalDavTransport::isSecureUrl(QUrl("https://localhost/")));
}

// --- isSameOrigin: different hosts returns false ---

TEST(CalDavTransportGaps, IsSameOriginReturnsFalseForDifferentHosts)
{
    EXPECT_FALSE(DCalDavTransport::isSameOrigin(QUrl("https://host1/"), QUrl("https://host2/")));
}

// --- isSameOrigin: same host returns true ---

TEST(CalDavTransportGaps, IsSameOriginReturnsTrueForSameHost)
{
    EXPECT_TRUE(DCalDavTransport::isSameOrigin(QUrl("https://localhost/"), QUrl("https://localhost/")));
}
