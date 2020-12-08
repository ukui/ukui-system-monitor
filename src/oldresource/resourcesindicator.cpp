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


#include "resourcesindicator.h"

#include <QMouseEvent>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

ResourcesIndicator::ResourcesIndicator(int flag, QWidget *parent)
    : QWidget(parent)
    ,m_state(Normal)
    ,m_isChecked(false)
    ,m_currentPercent(0)
    ,m_posX(5)
    ,m_rectTopPadding(9)
    ,m_rectTotalHeight(38)
    ,m_rectTotalWidth(58)
    ,m_outsideBorderColor(Qt::transparent)
    ,qtSettings(nullptr)
{
    typeObject = flag;
    const QByteArray idd(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(idd))
    {
        qtSettings = new QGSettings(idd);
    }

    this->setFixedSize(188, 56);

    initThemeMode();

    if(currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
    {
        this->m_bgColor = QColor(0xff,0xff,0xff,0);    //button's lowest background
    }
    else if(currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black")
    {
        this->m_bgColor = QColor(0x13,0x14,0x14,0);
    }

    if (flag == 0) {
        m_borderColor = QColor(0x09,0x73,0xb4,0xff);
    }
    else if (flag == 1) {
        m_borderColor = QColor(0x95,0x28,0xb4,0xff);
    }
    else {
        m_borderColor = Qt::transparent;
    }
    setResourcesState(Normal);
}

ResourcesIndicator::~ResourcesIndicator()
{
}

void ResourcesIndicator::initThemeMode()
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

void ResourcesIndicator::enterEvent(QEvent *event)
{
    setCursor(Qt::PointingHandCursor);

    if (!m_isChecked) {
        setResourcesState(Hover);
    }

    event->accept();
    //QWidget::enterEvent(event);
}

void ResourcesIndicator::leaveEvent(QEvent *event)
{
    if (!m_isChecked) {
        setResourcesState(Normal);
    }

    event->accept();
    //QWidget::leaveEvent(event);
}

void ResourcesIndicator::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    setResourcesState(Checked);
    event->accept();
    //QWidget::mousePressEvent(event);
}

void ResourcesIndicator::mouseReleaseEvent(QMouseEvent *event)
{
    if (!rect().contains(event->pos()))
        return;

    m_isChecked = !m_isChecked;
    if (m_isChecked) {
        setResourcesState(Checked);
    } else {
        setResourcesState(Normal);
    }

    event->accept();
    //QWidget::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
        emit clicked();
}

void ResourcesIndicator::mouseMoveEvent(QMouseEvent *event)
{
    if (!rect().contains(event->pos())) {
        setResourcesState(Normal);
    }
}

void ResourcesIndicator::updateBgColor()
{
    switch (m_state) {
    case Hover:
        this->m_outsideBorderColor = Qt::transparent;
        if(currentThemeMode == "ukui-light"  || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
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
//        this->m_outsideBorderColor = QColor("#0973b4");
        this->m_outsideBorderColor = Qt::transparent;
        if (currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white") {
            this->m_bgColor = QColor(0x13,0x14,0x14,19); //#e9f8fd
        } else {
            this->m_bgColor = QColor(0xff,0xff,0xff,9);
        }
//        this->m_bgColor = palette().color(QPalette::Base);
        break;
    default:
        this->m_outsideBorderColor = Qt::transparent;
//        this->m_bgColor = QColor("#ffffff");
//        this->m_outsideBorderColor = palette().color(QPalette::Base);
        if (currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white") {
            this->m_bgColor = QColor(0xff,0xff,0xff,0);
        } else if (currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black") {
            this->m_bgColor = QColor(0x13,0x14,0x14,0);
        }
        break;
    }
    repaint();
}

void ResourcesIndicator::setResourcesState(ResourcesIndicator::ResourcesState state)
{
    if (m_state == state)
        return;
    m_state = state;
    updateBgColor();
}

void ResourcesIndicator::setChecked(bool flag)
{
    m_isChecked = flag;
    if (m_isChecked){
        setResourcesState(Checked);
    } else {
        setResourcesState(Normal);
    }
}

bool ResourcesIndicator::isChecked()
{
    return m_isChecked;
}

void ResourcesIndicator::setTitle(const QString &title)
{
    this->m_title = title;
}

void ResourcesIndicator::updatePercentAndInfo(double percent, const QString &info)
{

    this->m_currentPercent = static_cast<int>(percent);
    this->m_info = info;
    repaint();
}

ResourcesIndicator::ResourcesState ResourcesIndicator::getResourcesState() const
{
    return m_state;
}

void ResourcesIndicator::paintEvent(QPaintEvent *event)
{
    //渐变填充
    /*QRadialGradient gradient(50, 50, 50, 50, 50);
    gradient.setColorAt(0, QColor::fromRgbF(0, 1, 0, 1));
    gradient.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));
    QBrush brush(gradient);*/

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    //border of rect
    QPainterPath borderPath;
    borderPath.addRoundedRect(this->rect(), 0, 0);
    QPen pen(this->m_outsideBorderColor, 1);
//    QPen pen(QColor("#B22222"), 1);
    painter.setPen(pen);
    painter.drawPath(borderPath);

    //background of rect
    painter.setOpacity(1);
    QPainterPath path;
    path.addRoundedRect(QRectF(1, 1, width()-2, height()-2),4,4);
    painter.fillPath(path, this->m_bgColor);
//    painter.fillPath(path, QColor("#f6fcfe"));

    int convertPercent = static_cast<int>(m_rectTotalHeight*this->m_currentPercent/100);

    //percent rect
    painter.setPen(QPen(this->m_borderColor, 1));//设置画笔颜色、宽度
//    painter.setBrush(QColor("#ffffff"));//设置画刷颜色   //#ffffff
//    painter.setBrush(palette().color(QPalette::Base));

//    painter.setBrush(QColor("#CC00FF"));
    if(typeObject == 0)
    {
        painter.setOpacity(0.1);
        painter.setBrush(QColor("#0973b4"));
    }

    if(typeObject == 1)
    {
        painter.setOpacity(0.1);
        painter.setBrush(QColor("#9528b4"));
    }
    QRect pRect(this->m_posX, this->m_rectTopPadding, this->m_rectTotalWidth, this->m_rectTotalHeight );
//    QRect pRect(this->m_posX,this->m_rectTopPadding -4,this->m_rectTotalWidth +4,this->m_rectTotalHeight + 4);
    painter.drawRoundedRect(pRect, 4, 4);
//    painter.drawRect(this->m_posX, this->m_rectTopPadding, this->m_rectTotalWidth, this->m_rectTotalHeight );
//    painter.drawRect(this->m_posX, this->m_rectTopPadding + this->m_rectTotalHeight - convertPercent, this->m_rectTotalWidth, convertPercent);

//    QPainterPath picPath;
//    picPath.addRoundedRect();


    if(typeObject == 0)
    {
        painter.setOpacity(0.46);
        painter.setBrush(QColor("#0973b4"));
    }

    if(typeObject == 1)
    {
        painter.setOpacity(0.46);
        painter.setBrush(QColor("#9528b4"));
    }

//    painter.drawRect(this->m_posX, this->m_rectTopPadding, this->m_rectTotalWidth, this->height()-(this->m_rectTotalHeight - convertPercent));

    QRect occupyRect(this->m_posX, this->m_rectTopPadding + this->m_rectTotalHeight - convertPercent, this->m_rectTotalWidth, convertPercent);
//    painter.drawRoundedRect(occupyRect);
    painter.drawRect(this->m_posX, this->m_rectTopPadding + this->m_rectTotalHeight - convertPercent, this->m_rectTotalWidth, convertPercent);
//    qDebug()<<"this->->->->"<<this->m_posX<<this->m_rectTopPadding + this->m_rectTotalHeight - convertPercent<<this->m_rectTotalWidth<<convertPercent;

    //draw title
    painter.setOpacity(0.91);
    painter.setRenderHint(QPainter::Antialiasing, false);
    QFont font = painter.font();
    font.setPixelSize(16);
    painter.setFont(font);
    painter.setPen(palette().color(QPalette::WindowText));      //set the color of the typeface
    painter.drawText(QRect(68, 1, 118, 28), Qt::AlignLeft | Qt::AlignVCenter, this->m_title);

    //draw content ,draw cpu and swap contents here
    font.setPixelSize(12);
    painter.setFont(font);
    QFontMetrics fm(font);
    QString text = fm.elidedText(this->m_info, Qt::ElideRight, 120);
    painter.drawText(QRect(68, 30, 120, 20), Qt::AlignLeft | Qt::AlignVCenter, text);

    QWidget::paintEvent(event);
}
