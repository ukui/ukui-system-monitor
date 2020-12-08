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
#include "../widgets/mytristatebutton.h"
#include "../widgets/myunderlinebutton.h"
#include "../widgets/mysearchedit.h"
#include "../shell/macro.h"
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
#include <KWindowEffects>

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
    m_queryIcon=new QLabel();
    QIcon queryIcon;
    queryIcon = QIcon::fromTheme("preferences-system-search-symbolic");
    pixmap = queryIcon.pixmap(QSize(21, 21));
    whichBox = new QList<int>();
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
    
    initFontSize();

    m_changeBox = new QComboBox();
    m_changeBox->setFixedSize(NORMALWIDTH,NORMALHEIGHT+2);
    m_changeBox->addItem(tr("Active Processes"));
    m_changeBox->addItem(tr("My Processes"));
    m_changeBox->addItem(tr("All Process"));
    m_changeBox->setFocusPolicy(Qt::NoFocus);
    m_changeBox->setView(new  QListView());
    QFont changeBoxFont;
    changeBoxFont.setPointSize(fontSize);
    m_changeBox->setFont(changeBoxFont);

    initThemeMode();
    getTransparentData();

    this->setWindowFlags(Qt::FramelessWindowHint);//this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint  | Qt::WindowCloseButtonHint);

    installEventFilter(this);
    setFixedHeight(MONITOR_TITLE_WIDGET_HEIGHT);

    this->setAutoFillBackground(true);
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    connect(m_searchTimer, SIGNAL(timeout()), this, SLOT(onRefreshSearchResult()));

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
        if (key == "styleName")
        {
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            if (currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
            {
                qDebug() << "The theme change to white";
                this->setObjectName("MonitorTitle");
                this->setStyleSheet("QFrame#MonitorTitle{background:rgba(255,255,255,0);border-top-left-radius:6px;border-top-right-radius:6px;color: palette(windowText);}");
                m_queryIcon->setPixmap(drawSymbolicBlackColoredPixmap(pixmap));

            }

            else/*(currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black")*/
            {
                this->setObjectName("MonitorTitle");
                this->setStyleSheet("QFrame#MonitorTitle{background:rgba(13,14,14,0);border-top-left-radius:6px;border-top-right-radius:6px;color: palette(windowText);}");
                m_queryIcon->setPixmap(drawSymbolicColoredPixmap(pixmap));
            }

        }
    });

    currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if (currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
    {
        this->setObjectName("MonitorTitle");
        this->setStyleSheet("QFrame#MonitorTitle{background:rgba(255,255,255,0);border-top-left-radius:6px;border-top-right-radius:6px;color: palette(windowText);}");
        m_queryIcon->setPixmap(drawSymbolicBlackColoredPixmap(pixmap));
    }

    if (currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black")
    {
        this->setObjectName("MonitorTitle");
        this->setStyleSheet("QFrame#MonitorTitle{background:rgba(13,14,14,0);border-top-left-radius:6px;border-top-right-radius:6px;color: palette(windowText);}");
        m_queryIcon->setPixmap(drawSymbolicColoredPixmap(pixmap));
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
        }
        QFont font;
        font.setPointSize(fontSize);
        titleLabel->setFont(font);

        QFont changeBoxFont;
        changeBoxFont.setPointSize(fontSize);
        m_changeBox->setFont(changeBoxFont);
    });
    fontSize = fontSettings->get(FONT_SIZE).toString().toFloat();
}

MonitorTitleWidget::~MonitorTitleWidget()
{
    delete emptyLabel;
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
    if(fontSettings)
    {
        delete fontSettings;
    }
    if(qtSettings)
    {
        delete qtSettings;
    }
    if(m_animation)
    {
        delete m_animation;
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
            }
        }
        else if (obj == m_searchEditNew) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Tab) {
//                m_searchEdit->clearEdit();
                m_searchEditNew->clear();
                emit canelSearchEditFocus();
            }
        }
    }

    if (obj == m_searchEditNew) {
        if (event->type() == QEvent::FocusIn)
        {
            emit SearchFocusIN();
            if(m_searchEditNew->text().isEmpty())
            {
                qDebug()<<"it is real";
                m_animation->stop();
                m_animation->setStartValue(QRectF((m_searchEditNew->width() - (m_queryIcon->width()+m_queryText->width()+10))/2,0,
                                                 m_queryIcon->width()+m_queryText->width()+10,(m_searchEditNew->height() + 20)/2));
                m_animation->setEndValue(QRectF(0,0,
                                               m_queryIcon->width()+3,(m_searchEditNew->height()+20)/2));
                m_animation->setEasingCurve(QEasingCurve::OutQuad);
                m_animation->start();
            }
            m_isSearching=true;
        }
        else if(event->type() == QEvent::FocusOut)
        {
            emit SearchFocusOut();
            m_searchEditNew->clear();
            m_searchEditNew->clearFocus();
            if(m_searchEditNew->text().isEmpty())
            {
                m_queryText->adjustSize();
                m_animation->setStartValue(QRectF(0,0,
                                                 m_queryIcon->width()+3,(m_searchEditNew->height()+20)/2));
                m_animation->setEndValue(QRectF((m_searchEditNew->width() - (m_queryIcon->width()+m_queryText->width()+10))/2,0,
                                               m_queryIcon->width()+m_queryText->width()+10,(m_searchEditNew->height()+20)/2));
                m_animation->setEasingCurve(QEasingCurve::InQuad);
                m_animation->start();
            }
            m_isSearching=false;
        }
    }
    if(obj != m_searchEditNew && event->type() == QEvent::MouseButtonPress)
    {
        m_searchEditNew->clear();
        m_searchEditNew->clearFocus();
    }

    return QFrame::eventFilter(obj, event);
}

void MonitorTitleWidget::setSearchEditFocus()
{
//    if (m_searchEdit->searchedText() != "") {
//        m_searchEdit->getLineEdit()->setFocus();
//    } else {
//        m_searchEdit->setFocus();
//    }
}

void MonitorTitleWidget::onRefreshSearchResult()
{
//    if (m_searchEdit->searchedText() == searchTextCache) {
//        emit this->searchSignal(searchTextCache);
//    }
    if (m_searchEditNew->text() == searchTextCache) {
        emit this->searchSignal(searchTextCache);
    }
}

void MonitorTitleWidget::handleSearchTextChanged()
{
//    searchTextCache = m_searchEdit->searchedText();
    searchTextCache = m_searchEditNew->text();
    if (m_searchTimer->isActive()) {
        m_searchTimer->stop();
    }
    m_searchTimer->start(300);
}

void MonitorTitleWidget::onCancelSearchBtnClicked(bool b)
{
//    m_searchEdit->clearAndFocusEdit();
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
    m_titleMiddleLayout->setContentsMargins(0, 0, 0, 0);

    titleLabel = new QLabel;
    QLabel *picLabel = new QLabel;
    QFont font;
    font.setPointSize(fontSize-2);
    titleLabel->setFont(font);
    titleLabel->setText(tr("Kylin System Monitor"));
    picLabel->setPixmap(QPixmap(":img/ukui-system-monitor.png"));
    m_titleMiddleLayout->addWidget(picLabel);
    m_titleMiddleLayout->addWidget(titleLabel);
    m_topLayout->addWidget(w);
}

void MonitorTitleWidget::initTitlebarRightContent()
{
    QWidget *w = new QWidget;
    m_titleRightLayout = new QHBoxLayout(w);
    m_titleRightLayout->setContentsMargins(0, 8, 8, 0);
    m_titleRightLayout->setSpacing(0);

    m_topLayout->addWidget(w, 1, Qt::AlignRight);
    QPushButton *minBtn = new QPushButton(this);
    minBtn->setToolTip(tr("minimize"));
    minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    minBtn->setProperty("isWindowButton", 0x1);
    minBtn->setProperty("useIconHighlightEffect", 0x2);
    minBtn->setFlat(true);
    connect(minBtn, SIGNAL(clicked()), this, SLOT(onMinBtnClicked()));

    maxTitleBtn = new QPushButton();
    maxTitleBtn->setToolTip(tr("maximize/restore"));
    maxTitleBtn->setProperty("isWindowButton", 0x1);
    maxTitleBtn->setProperty("useIconHighlightEffect", 0x2);
    maxTitleBtn->setFlat(true);
    maxTitleBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    connect(maxTitleBtn, SIGNAL(clicked()), this, SLOT(onMaxBtnClicked()));
    QPushButton *closeBtn = new QPushButton(this);
    closeBtn->setToolTip(tr("close"));
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setFlat(true);
    closeBtn->setProperty("isWindowButton", 0x2);
    closeBtn->setProperty("useIconHighlightEffect", 0x8);
    connect(closeBtn, SIGNAL(clicked()), this, SLOT(onCloseBtnClicked()));
    //进行GSettings设置，记忆用户选择的combox的内容
    whichNum = ifsettings->get(WHICH_MENU).toInt();
    m_changeBox->setCurrentIndex(whichNum);
    connect(m_changeBox,SIGNAL(currentIndexChanged(int)),this,SLOT(switchChangeItemProcessSignal(int)));
    m_titleRightLayout->addWidget(minBtn);
    m_titleRightLayout->addWidget(maxTitleBtn);
    m_titleRightLayout->addWidget(closeBtn);
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
}

void MonitorTitleWidget::onUpdateMaxBtnStatusChanged()
{
}

void MonitorTitleWidget::initToolbarLeftContent()
{
    QWidget *w = new QWidget;
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_toolLeftLayout = new QHBoxLayout(w);
    m_toolLeftLayout->setContentsMargins(0, 0, 0, 0);
    QPushButton *processButton = new QPushButton();
    processButton->setText(tr("Processes"));
    processButton->setChecked(true);
    processButton->setFixedSize(NORMALWIDTH,NORMALHEIGHT+2);

    QPushButton *resourcesButton = new QPushButton();
    resourcesButton->setText(tr("Resources"));
    resourcesButton->setChecked(false);
    resourcesButton->setFixedSize(NORMALWIDTH,NORMALHEIGHT+2);

    QPushButton *disksButton = new QPushButton();
    disksButton->setText(tr("File Systems"));
    disksButton->setChecked(false);
    disksButton->setFixedSize(NORMALWIDTH,NORMALHEIGHT+2);

    connect(processButton, &QPushButton::clicked, this, [=] {
        emit this->changePage(0);
        processButton->setChecked(true);
        resourcesButton->setChecked(false);
        disksButton->setChecked(false);
//        if (!m_searchEdit->isVisible())
//            m_searchEdit->setVisible(true);
        if (!m_changeBox->isVisible())
            m_changeBox->setVisible(true);
        if (!m_searchEditNew->isVisible())
            m_searchEditNew->setVisible(true);
    });
    connect(resourcesButton, &QPushButton::clicked, this, [=] {
        emit this->changePage(1);
        processButton->setChecked(false);
        resourcesButton->setChecked(true);
        disksButton->setChecked(false);
//        if (m_searchEdit->isVisible())
//            m_searchEdit->setVisible(false);
        if (m_changeBox->isVisible())
            m_changeBox->setVisible(false);
        if (m_searchEditNew->isVisible())
            m_searchEditNew->setVisible(false);
//        m_searchEdit->clearEdit();
        m_searchEditNew->clear();
        emit canelSearchEditFocus();
    });
    connect(disksButton, &QPushButton::clicked, this, [=] {
        emit this->changePage(2);
        processButton->setChecked(false);
        resourcesButton->setChecked(false);
        disksButton->setChecked(true);
//        if (m_searchEdit->isVisible())
//            m_searchEdit->setVisible(false);
        if (m_changeBox->isVisible())
            m_changeBox->setVisible(false);
        if (m_searchEditNew->isVisible())
            m_searchEditNew->setVisible(false);
//        m_searchEdit->clearEdit();
        m_searchEditNew->clear();
        emit canelSearchEditFocus();
    });
    emptyWidget = new QWidget();

    processButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    resourcesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    disksButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    emptyWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_bottomLayout->addWidget(processButton);
    m_bottomLayout->addWidget(resourcesButton);
    m_bottomLayout->addWidget(disksButton);
    m_bottomLayout->addWidget(emptyWidget, 3);
}

void MonitorTitleWidget::initToolbarRightContent()
{
//    connect(m_searchEdit, &MySearchEdit::textChanged, this, &MonitorTitleWidget::handleSearchTextChanged, Qt::QueuedConnection);
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
    m_searchEditNew->setFixedSize(SPECIALWIDTH, NORMALHEIGHT +2);
    m_searchEditNew->installEventFilter(this);
    m_searchEditNew->setContextMenuPolicy(Qt::NoContextMenu);
    m_queryText=new QLabel();
    m_queryText->setText(tr("Enter the relevant info of process"));
    m_queryWid=new QWidget(m_searchEditNew);
    m_queryWid->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout* queryWidLayout = new QHBoxLayout;
    queryWidLayout->setContentsMargins(4,4,0,0);
    queryWidLayout->setAlignment(Qt::AlignJustify);
    queryWidLayout->setSpacing(0);
    m_queryWid->setLayout(queryWidLayout);

    queryWidLayout->addWidget(m_queryIcon);
    queryWidLayout->addWidget(m_queryText);

    m_queryWid->setGeometry(QRect((m_searchEditNew->width() - (m_queryIcon->width()+m_queryText->width()+10))/2,0,
                                        m_queryIcon->width()+m_queryText->width()-10,(m_searchEditNew->height() + 20)/2));   //设置显示label的区域

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
    m_layout->addWidget(topWidget, 0, Qt::AlignTop);

    QWidget *bottomWidget = new QWidget;
    m_bottomLayout = new QHBoxLayout(bottomWidget);
//    m_bottomLayout->setContentsMargins(10, 0, 10, 0);
    m_layout->addWidget(bottomWidget);
//    m_layout->setContentsMargins(0,0,0,0);

    this->setLayout(m_layout);

    initTitlebarLeftContent();
    initTitlebarMiddleContent();
    initTitlebarRightContent();
    initToolbarLeftContent();
    initToolbarRightContent();
}

void MonitorTitleWidget::animationFinishedSlot()
{
    if(m_isSearching)
    {
        m_queryWid->layout()->removeWidget(m_queryText);
        m_queryText->setParent(nullptr);
        m_searchEditNew->setTextMargins(30,1,0,1);
    }
    else
    {
        m_queryWid->layout()->addWidget(m_queryText);
    }
}

void MonitorTitleWidget::paintEvent(QPaintEvent *event)
{
//    QPainterPath path;
//    auto rect = this->rect();
//    rect.adjust(1, 1, -1, -1);
//    path.addRoundedRect(rect, 6, 6);
//    setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));

//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    QRect rectReal = this->rect();
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.setBrush(opt.palette.color(QPalette::Base));
//    p.setOpacity(m_transparency);
//    qDebug()<<"real transparency"<<m_transparency;
//    p.setPen(Qt::NoPen);
//    p.drawRoundedRect(rectReal, 6, 6);
//    QWidget::paintEvent(event);

//    KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(path.toFillPolygon().toPolygon()));
}
