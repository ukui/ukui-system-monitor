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

#include "util.h"

#include <QApplication>
#include <QIcon>
#include <QDirIterator>
#include <QDebug>

#include <glibtop/procstate.h>
#include <fstream>
#include <sstream>
#include <QSvgRenderer>
const QPixmap loadSvg(const QString &fileName, const int size)
{
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    return pixmap;
}

QPixmap drawSymbolicColoredPixmap(const QPixmap &source)
{
    QColor gray(128,128,128);
    QColor standard (31,32,34);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red()-gray.red())<20 && qAbs(color.green()-gray.green())<20 && qAbs(color.blue()-gray.blue())<20) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                }
                else if(qAbs(color.red()-standard.red())<20 && qAbs(color.green()-standard.green())<20 && qAbs(color.blue()-standard.blue())<20)
                {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                }
                else
                {
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap drawSymbolicBlackColoredPixmap(const QPixmap &source)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red())>=200 && qAbs(color.green())>=200 && qAbs(color.blue())>=200) {
                    color.setRed(56);
                    color.setGreen(56);
                    color.setBlue(56);
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

std::string make_string(char *c_str)
{
    if (!c_str) {
        return string();
    }
    string s(c_str);
    g_free(c_str);
    return s;
}

static inline unsigned divide(unsigned *q, unsigned *r, unsigned d)
{
    *q = *r / d;
    *r = *r % d;
    return *q != 0;
}

QString getElidedText(QFont font, QString str, int MaxWidth)
{
    if (str.isEmpty())
    {
        return "";
    }

    QFontMetrics fontWidth(font);

    //计算字符串宽度
    //calculat the width of the string
    int width = fontWidth.width(str);

    //当字符串宽度大于最大宽度时进行转换
    //Convert when string width is greater than maximum width
    if (width >= MaxWidth)
    {
        //右部显示省略号
        //show by ellipsis in right
        str = fontWidth.elidedText(str, Qt::ElideRight, MaxWidth);
    }
    //返回处理后的字符串
    //return the string that is been handled
    return str;
}

QString formatDurationForDisplay(unsigned centiseconds)
{
    unsigned weeks = 0, days = 0, hours = 0, minutes = 0, seconds = 0;

    (void)(divide(&seconds, &centiseconds, 100)
           && divide(&minutes, &seconds, 60)
           && divide(&hours, &minutes, 60)
           && divide(&days, &hours, 24)
           && divide(&weeks, &days, 7));

    QString formatTime;
    gchar *duration = NULL;

    if (weeks) {
        duration = g_strdup_printf("%uw%ud", weeks, days);
        formatTime = QString(QLatin1String(duration));
        if (duration) {
            g_free(duration);
            duration = NULL;
        }
        return formatTime;
    }

    if (days) {
        duration = g_strdup_printf("%ud%02uh", days, hours);
        formatTime = QString(QLatin1String(duration));
        if (duration) {
            g_free(duration);
            duration = NULL;
        }
        return formatTime;
    }

    if (hours) {
        duration = g_strdup_printf("%u:%02u:%02u", hours, minutes, seconds);
        formatTime = QString(QLatin1String(duration));
        if (duration) {
            g_free(duration);
            duration = NULL;
        }
        return formatTime;
    }

    duration = g_strdup_printf("%u:%02u.%02u", minutes, seconds, centiseconds);
    formatTime = QString(QLatin1String(duration));
    if (duration) {
        g_free(duration);
        duration = NULL;
    }
    return formatTime;
}

std::string getDesktopFileAccordProcName(QString procName, QString cmdline)
{
    QDirIterator dir("/etc/xdg/autostart", QDirIterator::Subdirectories);
    std::string desktopFile;
    QString procname = procName.toLower();
    procname.replace("_", "-");
    QString processFilename = procname + ".desktop";

    while(dir.hasNext()) {
        if (dir.fileInfo().suffix() == "desktop") {
            if (dir.fileName().toLower().contains(processFilename)) {
                desktopFile = dir.filePath().toStdString();
//                std::cout<<"---desktopFile---"<<desktopFile<<std::endl;
                break;
            }
        }
        dir.next();
    }
    return desktopFile;
}

std::string getDesktopFileAccordProcNameApp(QString procName, QString cmdline)
{
    QDirIterator dir("/usr/share/applications", QDirIterator::Subdirectories);
    std::string desktopFile;
    QString procname = procName.toLower();
    procname.replace("_", "-");
    QString processFilename = procname + ".desktop";

    while(dir.hasNext()) {
        if (dir.fileInfo().suffix() == "desktop") {
            if (dir.fileName().toLower().contains(processFilename)) {
                desktopFile = dir.filePath().toStdString();
                break;
            }
        }
        dir.next();
    }
    return desktopFile;
}

QPixmap getAppIconFromDesktopFile(std::string desktopFile, int iconSize)
{
    std::ifstream in;
    in.open(desktopFile);
    QIcon defaultExecutableIcon = QIcon::fromTheme("application-x-executable");//gnome-mine-application-x-executable
    if (defaultExecutableIcon.isNull()) {
        defaultExecutableIcon = QIcon("/usr/share/icons/ukui-icon-theme-default/48x48/mimetypes/application-x-executable.png");
        if (defaultExecutableIcon.isNull())
            defaultExecutableIcon = QIcon(":/res/autostart-default.png");
    }

    QIcon icon;
    QString iconName;
    while(!in.eof()) {
        std::string line;
        std::getline(in,line);
        iconName = QString::fromStdString(line);

        if (iconName.startsWith("Icon=")) {
            iconName.remove(0,5);
        }
        else {
            continue;
        }

        if (iconName.contains("/")) {
            icon = QIcon(iconName);
        }
        else {
            icon = QIcon::fromTheme(iconName, defaultExecutableIcon);
            break;
        }
    }
    in.close();

    qreal devicePixelRatio = qApp->devicePixelRatio();

    QPixmap pixmap = icon.pixmap(iconSize * devicePixelRatio, iconSize * devicePixelRatio);
    pixmap.setDevicePixelRatio(devicePixelRatio);

    return pixmap;
}

QString getDisplayNameAccordProcName(QString procName, std::string desktopFile)
{
    if (desktopFile.size() == 0) {
        return procName;
    }
    std::ifstream in;
    in.open(desktopFile);
    QString displayName = procName;
    while(!in.eof()) {
        std::string line;
        std::getline(in,line);
        QString lineContent = QString::fromStdString(line);
        QString localNameFlag = QString("Name[%1]=").arg(QLocale::system().name());
        QString nameFlag = "Name=";
        QString genericNameFlag = QString("GenericName[%1]=").arg(QLocale::system().name());

        if (lineContent.startsWith(localNameFlag)) {
            displayName = lineContent.remove(0, localNameFlag.size());
            break;
        }
        else if (lineContent.startsWith(genericNameFlag)) {
            displayName = lineContent.remove(0, genericNameFlag.size());
            break;
        }
        else if (lineContent.startsWith(nameFlag)) {
            displayName = lineContent.remove(0, nameFlag.size());
            continue;
        }
        else {
            continue;
        }
    }
    in.close();

    return displayName;
}

QString formatProcessState(guint state)
{
    QString status;
    switch (state)
    {
        case GLIBTOP_PROCESS_RUNNING:
            status = QString(QObject::tr("Running"));//运行中
            break;

        case GLIBTOP_PROCESS_STOPPED:
            status = QString(QObject::tr("Stopped"));//已停止
            break;

        case GLIBTOP_PROCESS_ZOMBIE:
            status = QString(QObject::tr("Zombie"));//僵死
            break;

        case GLIBTOP_PROCESS_UNINTERRUPTIBLE:
            status = QString(QObject::tr("Uninterruptible"));//不可中断
            break;

        default:
            status = QString(QObject::tr("Sleeping"));//睡眠中
            break;
    }

    return status;
}

QString getNiceLevel(int nice)
{
    if (nice < -7)
        return QObject::tr("Very High");
    else if (nice < -2)
        return QObject::tr("High");
    else if (nice < 3)
        return QObject::tr("Normal");
    else if (nice < 7)
        return QObject::tr("Low");
    else
        return QObject::tr("Very Low");
}

QString getNiceLevelWithPriority(int nice)
{
    if (nice < -7)
        return QObject::tr("Very High Priority");
    else if (nice < -2)
        return QObject::tr("High Priority");
    else if (nice < 3)
        return QObject::tr("Normal Priority");
    else if (nice < 7)
        return QObject::tr("Low Priority");
    else
        return QObject::tr("Very Low Priority");
}

void setFontSize(QPainter &painter, int textSize)
{
    QFont font = painter.font() ;
    font.setPixelSize(textSize);
//    font.setPointSize(textSize);
    painter.setFont(font);
}

QString formatUnitSize(double v, const char** orders, int nb_orders)
{
    int order = 0;
    while (v >= 1024 && order + 1 < nb_orders) {
        order++;
        v  = v/1024;
    }
    char buffer1[30];
    snprintf(buffer1, sizeof(buffer1), "%.1lf %s", v, orders[order]);

    return QString(buffer1);
}

QString formatByteCount(double v)
{
    static const char* orders[] = { "B", "KB", "MB", "GB", "TB" };
    return formatUnitSize(v, orders, sizeof(orders)/sizeof(orders[0]));
}

QString getDeviceMountedPointPath(const QString &line)
{
    const QStringList items = line.split(" ");
    if (items.length() > 4) {
        return items.at(1);
    }
    else {
        return "";
    }
}

QString getFileContent(const QString &filePath)
{
    QFile fd(filePath);
    QString fileContent = "";
    if (fd.open(QFile::ReadOnly)) {
        fileContent = QLatin1String(fd.readAll());
        fd.close();
    }
    return fileContent;
}

QSet<QString> getFileContentsLineByLine(const QString &filePath)
{
    QString fileContent = getFileContent(filePath);
    return QSet<QString>::fromList(fileContent.split("\n"));
}
