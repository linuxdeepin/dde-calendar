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
constexpr int kContentTop = 12;
constexpr int kTimeFrameHeight = 17;
constexpr int kTimeTitleSpacing = 6;
constexpr int kTitleFrameHeight = 17;
constexpr int kTitleSourceSpacing = 7;
constexpr int kSourceFrameHeight = 15;
constexpr int kContentBottom = 13;
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
    m_centerWidget->setFixedWidth(207);
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
    QFontMetrics metrics(textfont);
    textwidth = metrics.horizontalAdvance(m_ScheduleInfo->summary());
    textheight = metrics.height();
    const int  h_count = qCeil(textwidth / textRectWidth);
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
            } else {
                if (i + 1 == m_ScheduleInfo->summary().count()) {
                    // qCDebug(ClientLogger) << "Adding final line of text";
                    testList.append(text);
                }
            }
        }
    }

    qCDebug(ClientLogger) << "Final text list has" << testList.count() << "lines";
    sourceFont.setPixelSize(DDECalendar::FontSizeTwelve);
    const int sourceHeight = m_sourceText.isEmpty() ? 0 : kSourceFrameHeight;
    const int titleHeight = testList.count() * kTitleFrameHeight;
    const int sourceSpacing = m_sourceText.isEmpty() ? 0 : kTitleSourceSpacing;
    const int contentHeight = kContentTop + kTimeFrameHeight + kTimeTitleSpacing + titleHeight
        + sourceSpacing + sourceHeight + kContentBottom;
    this->setFixedHeight(contentHeight);
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
                     Qt::AlignLeft | Qt::AlignTop, timestr);
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
            Qt::AlignLeft, testList.at(i));
    }

    if (!m_sourceText.isEmpty()) {
        painter.setFont(sourceFont);
        painter.setPen(timeColor);
        const int sourceY = titleY + testList.count() * kTitleFrameHeight + kTitleSourceSpacing;
        const QFontMetrics sourceMetrics(sourceFont);
        painter.drawText(QRect(x, sourceY, textRectWidth, kSourceFrameHeight),
                         Qt::AlignLeft, sourceMetrics.elidedText(m_sourceText, Qt::ElideRight, textRectWidth));
    }
}
