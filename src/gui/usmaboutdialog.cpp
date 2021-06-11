#include "usmaboutdialog.h"
#include "../../shell/xatom-helper.h"
#include "../../shell/macro.h"
#include "../util.h"

#include <QDesktopServices>
#include <QPainterPath>
#include <QPainter>
#include <QTextDocument>
#include <QDebug>
#include <QAbstractTextDocumentLayout>
#include <QTextOption>
#include <QTextBlock>
#include <QTextBlockFormat>

USMAboutDialog::USMAboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setFixedSize(420, 486);

    //XAtomHelper::getInstance()->setUKUIDecoraiontHint(this->winId(), true);
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    m_fFontSize = DEFAULT_FONT_SIZE;
    initWidgets();
    initConnections();
}

USMAboutDialog::~USMAboutDialog()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

bool USMAboutDialog::initWidgets()
{
    m_mainVLayout = new QVBoxLayout();
    m_titleLayout = new QHBoxLayout();
    m_logoIconLayout = new QHBoxLayout();
    m_appNameLayout = new QHBoxLayout();
    m_versionLayout = new QHBoxLayout();
    m_introduceLayout = new QHBoxLayout();
    m_supportLayout = new QHBoxLayout();

    m_mainVLayout->setContentsMargins(0, 0, 0, 0);
    m_mainVLayout->setSpacing(0);
    m_titleLayout->setContentsMargins(8, 4, 4, 4);
    m_titleLayout->setSpacing(0);
    m_logoIconLayout->setContentsMargins(0, 0, 0, 0);
    m_logoIconLayout->setSpacing(0);
    m_appNameLayout->setContentsMargins(0, 0, 0, 0);
    m_appNameLayout->setSpacing(0);
    m_versionLayout->setContentsMargins(0, 0, 0, 0);
    m_versionLayout->setSpacing(0);
    m_introduceLayout->setContentsMargins(0, 0, 0, 0);
    m_introduceLayout->setSpacing(0);
    m_supportLayout->setContentsMargins(0, 0, 0, 0);
    m_supportLayout->setSpacing(8);

    initTitleWidget();

    initContentWidget();

    initSupportWidget();

    m_mainVLayout->addLayout(m_titleLayout);
    m_mainVLayout->addSpacing(34);
    m_mainVLayout->addLayout(m_logoIconLayout);
    m_mainVLayout->addSpacing(12);
    m_mainVLayout->addLayout(m_appNameLayout);
    m_mainVLayout->addSpacing(8);
    m_mainVLayout->addLayout(m_versionLayout);
    m_mainVLayout->addSpacing(8);
    m_mainVLayout->addLayout(m_introduceLayout);
    m_mainVLayout->addStretch();
    m_mainVLayout->addLayout(m_supportLayout);
    m_mainVLayout->addStretch();

    this->setLayout(m_mainVLayout);

    const QByteArray id(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(id))
    {
        m_styleSettings = new QGSettings(id);
    }
    initThemeStyle();

    return true;
}

bool USMAboutDialog::initTitleWidget()
{
    QIcon titleIcon = QIcon::fromTheme("ukui-system-monitor");

    m_labelTitleIcon  = new QLabel();
    m_labelTitleIcon->setPixmap(titleIcon.pixmap(QSize(24, 24)));

    m_labelTitleText = new QLabel(tr("Kylin System Monitor"));

    m_btnClose = new QPushButton();
    m_btnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_btnClose->setProperty("isWindowButton", 0x02);
    m_btnClose->setProperty("useIconHighlightEffect", 0x08);
    m_btnClose->setFlat(true);
    m_btnClose->setFixedSize(30, 30);

    m_titleLayout->addWidget(m_labelTitleIcon);
    m_titleLayout->addSpacing(16);
    m_titleLayout->addWidget(m_labelTitleText);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_btnClose);
    return true;
}

bool USMAboutDialog::initContentWidget()
{
    QIcon titleIcon = QIcon::fromTheme("ukui-system-monitor");
    m_labelLogoIcon = new QLabel();
    m_labelLogoIcon->setPixmap(titleIcon.pixmap(QSize(96, 96)));
    m_logoIconLayout->addStretch();
    m_logoIconLayout->addWidget(m_labelLogoIcon);
    m_logoIconLayout->addStretch();

    m_labelAppName = new QLabel(tr("Kylin System Monitor"));
    m_appNameLayout->addStretch();
    m_appNameLayout->addWidget(m_labelAppName);
    m_appNameLayout->addStretch();

    m_labelVersion = new QLabel(tr("version: ") + getUsmVersion());
    m_versionLayout->addStretch();
    m_versionLayout->addWidget(m_labelVersion);
    m_versionLayout->addStretch();

    m_labelIntroduce = new QLabel(tr("System monitor is a desktop application that face desktop users of Kylin operating system,"
                                        "It meets the needs of users to monitor the system process, system resources and file system"));
    m_labelIntroduce->setMinimumSize(356, 0);
    m_labelIntroduce->setMaximumSize(356, 16777215);
    m_labelIntroduce->setWordWrap(true);
    m_labelIntroduce->setAlignment(Qt::AlignJustify);
    m_introduceLayout->addWidget(m_labelIntroduce);
    return true;
}

bool USMAboutDialog::initSupportWidget()
{
    m_labelSupport = new QLabel(tr("Service and support team:"));

    m_supportLayout->addSpacing(32);
    m_supportLayout->addWidget(m_labelSupport);

    m_btnSupportUrl = new QPushButton("support@kylinos.cn");
    m_btnSupportUrl->setFocusPolicy(Qt::NoFocus);
    m_btnSupportUrl->setCursor( QCursor(Qt::PointingHandCursor));
    m_btnSupportUrl->setStyleSheet("QPushButton{background: transparent;border-radius: 4px;text-decoration: underline;} ");

    m_supportLayout->addWidget(m_btnSupportUrl);
    m_supportLayout->setAlignment(Qt::AlignLeft);
    connect(m_btnSupportUrl, &QPushButton::clicked, this,[=] {
        QDesktopServices::openUrl(QUrl("mailto:support@kylinos.cn"));
    });
    return true;
}

bool USMAboutDialog::initConnections()
{
    connect(m_btnClose, &QPushButton::clicked, this, [=]() {
       this->close();
    });
    return true;
}

void USMAboutDialog::initThemeStyle()
{
    if (!m_styleSettings) {
        return;
    }
    connect(m_styleSettings,&QGSettings::changed,[=](QString key)
    {
        if ("iconThemeName" == key) {
            this->setWindowIcon(QIcon::fromTheme("ukui-system-monitor"));
            if (m_labelTitleIcon)
                m_labelTitleIcon->setPixmap(QIcon::fromTheme("ukui-system-monitor").pixmap(24,24));
            if (m_labelLogoIcon)
                m_labelLogoIcon->setPixmap(QIcon::fromTheme("ukui-system-monitor").pixmap(96,96));
        } else if ("systemFont" == key || "systemFontSize" == key) {
            m_fFontSize = m_styleSettings->get(FONT_SIZE).toString().toFloat();
            onThemeFontChange(m_fFontSize);
        }
    });
    m_fFontSize = m_styleSettings->get(FONT_SIZE).toString().toFloat();
    onThemeFontChange(m_fFontSize);
}

void USMAboutDialog::setFontSize(QLabel *label,int fontSize)
{
    if (!label)
        return;
    QFont font = label->font();
    font.setPointSize(fontSize);
    label->setFont(font);
}

void USMAboutDialog::setFontSize(QTextEdit *txtEdit,int fontSize)
{
    if (!txtEdit)
        return;
    QFont font = txtEdit->font();
    font.setPointSize(fontSize);
    txtEdit->setFont(font);
}

void USMAboutDialog::setFontSize(QPushButton *btn,int fontSize)
{
    if (!btn)
        return;
    QFont font = btn->font();
    font.setPointSize(fontSize);
    btn->setFont(font);
}

void USMAboutDialog::paintEvent(QPaintEvent *event)
{
    QPainterPath path;
    QPainter painter(this);

    path.addRect(this->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().base());
    painter.setPen(Qt::transparent);
    painter.drawPath(path);
    QDialog::paintEvent(event);
}

void USMAboutDialog::onThemeFontChange(float fFontSize)
{
    if (m_btnSupportUrl) {
        setFontSize(m_btnSupportUrl, fFontSize);
    }
    if (m_labelAppName) {
        setFontSize(m_labelAppName, fFontSize * 1.3);
    }
}