/*
 * Copyright (C) 2013 ~ 2018 National University of Defense Technology(NUDT) & Tianjin Kylin Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "monitortitlewidget.h"
#include "../widgets/mytristatebutton.h"
#include "../widgets/myunderlinebutton.h"
#include "../widgets/mysearchedit.h"
#include "util.h"

#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QResizeEvent>
#include <QStyleFactory>
#include <QObject>
#include <QStandardItemModel>

MonitorTitleWidget::MonitorTitleWidget(QSettings *settings, QWidget *parent)
    :QFrame(parent)
    ,proSettings(settings)
{
    const QByteArray idd(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(idd))
    {
        qtSettings = new QGSettings(idd);
    }

    m_changeBox = new QComboBox();
    m_changeBox->setFixedSize(100,32);
    m_changeBox->addItem(tr("Active Processes"));
    m_changeBox->addItem(tr("My Processes"));
    m_changeBox->addItem(tr("All Process"));
    m_changeBox->setFocusPolicy(Qt::NoFocus);
    //m_changeBox->setCurrentIndex(0);
//    QStandardItemModel *pItemModel = qobject_cast<QStandardItemModel*>(m_changeBox->model());
//    pItemModel->item(0)->setForeground(QColor(255, 0, 0));
    //m_changeBox->setFont();


    m_changeBox->setView(new  QListView());

    initThemeMode();

    this->setWindowFlags(Qt::FramelessWindowHint);//this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint  | Qt::WindowCloseButtonHint);
//    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);//Attention: Qt::WindowCloseButtonHint make showMinimized() valid

    installEventFilter(this);
//    setMouseTracking(true);
    setFixedHeight(MONITOR_TITLE_WIDGET_HEIGHT);

    this->setAutoFillBackground(true);
//    this->setAttribute(Qt::WA_TranslucentBackground);

//    QPalette palette;
//    palette.setColor(QPalette::Background, QColor("#0d87ca"));
//    this->setPalette(palette);
//    this->setStyleSheet("QFrame{background:transparent;background-color:#0d87ca;border-radius:5px;border:1px solid red;}");
//    this->setStyleSheet("QFrame {padding: 5px 0;} QFrame:hover {background-color: rgba(255, 245, 250, 0.1);border-radius: 5px;}");

//    if(currentThemeMode == "ukui-white")
//    {
//        this->setObjectName("MonitorTitle");
//        this->setStyleSheet("QFrame#MonitorTitle{background:rgba(255,255,255,0.9);border-top-left-radius:6px;border-top-right-radius:6px;color: palette(windowText);}");
//    }
//    else
//    {
//        this->setObjectName("MonitorTitle");
//        this->setStyleSheet("QFrame#MonitorTitle{background:rgba(13,14,14,0.9);border-top-left-radius:6px;border-top-right-radius:6px;color: palette(windowText);}");
//    }

    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    connect(m_searchTimer, SIGNAL(timeout()), this, SLOT(onRefreshSearchResult()));

//    this->setStyleSheet("QWidget{"
//                        "background:rgba(19,19,20,1);"
//                        "border-top-left-radius:6px;"
//                        "border-top-right-radius:6px;"
//                        "}");



    initWidgets();
}

void MonitorTitleWidget::initThemeMode()
{
    //监听主题改变
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key)
    {

        if (key == "styleName")
        {
            qDebug()<<"yes this is in";
            auto style = qtSettings->get(key).toString();
            qApp->setStyle(new InternalStyle(style));
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            qDebug()<<"监听主题改变-------------------->"<<currentThemeMode<<endl;
            qApp->setStyle(new InternalStyle(currentThemeMode));
            //repaint();
            if (currentThemeMode == "ukui-white")
            {
                qDebug()<<"the theme is white";
                m_changeBox->setStyleSheet("QComboBox{background:rgba(13,14,14,0.08);border-radius:6px;color:rgba(0,0,0,0.57)}"
                                           "QComboBox::drop-down{border:0px;width:30px;}"
                                           "QComboBox::down-arrow{image:url(:/img/down_arrow.png);}"
                                           "QComboBox QAbstractItemView {margin:0px 0px 0px 0px;padding: 0px 0px;border-radius:0px;background-color:palette(windowText);outline:0px;}"
                                           "QComboBox QAbstractItemView::item{border-radius:0px;font-size:13px;color:rgba(0,0,0,0.57);height: 32px;background-color:palette(base);outline:0px;}"
                                           "QComboBox QAbstractItemView::item:hover{border-radius:0px;font-size:13px;color(palette(windowText));background-color:#3D6BE5;outline:0px;}"
                                           );
                this->setObjectName("MonitorTitle");
                this->setStyleSheet("QFrame#MonitorTitle{background:rgba(255,255,255,0);border-top-left-radius:6px;border-top-right-radius:6px;color: palette(windowText);}");

            }

            if (currentThemeMode == "ukui-black")
            {
                qDebug()<<"the theme is black";
                m_changeBox->setStyleSheet("QComboBox{background:rgba(255,255,255,0.08);border-radius:6px;color:rgba(255,255,255,0.57)}"
                                           "QComboBox::drop-down{border:0px;width:30px;}"
                                           "QComboBox::down-arrow{image:url(:/img/down_arrow.png);}"
                                           "QComboBox QAbstractItemView {margin:0px 0px 0px 0px;padding: 0px 0px;border-radius:0px;background-color:palette(windowText);outline:0px;}"
                                           "QComboBox QAbstractItemView::item{border-radius:0px;font-size:13px;color:rgba(255,255,255,0.57);height: 32px;background-color:palette(base);outline:0px;}"
                                           "QComboBox QAbstractItemView::item:hover{border-radius:0px;font-size:13px;color(palette(windowText));background-color:#3D6BE5;outline:0px;}"
                                           );
                this->setObjectName("MonitorTitle");
                this->setStyleSheet("QFrame#MonitorTitle{background:rgba(13,14,14,0);border-top-left-radius:6px;border-top-right-radius:6px;color: palette(windowText);}");
                qDebug()<<"if monitorTiltWidgetEffect";
            }

        }
    });
    qDebug()<<"come in Monitor opacity";
    currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if (currentThemeMode == "ukui-white")
    {
        m_changeBox->setStyleSheet("QComboBox{background:rgba(13,14,14,0.08);border-radius:6px;color:rgba(0,0,0,0.57)}"
                                   "QComboBox::drop-down{border:0px;width:30px;}"
                                   "QComboBox::down-arrow{image:url(:/img/down_arrow.png);}"
                                   "QComboBox QAbstractItemView {margin:0px 0px 0px 0px;padding: 0px 0px;border-radius:0px;background-color:palette(windowText);outline:0px;}"
                                   "QComboBox QAbstractItemView::item{border-radius:0px;font-size:13px;color:rgba(0,0,0,0.57);height: 32px;background-color:palette(base);outline:0px;}"
                                   "QComboBox QAbstractItemView::item:hover{border-radius:0px;font-size:13px;color(palette(windowText));background-color:#3D6BE5;outline:0px;}"
                                   );
        this->setObjectName("MonitorTitle");
        this->setStyleSheet("QFrame#MonitorTitle{background:rgba(255,255,255,0);border-top-left-radius:6px;border-top-right-radius:6px;color: palette(windowText);}");
    }

    if (currentThemeMode == "ukui-black")
    {
        m_changeBox->setStyleSheet("QComboBox{background:rgba(255,255,255,0.08);border-radius:6px;color:rgba(255,255,255,0.57)}"
                                   "QComboBox::drop-down{border:0px;width:30px;}"
                                   "QComboBox::down-arrow{image:url(:/img/down_arrow.png);}"
                                   "QComboBox QAbstractItemView {margin:0px 0px 0px 0px;padding: 0px 0px;border-radius:0px;background-color:palette(windowText);outline:0px;}"
                                   "QComboBox QAbstractItemView::item{border-radius:0px;font-size:13px;color:rgba(255,255,255,0.57);height: 32px;background-color:palette(base);outline:0px;}"
                                   "QComboBox QAbstractItemView::item:hover{border-radius:0px;font-size:13px;color(palette(windowText));background-color:#3D6BE5;outline:0px;}"
                                   );
        this->setObjectName("MonitorTitle");
        this->setStyleSheet("QFrame#MonitorTitle{background:rgba(13,14,14,0);border-top-left-radius:6px;border-top-right-radius:6px;color: palette(windowText);}");
    }

}

MonitorTitleWidget::~MonitorTitleWidget()
{
    delete emptyLabel;
    delete m_searchEdit;
    //delete m_cancelSearchBtn;
    delete maxBtn;

    if (m_searchTimer) {
        disconnect(m_searchTimer, SIGNAL(timeout()), this, SLOT(onRefreshSearchResult()));
        if(m_searchTimer->isActive()) {
            m_searchTimer->stop();
        }
        delete m_searchTimer;
        m_searchTimer = nullptr;
    }

    //Segmentation fault
    QLayoutItem *child;
    while ((child = m_titleLeftLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    while ((child = m_titleMiddleLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    while ((child = m_titleRightLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    while ((child = m_toolLeftLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    while ((child = m_toolRightLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    while ((child = m_topLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    while ((child = m_bottomLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    delete m_layout;
}

bool MonitorTitleWidget::eventFilter(QObject *obj, QEvent *event)    //set the esc and tab pressbutton effect
{
    if (event->type() == QEvent::KeyPress) {
        if (obj == this) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                m_searchEdit->clearEdit();
                emit canelSearchEditFocus();
            }
        }
        else if (obj == m_searchEdit->getLineEdit()) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Tab) {
                m_searchEdit->clearEdit();
                emit canelSearchEditFocus();
            }
        }
    }

    return QFrame::eventFilter(obj, event);
}

void MonitorTitleWidget::setSearchEditFocus()
{
    if (m_searchEdit->searchedText() != "") {
        m_searchEdit->getLineEdit()->setFocus();
    } else {
        m_searchEdit->setFocus();
    }
}

void MonitorTitleWidget::onRefreshSearchResult()
{
    if (m_searchEdit->searchedText() == searchTextCache) {
        emit this->searchSignal(searchTextCache);
    }
}

void MonitorTitleWidget::handleSearchTextChanged()
{
    searchTextCache = m_searchEdit->searchedText();

    //this->m_cancelSearchBtn->setVisible(!searchTextCache.isEmpty());

    if (m_searchTimer->isActive()) {
        m_searchTimer->stop();
    }
    m_searchTimer->start(300);
}

void MonitorTitleWidget::onCancelSearchBtnClicked(bool b)
{
    qDebug()<<"wocnm,wwj,nizhendeshiyigedashabi";
    //this->m_cancelSearchBtn->setVisible(false);
    m_searchEdit->clearAndFocusEdit();
    emit canelSearchEditFocus();
}

void MonitorTitleWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (window()->isMaximized())
            window()->showNormal();
        else if (! window()->isFullScreen())
            window()->showMaximized();
    }

    QFrame::mouseDoubleClickEvent(e);
}

/*void MonitorTitleWidget::paintEvent(QPaintEvent *event)
{
//    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    //draw background
    QPainterPath path;
    path.addRect(QRectF(0, 0, width(), height()));
    painter.setOpacity(1);
    painter.fillPath(path, QColor("#0d87ca"));


    //draw line
//    const QColor color(QColor(255, 255, 255, 127));
//    int borderHeight = 1;
//    QPainterPath path;
//    path.moveTo(QPointF(x(), y() + borderHeight - 0.5));
//    path.lineTo(QPointF(x() + width(), y() + borderHeight - 0.5));
//    painter.setPen(QPen(color));
//    painter.drawPath(path);

    //绘制圆角矩形
//    painter.setPen(QPen(QColor("#e9eef0"), 0));//边框颜色
//    painter.setBrush(QColor("#0d87ca"));//背景色
//    painter.setOpacity(1);
//    QRectF r(1, 1, width() - 2, height() - 2);//左边 上边 右边 下边
//    painter.drawRoundedRect(r, 5, 5);

    QFrame::paintEvent(event);
}*/

void MonitorTitleWidget::initTitlebarLeftContent()
{
    QWidget *w = new QWidget;
    m_titleLeftLayout = new QHBoxLayout(w);
    m_titleLeftLayout->setContentsMargins(6, 0, 0, 0);
    m_titleLeftLayout->setSpacing(0);

    emptyLabel = new QLabel;
    emptyLabel->setStyleSheet("QLabel{background-color:transparent;}");
    m_titleLeftLayout->addWidget(emptyLabel);

    m_topLayout->addWidget(w, 1, Qt::AlignLeft);
}

void MonitorTitleWidget::initTitlebarMiddleContent()
{
    QWidget *w = new QWidget;
    m_titleMiddleLayout = new QHBoxLayout(w);
    m_titleMiddleLayout->setContentsMargins(0, 18, 0, 0);

    QLabel *titleLabel = new QLabel;
    QLabel *picLabel = new QLabel;
    titleLabel->setStyleSheet("QLabel{background-color:transparent;color:palette(windowText); font-size:14px;}");
    titleLabel->setText(tr("Kylin System Monitor"));
    picLabel->setPixmap(QPixmap(":img/kylin-system-monitor.png"));
    m_titleMiddleLayout->addWidget(picLabel);
    m_titleMiddleLayout->addWidget(titleLabel);
    m_topLayout->addWidget(w);
}

void MonitorTitleWidget::initTitlebarRightContent()
{
    QWidget *w = new QWidget;
    m_titleRightLayout = new QHBoxLayout(w);
    m_titleRightLayout->setContentsMargins(0, 0, 1, 0);
    m_titleRightLayout->setSpacing(0);

    m_topLayout->addWidget(w, 1, Qt::AlignRight);

//    MyTristateButton *minBtn = new MyTristateButton;
    QIcon iconMin(tr(":/img/minimize.png"));
    QPushButton *minBtn = new QPushButton(this);
    //minBtn->setStyleSheet("QPushButton{background-color:transparent;}");
    minBtn->setIcon(iconMin);
    minBtn->setIconSize(QSize(25,25));
    //minBtn->setObjectName("MinButton");
    minBtn->setProperty("useIconHighlightEffect", true);
    minBtn->setProperty("iconHighlightEffectMode", 1);
    minBtn->setFlat(true);
    connect(minBtn, SIGNAL(clicked()), this, SLOT(onMinBtnClicked()));
//    connect(minBtn, &MyTristateButton::clicked, this, [=] {
//        if (parentWidget() && parentWidget()->parentWidget()) {
//            parentWidget()->parentWidget()->showMinimized();
//        }
//    });
//    /*MyTristateButton **/maxBtn = new MyTristateButton;

    QPushButton *maxBtn = new QPushButton();
//    maxBtn->setStyleSheet("QPushButton{background-color:#ffffff;}");
//    maxBtn->setObjectName("MaxButton");

    maxBtn->setIconSize(QSize(25,25));
    maxBtn->setProperty("useIconHighlightEffect", true);
    maxBtn->setProperty("iconHighlightEffectMode", 1);
    maxBtn->setFlat(true);
    QIcon iconMax(tr(":/img/fullscreen.png"));
    maxBtn->setIcon(iconMax);
    connect(maxBtn, SIGNAL(clicked()), this, SLOT(onMaxBtnClicked()));
//    connect(maxBtn, &MyTristateButton::clicked, sthis, [=] {
//        if (window()->isMaximized()) {
//            window()->showNormal();
//            maxBtn->setObjectName("MaxButton");
//        }
//        else {
//            window()->showMaximized();
//            maxBtn->setObjectName("UnMaxButton");
//        }
//    });
//    connect(this, &MonitorTitleWidget::updateMaxBtn, this, [=]{
//        if (window()->isMaximized()) {
//            maxBtn->setObjectName("UnMaxButton");
//        } else {
//            maxBtn->setObjectName("MaxButton");
//        }
//    });
//    MyTristateButton *closeBtn = new MyTristateButton;
    QIcon iconClose(tr(":/img/close.png"));
    QPushButton *closeBtn = new QPushButton(this);
    //closeBtn->setStyleSheet("QPushButton{background-color:transparent;}");
    closeBtn->setIcon(iconClose);
    closeBtn->setIconSize(QSize(25,25));
    //closeBtn->setObjectName("CloseButton");
    closeBtn->setProperty("useIconHighlightEffect", true);
    closeBtn->setProperty("iconHighlightEffectMode", 1);
    closeBtn->setFlat(true);
    connect(closeBtn, SIGNAL(clicked()), this, SLOT(onCloseBtnClicked()));
    connect(m_changeBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(switchChangeItemProcessSignal()));
//    connect(closeBtn, &MyTristateButton::clicked, this, [=] {
//        window()->close();
//    });

//    connect(this, SIGNAL(updateMaxBtn()), this, SLOT(onUpdateMaxBtnStatusChanged()));

    m_titleRightLayout->addWidget(minBtn);
    m_titleRightLayout->addWidget(maxBtn);
    m_titleRightLayout->addWidget(closeBtn);
}

void MonitorTitleWidget::onMinBtnClicked()
{
//    if (parentWidget() && parentWidget()->parentWidget()) {
//        parentWidget()->parentWidget()->showMinimized();
//    }
    window()->showMinimized();
}

void MonitorTitleWidget::onMaxBtnClicked()
{
    if (window()->isMaximized()) {
        window()->showNormal();
//        maxBtn->setObjectName("MaxButton");
    }
    else {
        window()->showMaximized();
//        maxBtn->setObjectName("UnMaxButton");
    }
}

void MonitorTitleWidget::onCloseBtnClicked()
{
    window()->close();
}

void MonitorTitleWidget::switchChangeItemProcessSignal()
{
    qDebug()<<"i love you baby;";
    //emit changeProcessItemDialog(a);
    {
        if (m_changeBox->currentIndex() == 0){
            emit changeProcessItemDialog(0);
        }else if (m_changeBox->currentIndex() == 1){
            emit changeProcessItemDialog(1);
        }else if (m_changeBox->currentIndex() == 2){
            emit changeProcessItemDialog(2);
        }
    }
}

void MonitorTitleWidget::onUpdateMaxBtnStatusChanged()
{
//    if (window()->isMaximized()) {
//        maxBtn->setObjectName("UnMaxButton");
//    } else {
//        maxBtn->setObjectName("MaxButton");
//    }
}

void MonitorTitleWidget::initToolbarLeftContent()
{
    QWidget *w = new QWidget;
    //w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_toolLeftLayout = new QHBoxLayout(w);
    m_toolLeftLayout->setContentsMargins(0, 0, 0, 0);
    m_toolLeftLayout->setSpacing(20);

    MyUnderLineButton *processButton = new MyUnderLineButton();
    processButton->setName(tr("Processes"));
    processButton->setChecked(true);
//    m_changeBox->setVisible(true);

    MyUnderLineButton *resourcesButton = new MyUnderLineButton();
    resourcesButton->setName(tr("Resources"));
    resourcesButton->setChecked(false);

    MyUnderLineButton *disksButton = new MyUnderLineButton();
    disksButton->setName(tr("File Systems"));
    disksButton->setChecked(false);

    connect(processButton, &MyUnderLineButton::clicked, this, [=] {
        emit this->changePage(0);
        processButton->setChecked(true);
        resourcesButton->setChecked(false);
        disksButton->setChecked(false);
        if (!m_searchEdit->isVisible())
            m_searchEdit->setVisible(true);
        if(!m_changeBox->isVisible())
            m_changeBox->setVisible(true);
    });
    connect(resourcesButton, &MyUnderLineButton::clicked, this, [=] {
        emit this->changePage(1);
        processButton->setChecked(false);
        resourcesButton->setChecked(true);
        disksButton->setChecked(false);
        if (m_searchEdit->isVisible())
            m_searchEdit->setVisible(false);
        if (m_changeBox->isVisible())
            m_changeBox->setVisible(false);
        m_searchEdit->clearEdit();
        emit canelSearchEditFocus();
    });
    connect(disksButton, &MyUnderLineButton::clicked, this, [=] {
        emit this->changePage(2);
        processButton->setChecked(false);
        resourcesButton->setChecked(false);
        disksButton->setChecked(true);
        if (m_searchEdit->isVisible())
            m_searchEdit->setVisible(false);
        if (m_changeBox->isVisible())
            m_changeBox->setVisible(false);
        m_searchEdit->clearEdit();
        emit canelSearchEditFocus();
    });

//    buttonWidget = new QWidget();
//    //buttonWidget->resize(303,32);
//    QHBoxLayout *button_h_BoxLayout = new QHBoxLayout();
//    button_h_BoxLayout->setContentsMargins(0,0,0,0);
//    m_processButton = new QPushButton(tr("Processes"));
//    m_processButton->setFixedSize(100,32);
//    m_processButton->setStyleSheet("QPushButton{"
//                        "background:rgba(61,107,229 ,1);"
//                        "border-radius:4px 0px 4px 4px;"
//                        "color:#ffffff;"
//                        "}");
//    m_resourceButton = new QPushButton(tr("Resources"));
//    m_resourceButton->setFixedSize(100,32);
//    m_resourceButton->setStyleSheet("QPushButton{"
//                        "background:rgba(25,107,229 ,1);"
//                        "border-radius:4px 0px 4px 4px;"
//                        "color:#ffffff;"
//                        "}");
//    m_filesystemButton = new QPushButton(tr("File Systems"));
//    m_filesystemButton->setFixedSize(100,32);
//    m_filesystemButton->setStyleSheet("QPushButton{"
//                        "background:rgba(255,255,255,0.08);"
//                        "border-radius:4px 0px 0px 4px;"
//                        "color:#ffffff;"
//                        "}");
//    button_h_BoxLayout->addSpacing(14);
//    button_h_BoxLayout->setSpacing(0);
//    button_h_BoxLayout->addWidget(m_processButton);
//    button_h_BoxLayout->addSpacing(1);
//    button_h_BoxLayout->addWidget(m_resourceButton);
//    button_h_BoxLayout->addSpacing(1);
//    button_h_BoxLayout->addWidget(m_filesystemButton);
//    buttonWidget->setLayout(button_h_BoxLayout);

    m_toolLeftLayout->addStretch();
    m_toolLeftLayout->addSpacing(14);
    m_toolLeftLayout->setSpacing(0);
    m_toolLeftLayout->addWidget(processButton);
    m_toolLeftLayout->addSpacing(5);
    m_toolLeftLayout->addWidget(resourcesButton);
    m_toolLeftLayout->addSpacing(5);
    m_toolLeftLayout->addWidget(disksButton);


    m_toolLeftLayout->addSpacing(32);
    m_toolLeftLayout->addWidget(m_changeBox);
    //m_toolLeftLayout->addWidget(buttonWidget);
    m_toolLeftLayout->addStretch();

//    m_bottomLayout->addWidget(w);
    m_bottomLayout->addWidget(w, 1, Qt::AlignLeft);
}

void MonitorTitleWidget::initToolbarRightContent()
{
    QWidget *w = new QWidget;
    m_toolRightLayout = new QHBoxLayout(w);
    m_toolRightLayout->setContentsMargins(0, 3, 6, 0);
    m_toolRightLayout->setSpacing(5);

//    m_cancelSearchBtn = new QPushButton;
//    m_cancelSearchBtn->setStyleSheet("QPushButton{background-color:transparent;text-align:center;font-family: 方正黑体_GBK;font-size:11px;color:#ffffff;}QPushButton:hover{color:#000000;}");
//    m_cancelSearchBtn->setText(tr("Cancel"));
//    m_cancelSearchBtn->setFocusPolicy(Qt::NoFocus);
//    m_cancelSearchBtn->setFixedSize(46, 25);
//    m_cancelSearchBtn->setVisible(false);
//    connect(m_cancelSearchBtn, SIGNAL(clicked(bool)), SLOT(onCancelSearchBtnClicked(bool)));
    connect(m_searchEdit, &MySearchEdit::textChanged, this, &MonitorTitleWidget::handleSearchTextChanged, Qt::QueuedConnection);
    m_toolRightLayout->addWidget(m_searchEdit);
    //m_toolRightLayout->addWidget(m_cancelSearchBtn);
    m_bottomLayout->addWidget(w, 1, Qt::AlignRight);
}

void MonitorTitleWidget::initWidgets()
{
    m_searchEdit = new MySearchEdit(this);
    //m_searchEdit->setVisible(true);
    //m_searchEdit->raise();
    m_searchEdit->setPlaceHolder(tr("Enter the relevant info of process"));
    m_searchEdit->setFixedSize(222, 30);
    //m_searchEdit->getLineEdit()->installEventFilter(this);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    QWidget *topWidget = new QWidget;
    m_topLayout = new QHBoxLayout(topWidget);
    m_topLayout->setContentsMargins(0, 0, 0, 0);
    m_topLayout->setSpacing(0);
    m_layout->addWidget(topWidget, 0, Qt::AlignTop);

    QWidget *bottomWidget = new QWidget;
    m_bottomLayout = new QHBoxLayout(bottomWidget);
    m_bottomLayout->setContentsMargins(0, 0, 0, 0);
    m_bottomLayout->setSpacing(0);
    m_layout->addWidget(bottomWidget);

    this->setLayout(m_layout);

    initTitlebarLeftContent();
    initTitlebarMiddleContent();
    initTitlebarRightContent();
    initToolbarLeftContent();
    initToolbarRightContent();   
}

void MonitorTitleWidget::resizeEvent(QResizeEvent *event)
{
    qDebug()<<this->geometry().x()<<"-----------------------"<<this->geometry().y();
}

void MonitorTitleWidget::paintEvent(QPaintEvent *event)
{
////    //paint background
////    QPainter painter(this);
////    QPainterPath path;
////    path.addRect(QRectF(rect()));
////    painter.setOpacity(1);
////    painter.fillPath(path, QColor("#131314"));
////    //paint rect
////    //QPainterPath path;
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    p.setBrush(QBrush(QColor(0x19,0x19,0x20,0xFF)));  //QColor(0x19,0x19,0x20,0xFF
//    //p.setPen(Qt::NoPen);
//    QPainterPath path;
//    //opt.rect.adjust(0,0,0,0);
////    path.addRoundRect(opt.rect.topLeft(),6);

//    path.addPath(path);
//    path.addRoundedRect(opt.rect,0,0);
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
////       p.drawRoundedRect(opt.rect,6,6);
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
////       p.setPen(QPen(QColor("#e9eef0"), 0));//边框颜色
////       p.setBrush(QColor("#0d87ca"));//背景色
////       QRectF r(1, 1, width() - 2, height() - 2);//左边 上边 右边 下边
//    p.drawPath(path);
///////////////////////////////////////////////////old setting
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);

////    p.setBrush(QBrush(QColor("#131314")));
//    p.setBrush(opt.palette.color(QPalette::Base));
//    //p.setOpacity(0.7);
//    p.setPen(Qt::NoPen);

//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.setClipping(true);
//    QPainterPath canDrawPathArea;
//    canDrawPathArea.addRoundedRect(rect(),6,6);
//    canDrawPathArea.setFillRule(Qt::WindingFill);   //multilateral areas containing modules
//    //p.drawRoundedRect(opt.rect,0,0);
//    canDrawPathArea.addRect(0,height()-6,6,6);      //fill left-top
//    canDrawPathArea.addRect(width()-6,height()-6,6,6);
//    p.drawRect(rect());

//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);///////////old setting
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    painter.setBrush(QColor(Qt::darkRed));
//    painter.setPen(Qt::blue);

//    QPainterPath path;
//    path.setFillRule(Qt::OddEvenFill);
//    path.addRect(0,0,this->width(),this->height());

//    const qreal radius = 10;
//    QRectF rect = QRect(0, 0, this->width(), this->height());

//    qDebug() << rect.bottomRight() << rect.bottomRight() + QPointF(0, radius);
//    path.moveTo(rect.bottomRight());
//    path.lineTo(rect.topRight() + QPointF(0, radius));
//    path.arcTo(QRectF(QPointF(rect.topRight() - QPointF(radius * 2, 0)), QSize(radius * 2, radius *2)), 0, 90);
//    path.lineTo(rect.topLeft() + QPointF(radius, 0));
//    path.arcTo(QRectF(QPointF(rect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
//    path.lineTo(rect.bottomLeft());
////    path.arcTo(QRectF(QPointF(rect.bottomLeft() - QPointF(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
////    path.lineTo(rect.bottomLeft() + QPointF(radius, 0));
////    path.arcTo(QRectF(QPointF(rect.bottomRight() - QPointF(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);
//    path.moveTo(rect.bottomRight());

//    painter.save();
//    painter.setCompositionMode(QPainter::CompositionMode_Clear);
//    painter.drawPath(path);
//    painter.restore();

//    QStyleOption opt;
//    opt.init(this);
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);


}
