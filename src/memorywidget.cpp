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

#include "memorywidget.h"
#include "memorycircle.h"

#include <QDebug>
#include <QObject>

MemoryWidget::MemoryWidget(QWidget *parent)
    : QWidget(parent)
{
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *w = new QWidget;
    qDebug()<<"wodew"<<w->width()<<w->height();
    m_widgetLayout = new QVBoxLayout(w);
    m_widgetLayout->setContentsMargins(0, 0, 0, 0);
    m_widgetLayout->setSpacing(0);

    m_title = new QLabel(tr("Memory"));
    m_title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);  //Qt::AlignLeft | Qt::AlignVCenter
    m_title->setStyleSheet("background:transparent;font-size:24px;color:palette(windowText)");
    /*QFont font = m_title->font();
    font.setPointSize(24);
    font.setWeight(QFont::Light);
    m_title->setFont(font);*/

    m_memoryCircle = new MemoryCircle;
    connect(m_memoryCircle, SIGNAL(rebackMemoryInfo(QString,double)), this, SIGNAL(rebackMemoryInfo(QString,double)));

    m_widgetLayout->addWidget(m_title);
    //m_widgetLayout->addSpacing(27);
    m_widgetLayout->addWidget(m_memoryCircle);

    mainLayout->addWidget(w, 0, Qt::AlignTop);  //Qt::AlignCenter
}

MemoryWidget::~MemoryWidget()
{
    delete m_title;
    delete m_memoryCircle;
    QLayoutItem *child;
    while ((child = m_widgetLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    delete mainLayout;
}

void MemoryWidget::onUpdateMemoryStatus()
{
    m_memoryCircle->onUpdateMemoryStatus();
}
