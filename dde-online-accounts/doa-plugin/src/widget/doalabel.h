/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
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
#ifndef DOALABEL_H
#define DOALABEL_H

#include <QLabel>

class DOALabel : public QLabel
{
    Q_OBJECT
public:
    explicit DOALabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    explicit DOALabel(const QString &text, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    void setShowText(const QString &text);

protected:
    void changeEvent(QEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    /**
     * @brief setTextByWidth        根据宽度设置显示字符
     */
    void setTextByWidth();
signals:

public slots:
private:
    QString m_text;
};

#endif // DOALABEL_H