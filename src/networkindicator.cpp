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

#include "networkindicator.h"
#include "smoothcurvegenerator.h"
#include "netcatogoryshow.h"
#include "util.h"

#include <QMouseEvent>
#include <QEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

#include <QPushButton>

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

NetworkIndicator::NetworkIndicator(QWidget *parent)
    : QWidget(parent)
    ,m_state(Normal)
    ,m_isChecked(false)
    ,m_rectTopPadding(9)
    ,m_rectTotalHeight(38)
    ,m_rectTotalWidth(58)
    ,m_outsideBorderColor(Qt::transparent)
    ,qtSettings(nullptr)
{
    const QByteArray idd(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(idd))
    {
        qtSettings = new QGSettings(idd);
    }

    this->setFixedSize(188, 56);

    initThemeMode();
    m_netMaxHeight = 30;
    m_pointSpace = 5;

    if(currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
    {
        this->m_bgColor = QColor(0xff,0xff,0xff,0);
    }
    else if(currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black")
    {
        this->m_bgColor = QColor(0x13,0x14,0x14,0);
    }

    m_pointsCount = int((this->width() -2) / m_pointSpace);
    m_downloadSpeedList = new QList<long>();
    for (int i = 0; i < m_pointsCount; i++) {
        m_downloadSpeedList->append(0);
    }

    m_uploadSpeedList = new QList<long>();
    for (int i = 0; i < m_pointsCount; i++) {
        m_uploadSpeedList->append(0);
    }

    m_gridY = new QList<int>();

    m_netCatogoryShow = new NetCatogoryShow;
    connect(this,SIGNAL(updateNetWorkData(long,long,long,long)),m_netCatogoryShow,SLOT(onUpdateNetworkStatus(long,long,long,long)));
    connect(this,SIGNAL(updateNetWorkData(long,long,long,long)),this,SLOT(setNetWorkText(long,long,long,long)));
    setNetworkState(Normal);
    m_netTitle = new QLabel(tr("net"));
    m_sentByte = new QLabel();
    m_recvByte = new QLabel();
    QFont font;
    font.setPointSize(9);
    m_sentByte->setFont(font);
    m_recvByte->setFont(font);

    initWidgets();
}

NetworkIndicator::~NetworkIndicator()
{
    delete m_downloadSpeedList;
    delete m_uploadSpeedList;
    delete m_gridY;

    if(qtSettings)
    {
        delete qtSettings;
    }
}

void NetworkIndicator::setNetWorkText(long revcTotalBtytes, long sentTotalBytes, long revcRateKbs, long sentRateKbs)
{
    const QString downloadRate = formatNetworkRate(revcRateKbs);
    const QString uploadRate = formatNetworkRate(sentRateKbs);
    QString showReceiveValue = downloadRate;
    QString sRecv = tr("recv:") + showReceiveValue;
    QString showSendValue = uploadRate;
    QString sSend = tr("send:") +showSendValue;
    m_recvByte->setText(sRecv);
    m_sentByte->setText(sSend);
}

void NetworkIndicator::initWidgets()
{
    QHBoxLayout *lineImg_H_BoxLayout = new QHBoxLayout();
    lineImg_H_BoxLayout->setSpacing(0);
    lineImg_H_BoxLayout->addSpacing(4);
    lineImg_H_BoxLayout->addWidget(m_netCatogoryShow);
    QVBoxLayout *content_V_BoxLayout = new QVBoxLayout();
    content_V_BoxLayout->addWidget(m_netTitle);
    content_V_BoxLayout->addWidget(m_recvByte);
    content_V_BoxLayout->addWidget(m_sentByte);
    lineImg_H_BoxLayout->addSpacing(2);
    lineImg_H_BoxLayout->addLayout(content_V_BoxLayout);
    lineImg_H_BoxLayout->addStretch();

    QVBoxLayout *main_V_BoxLayout = new QVBoxLayout;
    main_V_BoxLayout->addLayout(lineImg_H_BoxLayout);
    main_V_BoxLayout->setContentsMargins(0,0,0,0);
    this->setLayout(main_V_BoxLayout);

}

void NetworkIndicator::initThemeMode()
{
    if (!qtSettings) {
//        qWarning() << "Failed to load the gsettings: " << THEME_QT_SCHEMA;
        return;
    }

    //监听主题改变
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key)
    {
        if (key == "styleName")
        {
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            repaint();
            updateBgColor();
        }
    });
    currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
}

void NetworkIndicator::enterEvent(QEvent *event)
{
    setCursor(Qt::PointingHandCursor);

    if (!m_isChecked) {
        setNetworkState(Hover);
    }

    event->accept();
    //QWidget::enterEvent(event);
}

void NetworkIndicator::leaveEvent(QEvent *event)
{
    if (!m_isChecked) {
        setNetworkState(Normal);
    }

    event->accept();
    //QWidget::leaveEvent(event);
}

void NetworkIndicator::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    setNetworkState(Checked);

    event->accept();
    //QWidget::mousePressEvent(event);
}

void NetworkIndicator::mouseReleaseEvent(QMouseEvent *event)
{
    if (!rect().contains(event->pos()))
        return;

    m_isChecked = !m_isChecked;
    if (m_isChecked) {
        setNetworkState(Checked);
    } else {
        setNetworkState(Normal);
    }

    event->accept();
    //QWidget::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
        emit clicked();
}

void NetworkIndicator::mouseMoveEvent(QMouseEvent *event)
{
    if (!rect().contains(event->pos())) {
        setNetworkState(Normal);
    }
}

void NetworkIndicator::updateBgColor()
{
    switch (m_state) {
    case Hover:
        this->m_outsideBorderColor = Qt::transparent;
        if(currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
        {
            this->m_bgColor = QColor(13,14,14,49);  //#f6fcfe
        }
        else
        {
            this->m_bgColor = QColor(255,255,255,49);
        }
        break;
    case Press:
        this->m_outsideBorderColor = Qt::transparent;
        this->m_bgColor = QColor("#f6fcfe");
        break;
    case Checked:
//        this->m_outsideBorderColor = QColor("#009944");
        this->m_outsideBorderColor = Qt::transparent;
        qDebug()<<"1234567879.000";
//        this->m_outsideBorderColor = QColor("#0973b4");
        this->m_outsideBorderColor = Qt::transparent;
        if(currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
        {
            this->m_bgColor = QColor(0x13,0x14,0x14,19); //#e9f8fd
        }
        else
        {
            this->m_bgColor = QColor(0xff,0xff,0xff,9);
        }
        break;
    default:
        this->m_outsideBorderColor = Qt::transparent;
        if(currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
        {
            this->m_bgColor = QColor(0xff,0xff,0xff,0);
        }
        else if(currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black")
        {
            this->m_bgColor = QColor(0x13,0x14,0x14,0);
        }

        break;
    }
    repaint();
}

void NetworkIndicator::setNetworkState(NetworkIndicator::NetworkState state)
{
    if (m_state == state)
        return;

    m_state = state;
    updateBgColor();
}

void NetworkIndicator::setChecked(bool flag)
{
    m_isChecked = flag;
    if (m_isChecked){
        setNetworkState(Checked);
    } else {
        setNetworkState(Normal);
    }
}

bool NetworkIndicator::isChecked()
{
    return m_isChecked;
}

void NetworkIndicator::setTitle(const QString &title)
{
    this->m_title = title;
}

//void NetworkIndicator::updateNetworkPainterPath(QPainterPath downloadPath, QPainterPath uploadPath)
//{
//    this->m_downloadPath = downloadPath;
//    this->m_uploadPath = uploadPath;
//    repaint();
//}

NetworkIndicator::NetworkState NetworkIndicator::getNetworkState() const
{
    return m_state;
}

void NetworkIndicator::onUpdateNetworkStatus(long recvTotalBytes, long sentTotalBytes, long recvRateBytes, long sentRateBytes)
{
//    m_recvTotalBytes = recvTotalBytes;
//    m_sentTotalBytes = sentTotalBytes;
//    m_recvRateBytes = recvRateBytes;
//    m_sentRateBytes = sentRateBytes;

//    //download
//    QList<QPointF> downloadPoints;
//    m_downloadSpeedList->append(m_recvRateBytes);
//    if (m_downloadSpeedList->size() > m_pointsCount) {
//        m_downloadSpeedList->pop_front();
//    }
//    //计算出下载速度中最大的值
//    long downloadMaxHeight = 0;
//    for (int i = 0; i < m_downloadSpeedList->size(); i++) {
//        if (m_downloadSpeedList->at(i) > downloadMaxHeight) {
//            downloadMaxHeight = m_downloadSpeedList->at(i);
//        }
//    }
//    for (int i = 0; i < m_downloadSpeedList->size(); i++) {
//        if (downloadMaxHeight < m_netMaxHeight) {
//            downloadPoints.append(QPointF(i * m_pointSpace, m_downloadSpeedList->at(i)));
//        }
//        else {
//            downloadPoints.append(QPointF(i * m_pointSpace, m_downloadSpeedList->at(i) * m_netMaxHeight / downloadMaxHeight));
//        }
//    }
//    m_downloadPath = SmoothCurveGenerator::generateSmoothCurve(downloadPoints);

//    //upload
//    QList<QPointF> uploadPoints;
//    m_uploadSpeedList->append(m_sentRateBytes);
//    if (m_uploadSpeedList->size() > m_pointsCount) {
//        m_uploadSpeedList->pop_front();
//    }
//    //计算出上传速度中最大的值
//    long uploadMaxHeight = 0;
//    for (int i = 0; i < m_uploadSpeedList->size(); i++) {
//        if (m_uploadSpeedList->at(i) > uploadMaxHeight) {
//            uploadMaxHeight = m_uploadSpeedList->at(i);
//        }
//    }

//    for (int i = 0; i < m_uploadSpeedList->size(); i++) {
//        if (uploadMaxHeight < m_netMaxHeight) {
//            uploadPoints.append(QPointF(i * m_pointSpace, m_uploadSpeedList->at(i)));
//        }
//        else {
//            uploadPoints.append(QPointF(i * m_pointSpace, m_uploadSpeedList->at(i) * m_netMaxHeight / uploadMaxHeight));
//        }
//    }
//    m_uploadPath = SmoothCurveGenerator::generateSmoothCurve(uploadPoints);

//    repaint();
    emit updateNetWorkData(recvTotalBytes, sentTotalBytes, recvRateBytes, sentRateBytes);
}

void NetworkIndicator::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    //border of rect
    QPainterPath borderPath;
    borderPath.addRoundedRect(this->rect(), 0, 0);
    QPen pen(this->m_outsideBorderColor, 1);
    painter.setPen(pen);
    painter.drawPath(borderPath);

    //background of rect
    painter.setOpacity(1);
    QPainterPath path;
    path.addRoundedRect(QRectF(1, 1, width()-2, height()-2),4,4);
    painter.fillPath(path, this->m_bgColor);

//    painter.translate((rect().width() - m_pointsCount * m_pointSpace) / 2 + 2, 40);//将坐标第原点移动到该点
//    painter.scale(1, -1);//将横坐标扩大1倍,将纵坐标缩小1倍
//    //使用QPainterPath画贝塞尔曲线
//    painter.setPen(QPen(QColor("#009944"), 1));
//    painter.setBrush(QBrush());
//    painter.drawPath(m_downloadPath);//绘制前面创建的path:m_downloadPath
//    painter.translate(0, -8);//将点（0，-8）设为原点
//    painter.setPen(QPen(QColor("#e60012"), 1));
//    painter.setBrush(QBrush());
//    painter.drawPath(m_uploadPath);

//    QWidget::paintEvent(event);
}

