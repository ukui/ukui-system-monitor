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

#ifndef __PROCESSTABLEVIEW_H__
#define __PROCESSTABLEVIEW_H__

#include "ktableview.h"
#include "process/process_list.h"
#include "../widgets/mydialog.h"
#include "../widgets/myactiongroup.h"
#include "../widgets/myactiongroupitem.h"
#include "../widgets/myaction.h"
#include "../renicedialog.h"

#include <QLabel>
#include <QTreeView>
#include <QSettings>

using namespace sysmonitor::process;

class ProcessTableModel;
class ProcessSortFilterProxyModel;
class QShortcut;

/**
 * @brief Process table view
 */
class ProcessTableView : public KTableView
{
    Q_OBJECT

public:
    /**
     * @brief Process table view constructor
     * @param parent Parent object
     */
    explicit ProcessTableView(QSettings* proSettings = nullptr, QWidget *parent = nullptr);
    /**
     * @brief Destructor
     */
    ~ProcessTableView() override;

    /**
     * @brief eventFilter Filters events if this object has been installed as an event filter for the watched object
     * @param obj Object being watched
     * @param event Event to be filterted
     * @return To filter this event out, return true; otherwise return false
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

    void displayAllProcess();
    void displayActiveProcess();
    void displayCurrentUserProcess();

signals:
    void changeSortStatus(int index, bool isSort);
    void changeRefreshFilter(QString strFilter);
    void startScanProcess();
    void stopScanProcess();
    void changeColumnVisible(int index, bool visible, QList<bool> columnVisible);
    void closeDialog();

public Q_SLOTS:
    /**
     * @brief focus on this widget
     */
    void focusProcessView();
    /**
     * @brief stop process
     */
    void stopProcesses();
    /**
     * @brief continue process
     */
    void continueProcesses();
    /**
     * @brief end process
     */
    void endProcesses();
    /**
     * @brief kill process
     */
    void killProcesses();
    /**
     * @brief show process properties
     */
    void showPropertiesDialog();
    /**
     * @brief show end proc dialog
     */
    void showEndProcessDialog();
    /**
     * @brief show kill proc dialog
     */
    void showKillProcessDialog();
    /**
     * @brief on end dialog btn clicked
     */
    void endDialogButtonClicked(int index, QString buttonText);
    /**
     * @brief on kill dialog btn clicked
     */
    void killDialogButtonClicked(int index, QString buttonText);
    /**
     * @brief change process priority
     */
    void changeProcPriority(int nice);
    /**
     * @brief Filter process handler
     * @param text Text to be filtered out
     */
    void onSearch(const QString text);
    /**
     * @brief Switch process display mode
     * @param type Process display mode
     */
    void switchDisplayMode(FilterType type);
    /**
     * @brief Change process priority handler
     * @param priority Process priority
     */
    void changeProcessPriority(int priority);
    /**
     * @brief Change process filter
     * @param index filter index
     */
    void onChangeProcessFilter(int index);
    /**
     * @brief do search focus in event
     */
    void onSearchFocusIn();
    /**
     * @brief do search focus out event
     */
    void onSearchFocusOut();

    virtual void adjustColumnsSize();

protected:
    /**
     * @brief Load process table view backup settings
     * @return Settings load success or not
     */
    bool loadSettings();
    /**
     * @brief Backup process table view settings
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
     * @brief Show event handler
     * @param event Show event
     */
    void showEvent(QShowEvent *event) override;

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
    /**
     * @brief Adjust search result tip label's position & visibility
     */
    void adjustInfoLabelVisibility();

private:
    // Process model for process table view
    ProcessTableModel *m_model {};
    // Sort & filter model build upon process model
    ProcessSortFilterProxyModel *m_proxyModel {};

    // process priority group
    MyActionGroup * m_priorityGroup;
    // Process table view context menu
    QMenu *m_contextMenu {};
    // Process table header view context menu
    QMenu *m_headerContextMenu {};
    // Search result tip label
    QLabel *m_notFoundLabel {};

    // Currently selected PID
    QVariant m_selectedPID {};

    // End process shortcut
    QShortcut *m_endProcKP {};
    // Pause process shortcut
    QShortcut *m_pauseProcKP {};
    // Resume process shortcut
    QShortcut *m_resumeProcKP {};
    // View process property shortcut
    QShortcut *m_viewPropKP {};
    // Kill process shortcut
    QShortcut *m_killProcKP {};

    // program settings
    QSettings *m_proSettings = nullptr;
    // filter type
    QString m_strFilter = "all";
    // change nice dialog
    ReniceDialog *m_dlgRenice = nullptr;
};

#endif  // __PROCESSTABLEVIEW_H__
