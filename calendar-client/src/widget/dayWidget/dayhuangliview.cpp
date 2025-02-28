// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dayhuangliview.h"
#include "scheduledlg.h"

#include <DIcon>

#include <QAction>
#include <QListWidget>
#include <QLabel>
#include <QPainter>
#include <QHBoxLayout>
#include <QStylePainter>
#include <QRect>

CDayHuangLiLabel::CDayHuangLiLabel(QWidget *parent)
    : DLabel(parent)
{
    setContentsMargins(0, 0, 0, 0);
}

void CDayHuangLiLabel::setbackgroundColor(QColor backgroundColor)
{
    m_backgroundColor = backgroundColor;
}

void CDayHuangLiLabel::setTextInfo(QColor tcolor, QFont font)
{
    m_textcolor = tcolor;
    m_font = font;
}

void CDayHuangLiLabel::setHuangLiText(QStringList vhuangli, int type)
{
    m_vHuangli = vhuangli;
    m_type = type;
    if (!vhuangli.isEmpty()) {
        QString str = vhuangli.at(0);
        for (int i = 1; i < vhuangli.count(); i++) {
            str += "." + vhuangli.at(i);
        }
        setToolTip(str);
    } else {
        setToolTip(QString());
    }
    update();
}
void CDayHuangLiLabel::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    int labelwidth = width();
    int labelheight = height();
    const QSize iconSize = QSize(22, 22);

    QPainter painter(this);
    QRect fillRect = QRect(0, 0, labelwidth, labelheight);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setBrush(QBrush(m_backgroundColor));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(fillRect, 12, 12);

    QString iconPath = (m_type == 0) ? ":/icons/deepin/builtin/icons/dde_calendar_yi_32px.svg" : ":/icons/deepin/builtin/icons/dde_calendar_ji_32px.svg";
    // Use QIcon replace DIcon in order to fix image non-clear issue
    QPixmap pixmap = QIcon(iconPath).pixmap(iconSize);
    pixmap.setDevicePixelRatio(devicePixelRatioF());

    // 计算绘制区域，确保图像按比例缩放
    QRect pixmapRect = QRect(QPoint(m_leftMagin, m_topMagin + 1), iconSize);
    pixmapRect.setSize(pixmapRect.size() * devicePixelRatioF());

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(pixmapRect, pixmap);
    painter.restore();

    painter.setFont(m_font);
    painter.setPen(m_textcolor);
    int bw = m_leftMagin + 50;
    int bh = m_topMagin;
    int ss = 14;
    for (int i = 0; i < m_vHuangli.count(); i++) {
        int currentsw = m_vHuangli.at(i).size() * ss;
        if (bw + currentsw + 15 >= labelwidth) {
            painter.drawText(QRect(bw, bh, labelwidth - bw, 21), Qt::AlignLeft, "...");
            break;
        } else {
            painter.drawText(QRect(bw, bh, currentsw, 21), Qt::AlignLeft, m_vHuangli.at(i));
            bw += currentsw + 10;
        }
    }
}

void CDayHuangLiLabel::resizeEvent(QResizeEvent *event)
{
    m_leftMagin = static_cast<int>(0.0424 * width() + 0.5);
    m_topMagin = (height() - 20) / 2;
    DLabel::resizeEvent(event);
}
