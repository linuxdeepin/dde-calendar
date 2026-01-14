// SPDX-FileCopyrightText: 2015 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "timeedit.h"
#include "commondef.h"

#include <QEvent>
#include <QFocusEvent>
#include <QPainter>

#include <QVBoxLayout>
#include "calendarmanage.h"

//视图容器最大高度
const int viewContainerMaxHeight = 305;

CTimeEdit::CTimeEdit(QWidget *parent)
    : DComboBox(parent)
    , m_timeFormat(CalendarManager::getInstance()->getTimeFormat())
    , m_timeEdit(new CCustomTimeEdit())
    , m_hasFocus(false)
    , m_miniTime(QTime(0, 0, 0))
{
    qCDebug(ClientLogger) << "Creating CTimeEdit";
    initUI();
    initConnection();
}

CTimeEdit::~CTimeEdit()
{
    qCDebug(ClientLogger) << "Destroying CTimeEdit";
    delete m_timeEdit;
}

void CTimeEdit::setMineTime(const QTime &mineTime)
{
    qCDebug(ClientLogger) << "Setting minimum time:" << mineTime.toString();
    m_miniTime = mineTime;
}

void CTimeEdit::setTime(const QTime &time)
{
    qCDebug(ClientLogger) << "Setting time:" << time.toString();
    m_time = time;
    setSelectItem(m_time);
    m_timeEdit->getLineEdit()->setText(time.toString(m_timeFormat));
}

QTime CTimeEdit::getTime()
{
    //获取显示的text
    QString timetext = m_timeEdit->getLineEdit()->displayText();
    //将text转换为时间
    m_time = QTime::fromString(timetext, m_timeFormat);
    qCDebug(ClientLogger) << "Getting time:" << m_time.toString();
    return m_time;
}

void CTimeEdit::updateListItem(bool isShowTimeInterval)
{
    qCDebug(ClientLogger) << "Updating list items, show time interval:" << isShowTimeInterval;
    m_isShowTimeInterval = isShowTimeInterval;
    QTime topTimer(m_miniTime);
    if (!m_isShowTimeInterval) {
        qCDebug(ClientLogger) << "Time interval not shown, setting top timer";
        // 根据开始时间的分钟数设置结束时间下拉列表的分钟数，当前默认为0,故先注释
        // int m = m_miniTime.minute() >= 30 ? m_miniTime.minute() - 30 : m_miniTime.minute();
        int m = 0;
        topTimer = QTime(0, m);
    }
    //清除列表
    clear();
    for (int i = 0; i < 48; ++i) {
        qreal timeInterval = i * 0.5;
        QString timeIntervalStr;
        if (m_isShowTimeInterval) {
            if (timeInterval < 1) {
                timeIntervalStr = tr("(%1 mins)").arg(i * 30);
            } else if (qFuzzyCompare(timeInterval, 1)) {
                timeIntervalStr = tr("(%1 hour)").arg(timeInterval);
            } else {
                timeIntervalStr = tr("(%1 hours)").arg(timeInterval);
            }
        } else {
            timeIntervalStr = "";
        }
        QString userData = topTimer.addSecs(i * 30 * 60).toString(m_timeFormat);
        QString showStr = userData + timeIntervalStr;
        addItem(showStr, userData);
    }
    qCDebug(ClientLogger) << "Added" << count() << "time items to list";
}

void CTimeEdit::setTimeFormat(int value)
{
    qCDebug(ClientLogger) << "Setting time format value:" << value;
    //获取edit的当前时间
    QTime editCurrentTime = getTime();
    //根据value值,设置时间显示格式
    if (value) {
        qCDebug(ClientLogger) << "Setting 24-hour format";
        m_timeFormat = "hh:mm";
    } else {
        qCDebug(ClientLogger) << "Setting 12-hour format";
        m_timeFormat = "h:mm";
    }
    //先更新列表信息，更新列表信息后再设置编辑框显示时间
    updateListItem(m_isShowTimeInterval);
    m_timeEdit->setDisplayFormat(m_timeFormat);
    //设置改变格式后的时间
    setTime(editCurrentTime);
}

void CTimeEdit::slotFocusDraw(bool showFocus)
{
    qCDebug(ClientLogger) << "Focus draw slot called, show focus:" << showFocus;
    m_hasFocus = showFocus;
    update();
}

void CTimeEdit::setSelectItem(const QTime &time)
{
    qCDebug(ClientLogger) << "Setting select item for time:" << time.toString();
    //若有则设置选中，若没有则取消选中设置定位到相近值附近
    int similarNumber = -1;
    int diff = 24 * 60 * 60 * 1000;
    for (int i = 0 ; i < count(); ++i) {
        QVariant &&userData = itemData(i);
        QTime &&listTime = QTime::fromString(userData.toString(), m_timeFormat);
        int &&timeDiff = qAbs(listTime.msecsTo(time));
        //获取时间差较小的值
        //如果时间小于第一项的时间则不算统计，
        //比如 第一项时间位01:00 比对时间为00:59，时间差应该为23：59 而不是00：01，所以应该定位到最后一项
        if (timeDiff < diff && (!(time < listTime && 0 == i))) {
            diff = timeDiff;
            similarNumber = i;
        }
    }
    if (diff == 0) {
        qCDebug(ClientLogger) << "Exact time match found at index:" << similarNumber;
        setCurrentIndex(similarNumber);
    } else {
        qCDebug(ClientLogger) << "No exact match, closest time at index:" << similarNumber << "with diff:" << diff;
        setCurrentIndex(-1);
    }
    scrollPosition = this->model()->index(similarNumber, 0);
}

void CTimeEdit::slotSetPlainText(const QString &arg)
{
    qCDebug(ClientLogger) << "Setting plain text:" << arg;
    QString userData = currentData().toString();
    if (userData.isEmpty()) {
        qCDebug(ClientLogger) << "User data is empty, using arg text";
        this->lineEdit()->setText(arg);
    } else {
        qCDebug(ClientLogger) << "Using user data text:" << userData;
        this->lineEdit()->setText(userData);
    }
}

void CTimeEdit::slotActivated(const QString &arg)
{
    qCDebug(ClientLogger) << "Activated slot called with:" << arg;
    slotSetPlainText(arg);
    emit signaleditingFinished();
}

void CTimeEdit::slotEditingFinished()
{
    qCDebug(ClientLogger) << "Editing finished slot called";
    setTime(m_timeEdit->time());
    emit signaleditingFinished();
}

void CTimeEdit::initUI()
{
    qCDebug(ClientLogger) << "Initializing UI";
    //关闭自动补全
    this->setCompleter(nullptr);
    //设置edit的宽度
    m_timeEdit->getLineEdit()->setFixedWidth(100);
    m_timeEdit->setDisplayFormat(m_timeFormat);
    updateListItem(m_isShowTimeInterval);
    setLineEdit(m_timeEdit->getLineEdit());
    m_timeEdit->setParent(this);
    setInsertPolicy(QComboBox::NoInsert);
}

void CTimeEdit::initConnection()
{
    qCDebug(ClientLogger) << "Initializing connections";
    connect(CalendarManager::getInstance(), &CalendarManager::signalTimeFormatChanged, this,
            &CTimeEdit::setTimeFormat);
    connect(m_timeEdit, &CCustomTimeEdit::signalUpdateFocus, this, &CTimeEdit::slotFocusDraw);
    connect(m_timeEdit->getLineEdit(), &QLineEdit::editingFinished, this,
            &CTimeEdit::slotEditingFinished);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    // Qt5.14+: 使用 textActivated 信号
    connect(this, &CTimeEdit::textActivated, this, &CTimeEdit::slotActivated);
#else
    // Qt5.11.3 兼容：使用 activated 信号
    // 注意：需要强制转换为 void (QComboBox::*)(const QString &) 以解决重载歧义
    void (QComboBox::*activatedSignal)(const QString &) = &QComboBox::activated;
    connect(this, activatedSignal, this, &CTimeEdit::slotActivated);
#endif
}

void CTimeEdit::showPopup()
{
    qCDebug(ClientLogger) << "Showing popup";
    DComboBox::showPopup();
    //获取下拉视图容器
    QFrame *viewContainer = this->findChild<QFrame *>();
    if (viewContainer) {
        qCDebug(ClientLogger) << "View container found, configuring";
        //移动前先隐藏
        viewContainer->hide();
        //如果显示视图容器则设置高度
        viewContainer->setFixedHeight(viewContainerMaxHeight);
        //设置最大高度
        viewContainer->setMaximumHeight(viewContainerMaxHeight + 1);
        //获取combobox底部坐标
        QPoint showPoint = mapToGlobal(this->rect().bottomLeft());
        //控制视图容器宽度 ，根据字体大小调整宽度
        int maxLen = 0;
        QFontMetrics fontMet(view()->font());
        for (int i = 0 ; i < count() ; ++i) {
            int &&itemWidth = fontMet.horizontalAdvance(this->itemText(i));
            maxLen = qMax(maxLen, itemWidth);
        }
        maxLen += 45;   //选项前√占用的大小
        //如果宽度小于box宽度则设置位box宽度
        maxLen = qMax(maxLen, this->width());
        viewContainer->setFixedWidth(maxLen);
        //将视图容器移动到combobox的底部
        viewContainer->move(showPoint.x(), showPoint.y());
        //显示
        viewContainer->show();
        qCDebug(ClientLogger) << "View container positioned at:" << showPoint << "with size:" << viewContainer->size();
    } else {
        qCDebug(ClientLogger) << "View container not found";
    }
    //因改变了容器的高度，所以需要重新定位当前位置
    if (this->view()->currentIndex() == scrollPosition) {
        qCDebug(ClientLogger) << "Scrolling to current position at center";
        this->view()->scrollTo(scrollPosition, QAbstractItemView::PositionAtCenter);
    } else {
        qCDebug(ClientLogger) << "Scrolling to position at top";
        this->view()->scrollTo(scrollPosition, QAbstractItemView::PositionAtTop);
    }
}

void CTimeEdit::focusInEvent(QFocusEvent *event)
{
    // qCDebug(ClientLogger) << "Focus in event, reason:" << event->reason();
    DComboBox::focusInEvent(event);
    //    如果为tab焦点进入则选中时间
    if (event->reason() == Qt::TabFocusReason) {
        // qCDebug(ClientLogger) << "Tab focus, setting focus to line edit";
        lineEdit()->setFocus(Qt::TabFocusReason);
    }
}

void CTimeEdit::paintEvent(QPaintEvent *e)
{
    DComboBox::paintEvent(e);
    //如果有焦点则设置焦点显示效果
    if (m_hasFocus) {
        // qCDebug(ClientLogger) << "Painting focus rectangle";
        QPainter painter(this);
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.backgroundColor = palette().color(QPalette::Window);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
}

void CTimeEdit::resizeEvent(QResizeEvent *e)
{
    // qCDebug(ClientLogger) << "Resize event, new size:" << size();
    DComboBox::resizeEvent(e);
    m_timeEdit->setFixedHeight(this->height());
}
