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
#ifndef SCHEDULETYPEEDITDLG_H
#define SCHEDULETYPEEDITDLG_H

#include "colorseletorwidget.h"
#include "dschedule.h"
#include "dscheduletype.h"

#include <DDialog>
#include <DLineEdit>

DWIDGET_USE_NAMESPACE

class ScheduleTypeEditDlg : public DDialog
{
    Q_OBJECT
public:
    //新增
    explicit ScheduleTypeEditDlg(QWidget *parent = nullptr);
    //iJobTypeNo==0 ？ 新增 or 修改
    explicit ScheduleTypeEditDlg(const DScheduleType &jobTypeOld, QWidget *parent = nullptr);

private:
    enum DialogType { //对话框类型
        DialogNewType, //新建日程类型
        DialogEditType //编辑日程类型
    };

signals:
    /**
     * @brief refreshInfo:刷新信息信号
     */
    void signalRefreshScheduleType();
public slots:
    //编辑器文本改变事件
    void slotEditTextChanged(const QString &);

    /**
     * @brief slotFocusChanged      输入框焦点改变处理
     * @param onFocus
     */
    void slotFocusChanged(bool onFocus);
    /**
     * @brief slotBtnCancel 取消按钮
     */
    void slotBtnCancel();

    /**
     * @brief slotBtnNext 保存按钮，检查输入，保存、更新日程类型信息
     */
    void slotBtnNext();
    /**
     * @brief slotEditingFinished       编辑框编辑结束后处理
     */
    void slotEditingFinished();

private:
    void init();
    void initView();
    void initData();

private:
    DScheduleType m_jobTypeOld; //被修改的日程类型
    DScheduleType m_jobTypeNew; //修改后的日程类型
    QString m_title = "";   //弹窗名
    DLineEdit *m_lineEdit = nullptr;    //编辑器
    QLabel *m_titleLabel = nullptr;     //弹窗名控件
    ColorSeletorWidget *m_colorSeletor = nullptr; //颜色选择器
    DialogType m_dialogType;
    QString m_typeText; //输入框上一次输入后的文本
};

#endif // SCHEDULETYPEEDITDLG_H
