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

//public slots:
//    void updateResourceStatus();
//    void onSwitchResoucesPage(int index);

//signals:
//    void updateCpuStatus(double percent);
//    void updateMemoryStatus();
//    void updateNetworkStatus(long recvTotalBytes, long sentTotalBytes, long recvRateBytes, long sentRateBytes);

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

public:
    int fontSize;
public slots:
    void updateResourceStatus();
    void cpuHistoySetText(double percent);
    void memoryandswapSetText(const QString &infoMemory, double percentMemory,const QString &infoSwap, double percentSwap);
    void networkSetText(long recvTotalBytes, long sentTotalBytes, long recvRateBytes, long sentRateBytes);
    void onUpdateMemoryAndSwapStatus();
    void initWidget();

signals:
    void updateCpuStatus(double percent);
    void rebackMemoryAndSwapInfo(const QString &infoMemory, double percentMemory,const QString &infoSwap, double percentSwap);
    void rebackMemoryAndSwapData(float memory,double percentMemory,float swap,double percentSwap);
//    void rebackSwapInfo(const QString &info, double percent);
    void updateMemoryAndSwapStatus();
    void updateNetworkStatus(long recvTotalBytes, long sentTotalBytes, long recvRateBytes, long sentRateBytes);
    void resetWidget();
};
