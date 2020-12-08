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

#ifndef MEMORYWIDGET_H
#define MEMORYWIDGET_H

#include <QWidget>
#include <QImage>
#include <QLabel>
#include <QVBoxLayout>

class MemoryCircle;

class MemoryWidget : public QWidget
{
    Q_OBJECT

public:
    MemoryWidget(QWidget *parent = 0);
    ~MemoryWidget();

public slots:
    void onUpdateMemoryStatus();

signals:
    void rebackMemoryInfo(const QString &info, double percent);

private:
    QLabel *m_title = nullptr;
    MemoryCircle *m_memoryCircle = nullptr;
    QVBoxLayout *m_widgetLayout = nullptr;
    QHBoxLayout *mainLayout = nullptr;
};

#endif // MEMORYWIDGET_H
