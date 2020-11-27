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

#include "netcatogoryshow.h"
#include "smoothcurvegenerator.h"

#include <QMouseEvent>
#include <QEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

NetCatogoryShow::NetCatogoryShow(QWidget *parent)
    : QWidget(parent)
    ,m_isChecked(false)
    ,m_rectTopPadding(9)
    ,m_rectTotalHeight(38)
    ,m_rectTotalWidth(58)
    ,m_outsideBorderColor(Qt::transparent)
    ,qtSettings(nullptr)
{

    const QByteArray idd(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(idd))
    {
        qtSettings = new QGSettings(idd);
    }

    this->setFixedSize(61, 40);
    initThemeMode();
    m_netMaxHeight = 10;
    m_pointSpace = 2;


    m_pointsCount = int((this->width() -2) / m_pointSpace);
    m_downloadSpeedList = new QList<long>();
    for (int i = 0; i < m_pointsCount; i++) {
        m_downloadSpeedList->append(0);
    }

    m_uploadSpeedList = new QList<long>();
    for (int i = 0; i < m_pointsCount; i++) {
        m_uploadSpeedList->append(0);
    }

    m_gridY = new QList<int>();
}

void NetCatogoryShow::initThemeMode()
{
    if (!qtSettings) {
//        qWarning() << "Failed to load the gsettings: " << THEME_QT_SCHEMA;
        return;
    }

    //监听主题改变
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key)
    {
        if (key == "styleName")
        {
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            updateBgColor();
            repaint();
        }
    });
    currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
}

void NetCatogoryShow::updateBgColor()
{
    if(currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
    {
        this->m_bgColor = QColor(13,14,14,100);  //#f6fcfe
    }
    else
    {
        this->m_bgColor = QColor(255,255,255,100);
    }
}

NetCatogoryShow::~NetCatogoryShow()
{
    delete m_downloadSpeedList;
    delete m_uploadSpeedList;
    delete m_gridY;

    if(qtSettings)
    {
        delete qtSettings;
    }
}

void NetCatogoryShow::onUpdateNetworkStatus(long recvTotalBytes, long sentTotalBytes, long recvRateBytes, long sentRateBytes)
{
    m_recvTotalBytes = recvTotalBytes;
    m_sentTotalBytes = sentTotalBytes;
    m_recvRateBytes = recvRateBytes;
    m_sentRateBytes = sentRateBytes;

    //download
    QList<QPointF> downloadPoints;
    m_downloadSpeedList->append(m_recvRateBytes);
    if (m_downloadSpeedList->size() > m_pointsCount) {
        m_downloadSpeedList->pop_front();
    }
    //计算出下载速度中最大的值
    long downloadMaxHeight = 0;
    for (int i = 0; i < m_downloadSpeedList->size(); i++) {
        if (m_downloadSpeedList->at(i) > downloadMaxHeight) {
            downloadMaxHeight = m_downloadSpeedList->at(i);
        }
    }
    for (int i = 0; i < m_downloadSpeedList->size(); i++) {
        if (downloadMaxHeight < m_netMaxHeight) {
            downloadPoints.append(QPointF(i * m_pointSpace, m_downloadSpeedList->at(i)));
        }
        else {
            downloadPoints.append(QPointF(i * m_pointSpace, m_downloadSpeedList->at(i) * m_netMaxHeight / downloadMaxHeight));
        }
    }
    m_downloadPath = SmoothCurveGenerator::generateSmoothCurve(downloadPoints);

    //upload
    QList<QPointF> uploadPoints;
    m_uploadSpeedList->append(m_sentRateBytes);
    if (m_uploadSpeedList->size() > m_pointsCount) {
        m_uploadSpeedList->pop_front();
    }
    //计算出上传速度中最大的值
    long uploadMaxHeight = 0;
    for (int i = 0; i < m_uploadSpeedList->size(); i++) {
        if (m_uploadSpeedList->at(i) > uploadMaxHeight) {
            uploadMaxHeight = m_uploadSpeedList->at(i);
        }
    }

    for (int i = 0; i < m_uploadSpeedList->size(); i++) {
        if (uploadMaxHeight < m_netMaxHeight) {
            uploadPoints.append(QPointF(i * m_pointSpace, m_uploadSpeedList->at(i)));
        }
        else {
            uploadPoints.append(QPointF(i * m_pointSpace, m_uploadSpeedList->at(i) * m_netMaxHeight / uploadMaxHeight));
        }
    }
    m_uploadPath = SmoothCurveGenerator::generateSmoothCurve(uploadPoints);

    repaint();
}

void NetCatogoryShow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    //border of rect
    QPainterPath borderPath;
    borderPath.addRoundedRect(this->rect(), 0, 0);
    QPen pen(this->m_outsideBorderColor, 1);
    painter.setPen(pen);
    painter.drawPath(borderPath);

    //background of rect
    painter.setOpacity(0.3);
    QPainterPath path;
    path.addRoundedRect(QRectF(1, 1, width()-2, height()-2),4,4);
    painter.fillPath(path, this->m_bgColor);
    qDebug()<<"m_bgColr---"<<this->m_bgColor;

//    painter.setClipRect(-50,0,rect().width() - 30,rect().height());

    painter.translate((rect().width() - m_pointsCount * m_pointSpace) / 2 + 2, 24);//将坐标第原点移动到该点
    painter.scale(1, -1);//将横坐标扩大1倍,将纵坐标缩小1倍
    //使用QPainterPath画贝塞尔曲线
    painter.setOpacity(1);
    painter.setPen(QPen(QColor("#009944"), 1));
    painter.setBrush(QBrush());
    painter.drawPath(m_downloadPath);//绘制前面创建的path:m_downloadPath
    painter.translate(0, -4);//将点（0，-8）设为原点
    painter.setPen(QPen(QColor("#e60012"), 1));
    painter.setBrush(QBrush());
    painter.drawPath(m_uploadPath);

    QWidget::paintEvent(event);
}
