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

#include "filesystemsortfilterproxymodel.h"
#include "../filesystem/filesystemworker.h"
#include "filesystemmodel.h"

#include <QCollator>
#include <QDebug>
#include <QLocale>

// proxy model constructor
FileSystemSortFilterProxyModel::FileSystemSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

// filters the row of specified parent with given pattern
bool FileSystemSortFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    if (filterRegExp().isEmpty()) {
        return true;
    }
    
    bool rc = false;
    // devname matches pattern
    const QModelIndex &devname = sourceModel()->index(row, FileSystemModel::DeviceNameColumn, parent);
    if (devname.isValid())
        rc |= devname.data().toString().contains(filterRegExp());
    if (rc) return rc;

    // mount uri matches pattern
    const QModelIndex &mountUri = sourceModel()->index(row, FileSystemModel::MountUriColumn, parent);
    if (mountUri.isValid())
        rc |= mountUri.data().toString().contains(filterRegExp());

    return rc;
}

// compare two items with the specified index
bool FileSystemSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    int sortcolumn = sortColumn();
    switch (sortcolumn) {
    case FileSystemModel::DeviceNameColumn: {
        const QString &lhs = left.data(Qt::UserRole).toString();
        const QString &rhs = right.data(Qt::UserRole).toString();
        return lhs.localeAwareCompare(rhs) < 0;
    }
    case FileSystemModel::MountUriColumn: {
        const QString &lhs = left.data(Qt::UserRole).toString();
        const QString &rhs = right.data(Qt::UserRole).toString();
        return lhs.localeAwareCompare(rhs) < 0;
    }
    case FileSystemModel::FileSystemTypeColumn: {
        const QString &lhs = left.data(Qt::UserRole).toString();
        const QString &rhs = right.data(Qt::UserRole).toString();
        return lhs.localeAwareCompare(rhs) < 0;
    }
    case FileSystemModel::TotalCapcityColumn: {
        qreal lfLeft, lfRight;
        lfLeft = left.data(Qt::UserRole+1).toReal();
        lfRight = right.data(Qt::UserRole+1).toReal();
        return lfLeft < lfRight;
    }
    case FileSystemModel::FreeCapcityColumn: {
        qreal lfLeft, lfRight;
        lfLeft = left.data(Qt::UserRole+1).toReal();
        lfRight = right.data(Qt::UserRole+1).toReal();
        return lfLeft < lfRight;
    }
    case FileSystemModel::AvalidCapcityColumn: {
        qreal lfLeft, lfRight;
        lfLeft = left.data(Qt::UserRole+1).toReal();
        lfRight = right.data(Qt::UserRole+1).toReal();
        return lfLeft < lfRight;
    }
    case FileSystemModel::UsedCapcityColumn: {
        qreal lfLeft, lfRight;
        lfLeft = left.data(Qt::UserRole+1).toReal();
        lfRight = right.data(Qt::UserRole+1).toReal();
        return lfLeft < lfRight;
    }
    default:
        break;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}
