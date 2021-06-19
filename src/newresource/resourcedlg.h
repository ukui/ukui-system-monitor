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
#ifndef __RESOURCEDLG_H__
#define __RESOURCEDLG_H__

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QBoxLayout>
#include <QCheckBox>
#include <QTimer>
#include <glibtop/mem.h>
#include <glibtop/swap.h>
#include <QGSettings>

#include "../shell/macro.h"
#include "sigcheck.h"
#include "cpuhistorychart.h"
#include "swapandmemorychart.h"
#include "networkchart.h"

typedef struct MemoryInfo
{
    float total;
    float free;
    float cached;
    float used;
    float user;
    float percent;

    float swaptotal;
    float swapfree;
    float swapcached;
    float swapused;
    float swappercent;

}Meminfo;

class ResouresDlg : public QWidget
{
    Q_OBJECT

public:
    explicit ResouresDlg(QWidget* parent = 0);
    ~ResouresDlg();

    void initThemeMode();
    void onThemeFontChange(float fFontSize);
    
private:
    bool InitCpuPanel();
    bool InitMemoryPanel();
    bool InitNetworkPanel();
    bool InitWidgets();
    bool InitConnections();

public slots:
    void updateResourceStatus();
    void cpuHistoySetText(double percent);
    void memoryandswapSetText(const QString &infoMemory, double percentMemory,const QString &infoSwap, double percentSwap);
    void networkSetText(quint64 recvTotalBytes, quint64 sentTotalBytes, quint64 recvRateBytes, quint64 sentRateBytes);
    void onUpdateMemoryAndSwapStatus();

signals:
    void updateCpuStatus(double percent);
    void rebackMemoryAndSwapInfo(const QString &infoMemory, double percentMemory,const QString &infoSwap, double percentSwap);
    void rebackMemoryAndSwapData(float memory,double percentMemory,float swap,double percentSwap);
    void updateMemoryAndSwapStatus();
    void updateNetworkStatus(quint64 recvTotalBytes, quint64 sentTotalBytes, quint64 recvRateBytes, quint64 sentRateBytes);

private:
    // cpu info
    QLabel *m_cpuHistoryLabel = nullptr;
    sigCheck *m_typeCheckCpu = nullptr;
    QLabel *m_cpuUnitDataLabel = nullptr;
    QLabel *m_onehundredLabel = nullptr;
    QLabel *m_threequarterLabel = nullptr;
    QLabel *m_halfLabel = nullptr;
    QLabel *m_onequarterLabel = nullptr;
    QLabel *m_nullLabel = nullptr;
    CpuHistoryChart *m_chartCpu = nullptr;

    // memory info
    QLabel *m_memoryAndSwapLabel = nullptr;
    sigCheck *m_typeCheckMemory = nullptr;
    sigCheck *m_typeCheckSwap = nullptr;
    QLabel *m_memoryUnitDataLabel = nullptr;
    QLabel *m_swapUnitDataLabel = nullptr;
    QLabel *m_tenGibLabel = nullptr;
    QLabel *m_sevenpointFiveGibLabel = nullptr;
    QLabel *m_fiveGibLabel = nullptr;
    QLabel *m_twopointfiveGibLabel = nullptr;
    QLabel *m_nullGibLabel = nullptr;
    SwapAndMemoryChart *m_chartSwapandmemory = nullptr;

    // network info
    QLabel *m_netWorkFlowLabel = nullptr;
    sigCheck *m_typeCheckNetRecv = nullptr;
    sigCheck *m_typeCheckNetSent = nullptr;
    QLabel *m_netrecvUnitDataLabel = nullptr;
    QLabel *m_netsentUnitDataLabel = nullptr;    
    QLabel *m_theFirtSpeedLabel = nullptr;
    QLabel *m_theSecondSpeedLabel = nullptr;
    QLabel *m_theThirdSpeedLabel = nullptr;
    QLabel *m_theFourthSpeedLabel = nullptr;
    QLabel *m_theFifthSpeedLabel = nullptr;
    NetWorkChart *m_chartNetwork = nullptr;

    // all layout
    QVBoxLayout *m_mainLayout = nullptr;;
    // cpu layout
    QVBoxLayout *m_cpuInfoLayout = nullptr;
    QVBoxLayout *m_cpuTitleVLayout = nullptr;
    QHBoxLayout *m_cpuTitleHLayout = nullptr;
    QVBoxLayout *m_cpuCurVLayout = nullptr;
    QHBoxLayout *m_cpuCurHLayout = nullptr;
    QHBoxLayout *m_cpuChartLayout = nullptr;
    QVBoxLayout *m_cpuLevelLayout= nullptr;
    // mem layout
    QVBoxLayout *m_memInfoLayout = nullptr;
    QHBoxLayout *m_memTitleHLayout = nullptr;
    QVBoxLayout *m_memTitleVLayout = nullptr;
    QVBoxLayout *m_memCurVLayout = nullptr;
    QHBoxLayout *m_memCurHLayout = nullptr;
    QHBoxLayout *m_memChartLayout = nullptr;
    QVBoxLayout *m_memLevelLayout = nullptr;
    // network layout
    QVBoxLayout *m_netInfoLayout = nullptr;
    QHBoxLayout *m_netTitleHLayout = nullptr;
    QVBoxLayout *m_netTitleVLayout = nullptr;
    QVBoxLayout *m_netCurVLayout = nullptr;
    QHBoxLayout *m_netCurHLayout = nullptr;
    QHBoxLayout *m_netChartLayout = nullptr;
    QVBoxLayout *m_netLevelLayout = nullptr;

    QGSettings *m_styleSettings = nullptr;
    Meminfo m_memInfo;
    QPalette m_controlPE;
    float m_fFontSize;

    //cpu
    unsigned long long m_prevCpuTotalTime = 0;
    unsigned long long m_prevCpuWorkTime = 0;
    unsigned long long m_cpuTotalTime = 0;
    unsigned long long m_cpuworkTime = 0;

    //network
    unsigned long long int m_totalRecvBytes = 0;
    unsigned long long int m_totalSentBytes = 0;
    unsigned long long int m_rateRecvBytes = 0;
    unsigned long long int m_rateSentBytes = 0;

    QTimer *m_updateStatusTimer = nullptr;
    QString m_currentThemeMode;
    QString m_strCurMemInfo;
    QString m_strCurSwapInfo;
};

#endif //__RESOURCEDLG_H__