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
#include <QPainterPath>

#define NETHIS_POINT_COUNT_MAX      100

NetWorkChart::NetWorkChart(QWidget *parent):QWidget(parent)
  ,m_outsideBorderColor("transparent")
  ,m_downLoadColor(QColor("#42b1eb"))
  ,m_upLoadColor(QColor("#f1bf48"))
{
    this->setMinimumSize(640,90);
    this->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    m_pointsCount = NETHIS_POINT_COUNT_MAX;
    m_downLoadList.clear();
    m_upLoadList.clear();
    m_curMaxLoadSpeed = 20 * 1024;
    m_bgColor = (QColor("#131414"));
}

NetWorkChart::~NetWorkChart()
{
}

void NetWorkChart::setBgColor(QColor bgColor)
{
    m_bgColor = bgColor;
}

void NetWorkChart::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();
    painter.setOpacity(0.08);
    QPainterPath framePath;
    QStyleOption opt;
    opt.init(this);
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

// calculate download path
    qreal lfPointSpace = (qreal)width()/m_pointsCount;
    qreal lfPointYP = (qreal)height()/100;
    QList<QPointF> downloadPoints;
    for (int i = 0; i < m_downLoadList.size(); i++)
    {
        downloadPoints.append(QPointF((m_downLoadList.size()-i-1) * lfPointSpace, m_downLoadList[i]*100*lfPointYP/m_curMaxLoadSpeed));
    }
    if (!downloadPoints.isEmpty()) {
        QPainterPath pathTmp;
        m_downLoadPath.swap(pathTmp);
        m_downLoadPath.moveTo(downloadPoints[0]);
        for (int n = 1; n < downloadPoints.size(); n++) {
            #if 1
            QPointF ctlPoint[2];
            ctlPoint[0].setX((downloadPoints[n-1].x()+downloadPoints[n].x())/2);
            ctlPoint[0].setY(downloadPoints[n-1].y());
            ctlPoint[1].setX((downloadPoints[n-1].x()+downloadPoints[n].x())/2);
            ctlPoint[1].setY(downloadPoints[n].y());
            m_downLoadPath.cubicTo(ctlPoint[0], ctlPoint[1], downloadPoints[n]);
            #else
            m_downLoadPath.lineTo(downloadPoints[n]);
            #endif
        }
    }
// calculate upload path
    QList<QPointF> uploadPoints;
    for (int i = 0; i < m_upLoadList.size(); i++)
    {
        uploadPoints.append(QPointF((m_upLoadList.size()-i-1) * lfPointSpace, m_upLoadList[i]*100*lfPointYP/m_curMaxLoadSpeed));
    }
    if (!uploadPoints.isEmpty()) {
        QPainterPath pathTmp;
        m_upLoadPath.swap(pathTmp);
        m_upLoadPath.moveTo(uploadPoints[0]);
        for (int n = 1; n < uploadPoints.size(); n++) {
            #if 1
            QPointF ctlPoint[2];
            ctlPoint[0].setX((uploadPoints[n-1].x()+uploadPoints[n].x())/2);
            ctlPoint[0].setY(uploadPoints[n-1].y());
            ctlPoint[1].setX((uploadPoints[n-1].x()+uploadPoints[n].x())/2);
            ctlPoint[1].setY(uploadPoints[n].y());
            m_upLoadPath.cubicTo(ctlPoint[0], ctlPoint[1], uploadPoints[n]);
            #else
            m_upLoadPath.lineTo(uploadPoints[n]);
            #endif
        }
    }
//draw download/upload line
    painter.save();
    painter.setOpacity(1);
    painter.translate(rect().right(),rect().bottom()-1);//将坐标的原点移动到该点
    painter.rotate(180);//将横纵坐标顺时针旋转180°
    QPen pen(this->m_downLoadColor,2);
    painter.setPen(pen);
    painter.setBrush(QBrush());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPath(m_downLoadPath);

    painter.translate(0,-1);//将坐标的原点移动到该点
    QPen penUpload(this->m_upLoadColor,2);
    painter.setPen(penUpload);
    painter.drawPath(m_upLoadPath);
    painter.restore();
    QWidget::paintEvent(event);
}

void NetWorkChart::onUpdateDownloadAndUploadData(quint64 recvTotalBytes, quint64 sentTotalBytes, quint64 recvRateBytes, quint64 sentRateBytes)
{
    quint64 lluMaxNetSpeed = 0;
    m_downLoadList.append(recvRateBytes);
    while (m_downLoadList.size() > m_pointsCount) {
        m_downLoadList.pop_front();
    }
    m_upLoadList.append(sentRateBytes);
    while(m_upLoadList.size() > m_pointsCount) {
        m_upLoadList.pop_front();
    }

    for (int n = 0; n < m_downLoadList.size(); n++) {
        if (lluMaxNetSpeed<m_downLoadList[n]) {
            lluMaxNetSpeed = m_downLoadList[n];
        }
        if (lluMaxNetSpeed<m_upLoadList[n]) {
            lluMaxNetSpeed = m_upLoadList[n];
        }
    }

    if (lluMaxNetSpeed < 20 * 1024) {
        m_curMaxLoadSpeed = 20 * 1024;
    } else if (lluMaxNetSpeed >= 20 *1024 && lluMaxNetSpeed < 100 * 1024) {
        m_curMaxLoadSpeed = 100 * 1024;
    } else if (lluMaxNetSpeed >= 100 *1024 && lluMaxNetSpeed < 500 * 1024) {
        m_curMaxLoadSpeed = 500 * 1024;
    } else if (lluMaxNetSpeed >= 500 *1024 && lluMaxNetSpeed < 1 * 1024 * 1024) {
        m_curMaxLoadSpeed = 1 * 1024 * 1024;
    } else if (lluMaxNetSpeed >= 1 * 1024 * 1024 && lluMaxNetSpeed < 2 * 1024 * 1024) {
        m_curMaxLoadSpeed = 2 * 1024 * 1024;
    } else if (lluMaxNetSpeed >= 2 * 1024 * 1024 && lluMaxNetSpeed < 4 * 1024 * 1024) {
        m_curMaxLoadSpeed = 4 * 1024 * 1024;
    } else if (lluMaxNetSpeed >= 4 * 1024 * 1024 && lluMaxNetSpeed < 8 * 1024 * 1024) {
        m_curMaxLoadSpeed = 8 * 1024 * 1024;
    } else if (lluMaxNetSpeed >= 8 * 1024 * 1024 && lluMaxNetSpeed < 16 * 1024 * 1024) {
        m_curMaxLoadSpeed = 16 * 1024 * 1024;
    } else if (lluMaxNetSpeed >= 16 * 1024 * 1024 && lluMaxNetSpeed < 32 * 1024 * 1024) {
        m_curMaxLoadSpeed = 32 * 1024 * 1024;
    } else if (lluMaxNetSpeed >= 32 * 1024 * 1024 && lluMaxNetSpeed < 64 * 1024 * 1024) {
        m_curMaxLoadSpeed = 64 * 1024 * 1024;
    } else if (lluMaxNetSpeed >= 64 * 1024 * 1024 && lluMaxNetSpeed < 128 * 1024 * 1024) {
        m_curMaxLoadSpeed = 128 * 1024 * 1024;
    } else if (lluMaxNetSpeed >= 128 * 1024 * 1024 && lluMaxNetSpeed < 256 * 1024 * 1024) {
        m_curMaxLoadSpeed = 256 * 1024 * 1024;
    } else if (lluMaxNetSpeed >= 256 * 1024 * 1024 && lluMaxNetSpeed < 512 * 1024 * 1024) {
        m_curMaxLoadSpeed = 512 * 1024 * 1024;
    } else if (lluMaxNetSpeed >= 512 * 1024 * 1024 && lluMaxNetSpeed < 1024 * 1024 * 1024) {
        m_curMaxLoadSpeed = (quint64)1024 * 1024 * 1024;
    } else {
        m_curMaxLoadSpeed = (quint64)2 * 1024 * 1024 * 1024;
    }
    emit this->speedToDynamicMax(m_curMaxLoadSpeed);
}

void NetWorkChart::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}
