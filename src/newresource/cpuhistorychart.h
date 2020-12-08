#ifndef _CPUHISTORYCHART_H_
#define _CPUHISTORYCHART_H_

#include <QWidget>
#include <QStyleOption>
#include <QPainterPath>
#include <QMap>

static int i =1;

class CpuHistoryChart : public QWidget
{
    Q_OBJECT
public:
    CpuHistoryChart(QWidget *parent = 0);
    ~CpuHistoryChart();
    void refreshCpuHistoryData(double cpu);

protected:
    void paintEvent(QPaintEvent *event);

private:   
    QPointF point;
    QColor m_outsideBorderColor;
    QColor m_cpuHistoryColor;

    QPainterPath m_cpuHistoryPath;
    QList<double> *m_CpuHistoryList;
    QList<qreal> xList; //x数据链
    QList<qreal> yDrawList; //y数据链

    int m_pointsCount;
    QColor m_bgColor;
    double m_Cpu;
    double m_cpuMaxHeight;

    int showValue;

public slots:
    void onUpdateCpuPercent(double value);
//    void onUpdateCpuHistoryStatus(double value);
};

#endif //_CPUHISTORYCHART_H_
