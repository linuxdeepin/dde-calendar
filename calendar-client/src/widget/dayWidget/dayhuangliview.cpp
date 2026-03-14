// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dayhuangliview.h"
#include "scheduledlg.h"
#include "commondef.h"

#include <DIcon>

#include <QAction>
#include <QListWidget>
#include <QLabel>
#include <QPainter>
#include <QHBoxLayout>
#include <QStylePainter>
#include <QRect>

// Named constants for magic numbers
static constexpr int kIconSize = 22;            // Icon width and height
static constexpr int kCornerRadius = 12;        // Rounded rect corner radius
static constexpr double kWidthRatio = 0.0424;   // Left margin ratio to width
static constexpr int kTextOffsetX = 50;         // Text start X offset from left margin
static constexpr int kCharWidth = 14;           // Estimated character width
static constexpr int kTextHeight = 21;          // Text line height
static constexpr int kCharSpacing = 10;         // Spacing between characters
static constexpr int kEllipsisMargin = 15;      // Margin for ellipsis
static constexpr int kHeightBase = 20;          // Base value for top margin calculation

CDayHuangLiLabel::CDayHuangLiLabel(QWidget *parent)
    : DLabel(parent)
{
    qCDebug(ClientLogger) << "CDayHuangLiLabel::CDayHuangLiLabel";
    setContentsMargins(0, 0, 0, 0);
}

void CDayHuangLiLabel::setbackgroundColor(QColor backgroundColor)
{
    qCDebug(ClientLogger) << "CDayHuangLiLabel::setbackgroundColor, color:" << backgroundColor;
    m_backgroundColor = backgroundColor;
}

void CDayHuangLiLabel::setTextInfo(QColor tcolor, QFont font)
{
    qCDebug(ClientLogger) << "CDayHuangLiLabel::setTextInfo, color:" << tcolor;
    m_textcolor = tcolor;
    m_font = font;
}

void CDayHuangLiLabel::setHuangLiText(QStringList vhuangli, int type)
{
    qCDebug(ClientLogger) << "CDayHuangLiLabel::setHuangLiText, type:" << type;
    m_vHuangli = vhuangli;
    m_type = type;
    if (!vhuangli.isEmpty()) {
        qCDebug(ClientLogger) << "Setting tooltip for Huangli text";
        QString str = vhuangli.at(0);
        for (int i = 1; i < vhuangli.count(); i++) {
            str += "." + vhuangli.at(i);
        }
        setToolTip(str);
    } else {
        qCDebug(ClientLogger) << "Clearing tooltip for empty Huangli text";
        setToolTip(QString());
    }
    update();
}
void CDayHuangLiLabel::paintEvent(QPaintEvent *e)
{
    // This function is called frequently, so logging is commented out.
    // qCDebug(ClientLogger) << "CDayHuangLiLabel::paintEvent";
    Q_UNUSED(e);
    int labelwidth = width();
    int labelheight = height();
    const QSize iconSize = QSize(kIconSize, kIconSize);

    QPainter painter(this);
    QRect fillRect = QRect(0, 0, labelwidth, labelheight);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setBrush(QBrush(m_backgroundColor));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(fillRect, kCornerRadius, kCornerRadius);

    // Check if we need to update cached pixmaps (first time or DPR changed)
    qreal dpr = devicePixelRatioF();
    if (qFuzzyCompare(m_cachedDpr, dpr) == false || m_yiPixmap.isNull() || m_jiPixmap.isNull()) {
        // Use qRound for precise mapping between logical size and device pixels
        QSize pixmapSize(qRound(iconSize.width() * dpr), qRound(iconSize.height() * dpr));

        // Cache "Yi" pixmap
        m_yiPixmap = QIcon(":/icons/deepin/builtin/icons/dde_calendar_yi_32px.svg").pixmap(pixmapSize);
        m_yiPixmap.setDevicePixelRatio(dpr);

        // Cache "Ji" pixmap
        m_jiPixmap = QIcon(":/icons/deepin/builtin/icons/dde_calendar_ji_32px.svg").pixmap(pixmapSize);
        m_jiPixmap.setDevicePixelRatio(dpr);

        m_cachedDpr = dpr;
    }

    // Use cached pixmap based on type
    const QPixmap &pixmap = (m_type == 0) ? m_yiPixmap : m_jiPixmap;

    // Draw at original icon size, not scaled
    QRect pixmapRect = QRect(QPoint(m_leftMargin, m_topMargin + 1), iconSize);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(pixmapRect, pixmap);
    painter.restore();

    painter.setFont(m_font);
    painter.setPen(m_textcolor);
    int bw = m_leftMargin + kTextOffsetX;
    int bh = m_topMargin;
    for (int i = 0; i < m_vHuangli.count(); i++) {
        int currentsw = m_vHuangli.at(i).size() * kCharWidth;
        if (bw + currentsw + kEllipsisMargin >= labelwidth) {
            painter.drawText(QRect(bw, bh, labelwidth - bw, kTextHeight), Qt::AlignLeft, "...");
            break;
        } else {
            painter.drawText(QRect(bw, bh, currentsw, kTextHeight), Qt::AlignLeft, m_vHuangli.at(i));
            bw += currentsw + kCharSpacing;
        }
    }
}

void CDayHuangLiLabel::resizeEvent(QResizeEvent *event)
{
    // This function is called frequently, so logging is commented out.
    // qCDebug(ClientLogger) << "CDayHuangLiLabel::resizeEvent";
    m_leftMargin = static_cast<int>(kWidthRatio * width() + 0.5);
    m_topMargin = (height() - kHeightBase) / 2;
    DLabel::resizeEvent(event);
}
