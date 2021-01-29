/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
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

#include "../component/utils.h"
#include "../widgets/mydialog.h"
#include "../widgets/myactiongroup.h"
#include "../widgets/myactiongroupitem.h"
#include "../widgets/myaction.h"
#include "processworker.h"
#include "processlistitem.h"
#include "processlistwidget.h"
#include "monitortitlewidget.h"
#include "singleProcessNet/scanthread.h"
#include "singleProcessNet/refreshthread.h"
#include "linebandwith.h"
#include "renicedialog.h"

#include <QLabel>
#include <QMap>
#include <QMenu>
#include <QPixmap>
#include <QString>
#include <QWidget>
#include <QDialog>
#include <QTimer>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QDebug>
#include <QHeaderView>
#include <QSettings>
#include <QAbstractItemModel>

class ProcessManager;
class ProcessCategory;

class ProcessDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ProcessDialog(QList<bool> toBeDisplayedColumns, int currentSortIndex, bool isSort, QSettings *settings, QWidget* parent = 0);
    ~ProcessDialog();

    ProcessListWidget* getProcessView();
    void displayAllProcess();
    void displayActiveProcess();
    void displayCurrentUserProcess();
    void clearOriginProcList();

signals:
    void changeColumnVisible(int index, bool visible, QList<bool> columnVisible);
    void changeSortStatus(int index, bool isSort);
    void activeWhoseProcessList(int index);
    void closeDialog();
    void changeProcessNetRefresh();
    void recoverProcessNetRefresh();

public slots:
    void focusProcessView();
    void onSearch(QString text);
    void stopProcesses();
    void continueProcesses();
    void endProcesses();
    void killProcesses();
    void popupMenu(QPoint pos, QList<ProcessListItem*> items);
    void showPropertiesDialog();
    void showEndProcessDialog();
    void showKillProcessDialog();
//    void showReniceProcessDialog();
    void endDialogButtonClicked(int index, QString buttonText);
    void killDialogButtonClicked(int index, QString buttonText);
    void updateStatus(QList<ProcessListItem*> items);
    void onActiveWhoseProcess(int index);
    void changeProcPriority(int nice);
    void refreshProcessList();
    //for renewe single process net information
    void refreshLine(const QString& procname, quint64 rcv, quint64 sent, int pid, unsigned int uid, const QString& devname);
    void onSearchFocusIn();
    void onSearchFocusOut();
//    void newSpeedRefresh();


protected:
//    void paintEvent(QPaintEvent *event);

private:
    ReniceDialog *w;
    QTimer *timer = nullptr;
    QSettings *proSettings = nullptr;
    guint64 cpu_total_time;
    guint64 cpu_total_time_last;
    guint64 process_total_time;
    guint64 process_total_time_last;
    QList <int> prevCpuTime;
    QList <int> prevProcessTime;
    MyDialog *killProcessDialog = nullptr;
    MyDialog *endProcessDialog = nullptr;
    ProcessListWidget *m_processListWidget = nullptr;
    QAction *m_propertiyAction = nullptr;
    QAction *m_stopAction = nullptr;//停止
    QAction *m_continueAction = nullptr;//继续进程
    QAction *m_endAction = nullptr;//结束
    QAction *m_killAction = nullptr;//杀死
    ProcessCategory *processCategory = nullptr;
    ProcessListItem *item = nullptr;

  QMenu *m_priorityMenu;
  MyActionGroup * priorityGroup;
  MyAction *veryHighAction;
  MyAction *highAction;
  MyAction *normalAction;
  MyAction *lowAction;
  MyAction *veryLowAction;
  MyAction *customAction;

    QList<pid_t> *actionPids;
    QMenu *m_menu = nullptr;
    QString whose_processes;
    gint num_cpus;
    unsigned frequency;

    QVBoxLayout *m_layout = nullptr;
    QHBoxLayout *m_categoryLayout = nullptr;

    ScanThread *scanThread = nullptr;
    RefreshThread *refreshThread = nullptr;
    pid_t haveNetPid;
    QMap<int,QString> pidMap;
    QMap<long long int,long long int> flowNetPrevMap;
    QMap<long long int,long long int> calDiskIoMap;
    QString addFlowNetPerSec;
    QString addDiskIoPerSec;
    long long int numAddFlowNetPerSec;
    lineBandwith *speedLineBandFlowNet;
    lineBandwith *speedLineBandDiskIo;

    long long int disk_io_bytes_total;
//    guint64 disk_read_bytes_current;
//    guint64 disk_write_bytes_current;
};
