#ifndef _NETWORKCHART_H_
#define _NETWORKCHSRT_H_

#include <QWidget>
#include <QStyleOption>
#include <QPainterPath>

class NetWorkChart : public QWidget
{
    Q_OBJECT
public:
    NetWorkChart(QWidget *parent = 0);
    ~NetWorkChart();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QColor m_outsideBorderColor;
    QColor m_downLoadColor;
    QColor m_upLoadColor;

    QPainterPath m_downLoadPath;
    QPainterPath m_upLoadPath;
    int m_pointsCount;
    QColor m_bgColor;
    double m_downLoadSpeed;
    double m_downLoadMaxHeight;
    double m_upLoadSpeed;
    double m_upLoadMaxHeight;

    QList<double> *m_downLoadList;
    QList<double> *m_upLoadList;

public slots:
    void onUpdateDownloadAndUploadData(long,long,long,long);
signals:
//    void speedToMib();
    void speedToLowKib();
//    void speedToMiddleKib();
    void speedToHighKib();
};

#endif //_NETWORKCHSRT_H_
