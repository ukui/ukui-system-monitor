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

#include "monitortitlewidget.h"
#include "../shell/macro.h"
#include "util.h"
#include "gui/usmaboutdialog.h"

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
#include <QMenu>
#include <QPainterPath>
#include "style/usmproxystyle.h"

#define MENU_SCHEMA "org.ukui.system-monitor.menu"
#define WHICH_MENU "which-menu"

MonitorTitleWidget::MonitorTitleWidget(QSettings *settings, QWidget *parent)
    :QFrame(parent)
    ,proSettings(settings)
    ,fontSettings(nullptr)
    ,qtSettings(nullptr)
{
    m_animation = nullptr;
    this->setAttribute(Qt::WA_TranslucentBackground);
    fontSize = DEFAULT_FONT_SIZE;
    m_queryIcon=new QLabel();
    pixmap = QIcon::fromTheme("preferences-system-search-symbolic").pixmap(QSize(16, 16));
    m_queryIcon->setPixmap(pixmap);
    m_queryIcon->setProperty("useIconHighlightEffect", 0x2);
    const QByteArray idd(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(idd))
    {
        qtSettings = new QGSettings(idd);
    }

    const QByteArray id(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(id))
    {
        fontSettings = new QGSettings(id);
    }

    const QByteArray ifid(MENU_SCHEMA);
    if(QGSettings::isSchemaInstalled(ifid))
    {
        ifsettings = new QGSettings(ifid);
    }

    const QByteArray idtrans(THEME_QT_TRANS);
    if(QGSettings::isSchemaInstalled(idtrans))
    {
        opacitySettings = new QGSettings(idtrans);
    }
    
    m_changeBox = new QComboBox();
    m_changeBox->setFixedSize(NORMALWIDTH+20,NORMALHEIGHT);
    m_changeBox->addItem(tr("Active Processes"));
    m_changeBox->addItem(tr("My Processes"));
    m_changeBox->addItem(tr("All Process"));
    m_changeBox->setFocusPolicy(Qt::NoFocus);
    m_changeBox->setView(new  QListView());
    QFont changeBoxFont;
    changeBoxFont.setPointSize(fontSize);
//    m_changeBox->setFont(changeBoxFont);

    initThemeMode();
    getTransparentData();

    this->setWindowFlags(Qt::FramelessWindowHint);//this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint  | Qt::WindowCloseButtonHint);

    installEventFilter(this);
    setFixedHeight(MONITOR_TITLE_WIDGET_HEIGHT);

    this->setAutoFillBackground(true);
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    connect(m_searchTimer, SIGNAL(timeout()), this, SLOT(onRefreshSearchResult()));

    initFontSize();
    initWidgets();
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->installEventFilter(this);
}

void MonitorTitleWidget::getTransparentData()
{
    if (!opacitySettings)
    {
        m_transparency = 0.95;
        return;
    }

    connect(opacitySettings, &QGSettings::changed, this, [=](const QString &key)
    {
        if (key == "transparency")
        {
            QStringList keys = opacitySettings->keys();
            if (keys.contains("transparency"))
            {
                m_transparency = opacitySettings->get("transparency").toString().toDouble();
            }
        }
        repaint();
    });
    
    QStringList keys = opacitySettings->keys();
    if(keys.contains("transparency"))
    {
        m_transparency = opacitySettings->get("transparency").toString().toDouble();
    }
}

void MonitorTitleWidget::initThemeMode()
{
    //监听主题改变
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key)
    {
        if (key == "styleName") {
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            if (currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
            {
                //qDebug() << "The theme change to white";
                this->setObjectName("MonitorTitle");
                // m_queryIcon->setPixmap(drawSymbolicBlackColoredPixmap(pixmap));
            }

            else/*(currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black")*/
            {
                this->setObjectName("MonitorTitle");
                // m_queryIcon->setPixmap(drawSymbolicColoredPixmap(pixmap));
            }
        } else if ("iconThemeName" == key) {
            if (m_picLabel)
                m_picLabel->setPixmap(QIcon::fromTheme("ukui-system-monitor").pixmap(24,24));
            if (m_queryIcon) {
                pixmap = QIcon::fromTheme("preferences-system-search-symbolic").pixmap(QSize(16, 16));
                m_queryIcon->setPixmap(pixmap);
            }
        }
    });

    currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if (currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
    {
        this->setObjectName("MonitorTitle");
        // m_queryIcon->setPixmap(drawSymbolicBlackColoredPixmap(pixmap));
    }

    if (currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black")
    {
        this->setObjectName("MonitorTitle");
        // m_queryIcon->setPixmap(drawSymbolicColoredPixmap(pixmap));
    }
}

void MonitorTitleWidget::initFontSize()
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

MonitorTitleWidget::~MonitorTitleWidget()
{
    if (emptyLabel) {
        delete emptyLabel;
        emptyLabel = nullptr;
    }

    if (m_searchTimer) {
        disconnect(m_searchTimer, SIGNAL(timeout()), this, SLOT(onRefreshSearchResult()));
        if(m_searchTimer->isActive()) {
            m_searchTimer->stop();
        }
    }

    if (m_toolLeftLayout) {
        delete m_toolLeftLayout;
        m_toolLeftLayout = nullptr;
    }
    if (m_toolRightLayout) {
        delete m_toolRightLayout;
        m_toolRightLayout = nullptr;
    }
    if(fontSettings)
    {
        delete fontSettings;
        fontSettings = nullptr;
    }
    if(qtSettings)
    {
        delete qtSettings;
        qtSettings = nullptr;
    }
    if(m_animation)
    {
        delete m_animation;
        m_animation = nullptr;
    }
}

void MonitorTitleWidget::showSearchFocusAnimation(bool bIn)
{
    static bool bInFocus = false;
    if (bInFocus == bIn) {
        return ;
    }
    bInFocus = bIn;
    if (bIn) {
        m_animation->stop();
        m_animation->setStartValue(m_rectSeachAnimationBegin);
        m_animation->setEndValue(m_rectSeachAnimationEnd);
        QRectF rect = m_animation->endValue().toRectF();
        m_animation->setEasingCurve(QEasingCurve::OutQuad);
        m_animation->start();
    } else {
        m_queryText->adjustSize();
        m_animation->setStartValue(m_rectSeachAnimationEnd);
        m_animation->setEndValue(m_rectSeachAnimationBegin);
        QRectF rect = m_animation->endValue().toRectF();
        m_animation->setEasingCurve(QEasingCurve::InQuad);
        m_animation->start();
    }
}

bool MonitorTitleWidget::eventFilter(QObject *obj, QEvent *event)    //set the esc and tab pressbutton effect
{
    if (event->type() == QEvent::KeyPress) {
        if (obj == this) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape || keyEvent->key() == Qt::Key_Tab) {
//                m_searchEdit->clearEdit();
                m_searchEditNew->clear();
                emit canelSearchEditFocus();
                showSearchFocusAnimation(false);
            }
        }
        else if (obj == m_searchEditNew) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Tab) {
//                m_searchEdit->clearEdit();
                m_searchEditNew->clear();
                emit canelSearchEditFocus();
                showSearchFocusAnimation(false);
            }
        }
    }

    if (obj == m_searchEditNew)
    {
        if (event->type() == QEvent::FocusIn)
        {
            emit SearchFocusIn();
            if(m_searchEditNew->text().isEmpty())
            {
                showSearchFocusAnimation(true);
            }
        }
        else if(event->type() == QEvent::FocusOut && m_isSearching == false)
        {
            emit SearchFocusOut();
            m_searchEditNew->clear();
            m_searchEditNew->clearFocus();
            if(m_searchEditNew->text().isEmpty())
            {
                showSearchFocusAnimation(false);
            }
            m_isSearching=false;
        }
    }
    if(obj != m_searchEditNew && event->type() == QEvent::MouseButtonPress)
    {
//        m_searchEditNew->clear();
//        m_searchEditNew->clearFocus();
    }

    return QFrame::eventFilter(obj, event);
}

void MonitorTitleWidget::setSearchEditFocus()
{
}

void MonitorTitleWidget::onRefreshSearchResult()
{
    if (m_searchEditNew->text() == searchTextCache) {
        emit this->searchSignal(searchTextCache);
    }
}

void MonitorTitleWidget::handleSearchTextChanged()
{
    searchTextCache = m_searchEditNew->text();
    if (searchTextCache.isEmpty()) {
        m_isSearching = false;
    } else {
        m_isSearching = true;
    }
    if (m_searchTimer->isActive()) {
        m_searchTimer->stop();
    }
    m_searchTimer->start(300);
}

void MonitorTitleWidget::onCancelSearchBtnClicked(bool b)
{
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

void MonitorTitleWidget::initTitlebarLeftContent()
{
    QWidget *w = new QWidget;
    m_titleMiddleLayout = new QHBoxLayout(w);
    m_titleMiddleLayout->setContentsMargins(8, 4, 0, 0);
    titleLabel = new QLabel;
    m_picLabel = new QLabel();
    titleLabel->setText(tr("Kylin System Monitor"));
    m_picLabel->setPixmap(QIcon::fromTheme("ukui-system-monitor").pixmap(24,24));
    m_titleMiddleLayout->addWidget(m_picLabel);
    m_titleMiddleLayout->addWidget(titleLabel);
    m_topLayout->addWidget(w);
}

void MonitorTitleWidget::initTitlebarMiddleContent()
{
}

void MonitorTitleWidget::initTitlebarRightContent()
{
    QWidget *w = new QWidget;
    m_titleRightLayout = new QHBoxLayout(w);
    m_titleRightLayout->setContentsMargins(0, 4, 4, 0);
    m_titleRightLayout->setSpacing(0);

    m_topLayout->addWidget(w, 1, Qt::AlignRight);
    QToolButton *menuBtn = new QToolButton(this);
    menuBtn->setToolTip(tr("menu"));
    menuBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    menuBtn->setProperty("isWindowButton", 0x1);
    menuBtn->setProperty("useIconHighlightEffect", 0x2);
    menuBtn->setFixedSize(30,30);
    menuBtn->setAutoRaise(true);
//    connect(menuBtn, SIGNAL(clicked()), this, SLOT(onMinBtnClicked()));
    m_toolMenu = new QMenu(this);
    QMenu *themeMenu = new QMenu(tr("theme"));
    QAction *helpAction = new QAction(tr("help"),m_toolMenu);
    QAction *aboutAction = new QAction(tr("about"),m_toolMenu);
    QAction *quitAction = new QAction(tr("quit"),m_toolMenu);
    actions <<helpAction <<aboutAction << quitAction;
//    m_toolMenu->addMenu(themeMenu);
    m_toolMenu->addActions(actions);
    menuBtn->setMenu(m_toolMenu);
    menuBtn->setPopupMode(QToolButton::InstantPopup);

    QList<QAction *>themeactions;
    QAction *followTheme = new QAction(tr("following theme"),themeMenu);
    QAction *darkTheme = new QAction(tr("dark theme"),themeMenu);
    QAction *lightTheme = new QAction(tr("light theme"),themeMenu);
    themeactions << followTheme << darkTheme << lightTheme;
    themeMenu->addActions(themeactions);

//    connect(darkTheme,&QAction::triggered,this,[=](){
//        qDebug()<<"dark theme has enterd";
//        system("gsettings set org.ukui.style style-name ukui-dark");
//    });
//    connect(lightTheme,&QAction::triggered,this,[=](){
//        qDebug()<<"light theme has enterd";
//        system("gsettings set org.ukui.style style-name ukui-default");
//    });

    connect(helpAction,&QAction::triggered,this,[=](){
        if(!daemonIsNotRunning())
        {
            showGuide(qAppName());
        }
    });
    connect(aboutAction,&QAction::triggered,this,[=]{
            USMAboutDialog *showaboutdialog = new USMAboutDialog(this);
            showaboutdialog->setAttribute(Qt::WA_DeleteOnClose);
            showaboutdialog->setModal(true);
            showaboutdialog->show();
    });
    connect(quitAction,&QAction::triggered,this,[=](){
       window()->close();
    });

    QToolButton *minBtn = new QToolButton(this);
    minBtn->setToolTip(tr("minimize"));
    minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    minBtn->setProperty("isWindowButton", 0x1);
    minBtn->setProperty("useIconHighlightEffect", 0x2);
    minBtn->setFixedSize(30,30);
    minBtn->setAutoRaise(true);
    connect(minBtn, SIGNAL(clicked()), this, SLOT(onMinBtnClicked()));

    maxTitleBtn = new QToolButton();
    maxTitleBtn->setToolTip(tr("maximize/restore"));
    maxTitleBtn->setProperty("isWindowButton", 0x1);
    maxTitleBtn->setProperty("useIconHighlightEffect", 0x2);
    maxTitleBtn->setFixedSize(30,30);
    maxTitleBtn->setAutoRaise(true);
    maxTitleBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    connect(maxTitleBtn, SIGNAL(clicked()), this, SLOT(onMaxBtnClicked()));
    QToolButton *closeBtn = new QToolButton(this);
    closeBtn->setToolTip(tr("close"));
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setAutoRaise(true);
    closeBtn->setProperty("isWindowButton", 0x2);
    closeBtn->setProperty("useIconHighlightEffect", 0x8);
    closeBtn->setFixedSize(30,30);
//    closeBtn->setIconSize(QSize(20,20));
    connect(closeBtn, SIGNAL(clicked()), this, SLOT(onCloseBtnClicked()));
    //进行GSettings设置，记忆用户选择的combox的内容
    whichNum = ifsettings->get(WHICH_MENU).toInt();
    m_changeBox->setCurrentIndex(whichNum);
    switchChangeItemProcessSignal(whichNum);
    connect(m_changeBox,SIGNAL(currentIndexChanged(int)),this,SLOT(switchChangeItemProcessSignal(int)));
    m_titleRightLayout->addWidget(menuBtn);
    m_titleRightLayout->addSpacing(4);
    m_titleRightLayout->addWidget(minBtn);
    m_titleRightLayout->addSpacing(4);
    m_titleRightLayout->addWidget(maxTitleBtn);
    m_titleRightLayout->addSpacing(4);
    m_titleRightLayout->addWidget(closeBtn);
}

int MonitorTitleWidget::daemonIsNotRunning()
{
    QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.isConnected())
        return 0;

    QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", service_name);
    //qDebug()<<"reply name"<<reply;
    return reply.value() == "";
}

void MonitorTitleWidget::showGuide(QString appName)
{
    QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());

    QDBusInterface *interface = new QDBusInterface(service_name,
                                                       KYLIN_USER_GUIDE_PATH,
                                                       KYLIN_USER_GUIDE_INTERFACE,
                                                       QDBusConnection::sessionBus(),
                                                       this);

    QDBusMessage msg = interface->call(QStringLiteral("showGuide"),"kylin-system-monitor");
    if (interface) {
        delete interface;
        interface = nullptr;
    }
}

void MonitorTitleWidget::setMyComBoxTootip(int index)
{
    if(index == 0) {
        m_changeBox->setToolTip(tr("Active Processes"));
    } else if(index == 1) {
        m_changeBox->setToolTip(tr("My Processes"));
    } else if(index == 2) {
        m_changeBox->setToolTip(tr("All Process"));
    } else if(index == 3) {
        m_changeBox->setToolTip(tr("Applications"));
    }
}

void MonitorTitleWidget::onMinBtnClicked()
{
    emit minimizeWindow();
}

void MonitorTitleWidget::onMaxBtnClicked()
{
    emit maximizeWindow();
}

void MonitorTitleWidget::resizeEvent(QResizeEvent *event)
{
    if(window()->isMaximized())
    {
        maxTitleBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
    }
    else
    {
        maxTitleBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    }
    //qDebug()<<"this---size"<<this->size();
}

void MonitorTitleWidget::onCloseBtnClicked()
{
    window()->close();
}

void MonitorTitleWidget::switchChangeItemProcessSignal(int a)
{
    qDebug()<<"whichNum----"<<whichNum;
    emit changeProcessItemDialog(a);
    whichNum = ifsettings->get(WHICH_MENU).toInt();
    ifsettings->set(WHICH_MENU,a);
    setMyComBoxTootip(a);
}

void MonitorTitleWidget::onUpdateMaxBtnStatusChanged()
{
}

void MonitorTitleWidget::initToolbarLeftContent()
{
    m_processButton = new KGroupButton();
    m_processButton->setCheckable(true);
    m_processButton->setAutoExclusive(true);
    m_processButton->setChecked(true);
    m_processButton->setPosition(GroupButtonStyleOption::Begin);
    m_processButton->setFixedSize(NORMALWIDTH,NORMALHEIGHT);

    m_resourceButton = new KGroupButton();
    m_resourceButton->setChecked(false);
    m_resourceButton->setCheckable(true);
    m_resourceButton->setAutoExclusive(true);
    m_resourceButton->setPosition(GroupButtonStyleOption::Middle);
    m_resourceButton->setFixedSize(NORMALWIDTH,NORMALHEIGHT);

    m_filesystemButton = new KGroupButton();
    m_filesystemButton->setChecked(false);
    m_filesystemButton->setCheckable(true);
    m_filesystemButton->setAutoExclusive(true);
    m_filesystemButton->setPosition(GroupButtonStyleOption::End);
    m_filesystemButton->setFixedSize(NORMALWIDTH,NORMALHEIGHT);

    connect(m_processButton, &QPushButton::clicked, this, [=] {
        emit this->changePage(0);
        if (!m_changeBox->isVisible())
            m_changeBox->setVisible(true);
        if (!m_searchEditNew->isVisible())
            m_searchEditNew->setVisible(true);
    });
    connect(m_resourceButton, &QPushButton::clicked, this, [=] {
        emit this->changePage(1);
        if (m_changeBox->isVisible())
            m_changeBox->setVisible(false);
        if (m_searchEditNew->isVisible())
            m_searchEditNew->setVisible(false);
        m_searchEditNew->clear();
        emit canelSearchEditFocus();
        showSearchFocusAnimation(false);
    });
    connect(m_filesystemButton, &QPushButton::clicked, this, [=] {
        emit this->changePage(2);
        if (m_changeBox->isVisible())
            m_changeBox->setVisible(false);
        if (m_searchEditNew->isVisible())
            m_searchEditNew->setVisible(false);
        m_searchEditNew->clear();
        emit canelSearchEditFocus();
        showSearchFocusAnimation(false);
    });
    emptyWidget = new QWidget();

    m_processButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_resourceButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_filesystemButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    emptyWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_bottomLayout->addWidget(m_processButton);
    m_bottomLayout->addWidget(m_resourceButton);
    m_bottomLayout->addWidget(m_filesystemButton);
    m_bottomLayout->addWidget(emptyWidget, 3);
    m_bottomLayout->setSpacing(1);
    if (qtSettings) {
        connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName") {
                qDebug()<<"style name changed";
                auto styleName = qtSettings->get("styleName").toString();

                if (styleName == "ukui-default" || styleName == "ukui-dark" || styleName == "ukui-white"
                        || styleName == "ukui-black" || styleName == "ukui-light" || styleName == "ukui") {
                    if (styleName == "ukui")
                        styleName = "ukui-default";
                    else if (styleName == "ukui-black")
                        styleName = "ukui-dark";
                    else if (styleName == "ukui-white")
                        styleName = "ukui-light";

                    m_processButton->setStyle(new USMProxyStyle(styleName));
                    m_resourceButton->setStyle(new USMProxyStyle(styleName));
                    m_filesystemButton->setStyle(new USMProxyStyle(styleName));
                }
            }
        });
        auto styleName = qtSettings->get("styleName").toString();
        if (styleName == "ukui-default" || styleName == "ukui-dark" || styleName == "ukui-white"
                || styleName == "ukui-black" || styleName == "ukui-light" || styleName == "ukui") {
            if (styleName == "ukui")
                styleName = "ukui-default";
            else if (styleName == "ukui-black")
                styleName = "ukui-dark";
            else if (styleName == "ukui-white")
                styleName = "ukui-light";

            m_processButton->setStyle(new USMProxyStyle(styleName));
            m_resourceButton->setStyle(new USMProxyStyle(styleName));
            m_filesystemButton->setStyle(new USMProxyStyle(styleName));
        }
    }
}

void MonitorTitleWidget::onThemeFontChange(unsigned uFontSize)
{
    int nEditHeight = NORMALHEIGHT;
    if (m_searchEditNew) {
        QFont eFont = m_searchEditNew->font();
        if (uFontSize >= 14) {
            eFont.setPointSize(14);
            m_searchEditNew->setFont(eFont);
            m_searchEditNew->setFixedSize(SPECIALWIDTH, NORMALHEIGHT+6);
        } else {
            eFont.setPointSize(12);
            m_searchEditNew->setFont(eFont);
            m_searchEditNew->setFixedSize(SPECIALWIDTH, NORMALHEIGHT+3);
        }
        nEditHeight = m_searchEditNew->height();
    }
    if (m_processButton) {
        QString strContent = tr("Processes");
        if (!strContent.isEmpty()) {
            QString ShowValue = getElidedText(m_processButton->font(), strContent, m_processButton->width()-4);
            m_processButton->setText(ShowValue);
            if (ShowValue != strContent) {
                m_processButton->setToolTip(strContent);
            } else {
                m_processButton->setToolTip("");
            }
        }
        m_processButton->setFixedSize(NORMALWIDTH, nEditHeight);
    }
    if (m_resourceButton) {
        QString strContent = tr("Resources");
        if (!strContent.isEmpty()) {
            QString ShowValue = getElidedText(m_resourceButton->font(), strContent, m_resourceButton->width()-4);
            m_resourceButton->setText(ShowValue);
            if (ShowValue != strContent) {
                m_resourceButton->setToolTip(strContent);
            } else {
                m_resourceButton->setToolTip("");
            }
        }
        m_resourceButton->setFixedSize(NORMALWIDTH, nEditHeight);
    }
    if (m_filesystemButton) {
        QString strContent = tr("File Systems");
        if (!strContent.isEmpty()) {
            QString ShowValue = getElidedText(m_filesystemButton->font(), strContent, m_filesystemButton->width()-4);
            m_filesystemButton->setText(ShowValue);
            if (ShowValue != strContent) {
                m_filesystemButton->setToolTip(strContent);
            } else {
                m_filesystemButton->setToolTip("");
            }
        }
        m_filesystemButton->setFixedSize(NORMALWIDTH, nEditHeight);
    }
    if (m_changeBox) {
        m_changeBox->setFixedHeight(nEditHeight);
    }
    if (m_queryText) {
        m_queryText->update();
    }
}

void MonitorTitleWidget::initToolbarRightContent()
{
    connect(m_searchEditNew,&QLineEdit::textChanged,this,&MonitorTitleWidget::handleSearchTextChanged,Qt::QueuedConnection);

    emptyWidget2 = new QWidget();
    emptyWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_bottomLayout->addWidget(m_changeBox);
    m_bottomLayout->addWidget(emptyWidget2, 1);
    m_bottomLayout->addWidget(m_searchEditNew);
}

void MonitorTitleWidget::initWidgets()
{
//新的搜索框设置
    m_searchEditNew = new QLineEdit();
    m_searchEditNew->setFixedSize(SPECIALWIDTH, NORMALHEIGHT);
    m_searchEditNew->installEventFilter(this);
    m_searchEditNew->setContextMenuPolicy(Qt::NoContextMenu);
    QFont ft;
    ft.setPixelSize(fontSize);
    //m_searchEditNew->setFont(ft);
    m_queryText=new QLabel();
    m_queryText->setText(tr("Search"));
    m_queryWid=new QWidget(m_searchEditNew);
    m_queryWid->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout* queryWidLayout = new QHBoxLayout;
    queryWidLayout->setContentsMargins(4,4,0,0);
    queryWidLayout->setAlignment(Qt::AlignJustify);
    queryWidLayout->setSpacing(0);
    m_queryWid->setLayout(queryWidLayout);

    m_queryIcon->setFixedWidth(pixmap.width());
    queryWidLayout->addWidget(m_queryIcon);
    queryWidLayout->addWidget(m_queryText);

    queryWidLayout->setAlignment(m_queryIcon,Qt::AlignVCenter);
    queryWidLayout->setAlignment(m_queryText,Qt::AlignVCenter);
    
    m_rectSeachAnimationBegin.setX((m_searchEditNew->width() - (m_queryIcon->width()+m_queryText->width()+10))/2);
    m_rectSeachAnimationBegin.setY(0);
    m_rectSeachAnimationBegin.setWidth(m_queryIcon->width()+m_queryText->width()+10);
    m_rectSeachAnimationBegin.setHeight((m_searchEditNew->height() + 30)/2);
    m_rectSeachAnimationEnd.setX(0);
    m_rectSeachAnimationEnd.setY(0);
    m_rectSeachAnimationEnd.setWidth(m_queryIcon->width()+5);
    m_rectSeachAnimationEnd.setHeight((m_searchEditNew->height()+30)/2);
    m_queryWid->setGeometry(m_rectSeachAnimationBegin.x(), m_rectSeachAnimationBegin.y(), 
        m_rectSeachAnimationBegin.width(), m_rectSeachAnimationBegin.height());   //设置显示label的区域

    m_animation= new QPropertyAnimation(m_queryWid, "geometry");
    m_animation->setDuration(50);

    connect(m_animation,&QPropertyAnimation::finished,this,&MonitorTitleWidget::animationFinishedSlot);
//////////////////

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    QWidget *topWidget = new QWidget;
    m_topLayout = new QHBoxLayout(topWidget);
    m_topLayout->setSpacing(0);
    m_topLayout->setContentsMargins(0,0,0,0);
    m_layout->addWidget(topWidget, 0, Qt::AlignTop);

    QWidget *bottomWidget = new QWidget;
    m_bottomLayout = new QHBoxLayout(bottomWidget);
    m_bottomLayout->setContentsMargins(20, 0, 20, 0);
    m_layout->addWidget(bottomWidget);
//    m_layout->setContentsMargins(0,0,0,0);

    this->setLayout(m_layout);

    initTitlebarLeftContent();
    initTitlebarMiddleContent();
    initTitlebarRightContent();
    initToolbarLeftContent();
    initToolbarRightContent();

    // adjust text show
    onThemeFontChange(fontSize);
}

void MonitorTitleWidget::animationFinishedSlot()
{
    if(m_queryText->parent() == m_queryWid) {
        m_queryWid->layout()->removeWidget(m_queryText);
        m_queryText->setParent(nullptr);
        m_searchEditNew->setTextMargins(30,1,0,1);
    } else {
        m_queryWid->layout()->addWidget(m_queryText);
    }
}

void MonitorTitleWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    QPainterPath path;
    path.setFillRule(Qt::WindingFill); // 多块区域组合填充模式
    path.addRect(this->rect());

    painter.setBrush(this->palette().base());
    painter.setPen(Qt::transparent);
    painter.drawPath(path);
    QWidget::paintEvent(event);
}
