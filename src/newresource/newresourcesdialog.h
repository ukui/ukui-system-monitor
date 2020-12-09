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

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QBoxLayout>
#include <qgsettings.h>
#include <QFont>
#include <QCheckBox>
#include <QTimer>
#include <glibtop/mem.h>
#include <glibtop/swap.h>

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

class NewResouresDialog : public QWidget
{
    Q_OBJECT

public:
    explicit NewResouresDialog(QWidget* parent = 0);
    ~NewResouresDialog();

    void startCpuTimer();
    void stopCpuTimer();

    void initFontSize();
    void tosetFontSize();
    void setChangeNetSpeedLabel();

private:

    //cpu
    unsigned long long m_prevCpuTotalTime;
    unsigned long long m_prevCpuWorkTime;
    unsigned long long m_cpuTotalTime;
    unsigned long long m_cpuworkTime;

    //network
    unsigned long long int m_totalRecvBytes;
    unsigned long long int m_totalSentBytes;
    unsigned long long int m_rateRecvBytes;
    unsigned long long int m_rateSentBytes;

    QTimer *updateStatusTimer = nullptr;
    QHBoxLayout *m_hlayout = nullptr;

    QLabel *cpuHistoryLabel = nullptr;
    QLabel *memoryAndSwapLabel = nullptr;
    QLabel *netWorkFlowLabel = nullptr;
    QLabel *cpuUnitDataLabel = nullptr;
    QLabel *memoryUnitDataLabel = nullptr;
    QLabel *swapUnitDataLabel = nullptr;
    QLabel *netrecvUnitDataLabel = nullptr;
    QLabel *netsentUnitDataLabel = nullptr;

    QLabel *theFirtSpeedLabel = nullptr;
    QLabel *theSecondSpeedLabel = nullptr;
    QLabel *theThirdSpeedLabel = nullptr;
    QLabel *theFourthSpeedLabel = nullptr;
    QLabel *theFifthSpeedLabel = nullptr;

    QWidget *cpuWidget = nullptr;
    QWidget *swapmemoryWiget = nullptr;
    QWidget *networkWidget = nullptr;

    QGSettings *fontSettings;
    sigCheck *typeCheckCpu;
    sigCheck *typeCheckMemory;
    sigCheck *typeCheckSwap;
    sigCheck *typeCheckNetRecv;
    sigCheck *typeCheckNetSent;

    CpuHistoryChart *histoyChart;
    SwapAndMemoryChart *swapandmemoryChart;
    NetWorkChart *networkChart;
    Meminfo mi;
    QPalette pe;

    QHBoxLayout *cputitle_H_BoxLayout;
    QHBoxLayout *cpuUnit_H_BoxLayout;
    QHBoxLayout *cpuChart_H_BoxLayout;

    QHBoxLayout *swapmemoryTitle_H_BoxLayout;
    QHBoxLayout *swapmemoryUnit_H_BoxLayout;
    QHBoxLayout *memoryswapChart_H_BoxLayout;

    QHBoxLayout *netWorkTitle_H_BoxLayout;
    QHBoxLayout *networkUnit_H_BoxLayout;
    QHBoxLayout *networkChart_H_BoxLayout;

    QVBoxLayout *main_V_BoxLayout;

public:
    int fontSize;
private:
    void initWidget();
    void initCpuHistory();
    void initSwapMeomoryHistory();
    void initNetSpeedHistory();
public slots:
    void updateResourceStatus();
    void cpuHistoySetText(double percent);
    void memoryandswapSetText(const QString &infoMemory, double percentMemory,const QString &infoSwap, double percentSwap);
    void networkSetText(long recvTotalBytes, long sentTotalBytes, long recvRateBytes, long sentRateBytes);
    void onUpdateMemoryAndSwapStatus();

signals:
    void updateCpuStatus(double percent);
    void rebackMemoryAndSwapInfo(const QString &infoMemory, double percentMemory,const QString &infoSwap, double percentSwap);
    void rebackMemoryAndSwapData(float memory,double percentMemory,float swap,double percentSwap);
    void updateMemoryAndSwapStatus();
    void updateNetworkStatus(long recvTotalBytes, long sentTotalBytes, long recvRateBytes, long sentRateBytes);
    void resetWidget();
};
