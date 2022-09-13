// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONFIRWFEEDBACKSTATE_H
#define CONFIRWFEEDBACKSTATE_H

#include "schedulestate.h"

class confirwFeedbackState : public scheduleState
{
public:
    confirwFeedbackState(CSchedulesDBus *dbus, scheduleBaseTask *task);
    Reply getReplyByIntent(bool isOK) override;

protected:
    Filter_Flag eventFilter(const JsonData *jsonData) override;
    Reply ErrEvent() override;
    Reply normalEvent(const JsonData *jsonData) override;
};

#endif // CONFIRWFEEDBACKSTATE_H
