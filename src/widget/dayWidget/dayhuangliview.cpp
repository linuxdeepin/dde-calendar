/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dayhuangliview.h"
#include "schceduledlg.h"

#include <DHiDPIHelper>

#include <QAction>
#include <QListWidget>
#include <QLabel>
#include <QPainter>
#include <QHBoxLayout>
#include <QStylePainter>
#include <QRect>
/**
 * @brief CDayHuangLiLabel::CDayHuangLiLabel 构造函数
 * @param parent 父类
 */
CDayHuangLiLabel::CDayHuangLiLabel(QWidget *parent)
    : DLabel(parent)
{
    setMargin(0);
}
/**
 * @brief CDayHuangLiLabel::setbackgroundColor 设置背景色
 * @param backgroundColor 背景色
 */
void CDayHuangLiLabel::setbackgroundColor(QColor backgroundColor)
{
    //背景色
    m_backgroundColor = backgroundColor;
}
/**
 * @brief CDayHuangLiLabel::setTextInfo 设置字体信息
 * @param tcolor 颜色
 * @param font 字体
 */
void CDayHuangLiLabel::setTextInfo(QColor tcolor, QFont font)
{
    //字体颜色
    m_textcolor = tcolor;
    //字体大小
    m_font = font;
}
/**
 * @brief CDayHuangLiLabel::setHuangLiText 设置黄历
 * @param vhuangli 黄历
 * @param type 宜忌
 */
void CDayHuangLiLabel::setHuangLiText(QStringList vhuangli, int type)
{
    //设置黄历信息
    m_vHuangli = vhuangli;
    //宜忌
    m_type = type;
    //stringlist不为空设置黄历
    if (!vhuangli.isEmpty()) {
        //设置第一个string
        QString  str = vhuangli.at(0);
        //遍历stringlist
        for (int i = 1; i < vhuangli.count(); i++) {
            str += "." + vhuangli.at(i);
        }
        //设置鼠标悬停显示
        setToolTip(str);
    } else {
        //stringlist为空，鼠标悬停显示空
        setToolTip(QString());
    }
    update();
}
/**
 * @brief CDayHuangLiLabel::paintEvent 绘制黄历
 * @param e 绘图事件
 */
void CDayHuangLiLabel::paintEvent( QPaintEvent *e )
{
    Q_UNUSED(e);
    //label宽度
    int labelwidth = width();
    //label高度
    int labelheight = height();
    //painter
    QPainter painter(this);
    //设置矩形
    QRect fillRect = QRect(0, 0, labelwidth, labelheight);
    painter.setRenderHints(QPainter::HighQualityAntialiasing);
    painter.setBrush(QBrush(m_backgroundColor));
    painter.setPen(Qt::NoPen);
    //绘制矩形
    painter.drawRoundedRect(fillRect, 12, 12);
    QPixmap pixmap;
    //设置宜忌图像
    if (m_type == 0)
        //宜图像
        pixmap = DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-yi.svg");
    else {
        //忌图像
        pixmap = DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-ji.svg");
    }
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    //绘制图像
    painter.drawPixmap(QRect(m_leftMagin, m_topMagin + 1, 22, 22), pixmap);
    painter.restore();

    painter.setFont(m_font);
    painter.setPen(m_textcolor);
    int bw = m_leftMagin + 34;
    int bh = m_topMagin;
    int ss = 14;
    for (int i = 0; i < m_vHuangli.count(); i++) {
        int currentsw = m_vHuangli.at(i).count() * ss;
        if (bw + currentsw + 6 >  labelwidth) {
            //宽度过大绘制...
            painter.drawText(QRect(bw, bh, labelwidth - bw, 21), Qt::AlignLeft, "...");
            break;
        } else {
            //绘制文字
            painter.drawText(QRect(bw, bh, currentsw, 21), Qt::AlignLeft, m_vHuangli.at(i));
            bw += currentsw + 10;
        }
    }
}
/**
 * @brief CDayHuangLiLabel::resizeEvent 调整窗口大小
 * @param event 窗口大小调整事件
 */
void CDayHuangLiLabel::resizeEvent(QResizeEvent *event)
{
    m_leftMagin = static_cast<int>(0.0424 * width() + 0.5);
    m_topMagin = (height() - 20) / 2;
    DLabel::resizeEvent(event);
}


