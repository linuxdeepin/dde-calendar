/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
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

#ifndef CCONFIGSETTINGS_H
#define CCONFIGSETTINGS_H

#include <QSettings>
#include <QPointer>

class CConfigSettings : QObject
{
    Q_OBJECT
public:
    static CConfigSettings *getInstance();
    void sync();
    /**
     * @brief value         根据key获取对应值
     * @param key           对应的key
     * @return
     */
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
    /**
     * @brief setOption     设置对应key的值
     * @param key
     * @param value
     */
    void setOption(const QString &key, const QVariant &value);
    /**
     * @brief remove        移除对应的配置信息
     * @param key
     */
    void remove(const QString &key);
    /**
     * @brief contains      判断是否包含对应的key
     * @param key
     * @return
     */
    bool contains(const QString &key) const;
    CConfigSettings *operator->() const ;

    void initSetting();

    //帐户侧边栏显示状态
    bool getUserSidebarStatus();
    void setUserSidebarStatus(bool);

signals:
    void signalFirstDayOfWeekChange();

public slots:

protected:
    CConfigSettings();
    ~CConfigSettings();
    void init();
    //释放配置指针
    void releaseInstance();
private:
    QPointer<QSettings> m_settings;
    bool m_userSidebarStatus = true;    //帐户侧边栏显示状态
};

#define gSetting CConfigSettings::getInstance()

#endif // CCONFIGSETTINGS_H
