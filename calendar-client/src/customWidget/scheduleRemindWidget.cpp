// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduleRemindWidget.h"
#include "constants.h"
#include "commondef.h"

#include <DGuiApplicationHelper>

#include <QPainter>
#include <QtMath>

DGUI_USE_NAMESPACE
ScheduleRemindWidget::ScheduleRemindWidget(QWidget *parent)
    : DArrowRectangle(DArrowRectangle::ArrowLeft, DArrowRectangle::FloatWidget, parent)
    , m_centerWidget(new CenterWidget(this))
{
    qCDebug(ClientLogger) << "ScheduleRemindWidget constructor";
    //如果dtk版本为5.3以上则使用新接口
#if (DTK_VERSION > DTK_VERSION_CHECK(5, 3, 0, 0))
    //设置显示圆角
    setRadiusArrowStyleEnable(true);
    //设置圆角
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
    this->setHeight(m_centerWidget->height() + 10);
}

/**
 * @brief ScheduleRemindWidget::setDirection       设置箭头方向
 * @param value
 */
void ScheduleRemindWidget::setDirection(DArrowRectangle::ArrowDirection value)
{
    qCDebug(ClientLogger) << "ScheduleRemindWidget::setDirection:" << value;
    //设置箭头方向
    this->setArrowDirection(value);
    //设置内容窗口
    this->setContent(m_centerWidget);
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

CenterWidget::CenterWidget(DWidget *parent)
    : DFrame(parent)
    , textwidth(0)
    , textheight(0)
{
    qCDebug(ClientLogger) << "CenterWidget constructor";
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
    UpdateTextList();
    update();
}

void CenterWidget::setTheMe(const int type)
{
    qCDebug(ClientLogger) << "CenterWidget::setTheMe with type:" << type;
    if (type == 2) {
        qCDebug(ClientLogger) << "Setting dark theme colors";
        timeColor = QColor("#C0C6D4");
        timeColor.setAlphaF(0.7);
        textColor = QColor("#C0C6D4");
        textColor.setAlphaF(1);
    } else {
        qCDebug(ClientLogger) << "Setting light theme colors";
        timeColor = QColor("#414D68");
        timeColor.setAlphaF(0.7);
        textColor = QColor("#414D68");
        textColor.setAlphaF(1);
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
    this->setFixedHeight(testList.count() * textheight + 30 + 8);
}

void CenterWidget::paintEvent(QPaintEvent *e)
{
    // qCDebug(ClientLogger) << "CenterWidget::paintEvent";
    Q_UNUSED(e);
    int diam = 8;
    int x = 40 - 13;
    QFont timeFont;
    timeFont.setPixelSize(DDECalendar::FontSizeTwelve);
    QPainter painter(this);
    //draw time
    QPen pen;
    pen.setColor(timeColor);
    painter.setPen(pen);
    painter.setFont(timeFont);
    QString timestr;
    timestr = m_ScheduleInfo->dtStart().time().toString(m_timeFormat);

    QFontMetrics metrics(timeFont);
    if (m_ScheduleInfo->allDay()) {
        // qCDebug(ClientLogger) << "All day event";
        timestr = tr("All Day");
    } else {
        // qCDebug(ClientLogger) << "Time-specific event:" << timestr;
    }
    int timewidth = metrics.horizontalAdvance(timestr);
    int timeheight = metrics.height();

    painter.drawText(QRect(x + 13, 7, timewidth, timeheight), Qt::AlignLeft | Qt::AlignTop, timestr);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(gdcolor.orginalColor));
    painter.drawEllipse(x, 7 + (timeheight - diam) / 2, diam, diam);
    pen.setColor(textColor);
    painter.setPen(pen);
    painter.setFont(textfont);

    // qCDebug(ClientLogger) << "Drawing" << testList.count() << "lines of text";
    for (int i = 0; i < testList.count(); i++) {
        painter.drawText(
            QRect(x, 30 + i * textheight, textRectWidth, textheight),
            Qt::AlignLeft, testList.at(i));
    }
}
