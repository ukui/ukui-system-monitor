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

#ifndef __DESKTOP_FILEINFO_H__
#define __DESKTOP_FILEINFO_H__

#include <QMap>
#include <QObject>

typedef struct _DTFileInfo_s {
    QString strExec = "";
    QString strSimpleExec = "";
    QString strName = "";
    QString strGenericName = "";
    QString strIcon = "";
}DTFileInfo;

class DesktopFileInfo : public QObject
{
    Q_OBJECT
public: 
    DesktopFileInfo(QObject *parent = nullptr);
    ~DesktopFileInfo();

    static DesktopFileInfo* instance();

    QString getDesktopFileNameByExec(QString strExec);
    QString getNameByExec(QString strExec);
    QString getIconByExec(QString strExec);
    QString getNameByDesktopFile(QString strDesktopFileName);
    QString getIconByDesktopFile(QString strDesktopFileName);

    void readAllDesktopFileInfo();

private:
    QMap<QString, DTFileInfo> m_mapDesktopInfoList;
};

#endif