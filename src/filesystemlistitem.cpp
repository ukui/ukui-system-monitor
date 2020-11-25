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

#include "filesystemlistitem.h"
#include <QCollator>
#include <QDebug>
#include <QLocale>
#include <QPainterPath>
#include "util.h"


#include <QStyleOptionProgressBar>
#include <QProgressBar>
#include <QApplication>

FileSystemListItem::FileSystemListItem(FileSystemData *info)
    :fontSettings(nullptr)
{
    m_data = info;
    iconSize = 20;
    padding = 14;
    textPadding = 5;
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd))
    {
        qtSettings = new QGSettings(idd);
    }

    const QByteArray id(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(id))
    {
        fontSettings = new QGSettings(id);
    }

    initFontSize();
}

FileSystemListItem::~FileSystemListItem()
{
    if(qtSettings)
    {
        delete qtSettings;
    }

    if(fontSettings)
    {
        delete fontSettings;
    }
}

void FileSystemListItem::initThemeMode()
{
    if (!qtSettings) {
        return;
    }
    //监听主题改变
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key)
    {

        if (key == "styleName")
        {
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            qDebug() <<" Current theme mode change to: "<<currentThemeMode<<endl;
        }
    });
    currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
}

void FileSystemListItem::initFontSize()
{
    if (!fontSettings) {
        fontSize = DEFAULT_FONT_SIZE;
        return;
    }

    connect(fontSettings, &QGSettings::changed, this, [=](QString key)
    {
        if("systemFont" == key || "systemFontSize" == key)
        {
            fontSize = fontSettings->get(FONT_SIZE).toString().toFloat();
        }
    });
    fontSize = fontSettings->get(FONT_SIZE).toString().toFloat();
}

bool FileSystemListItem::isSameItem(FileSystemListItem *item)
{
    return m_data->deviceName() == ((static_cast<FileSystemListItem*>(item)))->m_data->deviceName();
}

void FileSystemListItem::drawBackground(QRect rect, QPainter *painter, int index, bool isSelect, QString currentTheme)
{
    QPainterPath path;
    path.addRect(QRectF(rect));

    if (isSelect) {
        painter->setOpacity(0.08);
        painter->fillPath(path, QColor("palette(windowText)"));
    }
    else {
        painter->setOpacity(0.08);
        if(currentTheme == "ukui-light" || currentTheme == "ukui-default" || currentTheme == "ukui-white")
        {
            painter->fillPath(path, QColor("#ffffff"));
        }

        if(currentTheme == "ukui-dark" || currentTheme == "ukui-black")
        {
            painter->fillPath(path, QColor("#000000"));
        }
//        if (index % 2 == 0) {
//            painter->fillPath(path, QColor("#ffffff"));
//        } else {
//            painter->fillPath(path, QColor("#e9eef0"));
//        }
    }
}

void FileSystemListItem::drawForeground(QRect rect, QPainter *painter, int column, int, bool isSelect, bool isSeparator)
{
    setFontSize(*painter, fontSize);
    painter->setOpacity(0.85);
    //painter->setPen(QPen(QColor("#000000")));
    if (column == 0) {
        painter->drawPixmap(QRect(rect.x() + padding, rect.y() + (rect.height() - iconSize) / 2, iconSize, iconSize), QPixmap(":/img/drive-harddisk-system.png"));
        int nameMaxWidth = rect.width() - iconSize - padding * 3;
        QFont font = painter->font();
        QFontMetrics fm(font);
        QString deviceName = fm.elidedText(m_data->deviceName(), Qt::ElideRight, nameMaxWidth);//Qt::ElideMiddle
        painter->drawText(QRect(rect.x() + iconSize + padding * 2, rect.y(), nameMaxWidth, rect.height()), Qt::AlignCenter, deviceName);
        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
    else if (column == 1) {
        if (!m_data->mountDir().isEmpty()) {
            int maxWidth = rect.width();
            QFont font = painter->font();
            QFontMetrics fm(font);
            QString mountDir = fm.elidedText(m_data->mountDir(), Qt::ElideMiddle, maxWidth);
            painter->drawText(QRect(rect.x(), rect.y(), rect.width() - textPadding, rect.height()), Qt::AlignCenter, mountDir);
        }
        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
    else if (column == 2) {
        if (!m_data->diskType().isEmpty()) {
            int maxWidth = rect.width();
            QFont font = painter->font();
            QFontMetrics fm(font);
            QString diskType = fm.elidedText(m_data->diskType(), Qt::ElideRight, maxWidth);
            painter->drawText(QRect(rect.x(), rect.y(), rect.width() - textPadding, rect.height()), Qt::AlignCenter, diskType);
        }
        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
    else if (column == 3) {
        if (!m_data->totalCapacity().isEmpty()) {
            int maxWidth = rect.width();
            QFont font = painter->font();
            QFontMetrics fm(font);
            QString tCapacity = fm.elidedText(m_data->totalCapacity(), Qt::ElideRight, maxWidth);
            painter->drawText(QRect(rect.x(), rect.y(), rect.width() - textPadding, rect.height()), Qt::AlignCenter, tCapacity);
        }
        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
    else if (column == 4) {
        if (!m_data->freeCapacity().isEmpty()) {
            int maxWidth = rect.width();
            QFont font = painter->font();
            QFontMetrics fm(font);
            QString fCapacity = fm.elidedText(m_data->freeCapacity(), Qt::ElideRight, maxWidth);
            painter->drawText(QRect(rect.x(), rect.y(), rect.width() - textPadding, rect.height()), Qt::AlignCenter, fCapacity);
        }
        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
    else if (column == 5) {
        if (!m_data->availCapacity().isEmpty()) {
            int maxWidth = rect.width();
            QFont font = painter->font();
            QFontMetrics fm(font);
            QString aCapacity = fm.elidedText(m_data->availCapacity(), Qt::ElideRight, maxWidth);
            painter->drawText(QRect(rect.x(), rect.y(), rect.width() - textPadding, rect.height()), Qt::AlignCenter, aCapacity);
        }
        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
    else if (column == 6) {
        int maxWidth = rect.width();
        int leftPadding = 10;
        int topPadding = 5;
        int progressWidth = 100;
        int progressHeight = rect.height() - 2 * topPadding;
        int textMaxWidth = rect.width() - progressWidth - 2 * leftPadding;
        if (!m_data->usedCapactiy().isEmpty()) {
            QFont font = painter->font();
            QFontMetrics fm(font);
            QString uCapacity = fm.elidedText(m_data->usedCapactiy(), Qt::ElideRight, maxWidth);
//            painter->drawText(QRect(rect.x() + textPadding, rect.y(), textMaxWidth - textPadding, rect.height()), Qt::AlignCenter, uCapacity);
            painter->drawText(QRect(rect.x() , rect.y(), rect.width(), rect.height()), Qt::AlignCenter, uCapacity);
        }
//这部分代码为原来绘制未占用区域的灰色部分。//////////////
//        QPainterPath bgPath;
////        bgPath.addRect(QRectF(rect.x() + textMaxWidth + leftPadding, rect.y() + topPadding, progressWidth, progressHeight));
//        bgPath.addRect(QRectF(rect.x() + textMaxWidth + leftPadding, rect.y() + topPadding, progressWidth, 2));
//        painter->fillPath(bgPath, QColor("#C4BDBD"));
////        painter->fillPath(bgPath,QColor("palette(Base)"));
/////////////////////////////////////////////////
        QPainterPath fillPath;
//        fillPath.addRect(QRectF(rect.x() + textMaxWidth + leftPadding, rect.y() + topPadding, m_data->usedPercentage(), progressHeight));
        fillPath.addRect(QRectF(rect.x() + rect.width() - m_data->usedPercentage(), rect.y() + rect.height() -2, m_data->usedPercentage(), 2));
        painter->setOpacity(0.5);
        if (m_data->usedPercentage() < 75)
            painter->fillPath(fillPath, QColor("#0288d1"));
        else
            painter->fillPath(fillPath, QColor("#f8b551"));
//        painter->setOpacity(1);
//        painter->drawText(QRect(rect.x() + textMaxWidth + leftPadding, rect.y() + topPadding, progressWidth, progressHeight), Qt::AlignCenter, nullptr);

        /*
        QStyleOptionProgressBar progressBarStyle;//progressBarStyle.initFrom(this);
        progressBarStyle.rect = QRect(rect.x() + 60, rect.y() + topPadding, pWidth, pHeight);
        progressBarStyle.minimum = 0;
        progressBarStyle.maximum = 100;
        progressBarStyle.textAlignment = Qt::AlignCenter;
        progressBarStyle.progress = m_data->usedPercentage();
        progressBarStyle.text = QString("%1%").arg(m_data->usedPercentage());
        progressBarStyle.textVisible = true;
        QProgressBar progressBar;
        progressBar.setStyleSheet("QProgressBar{border: none;text-align: center;background:#eeeeee;}QProgressBar::chunk {background:#0288d1;}");
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarStyle, painter, &progressBar);//绘制进度条
        */

        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
}

QString FileSystemListItem::getDeviceName() const
{
    return m_data->deviceName();
}

QString FileSystemListItem::getDirectory() const
{
    return m_data->mountDir();
}
