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
#include "../util.h"
#include "procpropertiesdlg.h"

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
    this->setProperty("useIconHighlightEffect", 0x2);
    // model & sort filter proxy model instance
    m_model = new ProcessTableModel(this);
    m_proxyModel = new ProcessSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    // setModel must be called before calling loadSettings();
    setModel(m_proxyModel);

    // load process table view backup settings
    bool settingsLoaded = loadSettings();

    // init first scanproc filter
    ProcessMonitor::instance()->processList()->setScanFilter(m_strFilter);

    // initialize ui components & connections
    initUI(settingsLoaded);
    initConnections(settingsLoaded);

    // start process monitor thread
    ProcessMonitorThread::instance()->start();
}

// destructor
ProcessTableView::~ProcessTableView()
{
    // start process monitor thread
    ProcessMonitorThread::instance()->stop();
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

// focus on this widget
void ProcessTableView::focusProcessView()
{
    QTimer::singleShot(100, this, SLOT(setFocus()));
}

// filter process table based on searched text
void ProcessTableView::onSearch(const QString text)
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
}

// initialize ui components
void ProcessTableView::initUI(bool settingsLoaded)
{
    setAccessibleName("ProcessTableView");

    // search result not found tip label instance
    m_notFoundLabel = new QLabel(tr("No search result"), this);
    QFont font = m_notFoundLabel->font() ;
    font.setPointSize(22);
    m_notFoundLabel->setFont(font);
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
    m_contextMenu->setObjectName("MonitorMenu");
    m_headerContextMenu = new QMenu(this);
    m_headerContextMenu->setObjectName("MonitorMenu");

    // show default style
    // proc name
    setColumnWidth(ProcessTableModel::ProcessNameColumn, namepadding);
    // account
    setColumnWidth(ProcessTableModel::ProcessUserColumn, userpadding);
    // diskio
    setColumnWidth(ProcessTableModel::ProcessDiskIoColumn, diskpadding);
    // cpu
    setColumnWidth(ProcessTableModel::ProcessCpuColumn, cpupadding);
    // pid
    setColumnWidth(ProcessTableModel::ProcessIdColumn, idpadding);
    // flownet
    setColumnWidth(ProcessTableModel::ProcessFlowNetColumn, networkpadding);
    // memory
    setColumnWidth(ProcessTableModel::ProcessMemoryColumn, memorypadding);
    // priority
    setColumnWidth(ProcessTableModel::ProcessNiceColumn, prioritypadding);

    if (!settingsLoaded) {
        setColumnHidden(ProcessTableModel::ProcessNameColumn, false);
        setColumnHidden(ProcessTableModel::ProcessUserColumn, false);
        setColumnHidden(ProcessTableModel::ProcessDiskIoColumn, false);
        setColumnHidden(ProcessTableModel::ProcessCpuColumn, false);
        setColumnHidden(ProcessTableModel::ProcessIdColumn, false);
        setColumnHidden(ProcessTableModel::ProcessFlowNetColumn, false);
        setColumnHidden(ProcessTableModel::ProcessMemoryColumn, false);
        setColumnHidden(ProcessTableModel::ProcessNiceColumn, false);

        //sort
        sortByColumn(ProcessTableModel::ProcessCpuColumn, Qt::DescendingOrder);
    }
    saveSettings();
}

// initialize connections
void ProcessTableView::initConnections(bool settingsLoaded)
{
    connect(this,SIGNAL(changeRefreshFilter(QString)),
        ProcessMonitor::instance(),SLOT(onChangeRefreshFilter(QString)));
    connect(this,SIGNAL(startScanProcess()),
        ProcessMonitor::instance(),SLOT(onStartScanProcess()));
    connect(this,SIGNAL(stopScanProcess()),
        ProcessMonitor::instance(),SLOT(onStopScanProcess()));

    // table context menu
    connect(this, &ProcessTableView::customContextMenuRequested, this,
            &ProcessTableView::displayProcessTableContextMenu);
    auto *stopAction = new QAction(tr("Stop process"), this);
    connect(stopAction, &QAction::triggered, this, &ProcessTableView::stopProcesses);
    auto *continueAction = new QAction(tr("Continue process"), this);
    connect(continueAction, &QAction::triggered, this, &ProcessTableView::continueProcesses);
    auto *endAction = new QAction(tr("End process"), this);
    connect(endAction, &QAction::triggered, this, &ProcessTableView::showEndProcessDialog);
    auto *killAction = new QAction(tr("Kill process"), this);
    connect(killAction, &QAction::triggered, this, &ProcessTableView::showKillProcessDialog);

    m_priorityGroup = new MyActionGroup(this);
    auto *veryHighAction = new MyActionGroupItem(this, m_priorityGroup, "very_high_action", -20);
    auto *highAction = new MyActionGroupItem(this, m_priorityGroup, "high_action", -5);
    auto *normalAction = new MyActionGroupItem(this, m_priorityGroup, "normal_action", 0);
    auto *lowAction = new MyActionGroupItem(this, m_priorityGroup, "low_action", 5);
    auto *veryLowAction = new MyActionGroupItem(this, m_priorityGroup, "very_low_action", 19);
    auto *customAction = new MyActionGroupItem(this, m_priorityGroup, "custom_action", 32);
    {
        QAction *sep = new QAction(m_priorityGroup);
        sep->setSeparator(true);
    }
    veryHighAction->change(tr("Very High"));
    highAction->change(tr("High"));
    normalAction->change(tr("Normal"));
    lowAction->change(tr("Low"));
    veryLowAction->change(tr("Very Low"));
    customAction->change(tr("Custom"));
    connect(m_priorityGroup, SIGNAL(activated(int)), this, SLOT(changeProcPriority(int)));
    auto *priorityMenu = new QMenu();
    priorityMenu->setObjectName("MonitorMenu");
    priorityMenu->addActions(m_priorityGroup->actions());
    priorityMenu->menuAction()->setText(tr("Change Priority"));

    auto *propertiyAction = new QAction(tr("Properties"), this);
    connect(propertiyAction, &QAction::triggered, this, &ProcessTableView::showPropertiesDialog);
    
    m_contextMenu->addAction(stopAction);//停止
    m_contextMenu->addAction(continueAction);//继续进程
    m_contextMenu->addAction(endAction);//结束
    m_contextMenu->addAction(killAction);//杀死
    m_contextMenu->addSeparator();
    m_contextMenu->addMenu(priorityMenu);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(propertiyAction);

    auto *h = header();
    //connect(h, &QHeaderView::sectionResized, this, [ = ]() { saveSettings(); });
    connect(h, &QHeaderView::sectionMoved, this, [ = ]() { saveSettings(); });
    connect(h, &QHeaderView::sortIndicatorChanged, this, [ = ]() { saveSettings(); });
    connect(h, &QHeaderView::customContextMenuRequested, this,
            &ProcessTableView::displayProcessTableHeaderContextMenu);

    // header context menu
    // user action
    auto *userHeaderAction = m_headerContextMenu->addAction(tr("User"));
    userHeaderAction->setCheckable(true);
    connect(userHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::ProcessUserColumn, !b);
        saveSettings();
    });
    // diskio action
    auto *diskioHeaderAction = m_headerContextMenu->addAction(tr("Disk"));
    diskioHeaderAction->setCheckable(true);
    connect(diskioHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::ProcessDiskIoColumn, !b);
        saveSettings();
    });
    // cpu action
    auto *cpuHeaderAction = m_headerContextMenu->addAction(tr("CPU"));
    cpuHeaderAction->setCheckable(true);
    connect(cpuHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::ProcessCpuColumn, !b);
        saveSettings();
    });
    // id action
    auto *idHeaderAction = m_headerContextMenu->addAction(tr("ID"));
    idHeaderAction->setCheckable(true);
    connect(idHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::ProcessIdColumn, !b);
        saveSettings();
    });
    // flownet action
    auto *flownetHeaderAction = m_headerContextMenu->addAction(tr("Flownet Persec"));
    flownetHeaderAction->setCheckable(true);
    connect(flownetHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::ProcessFlowNetColumn, !b);
        saveSettings();
    });
    // memory action
    auto *memHeaderAction = m_headerContextMenu->addAction(tr("Memory"));
    memHeaderAction->setCheckable(true);
    connect(memHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::ProcessMemoryColumn, !b);
        saveSettings();
    });
    // priority action
    auto *priHeaderAction = m_headerContextMenu->addAction(tr("Priority"));
    priHeaderAction->setCheckable(true);
    connect(priHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::ProcessNiceColumn, !b);
        saveSettings();
    });

    // set default header context menu checkable state when settings load without success
    if (!settingsLoaded) {
        userHeaderAction->setChecked(true);
        diskioHeaderAction->setChecked(true);
        cpuHeaderAction->setChecked(true);
        idHeaderAction->setChecked(true);
        flownetHeaderAction->setChecked(true);
        memHeaderAction->setChecked(true);
        priHeaderAction->setChecked(true);
    }
    // set header context menu checkable state based on current header section's visible state before popup
    connect(m_headerContextMenu, &QMenu::aboutToShow, this, [ = ]() {
        bool b;
        b = header()->isSectionHidden(ProcessTableModel::ProcessUserColumn);
        userHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::ProcessDiskIoColumn);
        diskioHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::ProcessCpuColumn);
        cpuHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::ProcessIdColumn);
        idHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::ProcessFlowNetColumn);
        flownetHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::ProcessMemoryColumn);
        memHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::ProcessNiceColumn);
        priHeaderAction->setChecked(!b);
    });

    // on each model update, we adjust search result tip lable's visibility & positon, select the same process item before update if any
    connect(m_model, &ProcessTableModel::modelUpdated, this, [&]() {
        adjustInfoLabelVisibility();
        if (m_selectedPID.isValid()) {
            for (int i = 0; i < m_proxyModel->rowCount(); i++) {
                if (m_proxyModel->data(m_proxyModel->index(i, ProcessTableModel::ProcessIdColumn),
                                       Qt::UserRole) == m_selectedPID)
                    this->setCurrentIndex(m_proxyModel->index(i, 0));
            }
        }
    });
}

// show process table view context menu on specified positon
void ProcessTableView::displayProcessTableContextMenu(const QPoint &p)
{
    if (selectedIndexes().size() == 0 || !m_selectedPID.isValid())
        return;
    QPoint point = mapToGlobal(p);
    point.setY(point.y() + header()->sizeHint().height());
    pid_t selectPid = qvariant_cast<pid_t>(m_selectedPID);
    if (selectPid > -1) {
        sysmonitor::process::Process info = ProcessMonitor::instance()->processList()->getProcessById(selectPid);
        if (!info.isValid()) {
            m_priorityGroup->setActionsEnabled(false);
        } else {
            gint nice = info.getNice();
            int priority;
            if (nice < -7)
                priority = -20;
            else if (nice < -2)
                priority = -5;
            else if (nice < 3)
                priority = 0;
            else if (nice < 7)
                priority = 5;
            else
                priority = 19;
            m_priorityGroup->setChecked(priority);
        }
    } else {
        m_priorityGroup->setActionsEnabled(false);
    }
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

    KTableView::resizeEvent(event);
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
    int margin = 10;
    return std::max(header()->sizeHintForColumn(column) + margin * 2,
                    QTreeView::sizeHintForColumn(column) + margin * 2);
}

// adjust search result tip label's visibility & position
void ProcessTableView::adjustInfoLabelVisibility()
{
    setUpdatesEnabled(false);
    // show search not found label only when proxy model is empty & search text input is none empty
    m_notFoundLabel->setVisible(m_proxyModel->rowCount() == 0);
    // move label to center of the process table view
    if (m_notFoundLabel->isVisible())
        m_notFoundLabel->move(rect().center() - m_notFoundLabel->rect().center());
    setUpdatesEnabled(true);
}

void ProcessTableView::onSearchFocusIn()
{
    // emit stopScanProcess();
}

void ProcessTableView::onSearchFocusOut()
{
    // emit startScanProcess();
}

// load & restore table view settings from backup storage
bool ProcessTableView::loadSettings()
{
    if (m_proSettings) {
        // m_proSettings->beginGroup("PROCESS");
        // m_strFilter = m_proSettings->value("WhoseProcesses", m_strFilter).toString();
        // m_proSettings->endGroup();
        m_proSettings->beginGroup("PROCESS");
        QVariant opt = m_proSettings->value(SETTINGSOPTION_PROCESSTABLEHEADERSTATE);
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
void ProcessTableView::saveSettings()
{
    if (m_proSettings) {
        // m_proSettings->beginGroup("PROCESS");
        // m_proSettings->setValue("WhoseProcesses", m_strFilter);
        // m_proSettings->endGroup();
        QByteArray buf = header()->saveState();
        m_proSettings->beginGroup("PROCESS");
        m_proSettings->setValue(SETTINGSOPTION_PROCESSTABLEHEADERSTATE, buf.toBase64());
        m_proSettings->endGroup();
        m_proSettings->sync();
    }
}

// stop process
void ProcessTableView::stopProcesses()
{
    if (!m_selectedPID.isValid()) {
        return ;
    }
    pid_t currentPid = getpid();
    pid_t selectPid = qvariant_cast<pid_t>(m_selectedPID);

    if (selectPid != currentPid) {
        if (kill(selectPid, SIGSTOP) != 0) {
            //qDebug() << QString("Stop process %1 failed, permission denied.").arg(pid);
            QProcess process;
            process.execute(QString("pkexec %1 %2 %3 ").arg("kill").arg("-STOP").arg(selectPid));
        }
    }
}

// continue process
void ProcessTableView::continueProcesses()
{
    if (!m_selectedPID.isValid()) {
        return ;
    }
    pid_t selectPid = qvariant_cast<pid_t>(m_selectedPID);
    if (kill(selectPid, SIGCONT) != 0) {
        //qDebug() << QString("Resume process %1 failed, permission denied.").arg(pid);
        QProcess process;
        process.execute(QString("pkexec %1 %2 %3 ").arg("kill").arg("-CONT").arg(selectPid));
    }
}

// end process
void ProcessTableView::endProcesses()
{
    int error;
    if (!m_selectedPID.isValid()) {
        return ;
    }
    pid_t selectPid = qvariant_cast<pid_t>(m_selectedPID);
    error = kill(selectPid, SIGTERM);
    if(error != -1)  {
        //qDebug() << "success.....";
    }
    else {
        //need to be root
        if(errno == EPERM) {
            //qDebug() << QString("End process %1 failed, permission denied.").arg(pid);

            if (QFileInfo("/usr/bin/pkexec").exists()) {//sudo apt install policykit-1
                QProcess process;
                process.execute(QString("pkexec --disable-internal-agent %1 %2 %3").arg("kill").arg(SIGTERM).arg(selectPid));
            } else if (QFileInfo("/usr/bin/gksudo").exists()) {//sudo apt install gksu
                QProcess process;
                process.execute(QString("gksudo \"%1 %2 %3\"").arg("kill").arg(SIGTERM).arg(selectPid));
            } else if (QFileInfo("/usr/bin/gksu").exists()) {//sudo apt install gksu
                QProcess process;
                process.execute(QString("gksu \"%1 %2 %3\"").arg("kill").arg(SIGTERM).arg(selectPid));
            } else {
                qWarning() << "Failed to choose a tool to end process " << selectPid;
            }
        }
    }
}

// kill process
void ProcessTableView::killProcesses()
{
    int error;
    if (!m_selectedPID.isValid()) {
        return ;
    }
    pid_t selectPid = qvariant_cast<pid_t>(m_selectedPID);
    error = kill(selectPid, SIGTERM);
    if(error != -1)  {
        //qDebug() << "success.....";
    }
    else {
        //need to be root
        if(errno == EPERM) {
            //qDebug() << QString("End process %1 failed, permission denied.").arg(pid);

            if (QFileInfo("/usr/bin/pkexec").exists()) {//sudo apt install policykit-1
                QProcess process;
                process.execute(QString("pkexec --disable-internal-agent %1 %2 %3").arg("kill").arg(SIGTERM).arg(selectPid));
            } else if (QFileInfo("/usr/bin/gksudo").exists()) {//sudo apt install gksu
                QProcess process;
                process.execute(QString("gksudo \"%1 %2 %3\"").arg("kill").arg(SIGTERM).arg(selectPid));
            } else if (QFileInfo("/usr/bin/gksu").exists()) {//sudo apt install gksu
                QProcess process;
                process.execute(QString("gksu \"%1 %2 %3\"").arg("kill").arg(SIGTERM).arg(selectPid));
            } else {
                qWarning() << "Failed to choose a tool to end process " << selectPid;
            }
        }
    }
}

// show process properties
void ProcessTableView::showPropertiesDialog()
{
    if (!m_selectedPID.isValid()) {
        return ;
    }
    pid_t selectPid = qvariant_cast<pid_t>(m_selectedPID);
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        // Show attribute dialog if it has create, avoid create attribute dialog duplicate.
        if (qobject_cast<const ProcPropertiesDlg*>(widget) != 0) {
            ProcPropertiesDlg *dialog = qobject_cast<ProcPropertiesDlg*>(widget);
            if (dialog->getPid() == selectPid) {
                dialog->setModal(true);
                dialog->show();
                return;
            }
        }
    }

    ProcPropertiesDlg *dialog = new ProcPropertiesDlg(selectPid, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(true);
    dialog->show();
}

// show end proc dialog
void ProcessTableView::showEndProcessDialog()
{
    if (!m_selectedPID.isValid()) {
        return ;
    }
    pid_t selectPid = qvariant_cast<pid_t>(m_selectedPID);
    sysmonitor::process::Process info = ProcessMonitor::instance()->processList()->getProcessById(selectPid);
    if (!info.isValid()) {
        return;
    }
    QString strProcName = info.getDisplayName().isEmpty()?info.getProcName():info.getDisplayName();
    strProcName = getMiddleElidedText(this->font(), strProcName, 200);
    MyDialog* endProcessDialog = new MyDialog(QString(tr("End the selected process \"%1\"(PID:%2)?")).arg(strProcName).arg(selectPid), 
        QString(tr("Ending a process may destroy data, break the session or introduce a security risk. "
            "Only unresponsive processes should be ended.\nAre you sure to continue?")), this);
//    endProcessDialog->setWindowFlags(endProcessDialog->windowFlags() | Qt::WindowStaysOnTopHint);
    endProcessDialog->addButton(QString(tr("Cancel")), false);
    endProcessDialog->addButton(QString(tr("End process")), true);
    connect(endProcessDialog, &MyDialog::buttonClicked, this, &ProcessTableView::endDialogButtonClicked);
    connect(this,&ProcessTableView::closeDialog,endProcessDialog,&MyDialog::onButtonClicked);
    endProcessDialog->setAttribute(Qt::WA_DeleteOnClose);
    endProcessDialog->setModal(true);
    endProcessDialog->show();
}

// show kill proc dialog
void ProcessTableView::showKillProcessDialog()
{
    if (!m_selectedPID.isValid()) {
        return ;
    }
    pid_t selectPid = qvariant_cast<pid_t>(m_selectedPID);
    sysmonitor::process::Process info = ProcessMonitor::instance()->processList()->getProcessById(selectPid);
    if (!info.isValid()) {
        return;
    }
    QString strProcName = info.getDisplayName().isEmpty()?info.getProcName():info.getDisplayName();
    strProcName = getMiddleElidedText(this->font(), strProcName, 200);
    MyDialog* killProcessDialog = new MyDialog(QString(tr("Kill the selected process \"%1\"(PID:%2)?")).arg(strProcName).arg(selectPid), 
        QString(tr("Killing a process may destroy data, break the session or introduce a security risk. "
        "Only unresponsive processes should be killed.\nAre you sure to continue?")), this);
//    killProcessDialog->setWindowFlags(killProcessDialog->windowFlags() | Qt::WindowStaysOnTopHint);
    killProcessDialog->addButton(QString(tr("Cancel")), false);
    killProcessDialog->addButton(QString(tr("Kill process")), true);
    connect(killProcessDialog, &MyDialog::buttonClicked, this, &ProcessTableView::killDialogButtonClicked);
    connect(this,&ProcessTableView::closeDialog,killProcessDialog,&MyDialog::onButtonClicked);
    killProcessDialog->setAttribute(Qt::WA_DeleteOnClose);
    killProcessDialog->setModal(true);
    killProcessDialog->show();
}

// on end dialog btn clicked
void ProcessTableView::endDialogButtonClicked(int index, QString buttonText)
{
    emit closeDialog();
    if (index == 1) {//cancel:0   ok:1
        endProcesses();
    }
}

// on kill dialog btn clicked
void ProcessTableView::killDialogButtonClicked(int index, QString buttonText)
{
    emit closeDialog();
    if (index == 1) {//cancel:0   ok:1
        killProcesses();
    }
}

// change process priority
void ProcessTableView::changeProcPriority(int nice)
{
    //qDebug()<<"nice value"<<nice;
    if (!m_selectedPID.isValid()) {
        return ;
    }
    pid_t selectPid = qvariant_cast<pid_t>(m_selectedPID);
    if (nice == 32) {
        //show renice dialog
        if (selectPid > -1) {
            sysmonitor::process::Process info = ProcessMonitor::instance()->processList()->getProcessById(selectPid);
            if (!info.isValid()) {
                return ;
            }
            m_dlgRenice= new ReniceDialog(info.getDisplayName(), QString::number(selectPid), this);
            m_dlgRenice->loadData(info.getNice());
            connect(m_dlgRenice, &ReniceDialog::resetReniceValue, [=] (int value) {
                this->changeProcPriority(value);
            });
            m_dlgRenice->setAttribute(Qt::WA_DeleteOnClose);
            m_dlgRenice->setModal(true);
            m_dlgRenice->show();
        }
    }
    else {
        //qDebug()<<"has entered";
        if (selectPid > -1) {
            sysmonitor::process::Process info = ProcessMonitor::instance()->processList()->getProcessById(selectPid);
            if (!info.isValid() || info.getNice() == nice) {
                return;
            }
            /*
                * renice: sudo apt install bsdutils
                * Maybe: QProcess::startDetached(command)
                * QProcess::start()与QProcess::execute()都能完成启动外部程序的任务，区别在于start()是非阻塞的，而execute()是阻塞的: execute()=start()+waitforFinished()
            */
            if (QFileInfo("/usr/bin/pkexec").exists()) {//sudo apt install policykit-1
                QProcess process;
                process.execute(QString("pkexec --disable-internal-agent %1 %2 %3").arg("renice").arg(nice).arg(selectPid));
            }
            else if (QFileInfo("/usr/bin/gksudo").exists()) {//sudo apt install gksu
                QProcess process;
                process.execute(QString("gksudo \"%1 %2 %3\"").arg("renice").arg(nice).arg(selectPid));
            }
            else if (QFileInfo("/usr/bin/gksu").exists()) {//sudo apt install gksu
                QProcess process;
                process.execute(QString("gksu \"%1 %2 %3\"").arg("renice").arg(nice).arg(selectPid));
            }
            else {
                //
            }
        }
    }
}

void ProcessTableView::adjustColumnsSize()
{
    if (!model())
        return;

    if (model()->columnCount() == 0)
        return;

    // proc name
    setColumnWidth(ProcessTableModel::ProcessNameColumn, namepadding);
    // account
    setColumnWidth(ProcessTableModel::ProcessUserColumn, userpadding);
    // diskio
    setColumnWidth(ProcessTableModel::ProcessDiskIoColumn, diskpadding);
    // cpu
    setColumnWidth(ProcessTableModel::ProcessCpuColumn, cpupadding);
    // pid
    setColumnWidth(ProcessTableModel::ProcessIdColumn, idpadding);
    // flownet
    setColumnWidth(ProcessTableModel::ProcessFlowNetColumn, networkpadding);
    // memory
    setColumnWidth(ProcessTableModel::ProcessMemoryColumn, memorypadding);
    // priority
    setColumnWidth(ProcessTableModel::ProcessNiceColumn, prioritypadding);

    int rightPartsSize = userpadding + diskpadding + cpupadding + idpadding + networkpadding + memorypadding + prioritypadding;

    //set column 0 minimum width, fix header icon overlap with name issue
    if(columnWidth(0) < columnWidth(1))
        setColumnWidth(0, columnWidth(1));

    if (this->width() - rightPartsSize < namepadding) {
        int size = width() - namepadding;
        size /= header()->count() - 1;
        setColumnWidth(0, namepadding);
        for (int column = 1; column < model()->columnCount(); column++) {
            setColumnWidth(column, size);
        }
        return;
    }
    header()->resizeSection(ProcessTableModel::ProcessNameColumn, this->viewport()->width() - rightPartsSize);
}