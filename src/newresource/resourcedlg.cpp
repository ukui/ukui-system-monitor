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

#include "resourcedlg.h"
#include "util.h"

#include <glibtop/netload.h>
#include <glibtop/netlist.h>
#include <QDebug>
#include <QTimer>
#include <QSpacerItem>

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
    else if (size < factorList.at(T_INDEX))
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

void ResouresDlg::onUpdateMemoryAndSwapStatus()
{
    glibtop_mem mem;
    glibtop_swap swap;

    glibtop_get_mem(&mem);
    glibtop_get_swap(&swap);

    float swappercent = (swap.total ? (float)swap.used / (float)swap.total : 0.0f);
    float mempercent  = (mem.total ? (float)mem.user  / (float)mem.total : 0.0f);

    m_memInfo.percent = mempercent * 100;
    m_memInfo.swappercent = swappercent * 100;

    //初始单位为字节，需要修正
    m_memInfo.user = mem.user;
    m_memInfo.total = mem.total;

    m_memInfo.swapused = swap.used;
    m_memInfo.swaptotal = swap.total;

    const QString infoMemory = QString("%1/%2").arg(formatMemory(m_memInfo.user)).arg(formatMemory(m_memInfo.total));
    const QString infoSwap = QString("%1/%2").arg(formatMemory(m_memInfo.swapused)).arg(formatMemory(m_memInfo.swaptotal));
    emit rebackMemoryAndSwapInfo(infoMemory, m_memInfo.percent,infoSwap,m_memInfo.swappercent);
    emit rebackMemoryAndSwapData(m_memInfo.user,m_memInfo.total,m_memInfo.swapused,m_memInfo.swaptotal);
//    emit rebackSwapInfo(infoSwap,m_memInfo.swappercent);
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
    guint64 din = 0, dout = 0;
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

inline QString formatNetworkRate(guint64 rate)
{
    return formatNetworkBrandWidth(rate, false);
}

ResouresDlg::ResouresDlg(QWidget* parent)
    : QWidget(parent)
{
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd))
    {
        m_styleSettings = new QGSettings(idd);
    }
    initThemeMode();
    InitWidgets();
    InitConnections();
    onThemeFontChange(m_fFontSize);
}

ResouresDlg::~ResouresDlg()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

bool ResouresDlg::InitCpuPanel()
{
    m_cpuHistoryLabel = new QLabel(tr("CPU history"));
    m_cpuHistoryLabel->setAlignment(Qt::AlignLeft);
    m_typeCheckCpu = new sigCheck(this,REDTYPE);
    m_cpuUnitDataLabel = new QLabel();
    m_cpuUnitDataLabel->setPalette(m_controlPE);
    m_onehundredLabel = new QLabel(tr("100%"));
    m_onehundredLabel->setPalette(m_controlPE);
    m_threequarterLabel = new QLabel(tr("75%"));
    m_threequarterLabel->setPalette(m_controlPE);
    m_halfLabel = new QLabel(tr("50%"));
    m_halfLabel->setPalette(m_controlPE);
    m_onequarterLabel = new QLabel(tr("25%"));
    m_onequarterLabel->setPalette(m_controlPE);
    m_nullLabel = new QLabel(tr("0%"));
    m_nullLabel->setPalette(m_controlPE);
    m_chartCpu = new CpuHistoryChart;

    m_cpuTitleHLayout->setContentsMargins(23,0,25,0);
    m_cpuTitleVLayout->addWidget(m_cpuHistoryLabel);
    m_cpuTitleVLayout->addStretch();
    m_cpuCurHLayout->setSpacing(0);
    m_cpuCurHLayout->addStretch(1);
    m_cpuCurHLayout->addWidget(m_typeCheckCpu);
    m_cpuCurHLayout->addSpacing(8);
    m_cpuCurHLayout->addWidget(m_cpuUnitDataLabel);
    m_cpuCurVLayout->addStretch();
    m_cpuCurVLayout->addLayout(m_cpuCurHLayout);
    m_cpuTitleHLayout->addLayout(m_cpuTitleVLayout);
    m_cpuTitleHLayout->addLayout(m_cpuCurVLayout);
    QWidget *cpuTitleWidget = new QWidget();
    cpuTitleWidget->setFixedHeight(40);
    cpuTitleWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    cpuTitleWidget->setLayout(m_cpuTitleHLayout);

    m_cpuLevelLayout->setMargin(0);
    m_cpuLevelLayout->addWidget(m_onehundredLabel,1,Qt::AlignRight);
    m_cpuLevelLayout->addWidget(m_threequarterLabel,1,Qt::AlignRight);
    m_cpuLevelLayout->addWidget(m_halfLabel,1,Qt::AlignRight);
    m_cpuLevelLayout->addWidget(m_onequarterLabel,1,Qt::AlignRight);
    m_cpuLevelLayout->addWidget(m_nullLabel,1,Qt::AlignRight);
    QWidget *cpuWidget = new QWidget();
    cpuWidget->setFixedWidth(73);
    cpuWidget->setMinimumHeight(100);
    cpuWidget->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
    cpuWidget->setLayout(m_cpuLevelLayout);
    m_cpuChartLayout->setContentsMargins(2,0,26,0);
    m_cpuChartLayout->addWidget(cpuWidget);
    m_cpuChartLayout->addSpacing(18);
    QVBoxLayout *chartContentLayout = new QVBoxLayout();
    chartContentLayout->addStretch(1);
    chartContentLayout->addWidget(m_chartCpu, 10);
    chartContentLayout->addStretch(1);
    m_cpuChartLayout->addLayout(chartContentLayout);
    m_cpuInfoLayout->addWidget(cpuTitleWidget);
    m_cpuInfoLayout->addLayout(m_cpuChartLayout);

    if(m_currentThemeMode == "ukui-dark" || m_currentThemeMode == "ukui-black")
    {
        m_chartCpu->setBgColor(QColor(255,255,255));
    }
    else
    {
        m_chartCpu->setBgColor(QColor(13,14,14));
    }
    return true;
}

bool ResouresDlg::InitMemoryPanel()
{
    m_memoryAndSwapLabel = new QLabel(tr("memory and swap history"));
    m_memoryAndSwapLabel->setAlignment(Qt::AlignLeft);
    m_typeCheckMemory = new sigCheck(this,PURPLETYPE);
    m_typeCheckSwap = new sigCheck(this,GREENTYPE);
    m_memoryUnitDataLabel = new QLabel();
    m_memoryUnitDataLabel->setPalette(m_controlPE);
    m_swapUnitDataLabel = new QLabel();
    m_swapUnitDataLabel->setPalette(m_controlPE);
    m_tenGibLabel = new QLabel();
    m_tenGibLabel->setPalette(m_controlPE);
    m_sevenpointFiveGibLabel = new QLabel();
    m_sevenpointFiveGibLabel->setPalette(m_controlPE);
    m_fiveGibLabel = new QLabel();
    m_fiveGibLabel->setPalette(m_controlPE);
    m_twopointfiveGibLabel = new QLabel();
    m_twopointfiveGibLabel->setPalette(m_controlPE);
    m_nullGibLabel = new QLabel();
    m_nullGibLabel->setPalette(m_controlPE);
    m_chartSwapandmemory = new SwapAndMemoryChart;

    m_memTitleHLayout->setContentsMargins(23,0,25,0);
    m_memTitleVLayout->addWidget(m_memoryAndSwapLabel);
    m_memTitleVLayout->addStretch();
    m_memCurHLayout->setSpacing(0);
    m_memCurHLayout->addStretch(1);
    m_memCurHLayout->addWidget(m_typeCheckMemory);
    m_memCurHLayout->addSpacing(8);
    m_memCurHLayout->addWidget(m_memoryUnitDataLabel);
    m_memCurHLayout->addSpacing(24);
    m_memCurHLayout->addWidget(m_typeCheckSwap);
    m_memCurHLayout->addSpacing(8);
    m_memCurHLayout->addWidget(m_swapUnitDataLabel);
    m_memCurVLayout->addStretch();
    m_memCurVLayout->addLayout(m_memCurHLayout);
    m_memTitleHLayout->addLayout(m_memTitleVLayout);
    m_memTitleHLayout->addLayout(m_memCurVLayout);
    QWidget *memTitleWidget = new QWidget();
    memTitleWidget->setFixedHeight(40);
    memTitleWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    memTitleWidget->setLayout(m_memTitleHLayout);

    m_memLevelLayout->setMargin(0);
    m_memLevelLayout->addWidget(m_tenGibLabel,1,Qt::AlignRight);
    m_memLevelLayout->addWidget(m_sevenpointFiveGibLabel,1,Qt::AlignRight);
    m_memLevelLayout->addWidget(m_fiveGibLabel,1,Qt::AlignRight);
    m_memLevelLayout->addWidget(m_twopointfiveGibLabel,1,Qt::AlignRight);
    m_memLevelLayout->addWidget(m_nullGibLabel,1,Qt::AlignRight);
    QWidget *memWidget = new QWidget();
    memWidget->setFixedWidth(73);
    memWidget->setMinimumHeight(100);
    memWidget->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
    memWidget->setLayout(m_memLevelLayout);
    m_memChartLayout->setContentsMargins(2,0,26,0);
    m_memChartLayout->addWidget(memWidget);
    m_memChartLayout->addSpacing(18);
    QVBoxLayout *chartContentLayout = new QVBoxLayout();
    chartContentLayout->addStretch(1);
    chartContentLayout->addWidget(m_chartSwapandmemory, 10);
    chartContentLayout->addStretch(1);
    m_memChartLayout->addLayout(chartContentLayout);
    m_memInfoLayout->addWidget(memTitleWidget);
    m_memInfoLayout->addLayout(m_memChartLayout);

    if(m_currentThemeMode == "ukui-dark" || m_currentThemeMode == "ukui-black")
    {
        m_chartSwapandmemory->setBgColor(QColor(255,255,255));
    }
    else
    {
        m_chartSwapandmemory->setBgColor(QColor(13,14,14));
    }
    return true;
}

bool ResouresDlg::InitNetworkPanel()
{
    m_netWorkFlowLabel = new QLabel(tr("net work history"));
    m_netWorkFlowLabel->setAlignment(Qt::AlignLeft);
    m_typeCheckNetRecv = new sigCheck(this,BLUETYPE);
    m_typeCheckNetSent = new sigCheck(this,YELLOWTYPE);
    m_netrecvUnitDataLabel = new QLabel();
    m_netsentUnitDataLabel = new QLabel();
    m_netrecvUnitDataLabel->setPalette(m_controlPE);
    m_netsentUnitDataLabel->setPalette(m_controlPE);
    m_theFirtSpeedLabel = new QLabel();
    m_theFirtSpeedLabel->setPalette(m_controlPE);
    m_theSecondSpeedLabel = new QLabel();
    m_theSecondSpeedLabel->setPalette(m_controlPE);
    m_theThirdSpeedLabel = new QLabel();
    m_theThirdSpeedLabel->setPalette(m_controlPE);
    m_theFourthSpeedLabel = new QLabel();
    m_theFourthSpeedLabel->setPalette(m_controlPE);
    m_theFifthSpeedLabel = new QLabel();
    m_theFifthSpeedLabel->setPalette(m_controlPE);
    m_chartNetwork = new NetWorkChart;

    m_netTitleHLayout->setContentsMargins(23,0,25,0);
    m_netTitleVLayout->addWidget(m_netWorkFlowLabel);
    m_netTitleVLayout->addStretch();
    m_netCurHLayout->addStretch(1);
    m_netCurHLayout->addWidget(m_typeCheckNetRecv);
    m_netCurHLayout->addSpacing(8);
    m_netCurHLayout->addWidget(m_netrecvUnitDataLabel);
    m_netCurHLayout->addSpacing(24);
    m_netCurHLayout->addWidget(m_typeCheckNetSent);
    m_netCurHLayout->addSpacing(8);
    m_netCurHLayout->addWidget(m_netsentUnitDataLabel);
    m_netCurVLayout->addStretch();
    m_netCurVLayout->addLayout(m_netCurHLayout);
    m_netTitleHLayout->addLayout(m_netTitleVLayout);
    m_netTitleHLayout->addLayout(m_netCurVLayout);
    QWidget *netTitleWidget = new QWidget();
    netTitleWidget->setFixedHeight(40);
    netTitleWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    netTitleWidget->setLayout(m_netTitleHLayout);

    m_netLevelLayout->addWidget(m_theFifthSpeedLabel,1,Qt::AlignRight);
    m_netLevelLayout->addWidget(m_theFourthSpeedLabel,1,Qt::AlignRight);
    m_netLevelLayout->addWidget(m_theThirdSpeedLabel,1,Qt::AlignRight);
    m_netLevelLayout->addWidget(m_theSecondSpeedLabel,1,Qt::AlignRight);
    m_netLevelLayout->addWidget(m_theFirtSpeedLabel,1,Qt::AlignRight);
    QWidget *netWidget = new QWidget();
    netWidget->setFixedWidth(73);
    netWidget->setMinimumHeight(100);
    netWidget->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
    netWidget->setLayout(m_netLevelLayout);
    m_netChartLayout->setContentsMargins(2,0,26,0);
    m_netChartLayout->addWidget(netWidget);
    m_netChartLayout->addSpacing(18);
    QVBoxLayout *chartContentLayout = new QVBoxLayout();
    chartContentLayout->addStretch(1);
    chartContentLayout->addWidget(m_chartNetwork, 10);
    chartContentLayout->addStretch(1);
    m_netChartLayout->addLayout(chartContentLayout);
    m_netInfoLayout->addWidget(netTitleWidget);
    m_netInfoLayout->addLayout(m_netChartLayout);

    if(m_currentThemeMode == "ukui-dark" || m_currentThemeMode == "ukui-black")
    {
        m_chartNetwork->setBgColor(QColor(255,255,255));
    }
    else
    {
        m_chartNetwork->setBgColor(QColor(13,14,14));
    }
    return true;
}

bool ResouresDlg::InitWidgets()
{
    // all layout
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setSpacing(0);
    // cpu layout
    m_cpuInfoLayout = new QVBoxLayout();
    m_cpuInfoLayout->setContentsMargins(0,0,0,0);
    m_cpuInfoLayout->setSpacing(0);
    m_cpuTitleHLayout = new QHBoxLayout();
    m_cpuTitleHLayout->setContentsMargins(0,0,0,0);
    m_cpuTitleHLayout->setSpacing(0);
    m_cpuTitleVLayout = new QVBoxLayout();
    m_cpuTitleVLayout->setContentsMargins(0,0,0,0);
    m_cpuTitleVLayout->setSpacing(0);
    m_cpuCurVLayout = new QVBoxLayout();
    m_cpuCurVLayout->setContentsMargins(0,0,0,0);
    m_cpuCurVLayout->setSpacing(0);
    m_cpuCurHLayout = new QHBoxLayout();
    m_cpuCurHLayout->setContentsMargins(0,0,0,0);
    m_cpuCurHLayout->setSpacing(0);
    m_cpuChartLayout = new QHBoxLayout();
    m_cpuChartLayout->setContentsMargins(0,0,0,0);
    m_cpuChartLayout->setSpacing(0);
    m_cpuLevelLayout = new QVBoxLayout();
    m_cpuLevelLayout->setContentsMargins(0,0,0,0);
    m_cpuLevelLayout->setSpacing(0);
    // mem layout
    m_memInfoLayout = new QVBoxLayout();
    m_memInfoLayout->setContentsMargins(0,0,0,0);
    m_memInfoLayout->setSpacing(0);
    m_memTitleHLayout = new QHBoxLayout();
    m_memTitleHLayout->setContentsMargins(0,0,0,0);
    m_memTitleHLayout->setSpacing(0);
    m_memTitleVLayout = new QVBoxLayout();
    m_memTitleVLayout->setContentsMargins(0,0,0,0);
    m_memTitleVLayout->setSpacing(0);
    m_memCurVLayout = new QVBoxLayout();
    m_memCurVLayout->setContentsMargins(0,0,0,0);
    m_memCurVLayout->setSpacing(0);
    m_memCurHLayout = new QHBoxLayout();
    m_memCurHLayout->setContentsMargins(0,0,0,0);
    m_memCurHLayout->setSpacing(0);
    m_memChartLayout = new QHBoxLayout();
    m_memChartLayout->setContentsMargins(0,0,0,0);
    m_memChartLayout->setSpacing(0);
    m_memLevelLayout = new QVBoxLayout();
    m_memLevelLayout->setContentsMargins(0,0,0,0);
    m_memLevelLayout->setSpacing(0);
    // network layout
    m_netInfoLayout = new QVBoxLayout();
    m_netInfoLayout->setContentsMargins(0,0,0,0);
    m_netInfoLayout->setSpacing(0);
    m_netTitleHLayout = new QHBoxLayout();
    m_netTitleHLayout->setContentsMargins(0,0,0,0);
    m_netTitleHLayout->setSpacing(0);
    m_netTitleVLayout = new QVBoxLayout();
    m_netTitleVLayout->setContentsMargins(0,0,0,0);
    m_netTitleVLayout->setSpacing(0);
    m_netCurVLayout = new QVBoxLayout();
    m_netCurVLayout->setContentsMargins(0,0,0,0);
    m_netCurVLayout->setSpacing(0);
    m_netCurHLayout = new QHBoxLayout();
    m_netCurHLayout->setContentsMargins(0,0,0,0);
    m_netCurHLayout->setSpacing(0);
    m_netChartLayout = new QHBoxLayout();
    m_netChartLayout->setContentsMargins(0,0,0,0);
    m_netChartLayout->setSpacing(0);
    m_netLevelLayout = new QVBoxLayout();
    m_netLevelLayout->setContentsMargins(0,0,0,0);
    m_netLevelLayout->setSpacing(0);

    InitCpuPanel();
    InitMemoryPanel();
    InitNetworkPanel();

    m_mainLayout->addSpacing(10);
    m_mainLayout->addLayout(m_cpuInfoLayout);
    m_mainLayout->addSpacing(20);
    m_mainLayout->addLayout(m_memInfoLayout);
    m_mainLayout->addSpacing(20);
    m_mainLayout->addLayout(m_netInfoLayout);
    m_mainLayout->addSpacing(20);
    this->setLayout(m_mainLayout);
    return true;
}

bool ResouresDlg::InitConnections()
{
    m_updateStatusTimer = new QTimer(this);
    connect(m_updateStatusTimer, SIGNAL(timeout()), this, SLOT(updateResourceStatus()));
//cpu history conecct
    connect(this, SIGNAL(updateCpuStatus(double)), m_chartCpu, SLOT(onUpdateCpuPercent(double)));
    connect(this,SIGNAL(updateCpuStatus(double)),this,SLOT(cpuHistoySetText(double)));

//swap and memory connect
    connect(this,SIGNAL(updateMemoryAndSwapStatus()),this,SLOT(onUpdateMemoryAndSwapStatus()));
    connect(this,SIGNAL(rebackMemoryAndSwapInfo(const QString, double,const QString, double)),this,SLOT(memoryandswapSetText(QString,double,QString,double)));
    connect(this,SIGNAL(rebackMemoryAndSwapData(float,double,float,double)),m_chartSwapandmemory,SLOT(onUpdateMemoryAndSwapData(float,double,float,double)));
    connect(m_chartSwapandmemory,&SwapAndMemoryChart::spaceToDynamicMax,this,[=](qreal lfMaxSpace){
        m_tenGibLabel->setText(formatMemory(lfMaxSpace));
        m_sevenpointFiveGibLabel->setText(formatMemory(lfMaxSpace*3/4));
        m_fiveGibLabel->setText(formatMemory(lfMaxSpace*2/4));
        m_twopointfiveGibLabel->setText(formatMemory(lfMaxSpace*1/4));
        m_nullGibLabel->setText("0.0"+tr("GiB"));
    });

//network connect
    connect(this,SIGNAL(updateNetworkStatus(quint64,quint64,quint64,quint64)),this,SLOT(networkSetText(quint64,quint64,quint64,quint64)));
    connect(this,SIGNAL(updateNetworkStatus(quint64,quint64,quint64,quint64)),m_chartNetwork,SLOT(onUpdateDownloadAndUploadData(quint64,quint64,quint64,quint64)));
    connect(m_chartNetwork,&NetWorkChart::speedToDynamicMax,this,[=](quint64 lluMaxSpeed){
        m_theFifthSpeedLabel->setText(formatNetworkBrandWidth(lluMaxSpeed,true));
        m_theFourthSpeedLabel->setText(formatNetworkBrandWidth(lluMaxSpeed*3/4,true));
        m_theThirdSpeedLabel->setText(formatNetworkBrandWidth(lluMaxSpeed*2/4,true));
        m_theSecondSpeedLabel->setText(formatNetworkBrandWidth(lluMaxSpeed*1/4,true));
        m_theFirtSpeedLabel->setText(formatNetworkBrandWidth(0,true));
    });
    m_updateStatusTimer->start(1000);
    return true;
}

void ResouresDlg::initThemeMode()
{
    if (!m_styleSettings) {
        return;
    }
    //监听主题改变
    connect(m_styleSettings, &QGSettings::changed, this, [=](const QString &key)
    {
        if (key == "styleName")
        {
            m_currentThemeMode = m_styleSettings->get("styleName").toString();
            if(m_currentThemeMode == "ukui-dark" || m_currentThemeMode == "ukui-black")
            {
                m_controlPE.setColor(QPalette::WindowText,QColor(255,255,255,130));
                if (m_chartCpu)
                    m_chartCpu->setBgColor(QColor(255,255,255));
                if (m_chartSwapandmemory)
                    m_chartSwapandmemory->setBgColor(QColor(255,255,255));
                if (m_chartNetwork)
                    m_chartNetwork->setBgColor(QColor(255,255,255));
            }
            else
            {
                m_controlPE.setColor(QPalette::WindowText,QColor(13,14,13,130));
                if (m_chartCpu)
                    m_chartCpu->setBgColor(QColor(13,14,14));
                if (m_chartSwapandmemory)
                    m_chartSwapandmemory->setBgColor(QColor(13,14,14));
                if (m_chartNetwork)
                    m_chartNetwork->setBgColor(QColor(13,14,14));
            }
            m_onehundredLabel->setPalette(m_controlPE);
            m_threequarterLabel->setPalette(m_controlPE);
            m_halfLabel->setPalette(m_controlPE);
            m_onequarterLabel->setPalette(m_controlPE);
            m_nullLabel->setPalette(m_controlPE);
            m_tenGibLabel->setPalette(m_controlPE);
            m_sevenpointFiveGibLabel->setPalette(m_controlPE);
            m_fiveGibLabel->setPalette(m_controlPE);
            m_twopointfiveGibLabel->setPalette(m_controlPE);
            m_nullGibLabel->setPalette(m_controlPE);
            m_theFirtSpeedLabel->setPalette(m_controlPE);
            m_theSecondSpeedLabel->setPalette(m_controlPE);
            m_theThirdSpeedLabel->setPalette(m_controlPE);
            m_theFourthSpeedLabel->setPalette(m_controlPE);
            m_theFifthSpeedLabel->setPalette(m_controlPE);
            m_cpuUnitDataLabel->setPalette(m_controlPE);
            m_swapUnitDataLabel->setPalette(m_controlPE);
            m_memoryUnitDataLabel->setPalette(m_controlPE);
            m_netrecvUnitDataLabel->setPalette(m_controlPE);
            m_netsentUnitDataLabel->setPalette(m_controlPE);
            repaint();
        }
    });
    m_currentThemeMode = m_styleSettings->get(MODE_QT_KEY).toString();
    if( m_currentThemeMode == "ukui-dark" || m_currentThemeMode == "ukui-black" )
    {
        m_controlPE.setColor(QPalette::WindowText,QColor(255,255,255,130));
    }
    else
    {
        m_controlPE.setColor(QPalette::WindowText,QColor(13,14,13,130));
    }
    connect(m_styleSettings,&QGSettings::changed,[=](QString key)
    {
        if("systemFont" == key || "systemFontSize" == key)
        {
            m_fFontSize = m_styleSettings->get(FONT_SIZE).toFloat();
            onThemeFontChange(m_fFontSize);
        }
    });
    m_fFontSize = m_styleSettings->get(FONT_SIZE).toFloat();
}

void ResouresDlg::onThemeFontChange(float fFontSize)
{
    QFont fontTitle;
    fontTitle.setPointSize(fFontSize*1.3);
    m_cpuHistoryLabel->setFont(fontTitle);
    m_memoryAndSwapLabel->setFont(fontTitle);
    m_netWorkFlowLabel->setFont(fontTitle);

    QFont fontContext;
    fontContext.setPointSize(fFontSize*0.8);
    m_cpuUnitDataLabel->setFont(fontContext);
    m_swapUnitDataLabel->setFont(fontContext);
    m_memoryUnitDataLabel->setFont(fontContext);
    m_netrecvUnitDataLabel->setFont(fontContext);
    m_netsentUnitDataLabel->setFont(fontContext);

    /*QString strMemInfo = getElidedText(m_memoryUnitDataLabel->font(), m_strCurMemInfo, m_memoryUnitDataLabel->width()-4);
    m_memoryUnitDataLabel->setText(strMemInfo);
    if (strMemInfo != m_strCurMemInfo) {
        m_memoryUnitDataLabel->setToolTip(m_strCurMemInfo);
    } else {
        m_memoryUnitDataLabel->setToolTip("");
    }*/

    float leftUnit = fFontSize;
    QFont fontLeftUnit;
    #if 0
    if(leftUnit > 12 && leftUnit <=14)
    {
        leftUnit = 12;
    }
    else if(leftUnit >14 && leftUnit <= 16)
    {
        leftUnit = 12;
    }
    else
    {
        leftUnit = 11;
    }
    #endif
    fontLeftUnit.setPointSize(leftUnit*0.8);
    m_onehundredLabel->setFont(fontLeftUnit);
    m_threequarterLabel->setFont(fontLeftUnit);
    m_halfLabel->setFont(fontLeftUnit);
    m_onequarterLabel->setFont(fontLeftUnit);
    m_nullLabel->setFont(fontLeftUnit);
    m_tenGibLabel->setFont(fontLeftUnit);
    m_sevenpointFiveGibLabel->setFont(fontLeftUnit);
    m_fiveGibLabel->setFont(fontLeftUnit);
    m_twopointfiveGibLabel->setFont(fontLeftUnit);
    m_nullGibLabel->setFont(fontLeftUnit);
    m_theFifthSpeedLabel->setFont(fontLeftUnit);
    m_theFourthSpeedLabel->setFont(fontLeftUnit);
    m_theThirdSpeedLabel->setFont(fontLeftUnit);
    m_theSecondSpeedLabel->setFont(fontLeftUnit);
    m_theFirtSpeedLabel->setFont(fontLeftUnit);
}

void ResouresDlg::cpuHistoySetText(double value)
{
    QString showValue;
    showValue = QString::number(value,10,0);
    QString s = tr("CPU: ")+ showValue +"%";
    m_cpuUnitDataLabel->setText(s);
//    qDebug() << "cpu value: " << value;
}

void ResouresDlg::memoryandswapSetText(const QString &infoMemory, double percentMemory,const QString &infoSwap, double percentSwap)
{
    QString showMemoryValue;
    showMemoryValue = infoMemory;
    QString sMemory = tr("memory:")+infoMemory;
    m_memoryUnitDataLabel->setText(sMemory);
    QString showSwapValue;
    showSwapValue = infoSwap;
    QString sSwap = tr("swap:")+showSwapValue;
    m_swapUnitDataLabel->setText(sSwap);
}

void ResouresDlg::networkSetText(quint64 recvTotalBytes, quint64 sentTotalBytes, quint64 recvRateBytes, quint64 sentRateBytes)
{
    const QString downloadRate = formatNetworkRate(recvRateBytes);
    const QString uploadRate = formatNetworkRate(sentRateBytes);
    QString showReceiveValue = downloadRate;
    QString sRecv = tr("receive:") + showReceiveValue;
    QString showSendValue = uploadRate;
    QString sSend = tr("send:") +showSendValue;
    m_netrecvUnitDataLabel->setText(sRecv);
    m_netsentUnitDataLabel->setText(sSend);
}

void ResouresDlg::updateResourceStatus()
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