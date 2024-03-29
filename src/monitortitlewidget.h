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

#ifndef MONITORTITLEWIDGET_H
#define MONITORTITLEWIDGET_H

#include <QFrame>
#include <QTimer>
#include <QSettings>
#include <QComboBox>
#include <QListView>
#include <qgsettings.h>
#include <QPropertyAnimation>
#include <QToolButton>
#include <QtDBus/QtDBus>
#include <QLineEdit>

#include "shell/customstyle.h"
#include "../shell/macro.h"
#include "control/kgroupbutton.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;

class MonitorTitleWidget : public QFrame
{
    Q_OBJECT
public:
    MonitorTitleWidget(QSettings *settings, QWidget *parent);
    ~MonitorTitleWidget();
    void initTitlebarLeftContent();
    void initTitlebarMiddleContent();
    void initTitlebarRightContent();
    void initToolbarLeftContent();
    void initToolbarRightContent();
    void initWidgets();
    void setSearchEditFocus();
    void initThemeMode();
    void initFontSize();
    void getTransparentData();
    inline int whichProcType()
    { return whichNum; }

public slots:
    void onRefreshSearchResult();
    void handleSearchTextChanged();
    void onCancelSearchBtnClicked(bool b);

    void onMinBtnClicked();
    void onMaxBtnClicked();
    void onCloseBtnClicked();
    void onUpdateMaxBtnStatusChanged();
    void switchChangeItemProcessSignal(int a);
    void animationFinishedSlot();
    void onThemeFontChange(unsigned uFontSize);
    void onUpdateMaxBtnState();
private:
    int daemonIsNotRunning();
    void showGuide(QString appName);
    void setMyComBoxTootip(int index);
    void showSearchFocusAnimation(bool bIn);
signals:
    void updateMaxBtn();
    void changePage(int index);
    void searchSignal(QString searchContent);
    void canelSearchEditFocus();
    void changeProcessItemDialog(int index);
    void maximizeWindow();
    void minimizeWindow();
    void SearchFocusOut();
    void SearchFocusIn();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
//    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *, QEvent *event) override;
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
private:
    QPropertyAnimation *m_animation;
    QEasingCurve m_hideCurve;
    QEasingCurve m_showCurve;
    QSettings *proSettings;
    QLineEdit *m_searchEditNew = nullptr;
    QWidget *m_queryWid=nullptr;
    QLabel *m_queryIcon=nullptr;
    QLabel *m_queryText=nullptr;
    //QPushButton *m_cancelSearchBtn = nullptr;
    QString searchTextCache;
    QTimer *m_searchTimer = nullptr;
    QLabel *emptyLabel = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QHBoxLayout *m_topLayout = nullptr;
    QHBoxLayout *m_titleMiddleLayout = nullptr;
    QHBoxLayout *m_titleRightLayout = nullptr;
    QHBoxLayout *m_bottomLayout = nullptr;
    QHBoxLayout *m_titleLeftLayout = nullptr;
    QHBoxLayout *m_toolLeftLayout = nullptr;
    QHBoxLayout *m_toolRightLayout = nullptr;
    QWidget *buttonWidget = nullptr;
    KGroupButton *m_processButton = nullptr;
    KGroupButton *m_resourceButton = nullptr;
    KGroupButton *m_filesystemButton = nullptr;
    QToolButton *maxTitleBtn = nullptr;
    QComboBox *m_changeBox;
    QLabel *titleLabel;
    QLabel *m_picLabel = nullptr;
    QWidget *emptyWidget = nullptr;
    QWidget *emptyWidget2 = nullptr;
    QRectF m_rectSeachAnimationBegin;
    QRectF m_rectSeachAnimationEnd;
    QMenu *m_toolMenu = nullptr;

    float fontSize;

    QGSettings *qtSettings;
    QGSettings *fontSettings;
    QGSettings * ifsettings;
    QGSettings *opacitySettings;
    QString currentThemeMode;
    double m_transparency;

    int whichNum;
    int i=0;
    QPixmap pixmap;
    bool m_isSearching = false;
    QHBoxLayout* queryWidLayout;
    QList<QAction *>actions;
//    aboutdialog *showaboutdialog;
};

#endif // MONITORTITLEWIDGET_H
