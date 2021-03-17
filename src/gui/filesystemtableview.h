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

#ifndef __FILESYSTEMTABLEVIEW_H__
#define __FILESYSTEMTABLEVIEW_H__

#include "ktableview.h"
#include "../filesystem/filesystemworker.h"

#include <QLabel>
#include <QTreeView>
#include <QSettings>

class FileSystemModel;
class FileSystemSortFilterProxyModel;
class QShortcut;

/**
 * @brief Filesystem table view
 */
class FileSystemTableView : public KTableView
{
    Q_OBJECT

public:
    /**
     * @brief Filesystem table view constructor
     * @param parent Parent object
     */
    explicit FileSystemTableView(QSettings* proSettings = nullptr, QWidget *parent = nullptr);
    /**
     * @brief Destructor
     */
    ~FileSystemTableView() override;

    /**
     * @brief eventFilter Filters events if this object has been installed as an event filter for the watched object
     * @param obj Object being watched
     * @param event Event to be filterted
     * @return To filter this event out, return true; otherwise return false
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:

private Q_SLOTS:
    /**
     * @brief Filesystem table item doubleclicked event
     */
    void onDoubleClicked(const QModelIndex &index);

protected:
    /**
     * @brief Load Filesystem table view backup settings
     * @return Settings load success or not
     */
    bool loadSettings();
    /**
     * @brief Backup Filesystem table view settings
     */
    void saveSettings();
    /**
     * @brief Initialize ui components
     * @param Backup settings loaded or not flag
     */
    void initUI(bool settingsLoaded);
    /**
     * @brief Initialize connections
     * @param Backup settings loaded or not flag
     */
    void initConnections(bool settingsLoaded);
    /**
     * @brief Show process table view context menu
     * @param p Where to show context menu
     */
    void displayProcessTableContextMenu(const QPoint &p);
    /**
     * @brief Show process table header view context menu
     * @param p Where to show context menu
     */
    void displayProcessTableHeaderContextMenu(const QPoint &p);

    /**
     * @brief resizeEvent Resize event handler
     * @param event Resize event
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief selectionChanged Selection changed event handler
     * @param selected Selected items
     * @param deselected Deselected items
     */
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) override;
    /**
     * @brief sizeHintForColumn Size hint for specific header view column
     * @param column Header view column
     * @return Hinted size for column
     */
    int sizeHintForColumn(int column) const override;

private:
    // Filesystem model for filesystem table view
    FileSystemModel *m_model {};
    // Sort & filter model build upon filesystem model
    FileSystemSortFilterProxyModel *m_proxyModel {};

    // Process table view context menu
    QMenu *m_contextMenu {};
    // Process table header view context menu
    QMenu *m_headerContextMenu {};
    // Currently selected devname
    QVariant m_selectedDevName {};

    // program settings
    QSettings *m_proSettings = nullptr;
};

#endif  // __FILESYSTEMTABLEVIEW_H__
