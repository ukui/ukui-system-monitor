/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *  rxy         renxinyu@kylinos.cn
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

#include "mytristatebutton.h"

#include <QMouseEvent>
#include <QEvent>

MyTristateButton::MyTristateButton(QWidget *parent)
    : QLabel(parent)
{
    setCheckable(false);//setCheckable(true);
    updateIcon();
    this->setStyleSheet("QLabel{background-color:transparent;border:none;}");
    m_pClearTextButton = new QPushButton;
    //QPixmap fileName(":/img/button-close-default-add-background-three.svg");
    this->setFixedSize(19, 21);
    m_pClearTextButton->setIconSize(QSize(9, 9));

    m_pClearTextButton->setStyleSheet("QPushButton{border-image:url(:/image/button-close-default-add-background-three.svg)}");
    QHBoxLayout *cancel_h_BoxLayout = new QHBoxLayout(this);
    cancel_h_BoxLayout->addWidget(m_pClearTextButton);
}

MyTristateButton::~MyTristateButton()
{
}

void MyTristateButton::enterEvent(QEvent *event)
{
    setCursor(Qt::PointingHandCursor);

    if (!m_isChecked){
        setState(Hover);
    }

    event->accept();
    //QLabel::enterEvent(event);
}

void MyTristateButton::leaveEvent(QEvent *event)
{
    if (!m_isChecked){
        setState(Normal);
    }

    event->accept();
    //QLabel::leaveEvent(event);
}

void MyTristateButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    setState(Press);

    event->accept();
    //QLabel::mousePressEvent(event);
}

void MyTristateButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (!rect().contains(event->pos()))
        return;

    if (m_isCheckable){
        m_isChecked = !m_isChecked;
        if (m_isChecked){
            setState(Checked);
        } else {
            setState(Normal);
        }
    } else {
        setState(Hover);
    }

    event->accept();
    //QLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
        emit clicked();
}

void MyTristateButton::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isCheckable && !rect().contains(event->pos())) {
        setState(Normal);
    }
}

void MyTristateButton::updateIcon()
{
    switch (m_state) {
    case Hover:     if (!m_hoverPic.isEmpty()) setPixmap(QPixmap(m_hoverPic));      break;
    case Press:     if (!m_pressPic.isEmpty()) setPixmap(QPixmap(m_pressPic));      break;
    case Checked:   if (!m_checkedPic.isEmpty()) setPixmap(QPixmap(m_checkedPic));  break;
    default:        if (!m_normalPic.isEmpty()) setPixmap(QPixmap(m_normalPic));    break;
    }

    setAlignment(Qt::AlignCenter);
}

void MyTristateButton::setState(MyTristateButton::ButtonState state)
{
    if (m_state == state)
        return;

    m_state = state;

    updateIcon();
}

void MyTristateButton::setCheckable(bool flag)
{
    m_isCheckable = flag;

    if (!m_isCheckable){
        setState(Normal);
    }
}

void MyTristateButton::setChecked(bool flag)
{
    if (m_isCheckable == false){
        return;
    }

    m_isChecked = flag;
    if (m_isChecked){
        setState(Checked);
    } else {
        setState(Normal);
    }
}

bool MyTristateButton::isChecked()
{
    return m_isChecked;
}

bool MyTristateButton::isCheckable()
{
    return m_isCheckable;
}

void MyTristateButton::setNormalPic(const QString &normalPicPixmap)
{
    m_normalPic = normalPicPixmap;
    updateIcon();
}

void MyTristateButton::setHoverPic(const QString &hoverPicPixmap)
{
    m_hoverPic = hoverPicPixmap;
    updateIcon();
}

void MyTristateButton::setPressPic(const QString &pressPicPixmap)
{
    m_pressPic = pressPicPixmap;
    updateIcon();
}

void MyTristateButton::setCheckedPic(const QString &checkedPicPixmap)
{
    m_checkedPic = checkedPicPixmap;
    updateIcon();
}

MyTristateButton::ButtonState MyTristateButton::getButtonState() const
{
    return m_state;
}
