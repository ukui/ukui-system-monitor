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

#ifndef __FILESYSTEM_MODEL_H__
#define __FILESYSTEM_MODEL_H__

#include "../filesystem/filesystemworker.h"
#include "../filesystem/filesystemdata.h"
#include "../filesystem/filesystemwatcher.h"

#include <QAbstractTableModel>
#include <QList>
#include <QMap>

/**
 * @brief FileSystem model class
 */
class FileSystemModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @brief FileSystem column index
     */
    enum Column {
        DeviceNameColumn = 0, // device name column index
        MountUriColumn, // mount uri column index
        FileSystemTypeColumn, // filesystem type column index
        TotalCapcityColumn, // total capcity column index
        FreeCapcityColumn, // free capcity column index
        AvalidCapcityColumn, // avalid capcity column index
        UsedCapcityColumn, // used capcity column index

        FileSystemColumnCount // total number of columns
    };

    /**
     * @brief Model constructor
     * @param parent Parent object
     */
    explicit FileSystemModel(QObject *parent = nullptr);

    /**
     * @brief Returns the number of rows under the given parent
     * @param parent Parent index
     * @return Row count
     */
    int rowCount(const QModelIndex &parent = {}) const override;
    /**
     * @brief Returns the number of columns for the children of the given parent
     * @param parent Parent index
     * @return Column count
     */
    int columnCount(const QModelIndex &parent = {}) const override;
    /**
     * @brief Returns the data stored under the given role for the item referred to by the index
     * @param index Index of the data
     * @param role Role of the data
     * @return Data of specified role referred with index within model
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /**
     * @brief Returns the data for the given role and section in the header with the specified orientation
     * @param section Section of the header
     * @param orientation Orientation of the header
     * @param role Data role
     * @return Data of the give role & section with specified orientation within data source
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    /**
     * @brief Returns the item flags for the given index
     * @param index Model index to get flags for
     * @return Combination of item flags
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

Q_SIGNALS:
    /**
     * @brief Model updated signal
     */
    void modelUpdated();

public Q_SLOTS:
    /**
     * @brief Update filesystem model with the data provided by list
     * @param list filesysetem entry list
     */
    void updateFileSystemList();

private Q_SLOTS:
    /**
     * @brief update The FileSystem list
     */
    void updateFileSystemListDelay();

private:
    QList<QString> m_devNameList; // dev name list
    QList<FileSystemData> m_fileSystemDataList; // system data list
    FileSystemWorker *m_fileSystemWorker;    // list provider
    FileSystemWatcher *m_fileSystemWatcher;  // monitor the filesystem mount or unmount
};

#endif  // __FILESYSTEM_MODEL_H__
