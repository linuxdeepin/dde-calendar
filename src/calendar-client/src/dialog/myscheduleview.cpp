// SPDX-FileCopyrightText: 2017 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "myscheduleview.h"
#include "scheduledlg.h"
#include "scheduledatamanage.h"
#include "cdynamicicon.h"
#include "constants.h"
#include "cscheduleoperation.h"
#include "lunarmanager.h"
#include "commondef.h"
#include "dcaldavaccountstatus.h"
#include "dcaldavprofile.h"

#include <DPalette>
#include <DFontSizeManager>
#include <DLabel>

#include <QShortcut>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QtMath>

DGUI_USE_NAMESPACE
namespace {
QHBoxLayout *findDialogButtonLayout(QObject *object, QAbstractButton *button)
{
    if (QHBoxLayout *layout = qobject_cast<QHBoxLayout *>(object)) {
        if (layout->indexOf(button) >= 0)
            return layout;
    }

    for (QObject *child : object->children()) {
        if (QHBoxLayout *layout = findDialogButtonLayout(child, button))
            return layout;
    }

    return nullptr;
}
}

CMyScheduleView::CMyScheduleView(const DSchedule::Ptr &schduleInfo, QWidget *parent)
    : DCalendarDDialog(parent)
{
    qCDebug(ClientLogger) << "CMyScheduleView constructor with schedule:" << schduleInfo->summary();
    setContentsMargins(0, 0, 0, 0);
    m_scheduleInfo = schduleInfo;
    initUI();
    initConnection();
    //根据主题type设置颜色
    setLabelTextColor(DGuiApplicationHelper::instance()->themeType());
    setFixedWidth(400);
    setMinimumHeight(160);
    //设置初始化弹窗内容
    updateDateTimeFormat();
    focusNextPrevChild(false);
    slotAccountStateChange();
}

void CMyScheduleView::setSchedules(const DSchedule::Ptr &schduleInfo)
{
    qCDebug(ClientLogger) << "Setting schedule to:" << schduleInfo->summary();
    m_scheduleInfo = schduleInfo;
}

void CMyScheduleView::updateFormat()
{
    qCDebug(ClientLogger) << "Updating format for schedule:" << m_scheduleInfo->summary();
    updateDateTimeFormat();
    slotAccountStateChange();
}

/**
 * @brief CMyScheduleView::AutoFeed     字体改变更改界面显示
 * @param text
 */
void CMyScheduleView::slotAutoFeed(const QFont &font)
{
    qCDebug(ClientLogger) << "Auto-adjusting text layout for schedule:" << m_scheduleInfo->summary();
    Q_UNUSED(font)
    if (nullptr == m_timeLabel || nullptr == m_scheduleLabel) {
        qCWarning(ClientLogger) << "Time label or schedule label is null";
        return;
    }

    const QString strText = m_scheduleInfo->summary();
    QString resultStr;
    QFont labelF;
    labelF.setWeight(QFont::Medium);
    labelF = DFontSizeManager::instance()->get(DFontSizeManager::T6, labelF);
    const QFontMetrics fm(labelF);
    const int titleWidth = 330;
    const int lineHeight = fm.height();
    QStringList strList;
    QString line;

    // Keep the existing character-based wrapping, but always consume a
    // character even when a single glyph is wider than the content area.
    // Otherwise the old remove-and-retry loop could never advance.
    for (int i = 0; i < strText.size();) {
        const QString next = line + strText.at(i);
        if (!line.isEmpty() && fm.horizontalAdvance(next) > titleWidth) {
            strList.append(line);
            resultStr += line + QLatin1Char('\n');
            line.clear();
            continue;
        }
        line = next;
        ++i;
    }
    if (!line.isEmpty() || strText.isEmpty()) {
        strList.append(line);
        resultStr += line;
    }

    const int contentHeight = qMax(17, strList.count() * lineHeight);
    const bool needScroll = contentHeight > 100;
    // Keep the designed popup height for normal titles. Long titles remain
    // fully available through the existing scroll area instead of being
    // clipped at the bottom.
    m_scheduleLabelH = qMin(contentHeight, 100);
    area->setFixedHeight(m_scheduleLabelH);
    area->setVerticalScrollBarPolicy(needScroll ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scheduleLabel->setText(resultStr);
    m_scheduleLabel->setFixedHeight(contentHeight);

    QString timeName = m_timeLabel->text();
    if (m_scheduleInfo->lunnar()) {
        const int separatorIndex = timeName.indexOf(QLatin1String("~"));
        if (separatorIndex > 0) {
            QString singleLineTime = timeName;
            singleLineTime[separatorIndex - 1] = QLatin1Char(' ');
            const QFontMetrics timeMetrics(m_timeLabel->font());
            if (timeMetrics.horizontalAdvance(singleLineTime) > m_timeLabel->width()) {
                timeName[separatorIndex - 1] = QLatin1Char('\n');
            }
        }
    }
    m_timeLabel->setText(timeName);
    m_timeLabel->setWordWrap(true);
    const QFontMetrics timeMetrics(m_timeLabel->font());
    const int timeWidth = qMax(1, m_timeLabel->width());
    const QRect timeRect = timeMetrics.boundingRect(
        QRect(0, 0, timeWidth, 0), Qt::AlignCenter | Qt::TextWordWrap, timeName);
    m_timeLabelH = qMax(26, timeRect.height());
    //更新控件高度
    m_timeLabel->setFixedHeight(m_timeLabelH);

    const int timeSpacing = m_timeSpacing != nullptr ? m_timeSpacing->height() : 0;
    const int sourceHeight = m_sourceLabel != nullptr && m_sourceLabel->isVisible()
        ? m_sourceLabel->height() : 0;
    const int sourceSpacing = m_sourceSpacing != nullptr && m_sourceSpacing->isVisible()
        ? m_sourceSpacing->height() : 0;
    const int sourceBottomSpacing = m_sourceBottomSpacing != nullptr && m_sourceBottomSpacing->isVisible()
        ? m_sourceBottomSpacing->height() : 0;
    //更新界面高度
    setFixedHeight(m_defaultH + m_scheduleLabelH + timeSpacing + m_timeLabelH
                   + sourceSpacing + sourceHeight + sourceBottomSpacing);
    qCDebug(ClientLogger) << "Updated view height to:"
                          << (m_defaultH + m_scheduleLabelH + timeSpacing + m_timeLabelH
                              + sourceSpacing + sourceHeight + sourceBottomSpacing);
}

void CMyScheduleView::slotAccountStateChange()
{
    qCDebug(ClientLogger) << "Account state changed for schedule:" << m_scheduleInfo->summary();
    AccountItem::Ptr item = gAccountManager->getAccountItemByScheduleTypeId(m_scheduleInfo->scheduleTypeID());
    if (!item) {
        qCWarning(ClientLogger) << "No account found for schedule type ID:" << m_scheduleInfo->scheduleTypeID();
        return;
    }
    if (m_sourceLabel != nullptr) {
        const DAccount::Ptr account = item->getAccount();
        QString source;
        if (account->accountType() == DAccount::Account_Local) {
            source = tr("Local calendar");
        } else if (account->accountType() == DAccount::Account_UnionID) {
            source = tr("UOS ID") + QStringLiteral("-") + account->accountName();
        } else if (account->accountType() == DAccount::Account_CalDav) {
            const DCalDavAccountStatus status =
                gAccountManager->getCalDavAccountStatus(account->accountID());
            source = DCalDavProviderProfile::accountDisplayName(
                static_cast<DCalDavProviderProfile::ProviderType>(status.providerType),
                account->displayName());
        }
        if (source.isEmpty()) {
            m_sourceLabel->clear();
            m_sourceLabel->hide();
            m_sourceSpacing->hide();
            m_sourceBottomSpacing->hide();
        } else {
            m_sourceLabel->setText(tr("Calendar Source") + QStringLiteral(": ") + source);
            m_sourceLabel->setToolTip(QString());
            m_sourceSpacing->show();
            m_sourceLabel->show();
            m_sourceBottomSpacing->show();
        }
    }
    bool canSync = item->isCanSyncShedule();
    qCDebug(ClientLogger) << "Account sync state changed for schedule:" << m_scheduleInfo->summary()
                          << "can sync:" << canSync;
    //根据可同步状态设置删除按钮是否可用
    getButtons()[0]->setEnabled(canSync);
    slotAutoFeed();
}

/**
 * @brief setLabelTextColor     设置label文字颜色
 * @param type  主题type
 */
void CMyScheduleView::setLabelTextColor(const int type)
{
    qCDebug(ClientLogger) << "Setting label text colors for theme type:" << type;
    //标题显示颜色
    QColor titleColor;
    //日程显示颜色
    QColor scheduleTitleColor;
    //时间显示颜色
    QColor timeColor;
    if (type == 2) {
        titleColor = "#FFFFFF";
        titleColor.setAlphaF(0.85);
        scheduleTitleColor = "#FFFFFF";
        timeColor = "#FFFFFF";
        timeColor.setAlphaF(0.7);
        qCDebug(ClientLogger) << "Using dark theme colors";
    } else {
        titleColor = "#000000";
        titleColor.setAlphaF(0.85);
        scheduleTitleColor = "#000000";
        scheduleTitleColor.setAlphaF(0.9);
        timeColor = "#000000";
        timeColor.setAlphaF(0.6);
        qCDebug(ClientLogger) << "Using light theme colors";
    }
    //设置颜色
    setPaletteTextColor(m_Title, titleColor);
    setPaletteTextColor(m_scheduleLabel, scheduleTitleColor);
    setPaletteTextColor(m_timeLabel, timeColor);
}

void CMyScheduleView::updateDialogIcon()
{
    setIcon(QIcon::fromTheme("dde-calendar"));
}

/**
 * @brief setPaletteTextColor   设置调色板颜色
 * @param widget                需要设置的widget
 * @param textColor             显示颜色
 */
void CMyScheduleView::setPaletteTextColor(QWidget *widget, QColor textColor)
{
    qCDebug(ClientLogger) << "Setting palette text color:" << textColor.name();
    //如果为空指针则退出
    if (nullptr == widget) {
        qCWarning(ClientLogger) << "Widget is null, cannot set palette color";
        return;
    }
    DPalette palette = widget->palette();
    //设置文字显示颜色
    palette.setColor(DPalette::WindowText, textColor);
    widget->setPalette(palette);
}

/**
 * @brief CMyScheduleView::updateDateTimeFormat 更新显示时间格式
 */
void CMyScheduleView::updateDateTimeFormat()
{
    qCDebug(ClientLogger) << "Updating date time format for schedule:" << m_scheduleInfo->summary();
    //如果为节假日
    if (CScheduleOperation::isFestival(m_scheduleInfo)) {
        m_timeLabel->setText(m_scheduleInfo->dtStart().toString(m_dateFormat));
        qCDebug(ClientLogger) << "Festival schedule date format:" << m_scheduleInfo->dtStart().toString(m_dateFormat);
    } else {
        QString showTime;
        QString beginName, endName;
        if (m_scheduleInfo->allDay()) {
            if (m_scheduleInfo->isMultiDay()) {
                beginName = getDataByFormat(m_scheduleInfo->dtStart().date(), m_dateFormat);
                endName = getDataByFormat(m_scheduleInfo->dtEnd().date(), m_dateFormat);
                showTime = beginName + " ~ " + endName;
                qCDebug(ClientLogger) << "Multi-day all-day schedule:" << showTime;
            } else {
                showTime = getDataByFormat(m_scheduleInfo->dtStart().date(), m_dateFormat);
                qCDebug(ClientLogger) << "Single-day all-day schedule:" << showTime;
            }

        } else {
            const QDateTime localStart = m_scheduleInfo->dtStart().toLocalTime();
            const QDateTime localEnd = m_scheduleInfo->dtEnd().toLocalTime();
            beginName = getDataByFormat(localStart.date(), m_dateFormat) + " " + localStart.time().toString(m_timeFormat);
            endName = getDataByFormat(localEnd.date(), m_dateFormat) + " " + localEnd.time().toString(m_timeFormat);
            showTime = beginName + " ~ " + endName;
            qCDebug(ClientLogger) << "Time-specific schedule:" << showTime;
        }
        m_timeLabel->setText(showTime);
    }
    slotAutoFeed();
}

QString CMyScheduleView::getDataByFormat(const QDate &date, QString format)
{
    qCDebug(ClientLogger) << "Getting formatted date for:" << date << "with format:" << format;
    QString name = date.toString(format);
    if (m_scheduleInfo->lunnar()) {
        //接入农历时间
        name += gLunarManager->getHuangLiShortName(date);
        qCDebug(ClientLogger) << "Added lunar calendar info to date:" << name;
    }
    return name;
}

/**
 * @brief CMyScheduleView::slotBtClick      按钮点击事件
 * @param buttonIndex
 * @param buttonName
 */
void CMyScheduleView::slotBtClick(int buttonIndex, const QString &buttonName)
{
    qCDebug(ClientLogger) << "Button clicked: index=" << buttonIndex << "name=" << buttonName;
    Q_UNUSED(buttonName);
    if (buttonIndex == 0) {
        qCDebug(ClientLogger) << "Delete button clicked for schedule:" << m_scheduleInfo->summary();
        //删除日程
        if (CScheduleOperation(m_scheduleInfo->scheduleTypeID(), this).deleteSchedule(m_scheduleInfo)) {
            qCDebug(ClientLogger) << "Schedule deleted successfully";
            accept();
        } else {
            qCWarning(ClientLogger) << "Failed to delete schedule";
        }
        return;
    }
    if (buttonIndex == 1) {
        qCDebug(ClientLogger) << "Edit button clicked for schedule:" << m_scheduleInfo->summary();
        //编辑日程
        CScheduleDlg dlg(0, this);
        dlg.setData(m_scheduleInfo);
        if (dlg.exec() == DDialog::Accepted) {
            qCDebug(ClientLogger) << "Schedule edited successfully";
            accept();
        } else {
            qCDebug(ClientLogger) << "Schedule edit cancelled";
            close();
        }
        return;
    }
}

/**
 * @brief CMyScheduleView::initUI       界面初始化
 */
void CMyScheduleView::initUI()
{
    qCDebug(ClientLogger) << "Initializing UI for schedule view";
    //在点击任何对话框上的按钮后不关闭对话框，保证关闭子窗口时不被一起关掉
    setOnButtonClickedClose(false);

    m_Title = new QLabel(this);
    m_Title->setFixedSize(220, 51);
    m_Title->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_Title, DFontSizeManager::T5, QFont::DemiBold);
    //设置日期图标
    updateDialogIcon();
    m_Title->setText(tr("My Event"));
    m_Title->move(90, 0); // center x: (400-220)/2

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    area = new QScrollArea(this);
    //设置日程显示区域不能选中
    area->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    area->setFrameShape(QFrame::NoFrame);
    area->setFixedWidth(363);
    area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setBackgroundRole(QPalette::Window);
    area->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    area->setWidgetResizable(true);
    area->setAlignment(Qt::AlignCenter);

    m_scheduleLabel = new QLabel(this);
    m_scheduleLabel->setTextFormat(Qt::PlainText); //纯文本格式
    m_scheduleLabel->installEventFilter(this);
    m_scheduleLabel->setFixedWidth(330);
    m_scheduleLabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_scheduleLabel, DFontSizeManager::T6);
    labelF.setWeight(QFont::Medium);
    m_scheduleLabel->setFont(labelF);

    area->setWidget(m_scheduleLabel);
    mainLayout->addWidget(area);

    m_timeLabel = new DLabel(this);
    m_timeLabel->setFixedHeight(26);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    QFont timeFont;
    timeFont.setWeight(QFont::Normal);
    m_timeLabel->setFont(timeFont);
    m_timeLabel->setFixedWidth(363);

    m_timeSpacing = new QWidget(this);
    m_timeSpacing->setFixedHeight(5);
    mainLayout->addWidget(m_timeSpacing);
    mainLayout->addWidget(m_timeLabel);

    m_sourceSpacing = new QWidget(this);
    m_sourceSpacing->setFixedHeight(15);
    m_sourceSpacing->hide();
    mainLayout->addWidget(m_sourceSpacing);

    m_sourceLabel = new DLabel(this);
    m_sourceLabel->setAlignment(Qt::AlignCenter);
    m_sourceLabel->setFixedHeight(22);
    m_sourceLabel->hide();
    mainLayout->addWidget(m_sourceLabel);

    m_sourceBottomSpacing = new QWidget(this);
    m_sourceBottomSpacing->setFixedHeight(15);
    m_sourceBottomSpacing->hide();
    mainLayout->addWidget(m_sourceBottomSpacing);

    //如果为节假日日程
    if (CScheduleOperation::isFestival(m_scheduleInfo)) {
        qCDebug(ClientLogger) << "Adding OK button for festival schedule";
        addButton(tr("OK", "button"), false, DDialog::ButtonNormal);
        QAbstractButton *button_ok = getButton(0);
        button_ok->setFixedSize(380, 36);
    } else {
        qCDebug(ClientLogger) << "Adding Delete and Edit buttons for regular schedule";
        addButton(tr("Delete", "button"), false, DDialog::ButtonNormal);
        addButton(tr("Edit", "button"), false, DDialog::ButtonRecommend);
        for (int i = 0; i < buttonCount(); i++) {
            QAbstractButton *button = getButton(i);
            button->setFixedSize(180, 36);
        }

        if (QHBoxLayout *buttonLayout = findDialogButtonLayout(this, getButton(0))) {
            const QMargins margins = buttonLayout->contentsMargins();
            buttonLayout->setContentsMargins(10, margins.top(), 10, margins.bottom());
            buttonLayout->setSpacing(9);
            for (int i = 0; i < buttonLayout->count(); ++i) {
                QWidget *widget = buttonLayout->itemAt(i)->widget();
                if (widget != nullptr && widget->objectName() == QLatin1String("VLine"))
                    widget->setFixedWidth(2);
            }
        }
        //TODO:如果为不可修改日程则设置删除按钮无效
    }

    //这种中心铺满的weiget，显示日程标题和时间的控件
    DWidget *centerWidget = new DWidget(this);
    centerWidget->setLayout(mainLayout);
    //获取widget的调色板
    DPalette centerWidgetPalette = centerWidget->palette();
    //设置背景色为透明
    centerWidgetPalette.setColor(DPalette::Window, Qt::transparent);
    centerWidget->setPalette(centerWidgetPalette);
    //添加窗口为剧中对齐
    addContent(centerWidget, Qt::AlignCenter);
    qCDebug(ClientLogger) << "UI initialization complete";
}

void CMyScheduleView::initConnection()
{
    qCDebug(ClientLogger) << "Initializing connections for schedule view";
    //关联主题改变事件
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this,
                     &CMyScheduleView::setLabelTextColor);
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this,
                     &CMyScheduleView::updateDialogIcon);
    //如果为节假日日程
    if (CScheduleOperation::isFestival(m_scheduleInfo)) {
        qCDebug(ClientLogger) << "Adding close button for festival schedule";
        connect(this, &DDialog::buttonClicked, this, &CMyScheduleView::close);
    } else {
        qCDebug(ClientLogger) << "Adding delete and edit buttons for regular schedule";
        connect(this, &DDialog::buttonClicked, this, &CMyScheduleView::slotBtClick);
    }
    QObject::connect(qGuiApp, &DApplication::fontChanged, this, &CMyScheduleView::slotAutoFeed);

    QShortcut *shortcut = new QShortcut(this);
    shortcut->setKey(QKeySequence(QLatin1String("ESC")));
    connect(shortcut, SIGNAL(activated()), this, SLOT(close()));
    connect(gAccountManager, &AccountManager::signalAccountStateChange, this, &CMyScheduleView::slotAccountStateChange);
}
