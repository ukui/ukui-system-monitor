/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *  rxy     renxinyu@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MYUNDERLINEBUTTON_H
#define MYUNDERLINEBUTTON_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QStyleOption>
#include <qgsettings.h>
#include <QApplication>

#include "shell/customstyle.h"
#include "../shell/macro.h"

class QVBoxLayout;

class MyUnderLineButton : public QWidget
{
    Q_OBJECT

    enum ButtonState {Normal, Hover, Press, Checked};

public:
    MyUnderLineButton(QWidget * parent=0);
    ~MyUnderLineButton();

    void setChecked(bool flag);
    bool isChecked();
    void setName(const QString &name);
    ButtonState getButtonState() const;
    void initThemeMode();
    void initFontSize();

signals:
    void clicked();

protected:
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) override;

private:
    void updateStyleSheet();
    void setState(ButtonState state);

private:
    ButtonState m_state;
    ButtonState m_button_state;
    bool m_isChecked;
    QLabel *m_textLabel = nullptr;
    //QLabel *m_underlineLabel = nullptr;
    QVBoxLayout *m_layout = nullptr;

    int fontSize;

    QGSettings *qtSettings;
    QGSettings *fontSettings;
    QString currentThemeMode;

};

#endif // MYUNDERLINEBUTTON_H
