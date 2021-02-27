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

#include "processtableview.h"

#include "model/processsortfilterproxymodel.h"
#include "model/processtablemodel.h"
#include "process/process_list.h"
#include "process/process_monitor.h"
#include "../shell/macro.h"

#include <QApplication>
#include <QDialog>
#include <QErrorMessage>
#include <QFrame>
#include <QLabel>
#include <QMenu>
#include <QPalette>
#include <QSlider>
#include <QToolTip>
#include <QWidget>
#include <QHeaderView>

#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QKeyEvent>
#include <QShortcut>

// constructor
ProcessTableView::ProcessTableView(QSettings* proSettings, QWidget *parent)
    : KTableView(parent), m_proSettings(proSettings)
{
    // install event filter for table view to handle key events
    installEventFilter(this);

    // model & sort filter proxy model instance
    m_model = new ProcessTableModel(this);
    m_proxyModel = new ProcessSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    // setModel must be called before calling loadSettings();
    setModel(m_proxyModel);

    // load process table view backup settings
    bool settingsLoaded = false;
    connect(this,SIGNAL(changeRefreshFilter(QString)),
        ProcessMonitorThread::instance()->procMonitorInstance(),SLOT(onChangeRefreshFilter(QString)));
    if (proSettings) {
        proSettings->beginGroup("PROCESS");
        m_strFilter = proSettings->value("WhoseProcesses", m_strFilter).toString();
        proSettings->endGroup();
        ProcessMonitorThread::instance()->procMonitorInstance()->processList()->setScanFilter(m_strFilter);
    }

    // initialize ui components & connections
    initUI(settingsLoaded);
    initConnections(settingsLoaded);

    // start process monitor thread
    ProcessMonitorThread::instance()->start();
}

// destructor
ProcessTableView::~ProcessTableView()
{
}

// event filter
bool ProcessTableView::eventFilter(QObject *obj, QEvent *event)
{
    // handle key press events for process table view
    return KTableView::eventFilter(obj, event);
}

void ProcessTableView::displayAllProcess()
{
    m_strFilter = "all";
    emit changeRefreshFilter(m_strFilter);
}

void ProcessTableView::displayActiveProcess()
{
    m_strFilter = "active";
    emit changeRefreshFilter(m_strFilter);
}

void ProcessTableView::displayCurrentUserProcess()
{
    m_strFilter = "user";
    emit changeRefreshFilter(m_strFilter);
}

// end process handler
void ProcessTableView::endProcess()
{
    // no selected item, do nothing
    if (m_selectedPID.isNull()) {
        return;
    }
}

// pause process handler
void ProcessTableView::pauseProcess()
{
    auto pid = qvariant_cast<pid_t>(m_selectedPID);
}

// resume process handler
void ProcessTableView::resumeProcess()
{
    auto pid = qvariant_cast<pid_t>(m_selectedPID);
    //no selected item or app self been selected, then do nothing
}

// open process bin path in file manager
void ProcessTableView::openExecDirWithFM()
{
}

// show process attribute dialog
void ProcessTableView::showProperties()
{
}

// kill process handler
void ProcessTableView::killProcess()
{
    // no selected item, do nothing
    if (m_selectedPID.isNull()) {
        return;
    }

}

// filter process table based on searched text
void ProcessTableView::search(const QString &text)
{
    m_proxyModel->setSortFilterString(text);
    // adjust search result tip label's visibility & position if needed
    adjustInfoLabelVisibility();
}

// switch process table view display mode
void ProcessTableView::switchDisplayMode(FilterType type)
{
    m_proxyModel->setFilterType(type);
}

// change process priority
void ProcessTableView::changeProcessPriority(int priority)
{
    
}

// Change process filter
void ProcessTableView::onChangeProcessFilter(int index)
{
    if (index == 0) {
        if (this->m_strFilter != "active")
            this->displayActiveProcess();
    } else if (index == 1) {
        if (this->m_strFilter != "user")
            this->displayCurrentUserProcess();
    } else {
        if (this->m_strFilter != "all")
            this->displayAllProcess();
    }

    if (m_proSettings) {
        m_proSettings->beginGroup("PROCESS");
        m_proSettings->setValue("WhoseProcesses", m_strFilter);
        m_proSettings->endGroup();
        m_proSettings->sync();
    }
}

// initialize ui components
void ProcessTableView::initUI(bool settingsLoaded)
{
    setAccessibleName("ProcessTableView");

    // search result not found tip label instance
    m_notFoundLabel = new QLabel(tr("No search results"), this);
    //QFontSizeManager::instance()->bind(m_notFoundLabel, QFontSizeManager::T4);
    // change text color
    auto palette = m_notFoundLabel->palette();//QApplicationHelper::instance()->palette(m_notFoundLabel);
    QColor labelColor = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::Text, labelColor);
    m_notFoundLabel->setPalette(palette);
    m_notFoundLabel->setVisible(false);

    // header view options
    // header section movable
    header()->setSectionsMovable(true);
    // header section clickable
    header()->setSectionsClickable(true);
    // header section resizable
    header()->setSectionResizeMode(QHeaderView::Interactive);
    // stretch last header section
    header()->setStretchLastSection(true);
    // show sort indicator on sort column
    header()->setSortIndicatorShown(true);
    // header section default alignment
    header()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    // header section context menu policy
    header()->setContextMenuPolicy(Qt::CustomContextMenu);

    // table options
    setSortingEnabled(true);
    // only single row selection allowed
    setSelectionMode(QAbstractItemView::SingleSelection);
    // can only select whole row
    setSelectionBehavior(QAbstractItemView::SelectRows);
    // table view context menu policy
    setContextMenuPolicy(Qt::CustomContextMenu);

    // context menu & header context menu instance
    m_contextMenu = new QMenu(this);
    m_headerContextMenu = new QMenu(this);

    // if no backup settings loaded, show default style
    if (!settingsLoaded) {
        // proc name
        setColumnWidth(ProcessTableModel::ProcessNameColumn, namepadding);
        setColumnHidden(ProcessTableModel::ProcessNameColumn, false);

        // account
        setColumnWidth(ProcessTableModel::ProcessUserColumn, userpadding);
        setColumnHidden(ProcessTableModel::ProcessUserColumn, false);

        // diskio
        setColumnWidth(ProcessTableModel::ProcessDiskIoColumn, diskpadding);
        setColumnHidden(ProcessTableModel::ProcessDiskIoColumn, false);

        // cpu
        setColumnWidth(ProcessTableModel::ProcessCpuColumn, cpupadding);
        setColumnHidden(ProcessTableModel::ProcessCpuColumn, false);

        // pid
        setColumnWidth(ProcessTableModel::ProcessIdColumn, idpadding);
        setColumnHidden(ProcessTableModel::ProcessIdColumn, false);

        // flownet
        setColumnWidth(ProcessTableModel::ProcessFlowNetColumn, networkpadding);
        setColumnHidden(ProcessTableModel::ProcessFlowNetColumn, false);

        // memory
        setColumnWidth(ProcessTableModel::ProcessMemoryColumn, memorypadding);
        setColumnHidden(ProcessTableModel::ProcessMemoryColumn, false);

        // priority
        setColumnWidth(ProcessTableModel::ProcessNiceColumn, prioritypadding);
        setColumnHidden(ProcessTableModel::ProcessNiceColumn, false);

        //sort
        sortByColumn(ProcessTableModel::ProcessCpuColumn, Qt::DescendingOrder);
    }
}

// initialize connections
void ProcessTableView::initConnections(bool settingsLoaded)
{

}

// show process table view context menu on specified positon
void ProcessTableView::displayProcessTableContextMenu(const QPoint &p)
{
    if (selectedIndexes().size() == 0)
        return;

    QPoint point = mapToGlobal(p);
    point.setY(point.y() + header()->sizeHint().height());
    m_contextMenu->popup(point);
}

// show process header view context menu on specified position
void ProcessTableView::displayProcessTableHeaderContextMenu(const QPoint &p)
{
    m_headerContextMenu->popup(mapToGlobal(p));
}

// resize event handler
void ProcessTableView::resizeEvent(QResizeEvent *event)
{
    // adjust search result tip label's visibility & position when resizing
    adjustInfoLabelVisibility();

    QTreeView::resizeEvent(event);
}

// show event handler
void ProcessTableView::showEvent(QShowEvent *)
{
    // hide search result not found on initial show
    if (m_notFoundLabel) {
        m_notFoundLabel->hide();
    }
}

// backup current selected item's pid when selection changed
void ProcessTableView::selectionChanged(const QItemSelection &selected,
                                        const QItemSelection &deselected)
{
    // if no selection, do nothing
    if (selected.size() <= 0) {
        return;
    }

    m_selectedPID = selected.indexes().value(ProcessTableModel::ProcessIdColumn).data();

    QTreeView::selectionChanged(selected, deselected);
}

// return hinted size for specified column, so column can be resized to a prefered width when double clicked
int ProcessTableView::sizeHintForColumn(int column) const
{
    QStyleOptionHeader option;
    option.initFrom(this);
    QStyle *style = dynamic_cast<QStyle *>(QApplication::style());
    int margin = 10;//style->pixelMetric(QStyle::PM_ContentsMargins, &option);
    return std::max(header()->sizeHintForColumn(column) + margin * 2,
                    QTreeView::sizeHintForColumn(column) + margin * 2);
}

// adjust search result tip label's visibility & position
void ProcessTableView::adjustInfoLabelVisibility()
{
    setUpdatesEnabled(false);
    // show search not found label only when proxy model is empty & search text input is none empty
    // m_notFoundLabel->setVisible(m_proxyModel->rowCount() == 0
    //                            && gApp->mainWindow()->toolbar()->isSearchContentEmpty());
    // move label to center of the process table view
    if (m_notFoundLabel->isVisible())
        m_notFoundLabel->move(rect().center() - m_notFoundLabel->rect().center());
    setUpdatesEnabled(true);
}

// show customize process priority dialog
void ProcessTableView::customizeProcessPriority()
{
    // priority dialog instance
    
}
