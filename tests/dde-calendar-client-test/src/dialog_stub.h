// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIALOG_STUB_H
#define DIALOG_STUB_H

#include "../third-party_stub/stub.h"

//DCalendarDDialog
//对话框 exec 打桩
int calendar_DDialog_Exec_stub(void *obj);

void calendarDDialogExecStub(Stub &stub);

extern int calendarDDialogExecReturn;

//QMenu
//菜单 exec 打桩（V-6590 引入，原定义未随附件上传，按 dialog_stub 既有样式补齐）
class QMenu;
void calendarQMenuExecStub(Stub &stub);

#endif // DIALOG_STUB_H
