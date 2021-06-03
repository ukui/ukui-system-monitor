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

#include "systemmonitor.h"
#include "../shell/macro.h"
#include "gui/ktableview.h"
#include "gui/processtableview.h"
#include "process/process_monitor.h"

#include "util.h"
#include <QFileSystemWatcher>
#include <QLabel>
#include <QDebug>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QFile>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QPainterPath>
#include <X11/Xlib.h>
#include <QDBusArgument>
//#include <KWindowSystem/NETWM>
#include <KWindowEffects>
#include <QTreeView>
#include <QFileSystemModel>
#include <QStandardItemModel>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

void  SystemMonitor::sltMessageReceived(const QString &msg)
{
    showNormal();
}

SystemMonitor::SystemMonitor(QWidget *parent)
    : QFrame(parent)
    , mousePressed(false)
    , opacitySettings(nullptr)
{
    this->setProperty("useStyleWindowManager",false);
    this->setObjectName("SystemMonitor");
    this->setAttribute(Qt::WA_TranslucentBackground);//背景透明
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    const QByteArray idtrans(THEME_QT_TRANS);

    if(QGSettings::isSchemaInstalled(idtrans))
    {
        opacitySettings = new QGSettings(idtrans);
    }

    getOsRelease();

    this->setAutoFillBackground(true);
    this->setWindowTitle(tr("Kylin System Monitor"));
    //this->setWindowIcon(QIcon::fromTheme("ukui-system-monitor")); //control show img in panel
    this->resize(MAINWINDOWWIDTH,MAINWINDOWHEIGHT);
    setMinimumSize(MAINWINDOWWIDTH, MAINWINDOWHEIGHT);  //set the minimum size of the mainwindow

    proSettings = new QSettings(UKUI_COMPANY_SETTING, UKUI_SETTING_FILE_NAME_SETTING, this);
    proSettings->setIniCodec("UTF-8");

    this->initTitleWidget();
    this->initPanelStack();
    this->initConnections();
    connect(m_titleWidget,SIGNAL(changeProcessItemDialog(int)),newProcessDialog,SLOT(onChangeProcessFilter(int)));  //配置文件中为whoseprocess赋值
    connect(m_titleWidget,SIGNAL(SearchFocusIn()),newProcessDialog,SLOT(onSearchFocusIn()));
    connect(m_titleWidget,SIGNAL(SearchFocusOut()),newProcessDialog,SLOT(onSearchFocusOut()));
    newProcessDialog->onChangeProcessFilter(m_titleWidget->whichProcType());
    getTransparentData();
    this->moveCenter();
    qDebug()<<"--+--"<<version;
}

void SystemMonitor::getTransparentData()
{
    if (!opacitySettings)
    {
        m_transparency = 0.9;
        return;
    }

    connect(opacitySettings, &QGSettings::changed, this, [=](const QString &key)
    {
        if (key == "transparency")
        {
            QStringList keys = opacitySettings->keys();
            if (keys.contains("transparency"))
            {
                m_transparency = opacitySettings->get("transparency").toString().toDouble();
            }
        }
        repaint();
    });

    QStringList keys = opacitySettings->keys();
    if(keys.contains("transparency"))
    {
        m_transparency = opacitySettings->get("transparency").toString().toDouble();
    }
}

SystemMonitor::~SystemMonitor()
{
    if (opacitySettings){
        delete opacitySettings;
        opacitySettings = nullptr;
    }
    if (styleSettings) {
        delete styleSettings;
        styleSettings = nullptr;
    }
    //qDebug()<<"SystemMonitor Destroyed!!";
}

void SystemMonitor::paintEvent(QPaintEvent *event)
{
    QPainterPath path;

    QPainter painter(this);
    painter.setOpacity(1);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    path.addRect(this->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().base());
    painter.setPen(Qt::transparent);

    painter.drawPath(path);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QFrame::paintEvent(event);
}

void SystemMonitor::resizeEvent(QResizeEvent *e)
{
    if (m_titleWidget) {
        m_titleWidget->resize(width() - 2, MONITOR_TITLE_WIDGET_HEIGHT);
    }
    if (m_sysMonitorStack) {
        m_sysMonitorStack->resize(width() - 2, this->height() - MONITOR_TITLE_WIDGET_HEIGHT - 2);
        m_sysMonitorStack->move(1, MONITOR_TITLE_WIDGET_HEIGHT + 1);
    }
    //qDebug()<<"m_titleWid.x"<<m_titleWidget->geometry();
}

void SystemMonitor::recordProcessVisibleColumn(int, bool, QList<bool> columnVisible)
{
    QList<QString> m_visibleColumns;
    m_visibleColumns << "name";

    if (columnVisible[1]) {
        m_visibleColumns << "user";
    }

//    if (columnVisible[2]) {
//        m_visibleColumns << "status";
//    }

    if (columnVisible[2]) {
        m_visibleColumns << "disk";
    }

    if (columnVisible[3]) {
        m_visibleColumns << "cpu";
    }

    if (columnVisible[4]) {
        m_visibleColumns << "pid";
    }

    if (columnVisible[5]) {
        m_visibleColumns << "flownetpersec";
    }

    if (columnVisible[6]) {
        m_visibleColumns << "memory";
    }

    if (columnVisible[7]) {
        m_visibleColumns << "priority";
    }

    QString displayedColumns = "";
    for (int i = 0; i < m_visibleColumns.length(); i++) {
        if (i != m_visibleColumns.length() - 1) {
            displayedColumns += QString("%1,").arg(m_visibleColumns[i]);
        } else {
            displayedColumns += m_visibleColumns[i];
        }
    }

    proSettings->beginGroup("PROCESS");
    proSettings->setValue("DisplayedColumns", displayedColumns);
    proSettings->endGroup();
    proSettings->sync();
}

void SystemMonitor::recordSortStatus(int index, bool isSort)
{
    QList<QString> columnNames = { "name", "user", "disk", "cpu", "pid", "flownetpersec", "memory", "priority"};

    proSettings->beginGroup("PROCESS");
    proSettings->setValue("CurrentSortColumn", columnNames[index]);
    proSettings->setValue("IsSort", isSort);
    proSettings->endGroup();
    proSettings->sync();
}

void SystemMonitor::recordFileSysVisibleColumn(int, bool, QList<bool> columnVisible)
{
    QList<QString> m_visibleColumns;
    m_visibleColumns << "device";

    if (columnVisible[1]) {
        m_visibleColumns << "directory";
    }

    if (columnVisible[2]) {
        m_visibleColumns << "type";
    }

    if (columnVisible[3]) {
        m_visibleColumns << "total";
    }

    if (columnVisible[4]) {
        m_visibleColumns << "free";
    }

    if (columnVisible[5]) {
        m_visibleColumns << "available";
    }

    if (columnVisible[6]) {
        m_visibleColumns << "used";
    }

    QString displayedColumns = "";
    for (int i = 0; i < m_visibleColumns.length(); i++) {
        if (i != m_visibleColumns.length() - 1) {
            displayedColumns += QString("%1,").arg(m_visibleColumns[i]);
        } else {
            displayedColumns += m_visibleColumns[i];
        }
    }

    proSettings->beginGroup("FileSystem");
    proSettings->setValue("DisplayedColumns", displayedColumns);
    proSettings->endGroup();
    proSettings->sync();
}

void SystemMonitor::initPanelStack()
{
    m_sysMonitorStack = new QStackedWidget(this);
    m_sysMonitorStack->resize(width() - 2, this->height() - TITLE_WIDGET_HEIGHT);
//    m_sysMonitorStack->move(1, TITLE_WIDGET_HEIGHT);
    m_sysMonitorStack->installEventFilter(this);

    newProcessDialog = new ProcessTableView(proSettings);
    newProcessDialog->installEventFilter(this);

    resources_dialog = new NewResouresDialog;

    filesystemView = new FileSystemTableView(proSettings);
    filesystemView->installEventFilter(this);

    m_sysMonitorStack->addWidget(newProcessDialog);
    m_sysMonitorStack->addWidget(resources_dialog);
    m_sysMonitorStack->addWidget(filesystemView);

    m_sysMonitorStack->setCurrentWidget(newProcessDialog);
}

void SystemMonitor::initTitleWidget()
{
    m_titleWidget = new MonitorTitleWidget(proSettings, this);
    m_titleWidget->resize(width() - 2, MONITOR_TITLE_WIDGET_HEIGHT);
    m_titleWidget->move(1, 1);
    connect(m_titleWidget,SIGNAL(maximizeWindow()),this,SLOT(onMaximizeWindow()));
    connect(m_titleWidget,SIGNAL(minimizeWindow()),this,SLOT(onMinimizeWindow()));
}

void SystemMonitor::onMaximizeWindow()
{
    if (this->isMaximized()) {
        this->showNormal();
    }
    else {
        this->showMaximized();
    }
}

void SystemMonitor::onMinimizeWindow()
{
    this->showMinimized();
}

void SystemMonitor::initConnections()
{
    connect(m_titleWidget, SIGNAL(changePage(int)), this, SLOT(onChangePage(int)));
    connect(m_titleWidget, SIGNAL(canelSearchEditFocus()), newProcessDialog, SLOT(focusProcessView()));
    connect(m_titleWidget, SIGNAL(searchSignal(QString)), newProcessDialog, SLOT(onSearch(QString)), Qt::QueuedConnection);
}

void SystemMonitor::onChangePage(int index)
{
    if (m_sysMonitorStack) {
        m_sysMonitorStack->setCurrentIndex(index);
    }
}

int SystemMonitor::getCurrentSortColumnIndex()
{
    proSettings->beginGroup("PROCESS");
    QString currentSortColumn = proSettings->value("CurrentSortColumn").toString();
    proSettings->endGroup();

    QList<QString> columnNames = {"name", "user", "disk", "cpu", "pid", "flownetpersec", "memory", "priority"};

    return columnNames.indexOf(currentSortColumn);
}

bool SystemMonitor::isSortOrNot()
{
    proSettings->beginGroup("PROCESS");
    bool value = proSettings->value("IsSort", true).toBool();
    proSettings->endGroup();

    return value;
}

QList<bool> SystemMonitor::getReadyDisplayProcessColumns()
{
    proSettings->beginGroup("PROCESS");
    QString displayedColumns = proSettings->value("DisplayedColumns", "name,user,disk,cpu,pid,flownetpersec,memory,priority").toString();
    proSettings->endGroup();

    if (displayedColumns.isEmpty()) {
        proSettings->beginGroup("PROCESS");
        displayedColumns = "name,user,disk,cpu,pid,flownetpersec,memory,priority";
        proSettings->setValue("DisplayedColumns", displayedColumns);
        proSettings->endGroup();
        proSettings->sync();
    }

    QList<bool> m_shows;
    m_shows << displayedColumns.contains("name");
    m_shows << displayedColumns.contains("user");
    m_shows << displayedColumns.contains("disk");
    m_shows << displayedColumns.contains("cpu");
    m_shows << displayedColumns.contains("pid");
    m_shows << displayedColumns.contains("flownetpersec");
    m_shows << displayedColumns.contains("memory");
    m_shows << displayedColumns.contains("priority");

    return m_shows;
}

QList<bool> SystemMonitor::getReadyDisplayFileSysColumns()
{
    proSettings->beginGroup("FileSystem");
    QString displayedColumns = proSettings->value("DisplayedColumns", "device,directory,type,total,free,available,used").toString();
    proSettings->endGroup();

    if (displayedColumns.isEmpty()) {
        proSettings->beginGroup("FileSystem");
        displayedColumns = "device,directory,type,total,free,available,used";
        proSettings->setValue("DisplayedColumns", displayedColumns);
        proSettings->endGroup();
        proSettings->sync();
    }

    QList<bool> m_shows;
    m_shows << displayedColumns.contains("device");
    m_shows << displayedColumns.contains("directory");
    m_shows << displayedColumns.contains("type");
    m_shows << displayedColumns.contains("total");
    m_shows << displayedColumns.contains("free");
    m_shows << displayedColumns.contains("available");
    m_shows << displayedColumns.contains("used");

    return m_shows;
}

void SystemMonitor::moveCenter()
{
    QPoint pos = QCursor::pos();
    QRect primaryGeometry;
    for (QScreen *screen : qApp->screens()) {
        if (screen->geometry().contains(pos)) {
            primaryGeometry = screen->geometry();
        }
    }

    if (primaryGeometry.isEmpty()) {
        primaryGeometry = qApp->primaryScreen()->geometry();
    }

    this->move(primaryGeometry.x() + (primaryGeometry.width() - this->width())/2,
               primaryGeometry.y() + (primaryGeometry.height() - this->height())/2);
}

void SystemMonitor::getOsRelease()
{
    QFile file("/etc/lsb-release");
    if (!file.open(QIODevice::ReadOnly)) qDebug() << "Read file Failed.";
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        if (str.contains("DISTRIB_ID")){
            version=str.remove("DISTRIB_ID=");
            version=str.remove("\n");
        }
    }
}

void SystemMonitor::closeEvent(QCloseEvent *event)
{
    event->accept();
}

/*void SystemMonitor::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    //绘制背景色
//    QPainterPath path;
//    path.addRect(QRectF(rect()));
//    painter.setOpacity(1);
//    painter.fillPath(path, QColor("#FFFFFF"));


    painter.setPen(QPen(QColor("#0d87ca"), 0));//边框颜色 #3f96e4
    painter.setBrush(QColor("#e9eef0"));//背景色
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    QRectF r(0 / 2.0, 0 / 2.0, width() - 0, height() - 0);//左边 上边 右边 下边
    painter.drawRoundedRect(r, 4, 4);

    QFrame::paintEvent(event);
}*/

void SystemMonitor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->dragPosition = event->globalPos() - frameGeometry().topLeft();
        this->mousePressed = true;
    }
    QFrame::mousePressEvent(event);
}

void SystemMonitor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->mousePressed = false;
    }

    QFrame::mouseReleaseEvent(event);
}

void SystemMonitor::mouseMoveEvent(QMouseEvent *event)
{
    if (this->mousePressed) {
        move(event->globalPos() - this->dragPosition);
    }

    QFrame::mouseMoveEvent(event);
}


void SystemMonitor::keyPressEvent(QKeyEvent *event)
{
    if(version != "Ubuntu")
    {
        if(event->key() == Qt::Key_F1)
        {
            if(!daemonIsNotRunning())
            {
                showGuide(qAppName());
            }
        }
    }
}

int SystemMonitor::daemonIsNotRunning()
{
    QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.isConnected())
        return 0;

    QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", service_name);
    //qDebug()<<"reply name"<<reply;
    return reply.value() == "";
}

void SystemMonitor::showGuide(QString appName)
{

    //qDebug() << Q_FUNC_INFO << appName;

    QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());

    QDBusInterface *interface = new QDBusInterface(service_name,
                                                       KYLIN_USER_GUIDE_PATH,
                                                       KYLIN_USER_GUIDE_INTERFACE,
                                                       QDBusConnection::sessionBus(),
                                                       this);

    QDBusMessage msg = interface->call(QStringLiteral("showGuide"),"kylin-system-monitor");
    if (interface) {
        delete interface;
        interface = nullptr;
    }
}
