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

#include "networkchart.h"
#include "../shell/macro.h"
#include "smoothlinegenerator.h"
#include <QPainter>
#include <QDebug>

NetWorkChart::NetWorkChart(QWidget *parent):QWidget(parent)
  ,m_outsideBorderColor("transparent")
  ,m_downLoadColor(QColor("#42b1eb"))
  ,m_upLoadColor(QColor("#f1bf48"))
{
    this->setMinimumSize(680,90);
    this->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    m_pointsCount = int((this->width()) /10);
    m_downLoadMaxHeight = int(this->width()/2);
    m_downLoadList = new QList<double>();
    m_upLoadMaxHeight = int(this->width()/2);
    m_upLoadList = new QList<double>();
}

NetWorkChart::~NetWorkChart()
{
    if(m_downLoadList)
    {
        delete m_downLoadList;
    }
    if(m_upLoadList)
    {
        delete m_upLoadList;
    }
}

void NetWorkChart::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();
    painter.setOpacity(0.08);
    QPainterPath framePath;
    QStyleOption opt;
    opt.init(this);
    m_bgColor = (QColor("#131414"));
    framePath.addRoundedRect(rect(),4,4);
    painter.fillPath(framePath, this->m_bgColor);//painter.drawPath(framePath);
    painter.restore();

//draw separate lines
    painter.save();
    painter.setOpacity(0.3);
    int distance = this->rect().height()/4;
    painter.setPen(this->palette().color(QPalette::Base));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(rect().x(), rect().y() + distance, rect().right(), rect().y() + distance);
    painter.drawLine(rect().x(), rect().y() + distance * 2, rect().right(), rect().y() + distance * 2);
    painter.drawLine(rect().x(), rect().y() + distance * 3, rect().right(), rect().y() + distance * 3);
    painter.restore();

//draw download line
    painter.save();
//    painter.translate((rect().width() - m_pointsCount * POINTSPACE - 2) / 2 + 6, 89);//将坐标的原点移动到该点
    painter.translate(rect().right(),rect().bottom() - 2);
    painter.scale(-1, -1);//将横坐标扩大1倍,将纵坐标缩小1倍
    QPen pen(this->m_downLoadColor,2);
    painter.setPen(pen);
    painter.setBrush(QBrush());//painter.setBrush(QBrush(QColor("#f4f2f4")));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPath(m_downLoadPath);//绘制前面创建的path:m_downloadPath
    painter.restore();

//draw upload line
    painter.save();
    painter.setOpacity(1);
    painter.translate(rect().right(),rect().bottom());//将坐标的原点移动到该点
    painter.scale(-1, -1);//将横坐标扩大1倍,将纵坐标缩小1倍
    QPen penUpload(this->m_upLoadColor,2);
    painter.setPen(penUpload);
    painter.setBrush(QBrush());//painter.setBrush(QBrush(QColor("#f4f2f4")));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPath(m_upLoadPath);//绘制前面创建的path:m_downloadPath
    painter.restore();
    QWidget::paintEvent(event);
}

void NetWorkChart::onUpdateDownloadAndUploadData(long recvTotalBytes, long sentTotalBytes, long recvRateBytes, long sentRateBytes)
{
//    qDebug()<<"recvRateBytes"<<recvRateBytes;
//    qDebug()<<"sentRateBytes"<<sentRateBytes;
//    if(recvRateBytes + sentRateBytes >= 1024 * 1024)
//    {
//        emit this->speedToMib();
//    }
    if( recvRateBytes + sentRateBytes <= 20 * 1024)
    {
        emit this->speedToLowKib();
    }

//    if(recvRateBytes + sentRateBytes > 20 *1024 && recvRateBytes + sentRateBytes <= 100 * 1024)
//    {
//        emit this->speedToMiddleKib();
//    }
    if(recvRateBytes + sentRateBytes > 100 *1024 && recvRateBytes + sentRateBytes <= 1000 * 1024)
    {
        emit this->speedToHighKib();
    }

    if(recvRateBytes+sentRateBytes < 1024 * 1024 *100)
    {
        if(recvRateBytes+sentRateBytes < 20 * 1024)
        {
            m_downLoadSpeed = (recvRateBytes/1024) * (this->height()/20);
            if(m_upLoadSpeed < 1024)
            {
                m_upLoadSpeed = 0;
            }
            if(m_upLoadSpeed > 1024 && m_upLoadSpeed < 20 * 1024)
            {
                m_upLoadSpeed = (sentRateBytes/1024) * this->height()/20;
            }
        }
        else
        {
            m_downLoadSpeed = (recvRateBytes/1000) * this->height()/1000;
            m_upLoadSpeed = (sentRateBytes/1024) * this->height()/1000;
            qDebug()<<"if i m_downLoadSpeed"<<m_downLoadSpeed;
            qDebug()<<"if i m_upLoadSpeed"<<m_upLoadSpeed;
        }
    }

//    m_downLoadSpeed = (recvRateBytes/1024) * this->height()/1000;
    QList<QPointF> downLoadPoints;
    m_downLoadList->append(m_downLoadSpeed);
    while (m_downLoadList->size() > m_pointsCount)
    {
        m_downLoadList->pop_front();
    }

    QList<QPointF> upLoadPoints;
    m_upLoadList->append(m_upLoadSpeed);
    while(m_upLoadList->size() > m_pointsCount)
    {
        m_upLoadList->pop_front();
    }

    double downLoadMaxHeight = 0.0;
    for (int i = 0; i < m_downLoadList->size(); i++)
    {
        if (m_downLoadList->at(i) > downLoadMaxHeight)
        {
            downLoadMaxHeight = m_downLoadList->at(i);
        }
    }

    for (int i = 0; i < m_downLoadList->size(); i++)
    {
//        if (downLoadMaxHeight < m_downLoadMaxHeight)
//        {
            downLoadPoints.append(QPointF((m_downLoadList->size() - i -2) * POINTSPACE, m_downLoadList->at(i)));
//        }
//        else
//        {
//            downLoadPoints.append(QPointF((m_downLoadList->size() - i -2) * POINTSPACE, m_downLoadList->at(i) * m_downLoadMaxHeight /downLoadMaxHeight));
//        }
    }
    m_downLoadPath = SmoothLineGenerator::generateSmoothCurve(downLoadPoints);

    double upLoadMaxHeight = 0.0;
    for (int i = 0; i < m_upLoadList->size(); i++)
    {
        if (m_upLoadList->at(i) > upLoadMaxHeight)
        {
            upLoadMaxHeight = m_upLoadList->at(i);
        }
    }
    for (int i = 0; i < m_upLoadList->size(); i++)
    {
        if (upLoadMaxHeight < m_upLoadMaxHeight)
        {
            upLoadPoints.append(QPointF((m_upLoadList->size() - i -2) * POINTSPACE, m_upLoadList->at(i)));
        }
        else
        {
            upLoadPoints.append(QPointF((m_upLoadList->size() - i -2) * POINTSPACE, m_upLoadList->at(i) * m_upLoadMaxHeight /upLoadMaxHeight));
        }
    }
    m_upLoadPath = SmoothLineGenerator::generateSmoothCurve(upLoadPoints);
}
