/*
 * mainwindow.cpp
 *
 * Copyright (c) 2016 Stephane List
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTableWidget>
#include <QFile>
#include <QMessageBox>
#include <QtDebug>

#include <QPushButton>

//#include <QMetaType>



#include <qpushbuttonarrow.h>

// for geteuid
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "scanthread.h"
#include "linebandwith.h"
#include "smallchart.h"

// Table 1
enum {
    COLUMN_PID = 0,
    COLUMN_USER_NAME,
    COLUMN_PROC_NAME,
    COLUMN_PROC_STATUS,
    COLUMN_RCV,
    COLUMN_SENT,
    COLUMN_RCV_PER_S,
    COLUMN_SENT_PER_S,
    COLUMN_DEV_NAME,
    COLUMN_MORE,
    COLUMN_LAST
};

// Table 2
enum {
    COLUMN_CNX_SIP = 0,
    COLUMN_CNX_SPORT,
    COLUMN_CNX_DIP,
    COLUMN_CNX_DPORT,
    COLUMN_CNX_LOOKUP,
    COLUMN_CNX_LAST
};

Q_DECLARE_METATYPE(QHostAddress)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tableWidget_2->hide();
    ui->pushButton_back->hide();

    total_speed_recv = 0;
    total_speed_sent = 0;

    ui->centralWidget->layout()->removeWidget(ui->lineEdit);
    SmallChart * s = new SmallChart(this);
    ui->centralWidget->layout()->addWidget(s);
    ui->centralWidget->layout()->addWidget(ui->lineEdit);

//    if (geteuid() != 0) {
//        ui->lineEdit->setText("You need to be root to run qNetHogs!");
//        return;
//    }

    scanThread = new ScanThread(this);
    scanThread->start(QThread::TimeCriticalPriority);

    refreshThread = new RefreshThread(this);
    refreshThread->start(QThread::HighPriority);

    connect(refreshThread, SIGNAL(procDetected(const QString &, quint64 , quint64 , int , unsigned int , const QString&)),
            this, SLOT(refreshLine(const QString &, quint64 , quint64 , int, unsigned int , const QString&)));

    connect(refreshThread, SIGNAL(procDeleted(int)),
            this, SLOT(deleteLine(int)));

    connect(refreshThread, SIGNAL(refresh_finished()),
            this, SLOT(updateGraph()));

    connect(refreshThread, SIGNAL(cnxDetected(const QHostAddress&, quint16, const QHostAddress&, quint16)),
            this, SLOT(cnxDetected(const QHostAddress&,quint16,const QHostAddress&,quint16)));

    connect(this, SIGNAL(new_speed(quint64,quint64)),
            s, SLOT(add_points(quint64,quint64)));

    connect(this, SIGNAL(reset_graph()),
            s, SLOT(reset()));

    ui->lineEdit->setText("正在运行!");





    qRegisterMetaType<QHostAddress>();


    //        int id = qRegisterMetaType<MyStruct>();



}

MainWindow::~MainWindow()
{
    delete ui;
    ui = nullptr;
    if (scanThread) {
        scanThread->stop();
        scanThread->wait();
    }
}

int MainWindow::searchRow(int pid)
{
    for (int i=0; i< ui->tableWidget->rowCount();i++) {
        if (ui->tableWidget->item(i, COLUMN_PID) && (ui->tableWidget->item(i, COLUMN_PID)->text().toInt() == pid)) {
            return i;
        }
    }
    return -1;
}

QString MainWindow::searchIcon(const QString & procname)
{
    int i;
    QString s,filename;

    if (procname.startsWith("/usr/lib/apt/methods/")) {
        if (QFile::exists("/usr/share/icons/hicolor/48x48/status/aptdaemon-download.png")) {
            return "/usr/share/icons/hicolor/48x48/status/aptdaemon-download.png";
        }
    }

    if (procname =="ssh") {
        if (QFile::exists("/usr/share/seahorse/icons/hicolor/48x48/apps/seahorse-key-ssh.png")) {
            return "/usr/share/seahorse/icons/hicolor/48x48/apps/seahorse-key-ssh.png";
        }
    }

    // remove path
    i= procname.lastIndexOf("/");
    if (i>0) {
        filename = procname.mid(i+1);
    } else {
        filename = procname;
    }
    s = filename;

    if (s == "gnome-software") {
        if (QFile::exists("/usr/share/icons/hicolor/48x48/apps/org.gnome.Software.png")) {
            return "/usr/share/icons/hicolor/48x48/apps/org.gnome.Software.png";
        }
    }

    if (s == "snapd") {
        if (QFile::exists("/usr/share/app-install/icons/snappy.png")) {
            return "/usr/share/app-install/icons/snappy.png";
        }
    }

    // remove end of prog after a '-' for example transmission-gtk gnome-software ...
    i= s.lastIndexOf("-");
    if (i>0) {
        filename = s.left(i);
    } else {
        filename = s;
    }

    s = filename;
    s.prepend("/usr/share/pixmaps/");
    s.append(".png");
    if (QFile::exists(s)) {
        return s;
    }

    s = filename;
    s.prepend("/usr/share/pixmaps/");
    s.append(".xpm");
    if (QFile::exists(s)) {
        return s;
    }

    s = filename;
    s.prepend("/usr/share/icons/hicolor/48x48/apps/");
    s.append(".png");
    if (QFile::exists(s)) {
        return s;
    }
    return NULL;
}

QString MainWindow::uid2username(uid_t uid)
{
    struct passwd * pwd = NULL;

    pwd = getpwuid(uid);
    if (pwd == NULL) {
        return "No name";
    }
    return QString(pwd->pw_name);
}

void MainWindow::updateGraph()
{
    emit new_speed(total_speed_recv,total_speed_sent);

    total_speed_recv = 0;
    total_speed_sent = 0;
}

void MainWindow::deleteLine(int pid)
{
    int row = searchRow(pid);

    QTableWidgetItem *p = new QTableWidgetItem("finished");
    p->setIcon(QIcon(":/red.ico"));
    ui->tableWidget->setItem(row, COLUMN_PROC_STATUS, p);
}


void MainWindow::refreshLine(const QString& procname, quint64 recv, quint64 sent, int pid, unsigned int uid, const QString& devname)
{
    int row;
    QString s, iconfile;
    bool new_row = false;

    if (pid == 0)
        return;

    row = searchRow(pid);
    if (row == -1) {
        ui->tableWidget->insertRow(0);
        row = 0;
        new_row = true;
    }

    if (new_row)
    {
        QTableWidgetItem *pname = new QTableWidgetItem(procname);
        iconfile = searchIcon(procname);
        if (iconfile != NULL) {
            pname->setIcon(QIcon(iconfile));
        }

        pname->setTextAlignment(Qt::AlignVCenter);
        ui->tableWidget->setItem(row, COLUMN_PROC_NAME, pname);

        s.setNum(recv);
        QTableWidgetItem *prcv = new QTableWidgetItem(s);
        prcv->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
        ui->tableWidget->setItem(row, COLUMN_RCV_PER_S, prcv );

        s.setNum(sent);
        QTableWidgetItem *psent = new QTableWidgetItem(s);
        psent->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
        ui->tableWidget->setItem(row, COLUMN_SENT_PER_S, psent);

        s.setNum(pid);
        QTableWidgetItem *ppid = new QTableWidgetItem(s);
        ppid->setTextAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
        ui->tableWidget->setItem(row, COLUMN_PID, ppid);

        QTableWidgetItem *pdevname = new QTableWidgetItem(devname);
        pdevname->setTextAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
        ui->tableWidget->setItem(row, COLUMN_DEV_NAME, pdevname);

        QTableWidgetItem *pusername = new QTableWidgetItem(uid2username(uid));
        pusername->setTextAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
        ui->tableWidget->setItem(row, COLUMN_USER_NAME, pusername);

        QTableWidgetItem *p = new QTableWidgetItem("正在运行");
        p->setIcon(QIcon(":/green.ico"));
        ui->tableWidget->setItem(row, COLUMN_PROC_STATUS, p);

        lineBandwith* br = new lineBandwith(this);
        ui->tableWidget->setCellWidget(row,COLUMN_RCV_PER_S,br);
        total_speed_recv +=br->new_count(recv);

        lineBandwith* bs = new lineBandwith(this);
        ui->tableWidget->setCellWidget(row,COLUMN_SENT_PER_S,bs);
        total_speed_sent += bs->new_count(sent);

        QPushButtonArrow* b = new QPushButtonArrow(this);
        b->pid = pid;
        ui->tableWidget->setCellWidget(row, COLUMN_MORE, b);
        connect(b, SIGNAL(display_cnx(int)), this, SLOT(update_cnx_list(int)));
        connect(b, SIGNAL(display_cnx(int)), refreshThread, SLOT(update_cnx_list(int)));

    } else {
        lineBandwith *br = qobject_cast<lineBandwith*>(ui->tableWidget->cellWidget(row, COLUMN_RCV_PER_S));
        total_speed_recv +=br->new_count(recv);

        lineBandwith *bs = qobject_cast<lineBandwith*>(ui->tableWidget->cellWidget(row, COLUMN_SENT_PER_S));
        total_speed_sent += bs->new_count(sent);
    }

    s.setNum(recv);
    QTableWidgetItem *prcv = new QTableWidgetItem(s);
    prcv->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
    ui->tableWidget->setItem(row, COLUMN_RCV, prcv);

    s.setNum(sent);
    QTableWidgetItem *psent = new QTableWidgetItem(s);
    psent->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
    ui->tableWidget->setItem(row, COLUMN_SENT, psent);

    for (int i=0; i<COLUMN_LAST; i++) {
        ui->tableWidget-> resizeColumnToContents(i);
    }
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "关于进程网络监控工具", "进程网络监控工具\n在GPL v3下是一款免费的软件!");
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionReset_triggered()
{
    emit reset_graph();
}

void MainWindow::update_cnx_list(int pid)
{
    ui->tableWidget_2->clearContents();
    ui->tableWidget_2->setRowCount(0);
    ui->tableWidget_2->show();
    ui->pushButton_back->show();
}

void MainWindow::cnxDetected(const QHostAddress& sourceip, quint16 sport, const QHostAddress& destip, quint16 dport)
{
     for (int i=0; i< ui->tableWidget_2->rowCount();i++) {
        if ((ui->tableWidget_2->item(i, COLUMN_CNX_SPORT)->text().toInt() == sport) &&
                (ui->tableWidget_2->item(i, COLUMN_CNX_DPORT)->text().toInt() == dport))
            return; // cnx already in table

    }

    // if not found add line
    ui->tableWidget_2->insertRow(0);

    QTableWidgetItem *psip = new QTableWidgetItem(sourceip.toString());
    ui->tableWidget_2->setItem(0, COLUMN_CNX_SIP, psip);

    QTableWidgetItem *pdip = new QTableWidgetItem(destip.toString());
    ui->tableWidget_2->setItem(0, COLUMN_CNX_DIP, pdip);

    QTableWidgetItem *psport = new QTableWidgetItem(QString::number(sport));
    ui->tableWidget_2->setItem(0, COLUMN_CNX_SPORT, psport);

    QTableWidgetItem *pdport = new QTableWidgetItem(QString::number(dport));
    ui->tableWidget_2->setItem(0, COLUMN_CNX_DPORT, pdport);

    for (int i=0; i<COLUMN_CNX_LAST; i++) {
        ui->tableWidget_2-> resizeColumnToContents(i);
    }

    QHostInfo::lookupHost(destip.toString(), this, SLOT(lookedUp(QHostInfo)));
}


void MainWindow::lookedUp(const QHostInfo &host)
{
    if (host.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << host.errorString();
        return;
    }

//    qDebug() << "host:" << host.hostName();

    foreach (const QHostAddress &address, host.addresses()) {
//        qDebug() << "Found address:" << address.toString();
        for (int row=0; row< ui->tableWidget_2->rowCount();row++) {
            if ((ui->tableWidget_2->item(row, COLUMN_CNX_DIP)->text() == address.toString())) {
                QTableWidgetItem *pname = new QTableWidgetItem(host.hostName());
                ui->tableWidget_2->setItem(row, COLUMN_CNX_LOOKUP, pname);
                //return;
            }
        }
    }
    ui->tableWidget_2-> resizeColumnToContents(COLUMN_CNX_LOOKUP);
}

void MainWindow::on_pushButton_back_clicked()
{
    ui->tableWidget_2->hide();
    ui->pushButton_back->hide();
}
