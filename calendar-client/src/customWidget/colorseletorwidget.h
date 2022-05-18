/*
* Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
*
* Author:     leilong <leilong@uniontech.com>
*
* Maintainer: leilong <leilong@uniontech.com>
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
#ifndef COLORSELETOR_H
#define COLORSELETOR_H

#include "cradiobutton.h"
#include "colorWidget/colorpickerWidget.h"
#include "scheduledatamanage.h"
#include "dschedule.h"
#include "dtypecolor.h"

#include <QWidget>
#include <QColor>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QMap>

//色彩控件选择类
class ColorSeletorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorSeletorWidget(QWidget *parent = nullptr);

    /**
     * @brief reset
     * 重置
     */
    void reset();
    /**
     * @brief getSelectColor
     * @return 当前已选在的色彩
     */
    DTypeColor getSelectedColorInfo();
    /**
     * @brief setUserColor 设置用户自定义的色彩
     */
    void setUserColor(const DTypeColor &);
    /**
     * @brief setSelectedColorByIndex 设置选择的色彩控件
     * @param index 色彩控件位置
     */
    void setSelectedColorByIndex(int index);
    /**
     * @brief setSelectedColorById 设置选择的色彩控件
     * @param colorId 色彩id
     */
    void setSelectedColorById(int colorId);
    /**
     * @brief selectColor 设置选择的色彩控件
     * @param color 色彩
     */
    void setSelectedColor(const DTypeColor &color);
    /**
     * @brief initColorButton
     * 初始化色彩控件
     * @param index 初始选择的色彩下标
     */
    void initColorButton(int index = -1);

signals:
    //选择的色彩改变信号
    void signalColorChange(DTypeColor);

public slots:
    //色彩控件点击信号
    void slotButtonClicked(int butId);
    //添加自定义色彩控件点击信号
    void slotAddColorButClicked();

private:
    void init();
    void initView();

    //添加色彩控件
    void addColor(const DTypeColor &);

private:
    QMap<int, DTypeColor> m_colorEntityMap; //所有色彩实体
    QHBoxLayout *m_colorLayout = nullptr;     //色彩控件布局类
    QButtonGroup *m_colorGroup = nullptr;     //所有色彩控件
    DTypeColor m_colorInfo; //当前已选择的色彩
    CRadioButton *m_userColorBtn = nullptr; //用户自定义的色彩控件
    int m_userColorBtnId = -1; //用户自定义的色彩控件id
};

#endif // COLORSELETOR_H
