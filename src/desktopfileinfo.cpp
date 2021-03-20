/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
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

#include "desktopfileinfo.h"

#include <QDirIterator>
#include <QSettings>
#include <QTextCodec>
#include <QDebug>

Q_GLOBAL_STATIC(DesktopFileInfo, theInstance)
DesktopFileInfo *DesktopFileInfo::instance()
{
    return theInstance();
}

DesktopFileInfo::DesktopFileInfo(QObject *parent)
    : QObject(parent)
{
    readAllDesktopFileInfo();
}
    
DesktopFileInfo::~DesktopFileInfo()
{
    m_mapDesktopInfoList.clear();
}

QString DesktopFileInfo::getDesktopFileNameByExec(QString strExec)
{
    QMap<QString, DTFileInfo>::iterator itDesktopFileInfo = m_mapDesktopInfoList.begin();
    QStringList execParamList = strExec.split(QRegExp("\\s+"));
    for (; itDesktopFileInfo != m_mapDesktopInfoList.end(); itDesktopFileInfo ++) {
        if (itDesktopFileInfo.value().strExec == strExec || itDesktopFileInfo.value().strSimpleExec == strExec) {
            if (itDesktopFileInfo.value().strExecParam.size() <= 1 && execParamList.size() <= 1) {
                return itDesktopFileInfo.key();
            } else if (itDesktopFileInfo.value().strExecParam.size() > 1 
                && execParamList.size() <= itDesktopFileInfo.value().strExecParam.size()) { // 实际进程命令参数长度要比预定的相同或更短
                bool isSameProc = true;
                for (int n = 1; n < itDesktopFileInfo.value().strExecParam.size(); n++) {
                    if (!itDesktopFileInfo.value().strExecParam[n].isEmpty()) {
                        if (itDesktopFileInfo.value().strExecParam[n].at(0) != '%') {   // 必传参数
                            if (execParamList.size() <= n) { // 缺少必传参数，进程不匹配
                                isSameProc = false;
                                break;
                            } else if (execParamList[n] != itDesktopFileInfo.value().strExecParam[n]) {  // 参数不必配，进程不匹配
                                isSameProc = false;
                                break;
                            }
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                }
                if (isSameProc) {
                    return itDesktopFileInfo.key();
                }
            }
        }
    }
    return "";
}

QString DesktopFileInfo::getNameByExec(QString strExec)
{
    QMap<QString, DTFileInfo>::iterator itDesktopFileInfo = m_mapDesktopInfoList.begin();
    QStringList execParamList = strExec.split(QRegExp("\\s+"));
    for (; itDesktopFileInfo != m_mapDesktopInfoList.end(); itDesktopFileInfo ++) {
        if (itDesktopFileInfo.value().strExec == strExec || itDesktopFileInfo.value().strSimpleExec == strExec) {
            if (itDesktopFileInfo.value().strExecParam.size() <= 1 && execParamList.size() <= 1) {
                if (itDesktopFileInfo.value().strName.isEmpty()) {
                    return itDesktopFileInfo.value().strGenericName;
                } else {
                    return itDesktopFileInfo.value().strName;
                }
            } else if (itDesktopFileInfo.value().strExecParam.size() > 1 
                && execParamList.size() <= itDesktopFileInfo.value().strExecParam.size()) { // 实际进程命令参数长度要比预定的相同或更短
                bool isSameProc = true;
                for (int n = 1; n < itDesktopFileInfo.value().strExecParam.size(); n++) {
                    if (!itDesktopFileInfo.value().strExecParam[n].isEmpty()) {
                        if (itDesktopFileInfo.value().strExecParam[n].at(0) != '%') {   // 必传参数
                            if (execParamList.size() <= n) { // 缺少必传参数，进程不匹配
                                isSameProc = false;
                                break;
                            } else if (execParamList[n] != itDesktopFileInfo.value().strExecParam[n]) {  // 参数不必配，进程不匹配
                                isSameProc = false;
                                break;
                            }
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                }
                if (isSameProc) {
                    if (itDesktopFileInfo.value().strName.isEmpty()) {
                        return itDesktopFileInfo.value().strGenericName;
                    } else {
                        return itDesktopFileInfo.value().strName;
                    }
                }
            }
        }
    }
    return "";
}

QString DesktopFileInfo::getIconByExec(QString strExec)
{
    QMap<QString, DTFileInfo>::iterator itDesktopFileInfo = m_mapDesktopInfoList.begin();
    QStringList execParamList = strExec.split(QRegExp("\\s+"));
    for (; itDesktopFileInfo != m_mapDesktopInfoList.end(); itDesktopFileInfo ++) {
        if (itDesktopFileInfo.value().strExec == strExec || itDesktopFileInfo.value().strSimpleExec == strExec) {
            if (itDesktopFileInfo.value().strExecParam.size() <= 1 && execParamList.size() <= 1) {
                return itDesktopFileInfo.value().strIcon;
            } else if (itDesktopFileInfo.value().strExecParam.size() > 1 
                && execParamList.size() <= itDesktopFileInfo.value().strExecParam.size()) { // 实际进程命令参数长度要比预定的相同或更短
                bool isSameProc = true;
                for (int n = 1; n < itDesktopFileInfo.value().strExecParam.size(); n++) {
                    if (!itDesktopFileInfo.value().strExecParam[n].isEmpty()) {
                        if (itDesktopFileInfo.value().strExecParam[n].at(0) != '%') {   // 必传参数
                            if (execParamList.size() <= n) { // 缺少必传参数，进程不匹配
                                isSameProc = false;
                                break;
                            } else if (execParamList[n] != itDesktopFileInfo.value().strExecParam[n]) {  // 参数不必配，进程不匹配
                                isSameProc = false;
                                break;
                            }
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                }
                if (isSameProc) {
                    return itDesktopFileInfo.value().strIcon;
                }
            }
        }
    }
    return "";
}

QString DesktopFileInfo::getNameByDesktopFile(QString strDesktopFileName)
{
    QMap<QString, DTFileInfo>::iterator itDesktopFileInfo = m_mapDesktopInfoList.find(strDesktopFileName);
    if (itDesktopFileInfo != m_mapDesktopInfoList.end()) {
        if (itDesktopFileInfo.value().strName.isEmpty()) {
            return itDesktopFileInfo.value().strGenericName;
        } else {
            return itDesktopFileInfo.value().strName;
        }
    }
    return "";
}

QString DesktopFileInfo::getIconByDesktopFile(QString strDesktopFileName)
{
    QMap<QString, DTFileInfo>::iterator itDesktopFileInfo = m_mapDesktopInfoList.find(strDesktopFileName);
    if (itDesktopFileInfo != m_mapDesktopInfoList.end()) {
        return itDesktopFileInfo.value().strIcon;
    }
    return "";
}

void DesktopFileInfo::readAllDesktopFileInfo()
{
    m_mapDesktopInfoList.clear();
    // read all autostart desktop file
    QDirIterator dirXdgAutostart("/etc/xdg/autostart", QDirIterator::Subdirectories);
    while(dirXdgAutostart.hasNext()) {
        if (dirXdgAutostart.fileInfo().suffix() == "desktop") {
            QSettings* desktopFile = new QSettings(dirXdgAutostart.filePath(), QSettings::IniFormat);
            if (desktopFile) {
                DTFileInfo dtFileInfo;
                desktopFile->setIniCodec("utf-8");
                dtFileInfo.strExec = desktopFile->value(QString("Desktop Entry/Exec")).toString();
                dtFileInfo.strExecParam = dtFileInfo.strExec.split(QRegExp("\\s+"));
                if (dtFileInfo.strExecParam.size() > 0) {
                    QStringList simpleExecList = dtFileInfo.strExecParam[0].split("/");
                    if (simpleExecList.size() > 0) {
                        dtFileInfo.strSimpleExec = simpleExecList[simpleExecList.size()-1];
                    }
                }
                dtFileInfo.strName = desktopFile->value(QString("Desktop Entry/Name[%1]").arg(QLocale::system().name())).toString();
                dtFileInfo.strGenericName = desktopFile->value(QString("Desktop Entry/GenericName[%1]").arg(QLocale::system().name())).toString();
                dtFileInfo.strIcon = desktopFile->value(QString("Desktop Entry/Icon")).toString();
                m_mapDesktopInfoList[dirXdgAutostart.fileName()] = dtFileInfo;
                //qDebug()<<""
                delete desktopFile;
                desktopFile = nullptr;
            }
        }
        dirXdgAutostart.next();
    }
    // read all application desktop file
    QDirIterator dirApplication("/usr/share/applications", QDirIterator::Subdirectories);
    while(dirApplication.hasNext()) {
        if (dirApplication.fileInfo().suffix() == "desktop") {
            QSettings* desktopFile = new QSettings(dirApplication.filePath(), QSettings::IniFormat);
            if (desktopFile) {
                DTFileInfo dtFileInfo;
                desktopFile->setIniCodec("utf-8");
                dtFileInfo.strExec = desktopFile->value(QString("Desktop Entry/Exec")).toString();
                dtFileInfo.strExecParam = dtFileInfo.strExec.split(" ");
                if (dtFileInfo.strExecParam.size() > 0) {
                    QStringList simpleExecList = dtFileInfo.strExecParam[0].split("/");
                    if (simpleExecList.size() > 0) {
                        dtFileInfo.strSimpleExec = simpleExecList[simpleExecList.size()-1];
                    }
                }
                dtFileInfo.strName = desktopFile->value(QString("Desktop Entry/Name[%1]").arg(QLocale::system().name())).toString();
                dtFileInfo.strGenericName = desktopFile->value(QString("Desktop Entry/GenericName[%1]").arg(QLocale::system().name())).toString();
                dtFileInfo.strIcon = desktopFile->value(QString("Desktop Entry/Icon")).toString();
                m_mapDesktopInfoList[dirApplication.fileName()] = dtFileInfo;
                delete desktopFile;
                desktopFile = nullptr;
            }
        }
        dirApplication.next();
    }
}