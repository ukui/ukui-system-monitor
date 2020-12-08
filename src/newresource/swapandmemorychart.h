#ifndef _SWAPANDMEMORYCHART_H
#define _SWAPANDMEMORYCHART_H

#include <QWidget>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <glibtop/mem.h>
#include <glibtop/swap.h>

#include "smoothlinegenerator.h"

static int i1 =1;

class SwapAndMemoryChart : public QWidget
{
    Q_OBJECT
public:
    SwapAndMemoryChart(QWidget *parent = 0);
    ~SwapAndMemoryChart();
    void refreshMemoryAndSwapData(float,double,float,double);
protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QColor m_outsideBorderColor;
    QColor m_memoryColor;
    QColor m_swapColor;
    QColor m_bgColor;

    float m_memoryData;
    float m_swapData;

    QList<float> *m_memoryDataList;
    QList<float> *m_swapDataList;

    int m_pointsCount;
    int m_pointSpace;

    int m_memoryMaxHeight;
    int m_swapMaxHeight;
    QPainterPath m_memoryPath;
    QPainterPath m_swapPath;
    int countWidth;

    QList<qreal> xList; //x数据链
    QList<qreal> yDrawList; //y数据链
    int showValue;

public slots:
    void onUpdateMemoryAndSwapData(float,double,float,double);
};

#endif //#define _SWAPANDMEMORYCHART_H
