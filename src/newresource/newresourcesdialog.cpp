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

#include "newresourcesdialog.h"
#include "util.h"

#include <glibtop/netload.h>
#include <glibtop/netlist.h>
#include <QDebug>
#include <QTimer>

//get cpu history data
static unsigned long long getCpuTimeData(unsigned long long &workTime);

unsigned long long getCpuTimeData(unsigned long long &workTime)
{
    FILE *file = fopen("/proc/stat", "r");
    if (file == NULL) {
        return 0;
    }

    char buffer[1024] = {0};
    unsigned long long user = 0, nice = 0, system = 0, idle = 0;
    unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guestnice = 0;

    char* ret = fgets(buffer, sizeof(buffer) - 1, file);
    if (ret == NULL) {
        fclose(file);
        return 0;
    }
    fclose(file);

    sscanf(buffer, "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestnice);
    workTime = user + nice + system;

    return user + nice + system + idle + iowait + irq + softirq + steal;
}

//get swap and memory data

inline QString formatMemory(guint64 size)
{
    enum {
        K_INDEX,
        M_INDEX,
        G_INDEX,
        T_INDEX
    };
    QList<guint64> factorList;
    factorList.append(G_GUINT64_CONSTANT(1) << 10);//KiB
    factorList.append(G_GUINT64_CONSTANT(1) << 20);//MiB
    factorList.append(G_GUINT64_CONSTANT(1) << 30);//GiB
    factorList.append(G_GUINT64_CONSTANT(1) << 40);//TiB

    if (size < factorList.at(K_INDEX))
    {
        if ((guint) size > 1)
        {
            return QString("%1 %2").arg((guint) size).arg(QObject::tr("bit"));
        }
        else
        {
            return QString("%1 %2").arg((guint) size).arg(QObject::tr("bits"));
        }
    }
    else
    {
        guint64 factor;
        QString format;
        if (size < factorList.at(M_INDEX))
        {
            factor = factorList.at(K_INDEX);
            format = QObject::tr("KiB");
        }
        else if (size < factorList.at(G_INDEX))
        {
            factor = factorList.at(M_INDEX);
            format = QObject::tr("MiB");
        }
        if (size < factorList.at(T_INDEX))
        {
            factor = factorList.at(G_INDEX);
            format = QObject::tr("GiB");
        }
        else
        {
            factor = factorList.at(T_INDEX);
            format = QObject::tr("TiB");
        }
        std::string formatted_result(make_string(g_strdup_printf("%.1f", size / (double)factor)));
        return QString::fromStdString(formatted_result) + format;
    }
}

void NewResouresDialog::onUpdateMemoryAndSwapStatus()
{
    glibtop_mem mem;
    glibtop_swap swap;

    glibtop_get_mem(&mem);
    glibtop_get_swap(&swap);

    float swappercent = (swap.total ? (float)swap.used / (float)swap.total : 0.0f);
    float mempercent  = (float)mem.user  / (float)mem.total;

    mi.percent = mempercent * 100;
    mi.swappercent = swappercent * 100;

    //初始单位为字节，需要修正
    mi.user = mem.user;
    mi.total = mem.total;

    mi.swapused = swap.used;
    mi.swaptotal = swap.total;

    const QString infoMemory = QString("%1/%2").arg(formatMemory(mi.user)).arg(formatMemory(mi.total));
    const QString infoSwap = QString("%1/%2").arg(formatMemory(mi.swapused)).arg(formatMemory(mi.swaptotal));
    emit rebackMemoryAndSwapInfo(infoMemory, mi.percent,infoSwap,mi.swappercent);
    emit rebackMemoryAndSwapData(mi.user,mi.percent,mi.swapused,mi.swappercent);
//    emit rebackSwapInfo(infoSwap,mi.swappercent);

    repaint();//this->update();
}

//get network data

typedef struct _net
{
    guint64 last_in, last_out;
    GTimeVal time;
} net;
net netinfo;

inline void getNetworkBytesData(unsigned long long int &receiveBytes, unsigned long long int &sendBytes, unsigned long long int &receiveRateBytes, unsigned long long int &sendRateBytes)
{
    glibtop_netlist netlist;
    char **ifnames;
    guint32 i;
    guint64 in = 0, out = 0;
    GTimeVal time;
    guint64 din, dout;
    ifnames = glibtop_get_netlist(&netlist);

    for (i = 0; i < netlist.number; ++i) {
        glibtop_netload netload;
        glibtop_get_netload(&netload, ifnames[i]);

        if (netload.if_flags & (1 << GLIBTOP_IF_FLAGS_LOOPBACK))
            continue;

        /* Skip interfaces without any IPv4/IPv6 address (or
           those with only a LINK ipv6 addr) However we need to
           be able to exclude these while still keeping the
           value so when they get online (with NetworkManager
           for example) we don't get a suddent peak.  Once we're
           able to get this, ignoring down interfaces will be
           possible too.  */
        if (not (netload.flags & (1 << GLIBTOP_NETLOAD_ADDRESS6)
                 and netload.scope6 != GLIBTOP_IF_IN6_SCOPE_LINK)
            and not (netload.flags & (1 << GLIBTOP_NETLOAD_ADDRESS)))
            continue;

        /* Don't skip interfaces that are down (GLIBTOP_IF_FLAGS_UP)
           to avoid spikes when they are brought up */

        in  += netload.bytes_in;
        out += netload.bytes_out;
    }

    g_strfreev(ifnames);

    g_get_current_time(&time);

    if (in >= netinfo.last_in && out >= netinfo.last_out && netinfo.time.tv_sec != 0) {
        float dtime;
        dtime = time.tv_sec - netinfo.time.tv_sec +
                (double) (time.tv_usec - netinfo.time.tv_usec) / G_USEC_PER_SEC;
        din   = static_cast<guint64>((in  - netinfo.last_in)  / dtime);
        dout  = static_cast<guint64>((out - netinfo.last_out) / dtime);
    } else {
        /* Don't calc anything if new data is less than old (interface
           removed, counters reset, ...) or if it is the first time */
        din  = 0;
        dout = 0;
    }

    netinfo.last_in  = in;
    netinfo.last_out = out;
    netinfo.time     = time;

    receiveBytes = in;
    sendBytes = out;
    receiveRateBytes = din;
    sendRateBytes = dout;
}

inline QString formatNetworkBrandWidth(guint64 size, bool isTotal)
{
    enum {
        K_INDEX,
        M_INDEX,
        G_INDEX,
        T_INDEX
    };

    QList<guint64> factorList;
    factorList.append(G_GUINT64_CONSTANT(1) << 10);//KiB
    factorList.append(G_GUINT64_CONSTANT(1) << 20);//MiB
    factorList.append(G_GUINT64_CONSTANT(1) << 30);//GiB
    factorList.append(G_GUINT64_CONSTANT(1) << 40);//TiB

    if (size < factorList.at(K_INDEX)) {
        if ((guint) size > 1) {
                return QString("%1 %2").arg((guint) size).arg(QObject::tr("bits/s"));
        }
        else {
                return QString("%1 %2").arg((guint) size).arg(QObject::tr("bits/s"));
        }
    } else {
        guint64 factor;
        QString format;
        if (size < factorList.at(M_INDEX)) {
            factor = factorList.at(K_INDEX);
            if (isTotal)
                format = QObject::tr("KiB");
            else
                format = QObject::tr("KiB/s");
        }else if (size < factorList.at(G_INDEX)) {
            factor = factorList.at(M_INDEX);
            if (isTotal)
                format = QObject::tr("MiB");
            else
                format = QObject::tr("MiB/s");
        } else if (size < factorList.at(T_INDEX)) {
            factor = factorList.at(G_INDEX);
            if (isTotal)
                format = QObject::tr("GiB");
            else
                format = QObject::tr("GiB/s");
        } else {
            factor = factorList.at(T_INDEX);
            if (isTotal)
                format = QObject::tr("TiB");
            else
                format = QObject::tr("TiB/s");
        }
        std::string formatted_result(make_string(g_strdup_printf("%.1f", size / (double)factor)));
        return QString::fromStdString(formatted_result) + format;
    }
}

inline QString formatNetworkRate(guint64 rate)
{
    return formatNetworkBrandWidth(rate, false);
}

NewResouresDialog::NewResouresDialog(QWidget *parent)
      :QWidget(parent)
{
    cpuUnitDataLabel = new QLabel;
    memoryUnitDataLabel = new QLabel;
    swapUnitDataLabel = new QLabel;
    netrecvUnitDataLabel = new QLabel;
    netsentUnitDataLabel = new QLabel;
    pe.setColor(QPalette::WindowText,QColor(13,14,13,130));

    const QByteArray id(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(id))
    {
        fontSettings = new QGSettings(id);
    }

    initFontSize();
    initWidget();
    setChangeNetSpeedLabel();
    tosetFontSize();

    updateStatusTimer = new QTimer(this);
    connect(updateStatusTimer, SIGNAL(timeout()), this, SLOT(updateResourceStatus()));
//cpu history conecct
    connect(this, SIGNAL(updateCpuStatus(double)), histoyChart, SLOT(onUpdateCpuPercent(double)));
    connect(this,SIGNAL(updateCpuStatus(double)),this,SLOT(cpuHistoySetText(double)));
//swap and memory connect
    connect(this,SIGNAL(updateMemoryAndSwapStatus()),this,SLOT(onUpdateMemoryAndSwapStatus()));
    connect(this,SIGNAL(rebackMemoryAndSwapInfo(const QString, double,const QString, double)),this,SLOT(memoryandswapSetText(QString,double,QString,double)));
    connect(this,SIGNAL(rebackMemoryAndSwapData(float,double,float,double)),swapandmemoryChart,SLOT(onUpdateMemoryAndSwapData(float,double,float,double)));

//network connect
    connect(this,SIGNAL(updateNetworkStatus(long,long,long,long)),this,SLOT(networkSetText(long,long,long,long)));
    connect(this,SIGNAL(updateNetworkStatus(long,long,long,long)),networkChart,SLOT(onUpdateDownloadAndUploadData(long,long,long,long)));
    updateStatusTimer->start(500);
}

NewResouresDialog::~NewResouresDialog()
{
    QLayoutItem *child;
    while ((child = main_V_BoxLayout->takeAt(0)) != 0)
    {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}

void NewResouresDialog::setChangeNetSpeedLabel()
{
    connect(networkChart,&NetWorkChart::speedToLowKib,this,[=](){
        theFifthSpeedLabel->setText(tr("20Kib"));
        theFourthSpeedLabel->setText(tr("15Kib"));
        theThirdSpeedLabel->setText(tr("10Kib"));
        theSecondSpeedLabel->setText(tr("5Kib"));
        theFirtSpeedLabel->setText(tr("0Kib"));
    });

    connect(networkChart,&NetWorkChart::speedToHighKib,this,[=](){
        theFifthSpeedLabel->setText(tr("1000Kib"));
        theFourthSpeedLabel->setText(tr("750Kib"));
        theThirdSpeedLabel->setText(tr("500Kib"));
        theSecondSpeedLabel->setText(tr("250Kib"));
        theFirtSpeedLabel->setText(tr("0Kib"));
    });

}

void NewResouresDialog::cpuHistoySetText(double value)
{
    QString showValue;
    showValue = QString::number(value,10,0);
    QString s = "Cpu "+ showValue +":100%";
    cpuUnitDataLabel->setText(s);
    qDebug()<<"value"<<value;
}

void NewResouresDialog::memoryandswapSetText(const QString &infoMemory, double percentMemory,const QString &infoSwap, double percentSwap)
{
    QString showMemoryValue;
    showMemoryValue = infoMemory;
    QString sMemory = tr("memory:")+infoMemory;
    memoryUnitDataLabel->setText(sMemory);
    QString showSwapValue;
    showSwapValue = infoSwap;
    QString sSwap = tr("swap:")+showSwapValue;
    swapUnitDataLabel->setText(sSwap);
}

void NewResouresDialog::networkSetText(long recvTotalBytes, long sentTotalBytes, long recvRateBytes, long sentRateBytes)
{
    const QString downloadRate = formatNetworkRate(recvRateBytes);
    const QString uploadRate = formatNetworkRate(sentRateBytes);
    QString showReceiveValue = downloadRate;
    QString sRecv = tr("receive:") + showReceiveValue;
    QString showSendValue = uploadRate;
    QString sSend = tr("send:") +showSendValue;
    netrecvUnitDataLabel->setText(sRecv);
    netsentUnitDataLabel->setText(sSend);
}

void NewResouresDialog::initWidget()
{
    qDebug()<<this->height()<<"this.height-------";
    initCpuHistory();
    initSwapMeomoryHistory();
    initNetSpeedHistory();

    main_V_BoxLayout = new QVBoxLayout;
    main_V_BoxLayout->setSpacing(0);
    main_V_BoxLayout->addLayout(cputitle_H_BoxLayout);
    main_V_BoxLayout->addLayout(cpuUnit_H_BoxLayout);
    main_V_BoxLayout->addLayout(cpuChart_H_BoxLayout);
    main_V_BoxLayout->addLayout(swapmemoryTitle_H_BoxLayout);
    main_V_BoxLayout->addLayout(swapmemoryUnit_H_BoxLayout);
    main_V_BoxLayout->addLayout(memoryswapChart_H_BoxLayout);
    main_V_BoxLayout->addLayout(netWorkTitle_H_BoxLayout);
    main_V_BoxLayout->addLayout(networkUnit_H_BoxLayout);
    main_V_BoxLayout->addLayout(networkChart_H_BoxLayout);
    main_V_BoxLayout->addSpacing(15);

    this->setLayout(main_V_BoxLayout);

}

void NewResouresDialog::initCpuHistory()
{
    //set the part of CPU history
    cputitle_H_BoxLayout = new QHBoxLayout;
    cputitle_H_BoxLayout->setContentsMargins(13,0,0,0);
    cpuHistoryLabel = new QLabel(tr("CPU history"));
    cputitle_H_BoxLayout->addWidget(cpuHistoryLabel);

    cpuUnit_H_BoxLayout = new QHBoxLayout;
    typeCheckCpu = new sigCheck(this,REDTYPE);
    cpuUnitDataLabel->setPalette(pe);
    QHBoxLayout *cpuSig_H_BoxLayout = new QHBoxLayout;
    cpuSig_H_BoxLayout->setSpacing(0);
    cpuSig_H_BoxLayout->addStretch(1);
    cpuSig_H_BoxLayout->addWidget(typeCheckCpu);
    cpuSig_H_BoxLayout->addSpacing(8);
    cpuSig_H_BoxLayout->addWidget(cpuUnitDataLabel);
    cpuUnit_H_BoxLayout->addLayout(cpuSig_H_BoxLayout);

    cpuChart_H_BoxLayout = new QHBoxLayout;
    histoyChart = new CpuHistoryChart;
    QLabel *onehundredLabel = new QLabel();
    onehundredLabel->setText("100%");
    QLabel *threequarterLabel = new QLabel();
    threequarterLabel->setText("75%");
    QLabel *halfLabel = new QLabel();
    halfLabel->setText("50%");
    QLabel *onequarterLabel = new QLabel();
    onequarterLabel->setText("25%");
    QLabel *nullLabel = new QLabel();
    nullLabel->setText("0%");
    onehundredLabel->setPalette(pe);
    threequarterLabel->setPalette(pe);
    halfLabel->setPalette(pe);
    onequarterLabel->setPalette(pe);
    nullLabel->setPalette(pe);
    cpuWidget = new QWidget();
    cpuWidget->setFixedWidth(55);
    QVBoxLayout *cpupercent_V_BoxLayout = new QVBoxLayout();
    cpupercent_V_BoxLayout->setMargin(0);
    cpupercent_V_BoxLayout->addWidget(onehundredLabel,1,Qt::AlignRight);
    cpupercent_V_BoxLayout->addWidget(threequarterLabel,1,Qt::AlignRight);
    cpupercent_V_BoxLayout->addWidget(halfLabel,1,Qt::AlignRight);
    cpupercent_V_BoxLayout->addWidget(onequarterLabel,1,Qt::AlignRight);
    cpupercent_V_BoxLayout->addWidget(nullLabel,1,Qt::AlignRight);
    cpuWidget->setLayout(cpupercent_V_BoxLayout);
    cpuChart_H_BoxLayout->addWidget(cpuWidget);
    cpuChart_H_BoxLayout->addSpacing(10);
    cpuChart_H_BoxLayout->addWidget(histoyChart);
    cpuChart_H_BoxLayout->setSpacing(0);
}

void NewResouresDialog::initSwapMeomoryHistory()
{
    //set the swap and memory history
        swapmemoryTitle_H_BoxLayout = new QHBoxLayout;
        swapmemoryTitle_H_BoxLayout->setContentsMargins(13,0,0,0);
        memoryAndSwapLabel = new QLabel(tr("memory and swap history"));
        swapmemoryTitle_H_BoxLayout->addWidget(memoryAndSwapLabel);


        swapmemoryUnit_H_BoxLayout = new QHBoxLayout;
        typeCheckMemory = new sigCheck(this,PURPLETYPE);
        typeCheckSwap = new sigCheck(this,GREENTYPE);
        memoryUnitDataLabel->setPalette(pe);
        swapUnitDataLabel->setPalette(pe);
        QHBoxLayout *swapandmemory_H_BoxLayout = new QHBoxLayout;
        swapandmemory_H_BoxLayout->setSpacing(0);
        swapandmemory_H_BoxLayout->addStretch(1);
        swapandmemory_H_BoxLayout->addWidget(typeCheckMemory);
        swapandmemory_H_BoxLayout->addSpacing(8);
        swapandmemory_H_BoxLayout->addWidget(memoryUnitDataLabel);
        swapandmemory_H_BoxLayout->addSpacing(10);
        swapandmemory_H_BoxLayout->addWidget(typeCheckSwap);
        swapandmemory_H_BoxLayout->addSpacing(8);
        swapandmemory_H_BoxLayout->addWidget(swapUnitDataLabel);
        swapmemoryUnit_H_BoxLayout->addLayout(swapandmemory_H_BoxLayout);

        memoryswapChart_H_BoxLayout = new QHBoxLayout;
        swapandmemoryChart = new SwapAndMemoryChart;
        QVBoxLayout *swapmemory_V_BoxLayout = new QVBoxLayout();
        swapmemory_V_BoxLayout->setMargin(0);
        QLabel *tenGibLabel = new QLabel();
        tenGibLabel->setText("10.0Gib");
        swapmemoryWiget = new QWidget();
        swapmemoryWiget->setFixedWidth(55);
        swapmemory_V_BoxLayout->addWidget(tenGibLabel,1,Qt::AlignRight);
        tenGibLabel->setPalette(pe);
        QLabel *sevenpointFiveGibLabel = new QLabel();
        sevenpointFiveGibLabel->setText("7.5Gib");
        swapmemory_V_BoxLayout->addWidget(sevenpointFiveGibLabel,1,Qt::AlignRight);
        sevenpointFiveGibLabel->setPalette(pe);
        QLabel *fiveGibLabel = new QLabel();
        fiveGibLabel->setText("5.0Gib");
        swapmemory_V_BoxLayout->addWidget(fiveGibLabel,1,Qt::AlignRight);
        fiveGibLabel->setPalette(pe);
        QLabel *twopointfiveGibLabel = new QLabel();
        twopointfiveGibLabel->setText("2.5Gib");
        swapmemory_V_BoxLayout->addWidget(twopointfiveGibLabel,1,Qt::AlignRight);
        twopointfiveGibLabel->setPalette(pe);
        QLabel *nullGibLabel = new QLabel();
        nullGibLabel->setText("0.0Gib");
        swapmemory_V_BoxLayout->addWidget(nullGibLabel,1,Qt::AlignRight);
        nullGibLabel->setPalette(pe);
        swapmemoryWiget->setLayout(swapmemory_V_BoxLayout);
        memoryswapChart_H_BoxLayout->addWidget(swapmemoryWiget);
        memoryswapChart_H_BoxLayout->addSpacing(10);
        memoryswapChart_H_BoxLayout->addWidget(swapandmemoryChart);
}

void NewResouresDialog::initNetSpeedHistory()
{
    //set net history

        netWorkTitle_H_BoxLayout = new QHBoxLayout;
        netWorkTitle_H_BoxLayout->setContentsMargins(13,0,0,0);
        netWorkFlowLabel = new QLabel(tr("net work history"));
        netWorkTitle_H_BoxLayout->addWidget(netWorkFlowLabel);

        networkUnit_H_BoxLayout = new QHBoxLayout;
        typeCheckNetRecv = new sigCheck(this,BLUETYPE);
        typeCheckNetSent = new sigCheck(this,YELLOWTYPE);
        netrecvUnitDataLabel->setPalette(pe);
        netsentUnitDataLabel->setPalette(pe);
        QHBoxLayout *netrecvsent_H_BoxLayout = new QHBoxLayout;
        netrecvsent_H_BoxLayout->addStretch(1);
        netrecvsent_H_BoxLayout->addWidget(typeCheckNetRecv);
        netrecvsent_H_BoxLayout->addSpacing(8);
        netrecvsent_H_BoxLayout->addWidget(netrecvUnitDataLabel);
        netrecvsent_H_BoxLayout->addSpacing(10);
        netrecvsent_H_BoxLayout->addWidget(typeCheckNetSent);
        netrecvsent_H_BoxLayout->addSpacing(8);
        netrecvsent_H_BoxLayout->addWidget(netsentUnitDataLabel);
        networkUnit_H_BoxLayout->addLayout(netrecvsent_H_BoxLayout);

        networkChart_H_BoxLayout = new QHBoxLayout();
        QVBoxLayout *netspeed_V_BoxLayout = new QVBoxLayout();
        netspeed_V_BoxLayout->setMargin(0);
        networkWidget = new QWidget();
        networkWidget->setFixedWidth(55);
        theFifthSpeedLabel = new QLabel();
        theFifthSpeedLabel->setPalette(pe);
        netspeed_V_BoxLayout->addWidget(theFifthSpeedLabel,1,Qt::AlignRight);
        theFourthSpeedLabel = new QLabel();
        theFourthSpeedLabel->setPalette(pe);
        netspeed_V_BoxLayout->addWidget(theFourthSpeedLabel,1,Qt::AlignRight);
        theThirdSpeedLabel = new QLabel();
        theThirdSpeedLabel->setPalette(pe);
        netspeed_V_BoxLayout->addWidget(theThirdSpeedLabel,1,Qt::AlignRight);
        theSecondSpeedLabel = new QLabel();
        theSecondSpeedLabel->setPalette(pe);
        netspeed_V_BoxLayout->addWidget(theSecondSpeedLabel,1,Qt::AlignRight);
        theFirtSpeedLabel = new QLabel();
        theFirtSpeedLabel->setPalette(pe);
        netspeed_V_BoxLayout->addWidget(theFirtSpeedLabel,1,Qt::AlignRight);
        networkWidget->setLayout(netspeed_V_BoxLayout);
        networkChart = new NetWorkChart();
        networkChart_H_BoxLayout->addWidget(networkWidget);
        networkChart_H_BoxLayout->addSpacing(10);
        networkChart_H_BoxLayout->addWidget(networkChart);
}

void NewResouresDialog::tosetFontSize()
{
    QFont fontTitle;
    fontTitle.setPointSize(fontSize*1.5);
    cpuHistoryLabel->setFont(fontTitle);
    memoryAndSwapLabel->setFont(fontTitle);
    netWorkFlowLabel->setFont(fontTitle);
    QFont fontContext;
    fontContext.setPointSize(fontSize);
    cpuUnitDataLabel->setFont(fontContext);
    swapUnitDataLabel->setFont(fontContext);
    memoryUnitDataLabel->setFont(fontContext);
    netrecvUnitDataLabel->setFont(fontContext);
    netsentUnitDataLabel->setFont(fontContext);
}

void NewResouresDialog::initFontSize()
{
    connect(fontSettings,&QGSettings::changed,[=](QString key)
    {
        if("systemFont" == key || "systemFontSize" == key)
        {
            fontSize = fontSettings->get(FONT_SIZE).toInt();
        }
        tosetFontSize();
    });
    fontSize = fontSettings->get(FONT_SIZE).toInt();
}

void NewResouresDialog::updateResourceStatus()
{
//cpu data get signal
    m_prevCpuWorkTime = m_cpuworkTime;
    m_prevCpuTotalTime = m_cpuTotalTime;
    m_cpuTotalTime = getCpuTimeData(m_cpuworkTime);
    if (m_prevCpuWorkTime != 0 && m_prevCpuTotalTime != 0)
    {
        emit updateCpuStatus((m_cpuworkTime - m_prevCpuWorkTime) * 100.0 / (m_cpuTotalTime - m_prevCpuTotalTime));
    }
    else
    {
        emit updateCpuStatus(0);
    }

//swap and memory data get signal
    emit updateMemoryAndSwapStatus();

//net work data get signal
    getNetworkBytesData(m_totalRecvBytes, m_totalSentBytes, m_rateRecvBytes, m_rateSentBytes);
    emit this->updateNetworkStatus(m_totalRecvBytes, m_totalSentBytes, m_rateRecvBytes, m_rateSentBytes);
}


