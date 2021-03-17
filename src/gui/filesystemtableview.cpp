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

#include "filesystemtableview.h"

#include "model/filesystemsortfilterproxymodel.h"
#include "model/filesystemmodel.h"
#include "../filesystem/filesystemworker.h"
#include "../shell/macro.h"

#include <sys/types.h>
#include <unistd.h>
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
#include <QDesktopServices>

#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QKeyEvent>
#include <QShortcut>

// constructor
FileSystemTableView::FileSystemTableView(QSettings* proSettings, QWidget *parent)
    : KTableView(parent), m_proSettings(proSettings)
{
    // install event filter for table view to handle key events
    installEventFilter(this);

    // model & sort filter proxy model instance
    m_model = new FileSystemModel(this);
    m_proxyModel = new FileSystemSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    // setModel must be called before calling loadSettings();
    setModel(m_proxyModel);

    // load filesystem table view backup settings
    bool settingsLoaded = loadSettings();

    // initialize ui components & connections
    initUI(settingsLoaded);
    initConnections(settingsLoaded);
}

// destructor
FileSystemTableView::~FileSystemTableView()
{
}

// event filter
bool FileSystemTableView::eventFilter(QObject *obj, QEvent *event)
{
    // handle key press events for process table view
    return KTableView::eventFilter(obj, event);
}

// initialize ui components
void FileSystemTableView::initUI(bool settingsLoaded)
{
    setAccessibleName("FileSystemTableView");

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
    m_contextMenu->setObjectName("MonitorMenu");
    m_headerContextMenu = new QMenu(this);
    m_headerContextMenu->setObjectName("MonitorMenu");

    // show default style
    // dev name
    setColumnWidth(FileSystemModel::DeviceNameColumn, devicepadding);
    // mount uri
    setColumnWidth(FileSystemModel::MountUriColumn, mounturiadding);
    // type
    setColumnWidth(FileSystemModel::FileSystemTypeColumn, typepadding);
    // total
    setColumnWidth(FileSystemModel::TotalCapcityColumn, totalcapacitypadding);
    // free
    setColumnWidth(FileSystemModel::FreeCapcityColumn, idlepadding);
    // avalid
    setColumnWidth(FileSystemModel::AvalidCapcityColumn, avaliablepadding);
    // used
    setColumnWidth(FileSystemModel::UsedCapcityColumn, usedpadding);

    if (!settingsLoaded) {
        setColumnHidden(FileSystemModel::DeviceNameColumn, false);
        setColumnHidden(FileSystemModel::MountUriColumn, false);
        setColumnHidden(FileSystemModel::FileSystemTypeColumn, false);
        setColumnHidden(FileSystemModel::TotalCapcityColumn, false);
        setColumnHidden(FileSystemModel::FreeCapcityColumn, false);
        setColumnHidden(FileSystemModel::AvalidCapcityColumn, false);
        setColumnHidden(FileSystemModel::UsedCapcityColumn, false);

        //sort
        sortByColumn(FileSystemModel::DeviceNameColumn, Qt::DescendingOrder);
    }
    saveSettings();
}

// initialize connections
void FileSystemTableView::initConnections(bool settingsLoaded)
{
    // connect to double clicked
    connect(this, &QAbstractItemView::doubleClicked, this, &FileSystemTableView::onDoubleClicked);
    // table context menu
    connect(this, &FileSystemTableView::customContextMenuRequested, this,
            &FileSystemTableView::displayProcessTableContextMenu);
    auto *refreshAction = new QAction(tr("Refresh"), this);
    connect(refreshAction, &QAction::triggered, m_model, &FileSystemModel::updateFileSystemList);
    m_contextMenu->addAction(refreshAction);//刷新

    auto *h = header();
    connect(h, &QHeaderView::sectionResized, this, [ = ]() { saveSettings(); });
    connect(h, &QHeaderView::sectionMoved, this, [ = ]() { saveSettings(); });
    connect(h, &QHeaderView::sortIndicatorChanged, this, [ = ]() { saveSettings(); });
    connect(h, &QHeaderView::customContextMenuRequested, this,
            &FileSystemTableView::displayProcessTableHeaderContextMenu);

    // header context menu
    // mount uri action
    auto *mountUriHeaderAction = m_headerContextMenu->addAction(tr("Directory"));
    mountUriHeaderAction->setCheckable(true);
    connect(mountUriHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(FileSystemModel::MountUriColumn, !b);
        saveSettings();
    });
    // type action
    auto *typeHeaderAction = m_headerContextMenu->addAction(tr("Type"));
    typeHeaderAction->setCheckable(true);
    connect(typeHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(FileSystemModel::FileSystemTypeColumn, !b);
        saveSettings();
    });
    // total action
    auto *totalHeaderAction = m_headerContextMenu->addAction(tr("Total"));
    totalHeaderAction->setCheckable(true);
    connect(totalHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(FileSystemModel::TotalCapcityColumn, !b);
        saveSettings();
    });
    // free action
    auto *freeHeaderAction = m_headerContextMenu->addAction(tr("Free"));
    freeHeaderAction->setCheckable(true);
    connect(freeHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(FileSystemModel::FreeCapcityColumn, !b);
        saveSettings();
    });
    // avalid action
    auto *avalidHeaderAction = m_headerContextMenu->addAction(tr("Available"));
    avalidHeaderAction->setCheckable(true);
    connect(avalidHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(FileSystemModel::AvalidCapcityColumn, !b);
        saveSettings();
    });
    // used action
    auto *usedHeaderAction = m_headerContextMenu->addAction(tr("Used"));
    usedHeaderAction->setCheckable(true);
    connect(usedHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(FileSystemModel::UsedCapcityColumn, !b);
        saveSettings();
    });

    // set default header context menu checkable state when settings load without success
    if (!settingsLoaded) {
        mountUriHeaderAction->setChecked(true);
        typeHeaderAction->setChecked(true);
        totalHeaderAction->setChecked(true);
        freeHeaderAction->setChecked(true);
        avalidHeaderAction->setChecked(true);
        usedHeaderAction->setChecked(true);
    }
    // set header context menu checkable state based on current header section's visible state before popup
    connect(m_headerContextMenu, &QMenu::aboutToShow, this, [ = ]() {
        bool b;
        b = header()->isSectionHidden(FileSystemModel::MountUriColumn);
        mountUriHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(FileSystemModel::FileSystemTypeColumn);
        typeHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(FileSystemModel::TotalCapcityColumn);
        totalHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(FileSystemModel::FreeCapcityColumn);
        freeHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(FileSystemModel::AvalidCapcityColumn);
        avalidHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(FileSystemModel::UsedCapcityColumn);
        usedHeaderAction->setChecked(!b);
    });

    // on each model update, we adjust search result tip lable's visibility & positon, select the same process item before update if any
    connect(m_model, &FileSystemModel::modelUpdated, this, [&]() {
        if (m_selectedDevName.isValid()) {
            for (int i = 0; i < m_proxyModel->rowCount(); i++) {
                if (m_proxyModel->data(m_proxyModel->index(i, FileSystemModel::DeviceNameColumn),
                                       Qt::UserRole) == m_selectedDevName)
                    this->setCurrentIndex(m_proxyModel->index(i, 0));
            }
        }
    });
}

// show process table view context menu on specified positon
void FileSystemTableView::displayProcessTableContextMenu(const QPoint &p)
{
    if (selectedIndexes().size() == 0 || !m_selectedDevName.isValid())
        return;
    QPoint point = mapToGlobal(p);
    point.setY(point.y() + header()->sizeHint().height());
    QString strDevName = qvariant_cast<QString>(m_selectedDevName);
    m_contextMenu->popup(point);
}

// show process header view context menu on specified position
void FileSystemTableView::displayProcessTableHeaderContextMenu(const QPoint &p)
{
    m_headerContextMenu->popup(mapToGlobal(p));
}

// resize event handler
void FileSystemTableView::resizeEvent(QResizeEvent *event)
{
    QTreeView::resizeEvent(event);
}

// backup current selected item's pid when selection changed
void FileSystemTableView::selectionChanged(const QItemSelection &selected,
                                        const QItemSelection &deselected)
{
    // if no selection, do nothing
    if (selected.size() <= 0) {
        return;
    }

    m_selectedDevName = selected.indexes().value(FileSystemModel::DeviceNameColumn).data();

    QTreeView::selectionChanged(selected, deselected);
}

// double clicked event
void FileSystemTableView::onDoubleClicked(const QModelIndex &index)
{
    FileSystemModel* model = (FileSystemModel*)index.model();
    const QModelIndex &mountUri = model->index(index.row(), FileSystemModel::MountUriColumn);
    if (mountUri.isValid()) {
        QString strMountUri = mountUri.data().toString();
        //open the mount Uri
        QString targetPath = QString("file://%1").arg(strMountUri);
        QDesktopServices::openUrl(QUrl(targetPath));//xdg-open
    }
}

// return hinted size for specified column, so column can be resized to a prefered width when double clicked
int FileSystemTableView::sizeHintForColumn(int column) const
{
    QStyleOptionHeader option;
    option.initFrom(this);
    int margin = 10;
    return std::max(header()->sizeHintForColumn(column) + margin * 2,
                    QTreeView::sizeHintForColumn(column) + margin * 2);
}

// load & restore table view settings from backup storage
bool FileSystemTableView::loadSettings()
{
    if (m_proSettings) {
        m_proSettings->beginGroup("FileSystem");
        QVariant opt = m_proSettings->value(SETTINGSOPTION_FILESYSTEMTABLEHEADERSTATE);
        m_proSettings->endGroup();
        if (opt.isValid()) {
            QByteArray buf = QByteArray::fromBase64(opt.toByteArray());
            header()->restoreState(buf);
            return true;
        }
    }
    return false;
}

// save table view settings to backup storage
void FileSystemTableView::saveSettings()
{
    if (m_proSettings) {
        QByteArray buf = header()->saveState();
        m_proSettings->beginGroup("FileSystem");
        m_proSettings->setValue(SETTINGSOPTION_FILESYSTEMTABLEHEADERSTATE, buf.toBase64());
        m_proSettings->endGroup();
        m_proSettings->sync();
    }
}

