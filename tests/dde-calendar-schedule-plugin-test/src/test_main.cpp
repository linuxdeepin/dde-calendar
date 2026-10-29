// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <QTimer>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    QTimer::singleShot(0, &app, []() {
        int ret = RUN_ALL_TESTS();
        QApplication::exit(ret);
    });

    return app.exec();
}
