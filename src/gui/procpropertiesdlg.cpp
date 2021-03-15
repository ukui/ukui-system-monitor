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

#include "procpropertiesdlg.h"
#include "../process/process_list.h"
#include "../process/process_monitor.h"
#include "../util.h"
#include "../shell/xatom-helper.h"
#include "../shell/macro.h"

#include <QApplication>
#include <QDateTime>
#include <QPushButton>
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QScreen>
#include <QWidget>
#include <QFileInfo>
#include <QIcon>
#include <QLineEdit>

#define PROC_GRIDITEM_SPACING   6
#define PROC_GRIDLAYOUT_WIDTH   340
#define PROC_GRIDITEM_BOARD     2
#define PROC_GRIDITEM_TITLE_WIDTH  140
#define PROC_GRIDITEM_CONTENT_WIDTH  190
#define PROC_DIALOGTITLE_WIDTH  240

using namespace sysmonitor::process;

ProcPropertiesDlg::ProcPropertiesDlg(pid_t processId, QWidget *parent) : QDialog(parent)
  , mousePressed(false)
  , fontSettings(nullptr)
{
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint | Qt::Tool);

    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setAttribute(Qt::WA_Resized, false);

    this->setFixedWidth(380);

    const QByteArray id(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(id))
    {
        fontSettings = new QGSettings(id);
    }

    initFontSize();

    pid = processId;

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_topLayout = new QHBoxLayout;
    m_topLeftLayout = new QHBoxLayout;
    m_topLeftLayout->setContentsMargins(20, 20, 0, 0);
    m_topLeftLayout->setSpacing(10);
    m_topRightLayout = new QHBoxLayout;
    m_topRightLayout->setMargin(0);
    m_topRightLayout->setSpacing(5);
    m_topLayout->addLayout(m_topLeftLayout);
    m_topLayout->addStretch();
    m_topLayout->addLayout(m_topRightLayout);


    m_logoLabel = new QLabel();
    m_logoLabel->setFixedSize(44, 58);
    m_logoLabel->setContentsMargins(0, 0, 0, 0);
    m_logoLabel->setPixmap(QPixmap(":/res/sub_logo.png"));

    closeButton = new QPushButton(this);
    closeButton->setProperty("isWindowButton", 0x2);
    closeButton->setProperty("useIconHighlightEffect", 0x8);
    closeButton->setObjectName("CloseButton");
    closeButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeButton->setFlat(true);
    connect(closeButton, &QPushButton::clicked, this, [=]() {
       this->deleteLater();
       this->close();
    });
    closeButton->setProperty("isWindowButton", 0x2);
    closeButton->setProperty("useIconHighlightEffect", 0x8);

    m_iconLabel = new QLabel();
    m_iconLabel->setFixedSize(48, 48);
    m_iconLabel->setContentsMargins(0, 0, 0, 0);

    m_titleLabel = new QLabel();
    m_titleLabel->setFixedWidth(PROC_DIALOGTITLE_WIDTH);
    m_titleLabel->setWordWrap(true);

    m_topLeftLayout->addWidget(m_iconLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_topLeftLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);

    m_topRightLayout->addWidget(m_logoLabel, 0, Qt::AlignTop | Qt::AlignRight);
    m_topRightLayout->addWidget(closeButton, 0, Qt::AlignTop | Qt::AlignRight);
    m_topRightLayout->setContentsMargins(0,8,8,0);

    QLabel *topSplit = new QLabel();
    topSplit->setFixedSize(320, 1);

    QLabel *bottomSplit = new QLabel();
    bottomSplit->setFixedSize(320, 1);

    m_infoFrame = new QFrame;
    m_infoFrame->setMaximumWidth(PROC_GRIDLAYOUT_WIDTH);
    m_infoFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_bottomLayout = new QHBoxLayout;
    m_bottomLayout->setContentsMargins(0,0,20,0);
    m_bottomLayout->setSpacing(0);
    m_okBtn = new QPushButton;
    m_okBtn->setFixedSize(91, 30);
    m_okBtn->setObjectName("blackButton");
    m_okBtn->setFocusPolicy(Qt::NoFocus);
    m_okBtn->setText(tr("OK"));
    connect(m_okBtn, &QPushButton::clicked, this, [=] {
        this->close();
    });
    m_bottomLayout->addWidget(m_okBtn, 0, Qt::AlignBottom | Qt::AlignRight);

    m_layout->addLayout(m_topLayout);
    m_layout->addSpacing(10);
    m_layout->addWidget(topSplit, 0, Qt::AlignCenter);
    m_layout->addSpacing(10);
    m_layout->addWidget(m_infoFrame, 0, Qt::AlignCenter);
    m_layout->addSpacing(5);
    m_layout->addWidget(bottomSplit, 0, Qt::AlignCenter);
    m_layout->addSpacing(5);
    m_layout->addLayout(m_bottomLayout);
    m_layout->addSpacing(10);
    m_layout->addStretch();

    QGridLayout *infoGrid = new QGridLayout(m_infoFrame);
    infoGrid->setMargin(0);
    infoGrid->setHorizontalSpacing(PROC_GRIDITEM_SPACING);
    infoGrid->setVerticalSpacing(PROC_GRIDITEM_SPACING);
    infoGrid->setColumnStretch(0, 10);
    infoGrid->setColumnStretch(1, 100);

    m_listTitleValue << QObject::tr("User name:") << QObject::tr("Process name:") << QObject::tr("Command line:") << QObject::tr("Started Time:") << QObject::tr("CPU Time:");
    for (int i = 0; i < m_listTitleValue.length(); ++i) {
        QLabel *titleLabel = new QLabel();
        titleLabel->setMinimumHeight(20);
        titleLabel->setMaximumWidth(PROC_GRIDITEM_TITLE_WIDTH);
        QString ShowValue = getElidedText(titleLabel->font(), m_listTitleValue.value(i), PROC_GRIDITEM_TITLE_WIDTH-PROC_GRIDITEM_BOARD);
        titleLabel->setText(ShowValue);
        if (ShowValue != m_listTitleValue.value(i))
            titleLabel->setToolTip(m_listTitleValue.value(i));
        m_labelTitleList << titleLabel;

        QLabel *infoLabel = new QLabel();
        infoLabel->setMinimumHeight(28);
        infoLabel->setMaximumWidth(PROC_GRIDITEM_CONTENT_WIDTH);
        infoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_labelList << infoLabel;
        infoGrid->addWidget(titleLabel);
        infoGrid->addWidget(infoLabel);
    }

    this->moveToCenter();

    this->initProcproperties();

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(refreshProcproperties()));
    timer->start(3000);
}

ProcPropertiesDlg::~ProcPropertiesDlg()
{
    if (timer != NULL) {
        disconnect(timer,SIGNAL(timeout()),this,SLOT(refreshProcproperties()));
        if(timer->isActive()) {
            timer->stop();
        }
    }

    m_labelList.clear();
    m_labelTitleList.clear();

    if (fontSettings) {
        delete fontSettings;
        fontSettings = nullptr;
    }
}

void ProcPropertiesDlg::initFontSize()
{
    if (!fontSettings) {
        fontSize = DEFAULT_FONT_SIZE;
        return;
    }
    connect(fontSettings,&QGSettings::changed,[=](QString key)
    {
        if("systemFont" == key || "systemFontSize" == key)
        {
            fontSize = fontSettings->get(FONT_SIZE).toString().toFloat();
            this->onThemeFontChange(fontSize);
        }
    });
    fontSize = fontSettings->get(FONT_SIZE).toString().toFloat();
}

void ProcPropertiesDlg::onThemeFontChange(unsigned uFontSize)
{
    for (int i = 0; i < this->m_listTitleValue.length(); ++i) {
        QString ShowValue = getElidedText(m_labelTitleList.value(i)->font(), m_listTitleValue.value(i), PROC_GRIDITEM_TITLE_WIDTH-PROC_GRIDITEM_BOARD);
        this->m_labelTitleList.value(i)->setText(ShowValue);
        if (ShowValue != m_listTitleValue.value(i)) {
            this->m_labelTitleList.value(i)->setToolTip(m_listTitleValue.value(i));
        } else {
            this->m_labelTitleList.value(i)->setToolTip("");
        }
    }
    for (int i = 0; i < this->m_labelList.length(); ++i) {
        QString ShowValue = getElidedText(m_labelList.value(i)->font(), m_listValue.value(i), PROC_GRIDITEM_CONTENT_WIDTH-PROC_GRIDITEM_BOARD);
        this->m_labelList.value(i)->setText(ShowValue);
        if (ShowValue != m_listValue.value(i)) {
            this->m_labelList.value(i)->setToolTip(m_listValue.value(i));
        } else {
            this->m_labelList.value(i)->setToolTip("");
        }
    }
    if (m_titleLabel) {
        QString titleName = getElidedText(m_titleLabel->font(), m_strTitleName, PROC_DIALOGTITLE_WIDTH-PROC_GRIDITEM_BOARD);
        m_titleLabel->setText(titleName);
        if (titleName != m_strTitleName) {
            m_titleLabel->setToolTip(m_strTitleName);
        } else {
            m_titleLabel->setToolTip("");
        }
    }
}

void ProcPropertiesDlg::updateLabelFrameHeight()
{
    int labelTotalHeight = 0;
    foreach (QLabel *label, m_labelList) {
        label->adjustSize();
        labelTotalHeight += label->size().height() + PROC_GRIDITEM_SPACING;
    }
    m_infoFrame->setFixedHeight(labelTotalHeight);
    m_infoFrame->adjustSize();
    this->adjustSize();
}

void ProcPropertiesDlg::initProcproperties()
{
    sysmonitor::process::Process info = ProcessMonitor::instance()->processList()->getProcessById(pid);
    if (info.isValid()) {
        QString username = info.getProcUser();
        QString name = info.getProcName();

        std::string desktopFile;
        desktopFile = getDesktopFileAccordProcNameApp(name, "");
        if(desktopFile.empty())  //this is the way to detect that if the std::string is null or not.
        {
            desktopFile = getDesktopFileAccordProcName(name, "");
        }

        QPixmap icon_pixmap;
        int iconSize = 48;// * qApp->devicePixelRatio();

        QIcon defaultExecutableIcon = QIcon::fromTheme("application-x-executable");//gnome-mine-application-x-executable
        if (defaultExecutableIcon.isNull()) {
            defaultExecutableIcon = QIcon("/usr/share/icons/ukui-icon-theme-default/48x48/mimetypes/application-x-executable.png");
            if (defaultExecutableIcon.isNull())
                defaultExecutableIcon = QIcon(":/res/autostart-default.png");
        }
        QPixmap defaultPixmap = defaultExecutableIcon.pixmap(iconSize, iconSize);
        if (desktopFile.size() == 0) {
            icon_pixmap = defaultPixmap;
            //icon_pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
        } else {
            icon_pixmap = getAppIconFromDesktopFile(desktopFile, 48);
            if (icon_pixmap.isNull()) {
                icon_pixmap = defaultPixmap;
                //icon_pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
            }
        }
        m_strTitleName = getDisplayNameAccordProcName(name, desktopFile);
        m_iconLabel->setPixmap(icon_pixmap);
        QString titleName = getElidedText(m_titleLabel->font(), m_strTitleName, PROC_DIALOGTITLE_WIDTH-PROC_GRIDITEM_BOARD);
        m_titleLabel->setText(titleName);
        if (titleName != m_strTitleName) {
            m_titleLabel->setToolTip(m_strTitleName);
        } else {
            m_titleLabel->setToolTip("");
        }        

        m_listValue << username << QString(info.getProcName()) << QString(info.getProcArgments())
                   << QFileInfo(QString("/proc/%1").arg(pid)).created().toString("yyyy-MM-dd hh:mm:ss")
                   << formatDurationForDisplay(100 * info.getProcCpuTime() / info.getFrequency());
        

        for (int i = 0; i < this->m_labelList.length(); ++i) {
            QString ShowValue = getElidedText(this->m_labelList.value(i)->font(), m_listValue.value(i), PROC_GRIDITEM_CONTENT_WIDTH-PROC_GRIDITEM_BOARD);
            this->m_labelList.value(i)->setText(ShowValue);
            if (ShowValue != m_listValue.value(i)) {
                this->m_labelList.value(i)->setToolTip(m_listValue.value(i));
            } else {
                this->m_labelList.value(i)->setToolTip("");
            }
        }
    }
    this->updateLabelFrameHeight();
}

void ProcPropertiesDlg::refreshProcproperties()
{
    sysmonitor::process::Process info = ProcessMonitor::instance()->processList()->getProcessById(pid);
    if (info.isValid()) {
        for (int i = 0; i < this->m_labelList.length(); ++i) {
            if (i == 3) {
                this->m_labelList.value(i)->setText(QFileInfo(QString("/proc/%1").arg(pid)).created().toString("yyyy-MM-dd hh:mm:ss"));
                m_listValue[i] = this->m_labelList.value(i)->text();
                QString ShowValue = getElidedText(this->m_labelList.value(i)->font(), m_listValue.value(i), PROC_GRIDITEM_CONTENT_WIDTH-PROC_GRIDITEM_BOARD);
                this->m_labelList.value(i)->setText(ShowValue);
                if (ShowValue != m_listValue.value(i)) {
                    this->m_labelList.value(i)->setToolTip(m_listValue.value(i));
                } else {
                    this->m_labelList.value(i)->setToolTip("");
                }                    
            } else if (i == 4) {
                this->m_labelList.value(i)->setText(formatDurationForDisplay(100 * info.getProcCpuTime() / info.getFrequency()));
                m_listValue[i] = this->m_labelList.value(i)->text();
                QString ShowValue = getElidedText(this->m_labelList.value(i)->font(), m_listValue.value(i), PROC_GRIDITEM_CONTENT_WIDTH-PROC_GRIDITEM_BOARD);
                this->m_labelList.value(i)->setText(ShowValue);
                if (ShowValue != m_listValue.value(i)) {
                    this->m_labelList.value(i)->setToolTip(m_listValue.value(i));
                } else {
                    this->m_labelList.value(i)->setToolTip("");
                }
            }
        }
    }
}

pid_t ProcPropertiesDlg::getPid()
{
    return pid;
}

QRect ProcPropertiesDlg::getParentGeometry() const
{
    if (this->parentWidget()) {
        return this->parentWidget()->window()->geometry();
    } else {
        QPoint pos = QCursor::pos();

        for (QScreen *screen : qApp->screens()) {
            if (screen->geometry().contains(pos)) {
                return screen->geometry();
            }
        }
    }

    return qApp->primaryScreen()->geometry();
}

void ProcPropertiesDlg::moveToCenter()
{
    QRect qr = geometry();
    qr.moveCenter(this->getParentGeometry().center());
    move(qr.topLeft());
}

void ProcPropertiesDlg::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->dragPosition = event->globalPos() - frameGeometry().topLeft();
        this->mousePressed = true;
    }

    QDialog::mousePressEvent(event);
}

void ProcPropertiesDlg::mouseReleaseEvent(QMouseEvent *event)
{
    this->mousePressed = false;

    QDialog::mouseReleaseEvent(event);
}

void ProcPropertiesDlg::mouseMoveEvent(QMouseEvent *event)
{
    if (this->mousePressed) {
        move(event->globalPos() - this->dragPosition);
    }

    QDialog::mouseMoveEvent(event);
}

void ProcPropertiesDlg::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    //绘制圆角矩形
    painter.setPen(QPen(QColor("#808080"), 0));//边框颜色 #3f96e4
//    painter.setPen(Qt::NoPen);
    painter.setBrush(this->palette().base());//背景色
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    QRectF r(0 / 2.0, 0 / 2.0, width() - 0, height() - 0);//左边 上边 右边 下边
    painter.drawRoundedRect(r, 6, 6);

    QDialog::paintEvent(event);
}
