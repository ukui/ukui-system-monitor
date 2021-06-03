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
#include <QGSettings>
#include <QScrollArea>

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

    void initThemeMode();
    void initFontSize();
    void tosetFontSize();
    void getsetFontSize(int fSize,QLabel *label);
    void setChangeNetSpeedLabel();

    // main Wnd close
    virtual void onWndClose();

private:

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

    QLabel *onehundredLabel;
    QLabel *threequarterLabel;
    QLabel *halfLabel;
    QLabel *onequarterLabel;
    QLabel *nullLabel;

    QLabel *tenGibLabel;
    QLabel *sevenpointFiveGibLabel;
    QLabel *fiveGibLabel;
    QLabel *twopointfiveGibLabel;
    QLabel *nullGibLabel;

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

    QGSettings *qtSettings;
    QString currentThemeMode;

    QScrollArea *m_scrollArea;
    QFont font;

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
    void networkSetText(quint64 recvTotalBytes, quint64 sentTotalBytes, quint64 recvRateBytes, quint64 sentRateBytes);
    void onUpdateMemoryAndSwapStatus();

signals:
    void updateCpuStatus(double percent);
    void rebackMemoryAndSwapInfo(const QString &infoMemory, double percentMemory,const QString &infoSwap, double percentSwap);
    void rebackMemoryAndSwapData(float memory,double percentMemory,float swap,double percentSwap);
    void updateMemoryAndSwapStatus();
    void updateNetworkStatus(quint64 recvTotalBytes, quint64 sentTotalBytes, quint64 recvRateBytes, quint64 sentRateBytes);
    void resetWidget();
};
