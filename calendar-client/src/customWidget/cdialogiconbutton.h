// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CDIALOGICON_H
#define CDIALOGICON_H

#include "timejumpdialog.h"
#include <DIconButton>
#include <QLoggingCategory>

DWIDGET_USE_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(dialogIconLog)

//时间跳转控件
class CDialogIconButton : public DIconButton
{
    Q_OBJECT
public:
    explicit CDialogIconButton(QWidget *parent = nullptr);

signals:

public slots:
    //点击事件
    void slotIconClicked();

private:
    void initView();

private:
    TimeJumpDialog *m_dialog = nullptr; //时间跳转弹窗
};

#endif // CDIALOGICON_H
