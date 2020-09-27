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

#ifndef NETCATOGORYSHOW_H
#define NETCATOGORYSHOW_H

#include <QObject>
#include <QWidget>
#include <QPixmap>
#include <qgsettings.h>
#include <QApplication>
#include <QPainterPath>

#include "../shell/macro.h"
#include "shell/customstyle.h"

class NetCatogoryShow : public QWidget
{
    Q_OBJECT

public:
    NetCatogoryShow(QWidget * parent=0);
    ~NetCatogoryShow();

//    void updateNetworkPainterPath(QPainterPath downloadPath, QPainterPath uploadPath);

//    NetworkState getNetworkState() const;
    void initThemeMode();

public slots:
    void onUpdateNetworkStatus(long recvTotalBytes, long sentTotalBytes, long recvRateKbs, long sentRateKbs);

signals:
    void clicked();

protected:
//    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
//    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
//    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
//    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
//    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    void updateBgColor();
//    void setNetworkState(NetworkState state);

private:
    bool m_isChecked;
    QString m_title;
    int m_rectTopPadding;
    int m_rectTotalHeight;
    int m_rectTotalWidth;
    QColor m_outsideBorderColor;
    QColor m_bgColor;

    QList<long> *m_downloadSpeedList;
    QList<long> *m_uploadSpeedList;
    QPainterPath m_downloadPath;
    QPainterPath m_uploadPath;

    int m_netMaxHeight;
    int m_pointsCount;
    QList<int> *m_gridY;
    int m_pointSpace;

    long m_recvTotalBytes;
    long m_sentTotalBytes;
    long m_recvRateBytes;
    long m_sentRateBytes;

    QGSettings * qtSettings;
    QString currentThemeMode;

};

#endif // NETCATOGORYSHOW_H
