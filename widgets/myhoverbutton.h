/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MYHOVERBUTTON_H
#define MYHOVERBUTTON_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPixmap>

class MyHoverButton : public QLabel
{
    Q_OBJECT

    enum HoverButtonState {Normal, Hover, Press, Checked};

public:
    MyHoverButton(QWidget * parent=0);
    ~MyHoverButton();

    void setChecked(bool flag);
    bool isChecked();
    void setPicture(const QString &picture);
    HoverButtonState getButtonState() const;

signals:
    void clicked();
    void stateChanged();

protected:
    void enterEvent(QEvent * event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent * event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent * event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent * event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    void updateBackgroundColor();
    void setState(HoverButtonState state);

private:
    HoverButtonState m_state;
    bool m_isChecked;
};

#endif // MYHOVERBUTTON_H
