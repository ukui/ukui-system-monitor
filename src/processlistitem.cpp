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

#include "processlistitem.h"
#include <QCollator>
#include <QDebug>
#include <QLocale>
#include "util.h"
#include "../shell/macro.h"
static int number = 0;

ProcessListItem::ProcessListItem(ProcData info)
{

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

    m_data = info;
    iconSize = 20;
    padding = 14;
    textPadding = 10;
    //initThemeMode();
}

ProcessListItem::~ProcessListItem()
{
    if (qtSettings) {
        delete qtSettings;
    }
    if(fontSettings)
    {
        delete fontSettings;
    }
}

void ProcessListItem::initThemeMode()
{
    //监听主题改变
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key)
    {

        if (key == "styleName")
        {
            auto style = qtSettings->get(key).toString();
            qApp->setStyle(new InternalStyle(style));
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            qDebug()<<"监听主题改变-------------------->"<<currentThemeMode<<endl;
            qApp->setStyle(new InternalStyle(currentThemeMode));
            //repaint();
        }
    });
    currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
}

void ProcessListItem::initFontSize()
{
    connect(fontSettings,&QGSettings::changed,this,[=](QString key)
    {
        if("systemFont" == key || "systemFontSize" == key)
        {
            fontSize = fontSettings->get(FONT_SIZE).toInt();
        }
    });
    fontSize = fontSettings->get(FONT_SIZE).toInt();
}

bool ProcessListItem::isSameItem(ProcessListItem *item)
{
    return m_data.pid == ((static_cast<ProcessListItem*>(item)))->m_data.pid;
}

void ProcessListItem::drawCellBackground(QRect rect, QPainter *painter, int level)
{
    QPainterPath path;
    path.addRect(QRectF(rect.x(), rect.y(), rect.width(), rect.height()));
    painter->setOpacity(0.5);//0.1
    if (level == 0) {
        //painter->fillPath(path, QColor("#131414"));
        //painter->setOpacity(0.08);
    }
    else if (level == 1) {
//        painter->fillPath(path, QColor("#131414"));
//        painter->setOpacity(0.08);
    }
    else {
//        painter->fillPath(path, QColor("#131414"));
//        painter->setOpacity(0.08);
    }
}

void ProcessListItem::drawBackground(QRect rect, QPainter *painter, int index, bool isSelect ,QString currentThemeMode)
{
    QPainterPath path;
    path.addRect(QRectF(rect));

    if (isSelect) {
////        painter->setOpacity(0.08);
//        if(currentThemeMode == "ukui-white")
//        {
//            painter->setOpacity(0.08);
//            painter->fillPath(path, QColor("#000000"));
//        }

//        if(currentThemeMode == "ukui-black")
//        {
//            painter->setOpacity(0.08);
//            painter->fillPath(path, QColor("#ffffff"));
//        }
        painter->setOpacity(0.08);
        painter->fillPath(path,QColor("palette(windowText)"));

    }
    else {
        painter->setOpacity(0.08);
        if(currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
        {
            painter->fillPath(path, QColor("#ffffff"));
        }

        if(currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black")
        {
            painter->fillPath(path,QColor("#131414"));
        }

//        if (index % 2 == 0) {
//            painter->fillPath(path, QColor("#ffffff"));
//        } else {
//            painter->fillPath(path, QColor("#e9eef0"));
//        }
    }
}

void ProcessListItem::drawForeground(QRect rect, QPainter *painter, int column, int, bool isSelect, bool isSeparator)
{
    setFontSize(*painter, fontSize);
    painter->setOpacity(0.85);
    //painter->setPen(QPen(QColor(QPalette::Base)));
    if (column == 0) {
        painter->drawPixmap(QRect(rect.x() + padding, rect.y() + (rect.height() - iconSize) / 2, iconSize, iconSize), m_data.iconPixmap);
        QString name = m_data.processName;
        if (m_data.m_status == tr("Stopped")) {//已停止
            painter->setPen(QPen(QColor("#fca71d")));
            name = QString("(%1) %2").arg(tr("Suspend")).arg(m_data.processName);
        }
        else if (m_data.m_status == tr("Zombie")) {//僵死
            painter->setPen(QPen(QColor("#808080")));
            name = QString("(%1) %2").arg(tr("No response")).arg(m_data.processName);
        }
        else if (m_data.m_status == tr("Uninterruptible")) {//不可中断
            painter->setPen(QPen(QColor("#ff6a6a")));
            name = QString("(%1) %2").arg(tr("Uninterruptible")).arg(m_data.processName);
        }
        else {//Sleeping 睡眠中  Running 运行中
        }
        int nameMaxWidth = rect.width() - iconSize - padding * 3;
        QFont font = painter->font();
        QFontMetrics fm(font);
        QString procName = fm.elidedText(name, Qt::ElideRight, nameMaxWidth);
        painter->drawText(QRect(rect.x() + iconSize + padding * 2, rect.y(), nameMaxWidth, rect.height()), Qt::AlignLeft | Qt::AlignVCenter, procName);
        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
    else if (column == 1) {
        if (!m_data.user.isEmpty()) {
            QString name = m_data.user;
            int userMaxWidth = rect.width()  - padding * PADDING ;
            QFont font = painter->font();
            QFontMetrics fm(font);
            QString userName = fm.elidedText(name, Qt::ElideRight, userMaxWidth);
            painter->drawText(QRect(rect.x(), rect.y(), rect.width() - textPadding, rect.height()), Qt::AlignLeft | Qt::AlignVCenter, userName);
        }
        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
    else if (column == 2) {
//        if (!m_data.m_status.isEmpty()) {
            painter->drawText(QRect(rect.x(), rect.y(), rect.width() - textPadding, rect.height()), Qt::AlignRight | Qt::AlignVCenter, m_data.m_diskio);
//        }
        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
    else if (column == 3) {
        if (m_data.cpu < 10) {
            //this->drawCellBackground(QRect(rect.x(), rect.y(), rect.width(), rect.height()), painter, 0);
        }
        else if (m_data.cpu < 33) {
            //this->drawCellBackground(QRect(rect.x(), rect.y(), rect.width(), rect.height()), painter, 1);
        }
        else {
            //this->drawCellBackground(QRect(rect.x(), rect.y(), rect.width(), rect.height()), painter, 2);
        }
        painter->drawText(QRect(rect.x(), rect.y(), rect.width() - textPadding, rect.height()), Qt::AlignRight | Qt::AlignVCenter, QString("%1%").arg(m_data.cpu));
        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
    else if (column == 4) {
        painter->drawText(QRect(rect.x(), rect.y(), rect.width() - padding, rect.height()), Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(m_data.pid));
        if (!isSeparator) {
            painter->setOpacity(0.8);
//            QPainterPath separatorPath;
//            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
//            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
    else if (column == 5) {
        int flownetMaxWidth = rect.width();
        QFont font = painter->font();
        QFontMetrics fm(font);
        QString flownet = fm.elidedText(m_data.m_flownet, Qt::ElideRight, flownetMaxWidth);
        painter->drawText(QRect(rect.x(), rect.y(), flownetMaxWidth, rect.height()), Qt::AlignCenter | Qt::AlignVCenter, flownet);
        if (!isSeparator) {
            painter->setOpacity(0.8);
//            QPainterPath separatorPath;
//            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
//            painter->fillPath(separatorPath, QColor("#CC00FF"));  //e0e0e0
        }
    }
    else if (column == 6) {
        if (m_data.m_memory > 0) {
            painter->setOpacity(1);
            char *memory = g_format_size_full(m_data.m_memory, G_FORMAT_SIZE_IEC_UNITS);
            QString Memory = QString(memory);
            if (m_data.m_memory < 102400000) {//<100M
                //this->drawCellBackground(QRect(rect.x(), rect.y(), rect.width(), rect.height()), painter, 0);
            }
            else if (m_data.m_memory < 1024000000) {//1G
                //this->drawCellBackground(QRect(rect.x(), rect.y(), rect.width(), rect.height()), painter, 1);
            }
            else {
                //this->drawCellBackground(QRect(rect.x(), rect.y(), rect.width(), rect.height()), painter, 2);
            }
            painter->drawText(QRect(rect.x(), rect.y(), rect.width() - textPadding, rect.height()), Qt::AlignRight | Qt::AlignVCenter, Memory);
            g_free(memory);
        }
        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
    else if (column == 7) {
        painter->drawText(QRect(rect.x(), rect.y(), rect.width() - textPadding, rect.height()), Qt::AlignLeft | Qt::AlignVCenter, getNiceLevel(m_data.m_nice));
        if (!isSeparator) {
            painter->setOpacity(0.8);
            QPainterPath separatorPath;
            separatorPath.addRect(QRectF(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()));
            painter->fillPath(separatorPath, QColor("#e0e0e0"));
        }
    }
}

bool ProcessListItem::doSearch(const ProcessListItem *item, QString text)
{
    const ProcessListItem *procItem = static_cast<const ProcessListItem*>(item);
    QString content = text.toLower();
    return procItem->getProcessName().toLower().contains(content) || QString::number(procItem->getPid()).contains(content) || procItem->getDisplayName().toLower().contains(content) || procItem->getUser().toLower().contains(content);
}

bool ProcessListItem::sortByDiskIo(const ProcessListItem *item1, const ProcessListItem *item2, bool descendingSort)
{

    QString diskIo1 = (static_cast<const ProcessListItem*>(item1))->getDiskIo();
    QString  diskIo2 = (static_cast<const ProcessListItem*>(item2))->getDiskIo();
    bool isSort;

    if (diskIo1 == diskIo2)
    {
        double cpu1 = static_cast<const ProcessListItem*>(item1)->getCPU();
        double cpu2 = (static_cast<const ProcessListItem*>(item2))->getCPU();
        isSort = cpu1 > cpu2;
    }
    else
    {
        QCollator qco(QLocale::system());
        int result = qco.compare(diskIo1, diskIo2);
        isSort = result < 0;
    }

    return descendingSort ? isSort : !isSort;


}

bool ProcessListItem::sortByFlowNet(const ProcessListItem *item1, const ProcessListItem *item2, bool descendingSort)
{
    QString netFlow1 = (static_cast<const ProcessListItem*>(item1))->getFlowNet();
    QString netFlow2 = (static_cast<const ProcessListItem*>(item2))->getFlowNet();

    if (netFlow1 == netFlow2)
    {
        double cpu1 = static_cast<const ProcessListItem*>(item1)->getCPU();
        double cpu2 = (static_cast<const ProcessListItem*>(item2))->getCPU();
        isSort = cpu1 > cpu2;
    }
    else
    {
        QCollator qco(QLocale::system());
        int result = qco.compare(netFlow1, netFlow2);
        isSort = result < 0;
    }

    bool isSort;


    return descendingSort ? isSort : !isSort;
}

bool ProcessListItem::sortByName(const ProcessListItem *item1, const ProcessListItem *item2, bool descendingSort)
{
    QString name1 = (static_cast<const ProcessListItem*>(item1))->getDisplayName();
    QString name2 = (static_cast<const ProcessListItem*>(item2))->getDisplayName();
    bool isSort;

    if (name1 == name2) {
        double cpu1 = static_cast<const ProcessListItem*>(item1)->getCPU();
        double cpu2 = (static_cast<const ProcessListItem*>(item2))->getCPU();
        isSort = cpu1 > cpu2;
    }
    else {
        QCollator qco(QLocale::system());
        int result = qco.compare(name1, name2);
        isSort = result < 0;
    }

    return descendingSort ? isSort : !isSort;
}

bool ProcessListItem::sortByUser(const ProcessListItem *item1, const ProcessListItem *item2, bool descendingSort)
{
    QString user1 = (static_cast<const ProcessListItem*>(item1))->getUser();
    QString user2 = (static_cast<const ProcessListItem*>(item2))->getUser();
    bool isSort;
    if (user1 == user2) {
        double cpu1 = static_cast<const ProcessListItem*>(item1)->getCPU();
        double cpu2 = (static_cast<const ProcessListItem*>(item2))->getCPU();
        isSort = cpu1 > cpu2;
    }
    else {
        QCollator qco(QLocale::system());
        int result = qco.compare(user1, user2);
        isSort = result < 0;
    }

    return descendingSort ? isSort : !isSort;
}

bool ProcessListItem::sortByStatus(const ProcessListItem *item1, const ProcessListItem *item2, bool descendingSort)
{
    QString status1 = (static_cast<const ProcessListItem*>(item1))->getStatus();
    QString status2 = (static_cast<const ProcessListItem*>(item2))->getStatus();
    bool isSort;
    if (status1 == status2) {
        double cpu1 = static_cast<const ProcessListItem*>(item1)->getCPU();
        double cpu2 = (static_cast<const ProcessListItem*>(item2))->getCPU();
        isSort = cpu1 > cpu2;
    }
    else {
        QCollator qco(QLocale::system());
        int result = qco.compare(status1, status2);
        isSort = result < 0;
    }

    return descendingSort ? isSort : !isSort;
}


bool ProcessListItem::sortByCPU(const ProcessListItem *item1, const ProcessListItem *item2, bool descendingSort)
{
    double cpu1 = (static_cast<const ProcessListItem*>(item1))->getCPU();
    double cpu2 = (static_cast<const ProcessListItem*>(item2))->getCPU();
    bool isSort;
    if (cpu1 == cpu2) {
        long memory1 = static_cast<const ProcessListItem*>(item1)->getMemory();
        long memory2 = (static_cast<const ProcessListItem*>(item2))->getMemory();
        isSort = memory1 > memory2;
    }
    else {
        isSort = cpu1 > cpu2;
    }

    return descendingSort ? isSort : !isSort;
}

bool ProcessListItem::sortByPid(const ProcessListItem *item1, const ProcessListItem *item2, bool descendingSort)
{
    bool isSort = (static_cast<const ProcessListItem*>(item1))->getPid() > (static_cast<const ProcessListItem*>(item2))->getPid();

    return descendingSort ? isSort : !isSort;
}

//bool ProcessListItem::sortByCommand(const ProcessListItem *item1, const ProcessListItem *item2, bool descendingSort)
//{
//    QString command1 = (static_cast<const ProcessListItem*>(item1))->getCommandLine();
//    QString command2 = (static_cast<const ProcessListItem*>(item2))->getCommandLine();
//    bool isSort;
//    if (command1 == command2) {
//        double cpu1 = static_cast<const ProcessListItem*>(item1)->getCPU();
//        double cpu2 = (static_cast<const ProcessListItem*>(item2))->getCPU();
//        isSort = cpu1 > cpu2;
//    }
//    else {
//        QCollator qco(QLocale::system());
//        int result = qco.compare(command1, command2);
//        isSort = result < 0;
//    }

//    return descendingSort ? isSort : !isSort;
//}

bool ProcessListItem::sortByMemory(const ProcessListItem *item1, const ProcessListItem *item2, bool descendingSort)
{
    long memory1 = (static_cast<const ProcessListItem*>(item1))->getMemory();
    long memory2 = (static_cast<const ProcessListItem*>(item2))->getMemory();
    bool isSort;
    if (memory1 == memory2) {
        double cpu1 = static_cast<const ProcessListItem*>(item1)->getCPU();
        double cpu2 = (static_cast<const ProcessListItem*>(item2))->getCPU();
        isSort = cpu1 > cpu2;
    }
    else {
        isSort = memory1 > memory2;
    }

    return descendingSort ? isSort : !isSort;
}

bool ProcessListItem::sortByPriority(const ProcessListItem *item1, const ProcessListItem *item2, bool descendingSort)
{
    long nice1 = (static_cast<const ProcessListItem*>(item1))->getNice();
    long nice2 = (static_cast<const ProcessListItem*>(item2))->getNice();
    bool isSort;
    if (nice1 == nice2) {
        double cpu1 = static_cast<const ProcessListItem*>(item1)->getCPU();
        double cpu2 = (static_cast<const ProcessListItem*>(item2))->getCPU();
        isSort = cpu1 > cpu2;
    }
    else {
        isSort = nice1 > nice2;
    }

    return descendingSort ? isSort : !isSort;
}

QString ProcessListItem::getProcessName() const
{
    return m_data.processName;
}

QString ProcessListItem::getDisplayName() const
{
    return m_data.displayName;
}

QString ProcessListItem::getUser() const
{
    return m_data.user;
}

QString ProcessListItem::getStatus() const
{
    return m_data.m_status;
}

double ProcessListItem::getCPU() const
{
    return m_data.cpu;
}

pid_t ProcessListItem::getPid() const
{
    return m_data.pid;
}

long ProcessListItem::getMemory() const
{
    return m_data.m_memory;
}

long ProcessListItem::getNice() const
{
    return m_data.m_nice;
}

QString ProcessListItem::getFlowNet() const
{
    return m_data.m_flownet;
}

QString ProcessListItem::getDiskIo() const
{
    return m_data.m_diskio;
}

//QString ProcessListItem::getCommandLine() const
//{
//    return m_data.commandLine;
//}


