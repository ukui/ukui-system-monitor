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

#ifndef _NEWABOUT_DIALOG_H_
#define _NEWABOUT_DIALOG_H_

#include <QDialog>
#include <QPaintEvent>
#include <QPainter>
#include <QBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QPalette>
#include <QGSettings>

class newaboutdialog : public QDialog
{
    Q_OBJECT
public:
    newaboutdialog(QWidget *parent = 0);
    ~newaboutdialog();
    void initWidgets();
    void initTitleWidget();
    void initContentWidget();
    void initIntroduceWidget();
    void setFontSize(QLabel *label,int fontSize);

private Q_SLOTS:
    void openMailTo(QString strMailAddr);

protected:
//    void paintEvent(QPaintEvent *event);

private:
    void initThemeStyle();

private:
    QHBoxLayout *title_H_BoxLayout;
    QHBoxLayout *img_H_Boxlayout;
    QHBoxLayout *app_H_Boxlayout;
    QHBoxLayout *version_H_Boxlayout;
    QHBoxLayout *introduce_H_Boxlayout;
    QHBoxLayout *support_H_Boxlayout;
    QVBoxLayout *main_V_BoxLayout;
    QLabel *m_titleImgLabel = nullptr;
    QLabel *m_bigImgLabel = nullptr;
//    QLabel *m_titleImgLabel;
//    QLabel *m_titletextLabel;
    QToolButton *m_closeBtn;
    QPalette pe;
    QGSettings *styleSettings;
};

#endif //_NEWABOUT_DIALOG_H_
