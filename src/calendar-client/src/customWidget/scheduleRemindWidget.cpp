// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduleRemindWidget.h"
#include "constants.h"
#include "commondef.h"
#include "dataManage/accountmanager.h"
#include "dataManage/accountitem.h"
#include "daccount.h"
#include "dcaldavaccountstatus.h"
#include "dcaldavprofile.h"

#include <DGuiApplicationHelper>

#include <QCoreApplication>
#include <QPainter>
#include <QtMath>

namespace {
static const int kReminderArrowWidth = 18;
static const int kReminderArrowHeight = 10;
static const int kReminderMargin = 12;
static const int kReminderArrowInset = 10;
}

DGUI_USE_NAMESPACE

namespace {
constexpr int kContentTop = 4;
constexpr int kMinTextRectWidth = 165;
constexpr int kMaxTextRectWidth = 191;
constexpr int kMinContentWidth = 207;
constexpr int kContentWidthPadding = 35;
constexpr int kTimeFrameHeight = 17;
constexpr int kTimeTitleSpacing = 6;
constexpr int kTitleFrameHeight = 17;
constexpr int kTitleDetailSpacing = 9;
constexpr int kDetailFrameHeight = 22;
constexpr int kDetailSourceSpacing = 13;
constexpr int kDetailDividerOffset = 6;
constexpr int kTitleSourceSpacing = 7;
constexpr int kToolTipTextWidth = 300;

QString userDisplayNamePart(QString value)
{
    value = value.trimmed();
    if (value.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
        value.remove(0, 7);
    }

    const int parameterStart = value.indexOf(QLatin1Char(';'));
    if (parameterStart >= 0) {
        value.truncate(parameterStart);
    }
    value = value.trimmed();
    if (value.size() > 1 && ((value.front() == QLatin1Char('\"') && value.back() == QLatin1Char('\"'))
                             || (value.front() == QLatin1Char('\'') && value.back() == QLatin1Char('\'')))) {
        value = value.mid(1, value.size() - 2);
    }

    return value.section(QLatin1Char('@'), 0, 0);
}

QString userDisplayName(const QString &name, const QString &email)
{
    const QString displayName = userDisplayNamePart(name);
    return displayName.isEmpty() ? userDisplayNamePart(email) : displayName;
}

QString personDisplayName(const KCalendarCore::Person &person)
{
    return userDisplayName(person.name(), person.email());
}

QString attendeeDisplayName(const KCalendarCore::Attendee &attendee)
{
    return userDisplayName(attendee.name(), attendee.email());
}
constexpr int kContentBottom = 0;
}
ScheduleRemindWidget::ScheduleRemindWidget(QWidget *parent)
    : DArrowRectangle(DArrowRectangle::ArrowLeft, DArrowRectangle::FloatWidget, parent)
    , m_centerWidget(new CenterWidget(this))
{
    qCDebug(ClientLogger) << "ScheduleRemindWidget constructor";
    setBackgroundColor(DBlurEffectWidget::AutoColor);
    setLeftRightRadius(true);
    setArrowWidth(kReminderArrowWidth);
    setArrowHeight(kReminderArrowHeight);
    setMargin(kReminderMargin);
#if (DTK_VERSION > DTK_VERSION_CHECK(5, 3, 0, 0))
    setRadiusArrowStyleEnable(true);
    setRadius(DARROWRECT::DRADIUS);
#endif
    m_centerWidget->setFixedWidth(kMinContentWidth);
    m_centerWidget->setFixedHeight(57);
    setContent(m_centerWidget);
    this->resizeWithContent();
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     m_centerWidget,
                     &CenterWidget::setTheMe);
    m_centerWidget->setTheMe(DGuiApplicationHelper::instance()->themeType());
    updatePopupGeometry();
}

ScheduleRemindWidget::~ScheduleRemindWidget()
{
    qCDebug(ClientLogger) << "ScheduleRemindWidget destructor";
}

void ScheduleRemindWidget::setData(const DSchedule::Ptr &vScheduleInfo, const CSchedulesColor &gcolor)
{
    qCDebug(ClientLogger) << "ScheduleRemindWidget::setData for schedule:" << vScheduleInfo->uid();
    m_centerWidget->setData(vScheduleInfo, gcolor);
    m_ScheduleInfo = vScheduleInfo;
    gdcolor = gcolor;
    updatePopupGeometry();
}

/**
 * @brief ScheduleRemindWidget::setDirection       设置箭头方向
 * @param value
 */
void ScheduleRemindWidget::setDirection(DArrowRectangle::ArrowDirection value)
{
    qCDebug(ClientLogger) << "ScheduleRemindWidget::setDirection:" << value;
    this->setArrowDirection(value);
    this->setContent(m_centerWidget);
    updatePopupGeometry();
}

/**
 * @brief ScheduleRemindWidget::setTimeFormat 设置日期显示格式
 * @param timeformat 日期格式
 */
void ScheduleRemindWidget::setTimeFormat(QString timeformat)
{
    qCDebug(ClientLogger) << "ScheduleRemindWidget::setTimeFormat:" << timeformat;
    m_centerWidget->setTimeFormat(timeformat);
}

void ScheduleRemindWidget::show(int x, int y)
{
    updateArrowPosition();
    DArrowRectangle::show(x, y);
    updateArrowPosition();
}

void ScheduleRemindWidget::updateArrowPosition()
{
    setArrowY((height() - arrowWidth()) / 2);
}

void ScheduleRemindWidget::updatePopupGeometry()
{
    resizeWithContent();
    updateArrowPosition();
}

CenterWidget::CenterWidget(DWidget *parent)
    : DFrame(parent)
    , textwidth(0)
    , textheight(0)
{
    qCDebug(ClientLogger) << "CenterWidget constructor";
    setAutoFillBackground(false);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    textfont.setWeight(QFont::Medium);
}

CenterWidget::~CenterWidget()
{
    qCDebug(ClientLogger) << "CenterWidget destructor";
}

void CenterWidget::setData(const DSchedule::Ptr &vScheduleInfo, const CSchedulesColor &gcolor)
{
    qCDebug(ClientLogger) << "CenterWidget::setData for schedule:" << vScheduleInfo->uid();
    m_ScheduleInfo = vScheduleInfo;
    gdcolor = gcolor;
    textfont.setPixelSize(DDECalendar::FontSizeTwelve);
    m_organizerLabel = QCoreApplication::translate("CMyScheduleView", "Organizer");
    m_attendeeLabel = QCoreApplication::translate("CMyScheduleView", "Attendees");
    m_organizerLines.clear();
    m_attendeeLines.clear();
    m_sourceText.clear();

    const AccountItem::Ptr accountItem =
        gAccountManager->getAccountItemByScheduleTypeId(m_ScheduleInfo->scheduleTypeID());
    if (accountItem) {
        const DAccount::Ptr account = accountItem->getAccount();
        QString source;
        if (account->accountType() == DAccount::Account_Local) {
            source = QCoreApplication::translate("CMyScheduleView", "Local calendar");
        } else if (account->accountType() == DAccount::Account_UnionID) {
            source = QCoreApplication::translate("CMyScheduleView", "UOS ID")
                + QStringLiteral("-") + account->accountName();
        } else if (account->accountType() == DAccount::Account_CalDav) {
            const DCalDavAccountStatus status =
                gAccountManager->getCalDavAccountStatus(account->accountID());
            const QString accountName = account->displayName().isEmpty()
                ? account->accountName() : account->displayName();
            source = DCalDavProviderProfile::accountDisplayName(
                static_cast<DCalDavProviderProfile::ProviderType>(status.providerType), accountName);
        }
        if (!source.isEmpty()) {
            m_sourceText = QCoreApplication::translate("CMyScheduleView", "Source: %1").arg(source);
        }
    }

    UpdateTextList();
    update();
}

void CenterWidget::setTheMe(const int type)
{
    qCDebug(ClientLogger) << "CenterWidget::setTheMe with type:" << type;
    if (type == 2) {
        timeColor = QColor("#FFFFFF");
        timeColor.setAlphaF(0.6);
        textColor = QColor("#FFFFFF");
        textColor.setAlphaF(0.7);
    } else {
        timeColor = QColor("#000000");
        timeColor.setAlphaF(0.6);
        textColor = QColor("#000000");
        textColor.setAlphaF(0.7);
    }
    update();
}

/**
 * @brief CenterWidget::setTimeFormat 设置日期显示格式
 * @param timeFormat 日期格式
 */
void CenterWidget::setTimeFormat(QString timeFormat)
{
    qCDebug(ClientLogger) << "CenterWidget::setTimeFormat:" << timeFormat;
    m_timeFormat = timeFormat;
    update();
}

void CenterWidget::UpdateTextList()
{
    qCDebug(ClientLogger) << "CenterWidget::UpdateTextList";
    testList.clear();
    m_organizerLines.clear();
    m_attendeeLines.clear();

    sourceFont.setPixelSize(DDECalendar::FontSizeTwelve);
    const QFontMetrics metrics(textfont);
    const QFontMetrics sourceMetrics(sourceFont);
    const QString organizer = personDisplayName(m_ScheduleInfo->organizer());
    QStringList attendees;
    for (const KCalendarCore::Attendee &attendee : m_ScheduleInfo->attendees()) {
        const QString name = attendeeDisplayName(attendee);
        if (!name.isEmpty()) {
            attendees.append(name);
        }
    }
    const QString attendeeText = attendees.join(QStringLiteral("、"));

    m_detailLabelWidth = qMax(metrics.horizontalAdvance(m_organizerLabel),
                              metrics.horizontalAdvance(m_attendeeLabel)) + 6;
    int contentTextWidth = metrics.horizontalAdvance(m_ScheduleInfo->summary());
    if (!organizer.isEmpty()) {
        contentTextWidth = qMax(contentTextWidth,
                                 m_detailLabelWidth + metrics.horizontalAdvance(organizer));
    }
    if (!attendeeText.isEmpty()) {
        contentTextWidth = qMax(contentTextWidth,
                                 m_detailLabelWidth + metrics.horizontalAdvance(attendeeText));
    }
    contentTextWidth = qMax(contentTextWidth, sourceMetrics.horizontalAdvance(m_sourceText));
    textRectWidth = qBound(kMinTextRectWidth, contentTextWidth, kMaxTextRectWidth);
    setFixedWidth(qMax(kMinContentWidth, textRectWidth + kContentWidthPadding));

    textwidth = metrics.horizontalAdvance(m_ScheduleInfo->summary());
    textheight = metrics.height();
    const int h_count = qCeil(textwidth / textRectWidth);
    QString text;

    if (h_count < 1) {
        qCDebug(ClientLogger) << "Text fits in one line";
        testList.append(m_ScheduleInfo->summary());
    } else {
        qCDebug(ClientLogger) << "Text needs multiple lines, h_count:" << h_count;
        const int text_Max_Height = 108;
        const int text_HeightMaxCount = qFloor(text_Max_Height / textheight);

        for (int i = 0; i < m_ScheduleInfo->summary().count(); ++i) {
            text += m_ScheduleInfo->summary().at(i);
            if (metrics.horizontalAdvance(text) > textRectWidth) {
                text.remove(text.count() - 1, 1);
                testList.append(text);
                text = "";

                if (testList.count() == (text_HeightMaxCount - 1)) {
                    qCDebug(ClientLogger) << "Reached maximum line count, adding elided text";
                    text = m_ScheduleInfo->summary().right(m_ScheduleInfo->summary().count() - i);
                    testList.append(metrics.elidedText(text, Qt::ElideRight, textRectWidth));
                    break;
                }
                --i;
            } else if (i + 1 == m_ScheduleInfo->summary().count()) {
                testList.append(text);
            }
        }
    }

    const int detailValueWidth = textRectWidth - m_detailLabelWidth;
    const auto wrapDetailText = [&metrics, detailValueWidth](const QString &detailText) {
        QStringList lines;
        QString line;
        for (const QChar character : detailText) {
            if (metrics.horizontalAdvance(line + character) > detailValueWidth && !line.isEmpty()) {
                lines.append(line);
                line.clear();
            }
            line.append(character);
        }
        if (!line.isEmpty()) {
            lines.append(line);
        }

        constexpr int kMaximumDetailLines = 2;
        if (lines.count() > kMaximumDetailLines) {
            const QString remainingText = lines.mid(kMaximumDetailLines - 1).join(QString());
            lines = lines.mid(0, kMaximumDetailLines - 1);
            lines.append(metrics.elidedText(remainingText, Qt::ElideRight, detailValueWidth));
        }
        return lines;
    };

    if (!organizer.isEmpty()) {
        m_organizerLines = wrapDetailText(organizer);
    }
    if (!attendeeText.isEmpty()) {
        m_attendeeLines = wrapDetailText(attendeeText);
    }

    qCDebug(ClientLogger) << "Final text list has" << testList.count() << "lines";
    const int sourceFrameHeight = sourceMetrics.lineSpacing() + 2;
    const int detailLineCount = m_organizerLines.count() + m_attendeeLines.count();
    const bool hasParticipantInfo = detailLineCount > 0;
    const int detailHeight = detailLineCount * kDetailFrameHeight;
    const int detailSpacing = hasParticipantInfo ? kTitleDetailSpacing : 0;
    const int sourceHeight = m_sourceText.isEmpty() ? 0 : sourceFrameHeight;
    const int titleHeight = testList.count() * kTitleFrameHeight;
    const int sourceSpacing = m_sourceText.isEmpty()
        ? 0 : (hasParticipantInfo ? kDetailSourceSpacing : kTitleSourceSpacing);
    const int contentHeight = kContentTop + kTimeFrameHeight + kTimeTitleSpacing + titleHeight
        + detailSpacing + detailHeight + sourceSpacing + sourceHeight + kContentBottom;
    this->setFixedHeight(contentHeight);

    QStringList toolTipLines;
    const auto appendToolTipText = [&metrics, &toolTipLines](const QString &toolTipText) {
        QString line;
        for (const QChar character : toolTipText) {
            if (character == QLatin1Char('\n')) {
                toolTipLines.append(line);
                line.clear();
            } else if (metrics.horizontalAdvance(line + character) > kToolTipTextWidth && !line.isEmpty()) {
                toolTipLines.append(line);
                line = character;
            } else {
                line.append(character);
            }
        }
        if (!line.isEmpty()) {
            toolTipLines.append(line);
        }
    };

    if (testList.join(QString()) != m_ScheduleInfo->summary()) {
        appendToolTipText(m_ScheduleInfo->summary());
    }
    if (!m_organizerLines.isEmpty() && m_organizerLines.join(QString()) != organizer) {
        appendToolTipText(m_organizerLabel + QStringLiteral(": ") + organizer);
    }
    if (!m_attendeeLines.isEmpty() && m_attendeeLines.join(QString()) != attendeeText) {
        appendToolTipText(m_attendeeLabel + QStringLiteral(": ") + attendeeText);
    }
    if (!m_sourceText.isEmpty()
        && sourceMetrics.elidedText(m_sourceText, Qt::ElideRight, textRectWidth) != m_sourceText) {
        appendToolTipText(m_sourceText);
    }
    setToolTip(toolTipLines.join(QLatin1Char('\n')));
}

void CenterWidget::paintEvent(QPaintEvent *e)
{
    // qCDebug(ClientLogger) << "CenterWidget::paintEvent";
    Q_UNUSED(e);
    int diam = 8;
    int x = 40 - 13;
    QFont timeFont;
    timeFont.setPixelSize(DDECalendar::FontSizeTwelve);
    timeFont.setWeight(QFont::Normal);
    QPainter painter(this);
    //draw time
    QPen pen;
    pen.setColor(timeColor);
    painter.setPen(pen);
    painter.setFont(timeFont);
    QString timestr;
    timestr = m_ScheduleInfo->dtStart().toLocalTime().time().toString(m_timeFormat);

    QFontMetrics metrics(timeFont);
    if (m_ScheduleInfo->allDay()) {
        // qCDebug(ClientLogger) << "All day event";
        timestr = tr("All Day");
    } else {
        // qCDebug(ClientLogger) << "Time-specific event:" << timestr;
    }
    int timewidth = metrics.horizontalAdvance(timestr);
    const int timeheight = kTimeFrameHeight;

    const int titleY = kContentTop + timeheight + kTimeTitleSpacing;
    painter.drawText(QRect(x + 13, kContentTop, timewidth, kTimeFrameHeight),
                     Qt::AlignLeft | Qt::AlignVCenter, timestr);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(gdcolor.orginalColor));
    painter.drawEllipse(x, kContentTop + (timeheight - diam) / 2, diam, diam);
    pen.setColor(textColor);
    painter.setPen(pen);
    painter.setFont(textfont);

    // qCDebug(ClientLogger) << "Drawing" << testList.count() << "lines of text";
    for (int i = 0; i < testList.count(); i++) {
        painter.drawText(
            QRect(x, titleY + i * kTitleFrameHeight, textRectWidth, kTitleFrameHeight),
            Qt::AlignLeft | Qt::AlignVCenter, testList.at(i));
    }

    int detailY = titleY + testList.count() * kTitleFrameHeight;
    if (!m_organizerLines.isEmpty() || !m_attendeeLines.isEmpty()) {
        detailY += kTitleDetailSpacing;
    }

    const auto drawDetails = [&painter, this, x, &detailY](const QString &label,
                                                             const QStringList &lines) {
        if (lines.isEmpty()) {
            return;
        }
        painter.drawText(QRect(x, detailY, m_detailLabelWidth, kDetailFrameHeight),
                         Qt::AlignLeft | Qt::AlignVCenter, label);
        for (const QString &line : lines) {
            painter.drawText(QRect(x + m_detailLabelWidth, detailY,
                                   textRectWidth - m_detailLabelWidth, kDetailFrameHeight),
                             Qt::AlignLeft | Qt::AlignVCenter, line);
            detailY += kDetailFrameHeight;
        }
    };
    drawDetails(m_organizerLabel, m_organizerLines);
    drawDetails(m_attendeeLabel, m_attendeeLines);

    const bool hasParticipantInfo = !m_organizerLines.isEmpty() || !m_attendeeLines.isEmpty();
    if (!m_sourceText.isEmpty()) {
        const int sourceSpacing = hasParticipantInfo ? kDetailSourceSpacing : kTitleSourceSpacing;
        if (hasParticipantInfo) {
            QColor dividerColor = timeColor;
            dividerColor.setAlphaF(0.15);
            painter.setPen(dividerColor);
            const int dividerY = detailY + kDetailDividerOffset;
            painter.drawLine(x, dividerY, x + textRectWidth, dividerY);
        }

        painter.setFont(sourceFont);
        painter.setPen(timeColor);
        const int sourceY = detailY + sourceSpacing;
        const QFontMetrics sourceMetrics(sourceFont);
        const int sourceFrameHeight = sourceMetrics.lineSpacing() + 2;
        painter.drawText(QRect(x, sourceY, textRectWidth, sourceFrameHeight),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         sourceMetrics.elidedText(m_sourceText, Qt::ElideRight, textRectWidth));
    }
}
