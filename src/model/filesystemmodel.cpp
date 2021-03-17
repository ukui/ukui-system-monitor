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

#include "filesystemmodel.h"
#include "../filesystem/filesystemworker.h"
#include "../util.h"

#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QFileInfo>
#include <QIcon>

// model constructor
FileSystemModel::FileSystemModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    // init filesystem list provider
    m_fileSystemWorker = new FileSystemWorker(this);
    //update model's filesystem list cache on filesystem list updated signal
    m_fileSystemWatcher = FileSystemWatcher::instance();  //it will emit a signal to tell us if the device is added or removed
    connect(m_fileSystemWatcher, &FileSystemWatcher::deviceAdded, this, [=] (const QString &dev) {
        this->updateFileSystemList();
    });
    connect(m_fileSystemWatcher, &FileSystemWatcher::deviceRemoved, this, [=] (const QString &dev) {
        this->updateFileSystemList();
    });

    m_devNameList.clear();
    m_fileSystemDataList.clear();
    updateFileSystemList();
}

// update filesystem model with the data provided by list
void FileSystemModel::updateFileSystemList()
{
    QTimer::singleShot(0, this, SLOT(updateFileSystemListDelay()));
}

void FileSystemModel::updateFileSystemListDelay()
{
    if (m_fileSystemWorker) {
        m_fileSystemWorker->onFileSystemListChanged();

        const QList<QString> &newDevNameList = m_fileSystemWorker->diskDevNameList();
        QList<QString> oldDevNameList = m_devNameList;

        for (const auto &devname : newDevNameList) {
            int row = m_devNameList.indexOf(devname);
            if (row >= 0) {
                // update
                m_fileSystemWorker->getDiskInfo(devname, m_fileSystemDataList[row]);
                Q_EMIT dataChanged(index(row, 0), index(row, columnCount() - 1));
            } else {
                // insert
                row = m_devNameList.size();
                beginInsertRows({}, row, row);
                FileSystemData fsData;
                if (m_fileSystemWorker->getDiskInfo(devname, fsData)) {
                    m_devNameList << devname;
                    m_fileSystemDataList << fsData;
                }
                endInsertRows();
            }
        }

        // remove
        for (const auto &devname : oldDevNameList) {
            if (!newDevNameList.contains(devname)) {
                int row = m_devNameList.indexOf(devname);
                beginRemoveRows({}, row, row);
                m_devNameList.removeAt(row);
                m_fileSystemDataList.removeAt(row);
                endRemoveRows();
            }
        }
    }

    Q_EMIT modelUpdated();
}

// returns the number of rows under the given parent
int FileSystemModel::rowCount(const QModelIndex &) const
{
    return m_fileSystemDataList.size();
}

// returns the number of columns for the children of the given parent
int FileSystemModel::columnCount(const QModelIndex &) const
{
    return FileSystemColumnCount;
}

// returns the data for the given role and section in the header with the specified orientation
QVariant FileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
        switch (section) {
        case DeviceNameColumn:
            // device name column display text
            return tr("Device");
        case MountUriColumn:
            // mount uri column display text
            return tr("Directory");
        case FileSystemTypeColumn:
            // type column display text
            return tr("Type");
        case TotalCapcityColumn:
            // total capcity column display text
            return tr("Total");
        case FreeCapcityColumn:
            // free capcity column display text
            return tr("Free");
        case AvalidCapcityColumn:
            // avalid capcity column display text
            return tr("Available");
        case UsedCapcityColumn:
            // used capcity column display text
            return tr("Used");
        default:
            break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        // default header section alignment
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (role == Qt::InitialSortOrderRole) {
        // sort section descending by default
        return QVariant::fromValue(Qt::DescendingOrder);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

// returns the data stored under the given role for the item referred to by the index
QVariant FileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    // validate index
    if (index.row() < 0 || index.row() >= m_fileSystemDataList.size())
        return {};

    int row = index.row();
    const FileSystemData &fsData = m_fileSystemDataList[row];
    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
        switch (index.column()) {
        case DeviceNameColumn: {
            // filesystem's device name
            return fsData.deviceName();
        }
        case MountUriColumn:
            // filesystem's mount uri
            return fsData.mountDir();
        case FileSystemTypeColumn:
            // filesystem's type
            return fsData.diskType();
        case TotalCapcityColumn:
            // filesystem's total capcity
            return fsData.totalCapacity();
        case FreeCapcityColumn: 
            // filesystem's total capcity
            return fsData.freeCapacity();
        case AvalidCapcityColumn:
            // filesystem's total capcity
            return fsData.availCapacity();
        case UsedCapcityColumn:
            // filesystem's total capcity
            return fsData.usedCapactiy();
        default:
            break;
        }
    } else if (role == Qt::DecorationRole) {
        switch (index.column()) {
        case DeviceNameColumn:
        {
            // filesystem icon
            QIcon diskIcon = QIcon(":/img/drive-harddisk-system.png");
            return diskIcon.pixmap(30, 30);
        }
        default:
            return {};
        }
    } else if (role == Qt::UserRole) {
        // get filesystem's raw data
        switch (index.column()) {
        case DeviceNameColumn:
            return fsData.deviceName();
        case MountUriColumn:
            return fsData.mountDir();
        case FileSystemTypeColumn:
            return fsData.diskType();
        case TotalCapcityColumn:
            return fsData.totalCapacity();
        case FreeCapcityColumn:
            return fsData.freeCapacity();
        case AvalidCapcityColumn:
            return fsData.availCapacity();
        case UsedCapcityColumn:
            return fsData.usedCapactiy();
        default:
            return {};
        }
    } else if (role == (Qt::UserRole + 1)) {
        // get process's extra data
        switch (index.column()) {
        case TotalCapcityColumn:
            return fsData.totalCapacityValue();
        case FreeCapcityColumn:
            return fsData.freeCapacityValue();
        case AvalidCapcityColumn:
            return fsData.avalidCapacityValue();
        case UsedCapcityColumn:
            return fsData.usedCapacityValue();
        default:
            return {};
        }
    } else if (role == Qt::TextAlignmentRole) {
        // default data alignment
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (role == Qt::ForegroundRole) {
        // for text color
    } else if (role == Qt::ToolTipRole) {
        switch (index.column()) {
        case UsedCapcityColumn:
            return QString("%1%").arg(fsData.usedPercentage());
        default:
            break;
        }
    }
    return {};
}

// returns the item flags for the given index
Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
