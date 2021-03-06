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

#include "newaboutdialog.h"
#include "../shell/xatom-helper.h"
#include "util.h"
#include <QDebug>
#include <QTextEdit>
#include <QDir>
#include <QStyleOption>
#include <QProcess>

newaboutdialog::newaboutdialog(QWidget *parent) : QDialog(parent)
{
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    this->setWindowFlags(this->windowFlags() /*| Qt::FramelessWindowHint*/ | Qt::Tool);
//    this->setWindowFlags(Qt::Dialog |  Qt::CustomizeWindowHint); this can make dialog be moveable and be showable too
    this->setFixedHeight(450);
    this->setFixedWidth(420);
    initWidgets();
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    this->setWindowTitle(tr("Kylin System Monitor"));
}

newaboutdialog::~newaboutdialog()
{
}

void newaboutdialog::initWidgets()
{
    pe.setColor(QPalette::WindowText,QColor(59,59,59));

    initTitleWidget();
    initContentWidget();
    initIntroduceWidget();

    main_V_BoxLayout = new QVBoxLayout();
    main_V_BoxLayout->setSpacing(0);
    main_V_BoxLayout->addSpacing(40);
    main_V_BoxLayout->addLayout(img_H_Boxlayout);
    main_V_BoxLayout->addSpacing(16);
    main_V_BoxLayout->addLayout(app_H_Boxlayout);
    main_V_BoxLayout->addSpacing(12);
    main_V_BoxLayout->addLayout(version_H_Boxlayout);
    main_V_BoxLayout->addSpacing(12);
    main_V_BoxLayout->addLayout(introduce_H_Boxlayout);
    main_V_BoxLayout->addSpacing(24);
    main_V_BoxLayout->addLayout(support_H_Boxlayout);
    main_V_BoxLayout->addStretch(1);
    main_V_BoxLayout->setContentsMargins(0,0,0,0);
    this->setLayout(main_V_BoxLayout);
}

void newaboutdialog::initTitleWidget()
{
    QHBoxLayout *topLeftTilte_H_BoxLayout = new QHBoxLayout();
    QLabel *m_titleImgLabel = new QLabel();
    QPixmap pixmap("/usr/share/icons/hicolor/png/2-24*24/ukui-system-monitor.png");
    m_titleImgLabel->setPixmap(pixmap);
    QLabel *m_titletextLabel = new QLabel;
    m_titletextLabel->setText(tr("Kylin System Monitor"));
    setFontSize(m_titletextLabel,FontSize);
    topLeftTilte_H_BoxLayout->addWidget(m_titleImgLabel);
    topLeftTilte_H_BoxLayout->addSpacing(10);
    topLeftTilte_H_BoxLayout->addWidget(m_titletextLabel);
    topLeftTilte_H_BoxLayout->addStretch(1);
    topLeftTilte_H_BoxLayout->setContentsMargins(8,8,0,0);

    QHBoxLayout *topRightTitle_H_BoxLayout = new QHBoxLayout();
    m_closeBtn = new QToolButton();
    m_closeBtn->setToolTip(tr("close"));
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_closeBtn->setAutoRaise(true);
    m_closeBtn->setProperty("isWindowButton", 0x2);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x8);
    m_closeBtn->setFixedSize(30,30);
//    closeBtn->setIconSize(QSize(20,20));
    connect(m_closeBtn, &QToolButton::clicked, this, [=](){
        this->deleteLater();
        this->close();
    });
    topRightTitle_H_BoxLayout->addWidget(m_closeBtn);
    topRightTitle_H_BoxLayout->setContentsMargins(0,4,4,0);

    title_H_BoxLayout = new QHBoxLayout();
    title_H_BoxLayout->addLayout(topLeftTilte_H_BoxLayout);
    title_H_BoxLayout->addLayout(topRightTitle_H_BoxLayout);
    title_H_BoxLayout->setContentsMargins(0,0,0,0);
}

void newaboutdialog::initContentWidget()
{
    QLabel *bigImgLabel = new QLabel();
    QLabel *appLabel = new QLabel();
    QLabel *versionLabel = new QLabel();

    QPixmap pixmap("/usr/share/icons/hicolor/png/96*96/ukui-system-monitor.png");
    bigImgLabel->setPixmap(pixmap);

    appLabel->setText(tr("kylin system monitor"));
    setFontSize(appLabel,FontSize);

    QString versionText;
    QString tmpPath = "/tmp/ukui-system-monitor-" + QDir::home().dirName();
    QString cmd = "dpkg -l |grep ukui-system-monitor >" + tmpPath;
    //qDebug()<<"what is cmd"<<cmd;
    system(cmd.toUtf8().data());
    QFile file(tmpPath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        QString line;
        while(!stream.atEnd())
        {
            line = stream.readLine();
            QStringList fileList = line.split(" ");
            fileList.removeAll("");
            versionText = fileList[2];
        }
        file.close();
    }
    versionLabel->setText(tr("version: ") + versionText);  //此处需要获取的是当前版本系统监视器的版本号
    versionLabel->setPalette(pe);
    setFontSize(versionLabel,FontSize);

    img_H_Boxlayout = new QHBoxLayout();
    img_H_Boxlayout->addWidget(bigImgLabel,1,Qt::AlignHCenter);

    app_H_Boxlayout = new QHBoxLayout();
    app_H_Boxlayout->addWidget(appLabel,1,Qt::AlignHCenter);

    version_H_Boxlayout = new QHBoxLayout();
    version_H_Boxlayout->addWidget(versionLabel,1,Qt::AlignHCenter);

}

void newaboutdialog::initIntroduceWidget()
{
    QLabel *introduceLabel = new QLabel();
    introduceLabel->setText(tr("System monitor is a desktop application that face desktop users of Kylin operating system,"
                               "It meets the needs of users to monitor the system process, system resources and file system"));
    introduceLabel->setPalette(pe);
    setFontSize(introduceLabel,FontSize);
    introduceLabel->adjustSize();
    introduceLabel->setFixedWidth(356);
    introduceLabel->setWordWrap(true);
    introduceLabel->setAlignment(Qt::AlignTop);
//    QTextEdit *edit = new QTextEdit();
//    edit->setFrameShape(QTextEdit::NoFrame);
//    edit->setEnabled(false);
//    edit->setText(tr("System monitor is a desktop application that face desktop users of Kylin operating system,"
//                     "It meets the needs of users to monitor the system process, system resources and file system"));
    introduce_H_Boxlayout = new QHBoxLayout();
    introduce_H_Boxlayout->addWidget(introduceLabel);
    introduce_H_Boxlayout->setContentsMargins(32,0,32,0);

    QLabel *supporLabel = new QLabel();
    supporLabel->setText(QString("%1 <a href='support@kylinos.cn'>support@kylinos.cn</a>").arg(tr("Service and support team:")));
    supporLabel->setPalette(pe);
    connect(supporLabel,SIGNAL(linkActivated(QString)),this,SLOT(openMailTo(QString))); 
    setFontSize(supporLabel,FontSize);
    support_H_Boxlayout = new QHBoxLayout();
    support_H_Boxlayout->addWidget(supporLabel);
    support_H_Boxlayout->setContentsMargins(32,0,32,0);
}

//void newaboutdialog::paintEvent(QPaintEvent *event)
//{
//    QPainter painter(this);

//    //绘制圆角矩形
////    painter.setPen(QPen(QColor("#808080"), 0));//边框颜色 #3f96e4
////    painter.setBrush(this->palette().base());//背景色
//    painter.setRenderHint(QPainter::Antialiasing, true);
//    painter.setOpacity(1);
////    QRectF r(0 / 2.0, 0 / 2.0, width() - 0, height() - 0);//左边 上边 右边 下边
////    painter.drawRoundedRect(r, 6, 6);


//    //绘制背景色
//    QDialog::paintEvent(event);
////    QPainterPath path;
////    auto rect = this->rect();
////    rect.adjust(1, 1, -1, -1);
////    path.addRoundedRect(rect, 6, 6);
////    setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));

////    QStyleOption opt;
////    opt.init(this);
////    QPainter p(this);
////    QRect rectReal = this->rect();
////    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
////    p.setBrush(opt.palette.color(QPalette::Base));
////    p.setPen(Qt::NoPen);
////    p.drawRoundedRect(rectReal, 6, 6);
////    QWidget::paintEvent(event);
//}

void newaboutdialog::setFontSize(QLabel *label,int fontSize)
{
    QFont font;
    font.setPointSize(fontSize);
    label->setFont(font);
}

void newaboutdialog::openMailTo(QString strMailAddr)
{
    QProcess *process =new QProcess(this);
    process->startDetached(QString("claws-mail mailto://%1").arg(strMailAddr));
    process->deleteLater();
}