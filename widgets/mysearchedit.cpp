#include "mysearchedit.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QPushButton>
#include <QEvent>
#include <QFocusEvent>
#include <QResizeEvent>

/**
 * QT主题
 */
#define THEME_QT_SCHEMA "org.ukui.style"
#define MODE_QT_KEY "style-name"
/* QT图标主题 */
#define ICON_QT_KEY "icon-theme-name"

MySearchEdit::MySearchEdit(QWidget *parent)
    : QFrame(parent)
    ,m_showCurve(QEasingCurve::OutCubic)    
    ,m_hideCurve(QEasingCurve::InCubic)
{
    const QByteArray idd(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(idd))
    {
//        qDebug()<<"MySearchEdit--------------1------------->"<<endl;
        qtSettings = new QGSettings(idd);
    }


    this->setWindowFlags(Qt::FramelessWindowHint);
    //this->setStyleSheet("QFrame{background-color:#00376a;border-radius:0px;}");
//    this->setFixedSize(200,32);
    qDebug()<<"mySearchEdit.width"<<this->width()<<"mySearchEidt.height"<<this->height();

//    this->setObjectName("SearchBtn");
//    this->setStyleSheet("QFrame#SearchBtn{background:rgba(77,88,99,0.08);border-radius:4px;}");
    m_searchBtn = new QLabel;
    m_searchBtn->setStyleSheet("QLabel{background-color:transparent;border:none;background-image:url(:/img/search.png);}");
    m_searchBtn->setFixedSize(16, 16);

//    m_clearBtn = new MyTristateButton;
//    QPushButton *m_clearBtn = new QPushButton();
//    QIcon icon(tr(":/img/close.png"));
//    m_clearBtn->setIcon(icon);
//    m_clearBtn->setObjectName("ClearIcon");
//    m_clearBtn->hide();

    m_pClearTextButton = new QPushButton;
    m_pClearTextButton->setFixedSize(19, 21);
    m_pClearTextButton->setIconSize(QSize(19, 19));
    m_pClearTextButton->setObjectName("ClearTextButton");

    m_edit = new QLineEdit;

    QIcon ClearTextEditIcon;
//    ClearTextEditIcon.addFile(":/img/button-close-default-add-background-three.svg");
//    m_pClearTextButton->setIcon(ClearTextEditIcon);
    //m_pClearTextButton->setIcon(drawSymbolicColoredPixmap(QPixmap::fromImage(QIcon::fromTheme(":/img/button-close-default-add-background-three.svg").pixmap(24,24).toImage())));
    m_pClearTextButton->setCursor(Qt::ArrowCursor);



    m_edit->setAttribute(Qt::WA_Hover, true);
    m_edit->setStyle(new InternalStyle("ukui-default"));
    m_edit->setTextMargins(8,0,0,0);

    connect(m_edit, &QLineEdit::textChanged, this, &MySearchEdit::textChageSlots);

    QFont SearchLine;
    SearchLine = m_edit->font();
    SearchLine.setPixelSize(14);
    SearchLine.setFamily("Noto Sans CJK SC");
    m_edit->setFont(SearchLine);
    m_edit->setStyleSheet("color:palette(windowText)");

    QWidgetAction *action = new QWidgetAction(m_edit);
    action->setDefaultWidget(m_pClearTextButton);
    m_edit->addAction(action, QLineEdit::TrailingPosition);
    m_pClearTextButton->setVisible(false);

    connect(m_pClearTextButton, &QPushButton::clicked, this, [=](){
          m_edit->setText("");
    });
    //m_edit->setPlaceholderText("enter process info");

    m_placeHolder = new QLabel;  //about the font
//    m_placeHolder->setStyleSheet("QLabel{background-color:transparent;color:rgba(0,0,0,0.57);font-size:14px;margin: 2 0 0 0 px;}");

    initThemeMode();

    m_animation = new QPropertyAnimation(m_edit, "minimumWidth");

//    m_size = QSize(m_searchBtn->sizeHint().width() + m_edit->sizeHint().width() + m_clearBtn->sizeHint().width() + 6,
//                   qMax(m_searchBtn->sizeHint().height(), m_edit->sizeHint().height()));
    m_edit->setFixedWidth(0);
    m_edit->installEventFilter(this);

    m_pClearTextButton->setStyleSheet(
                "QPushButton{"
                "background:transparent;"
                "border:0px solid;"
                "}");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addStretch();
    layout->addWidget(m_searchBtn);
    layout->setAlignment(m_searchBtn, Qt::AlignCenter);
    layout->addWidget(m_placeHolder);
    layout->setAlignment(m_placeHolder, Qt::AlignCenter);
    layout->addWidget(m_edit);
    layout->setAlignment(m_edit, Qt::AlignCenter);
    layout->addWidget(m_pClearTextButton);
    layout->setAlignment(m_pClearTextButton,Qt::AlignCenter);
    layout->addStretch();
//    layout->addWidget(m_clearBtn);
//    layout->setAlignment(m_clearBtn, Qt::AlignCenter);



    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    setFocusPolicy(Qt::StrongFocus);

//    connect(m_edit, &QLineEdit::textChanged, [this] {m_clearBtn->setVisible(!m_edit->text().isEmpty());});
    connect(m_edit, &QLineEdit::textChanged, this, &MySearchEdit::textChanged, Qt::DirectConnection);
//    connect(m_clearBtn, SIGNAL(clicked()), this, SLOT(clearAndFocusEdit()));
//    connect(m_clearBtn, &MyTristateButton::clicked, this, [=] {
//        this->clearAndFocusEdit();
//    });

//    qDebug()<<this->width()<<"----------------------------------------------"<<this->height();
}


QPixmap MySearchEdit::drawSymbolicColoredPixmap(const QPixmap &source)
{
//    qDebug()<<"wwj,wozhendeshishishishsishishsishsihsishsishsishsihs"<<currentThemeMode;
    if(currentThemeMode == "ukui-white")
    {
        QImage img = source.toImage();
        for (int x = 0; x < img.width(); x++)
        {
            for (int y = 0; y < img.height(); y++)
            {
                auto color = img.pixelColor(x, y);
                if (color.alpha() > 0)
                {
                        color.setRed(0);
                        color.setGreen(0);
                        color.setBlue(0);
                        img.setPixelColor(x, y, color);
                }
            }
        }
        return QPixmap::fromImage(img);
    }

    if(currentThemeMode == "ukui-black")
    {
        QImage img = source.toImage();
        for (int x = 0; x < img.width(); x++)
        {
            for (int y = 0; y < img.height(); y++)
            {
                auto color = img.pixelColor(x, y);
                if (color.alpha() > 0)
                {
                        color.setRed(255);
                        color.setGreen(255);
                        color.setBlue(255);
                        img.setPixelColor(x, y, color);
                }
            }
        }
        return QPixmap::fromImage(img);
    }
}

MySearchEdit::~MySearchEdit()
{
    m_animation->deleteLater();
    delete m_edit;
    delete m_searchBtn;
    delete m_placeHolder;
//    delete m_clearBtn;
}

const QString MySearchEdit::searchedText() const
{
    return m_edit->text();
}

void MySearchEdit::clearAndFocusEdit()
{
    this->clearEdit();
    this->m_edit->setFocus();
}

void MySearchEdit::clearEdit()
{
    m_edit->clear();
    this->setStyleSheet("QFrame{background:rgba(77,88,99,0.08);border-radius:4px;}");
}

void MySearchEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return QFrame::mousePressEvent(event);

    setEditFocus();
    event->accept();
}

void MySearchEdit::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
}

bool MySearchEdit::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_edit && event->type() == QEvent::FocusOut && m_edit->text().isEmpty()) {
        auto focusEvent = dynamic_cast<QFocusEvent *>(event);
        if (focusEvent && focusEvent->reason() != Qt::PopupFocusReason) {
//            m_placeHolder->show();
            m_animation->stop();
            m_animation->setStartValue(m_edit->width());
            m_animation->setEndValue(0);
            m_animation->setEasingCurve(m_hideCurve);
            m_animation->start();
            connect(m_animation, &QPropertyAnimation::finished, m_placeHolder, &QLabel::show);
        }
    }
    return QFrame::eventFilter(object, event);
}

void MySearchEdit::setEditFocus()
{
    if (!m_placeHolder->isVisible()) {
        return;
    }
    disconnect(m_animation, &QPropertyAnimation::finished, m_placeHolder, &QLabel::show);
    m_animation->stop();
    m_animation->setStartValue(0);
    m_animation->setEndValue(m_size.width() - m_searchBtn->width() - 6);
    m_animation->setEasingCurve(m_showCurve);
    m_animation->start();
    m_placeHolder->hide();
    m_edit->setFocus();
    //this->setStyleSheet("QFrame{background-color:rgba(19,19,20,0.2);border:1px solid #CC00FF;border-radius:4px;}");
}

void MySearchEdit::setPlaceHolder(const QString &text)
{
    m_placeHolder->setText(text);
}

void MySearchEdit::setText(const QString & text)
{
    if (m_edit) {
        m_edit->setText(text);
    }
}

QLineEdit *MySearchEdit::getLineEdit() const
{
    return m_edit;
}

void MySearchEdit::resizeEvent(QResizeEvent *event)
{
    m_size = event->size();
    m_edit->setFixedHeight(m_size.height());
}

bool MySearchEdit::event(QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        const QFocusEvent *ev = static_cast<QFocusEvent*>(event);

        if (ev->reason() == Qt::TabFocusReason || ev->reason() == Qt::BacktabFocusReason || ev->reason() == Qt::OtherFocusReason || ev->reason() == Qt::ShortcutFocusReason) {
            setEditFocus();
        }
    }
    return QFrame::event(event);
}

void MySearchEdit::textChageSlots(const QString &text)
{
    if (text != "")
    {
        m_pClearTextButton  ->setVisible(true);
    }
    else
    {
        m_pClearTextButton->setVisible(false);
    }
}

//void MySearchEdit::paintEvent(QEvent *event)
//{
////    QStyleOption opt;
////    opt.init(this);
////    QPainter p(this);
////    p.setBrush(QBrush(QColor(0x19,0x19,0x20,0xFF)));
////    p.setPen(Qt::NoPen);
////    QPainterPath path;
////    opt.rect.adjust(0,0,0,0);
////    path.addRoundedRect(opt.rect,6,6);
////    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
////    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
////    p.drawRoundedRect(opt.rect, 6, 6);
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    p.setBrush(QBrush(QColor(0x19,0x19,0x20,0x19)));
//    //p.setPen(Qt::NoPen);
//    QPainterPath path;
//    //opt.rect.adjust(0,0,0,0);
////    path.addRoundRect(opt.rect.topLeft(),6);

//    path.addPath(path);
//    path.addRoundedRect(opt.rect,6,6);
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
////       p.drawRoundedRect(opt.rect,6,6);
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
////       p.setPen(QPen(QColor("#e9eef0"), 0));//边框颜色
////       p.setBrush(QColor("#0d87ca"));//背景色
////       QRectF r(1, 1, width() - 2, height() - 2);//左边 上边 右边 下边
//    p.drawPath(path);
//}

void MySearchEdit::initThemeMode()
{
    //监听主题改变
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key){

        if (key == "styleName") {
            auto style = qtSettings->get(key).toString();
            qApp->setStyle(new InternalStyle(style));
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
//            qDebug()<<"监听主题改变-------------------->"<<currentThemeMode<<endl;
            qApp->setStyle(new InternalStyle(currentThemeMode));
            m_pClearTextButton->setIcon(drawSymbolicColoredPixmap(QPixmap::fromImage(QIcon::fromTheme(":/img/button-close-default-add-background-three.svg").pixmap(24,24).toImage())));

            if(currentThemeMode == "ukui-white")
            {
                m_edit->setStyleSheet("QLineEdit{background:transparent;border-radius:4px;color:#00000;padding-right:12px;padding-bottom: 2px;}"); //#CC00FF transparent
                m_placeHolder->setStyleSheet("QLabel{background-color:transparent;color:rgba(0,0,0,0.57);font-size:14px;margin: 2 0 0 0 px;}");
                this->setObjectName("SearchBtn");
                this->setStyleSheet("QFrame#SearchBtn{background:rgba(13,14,14,0.08);border-radius:4px;}");
            }

            if(currentThemeMode == "ukui-black")
            {
                m_edit->setStyleSheet("QLineEdit{background:transparent;border-radius:4px;color:#FFFFFF;padding-right:12px;padding-bottom: 2px;}"); //#CC00FF transparent
                m_placeHolder->setStyleSheet("QLabel{background-color:transparent;color:rgba(255,255,255,0.57);font-size:14px;margin: 2 0 0 0 px;}");
                this->setObjectName("SearchBtn");
                this->setStyleSheet("QFrame#SearchBtn{background:rgba(255,255,255,0.08);border-radius:4px;}");
            }
        }

    });
    //获取当前主题
    currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    qDebug()<<"监听主题改变-------------------->"<<currentThemeMode<<endl;
    qApp->setStyle(new InternalStyle(currentThemeMode));
    m_pClearTextButton->setIcon(drawSymbolicColoredPixmap(QPixmap::fromImage(QIcon::fromTheme(":/img/button-close-default-add-background-three.svg").pixmap(24,24).toImage())));
    if(currentThemeMode == "ukui-white")
    {
        m_edit->setStyleSheet("QLineEdit{background:transparent;border-radius:4px;color:#00000;padding-right:12px;padding-bottom: 2px;}"); //#CC00FF transparent
        m_placeHolder->setStyleSheet("QLabel{background-color:transparent;color:rgba(0,0,0,0.57);font-size:14px;margin: 2 0 0 0 px;}");
        this->setObjectName("SearchBtn");
        this->setStyleSheet("QFrame#SearchBtn{background:rgba(13,14,14,0.08);border-radius:4px;}");
    }

    if(currentThemeMode == "ukui-black")
    {
        m_edit->setStyleSheet("QLineEdit{background:transparent;border-radius:4px;color:#FFFFFF;padding-right:12px;padding-bottom: 2px;}"); //#CC00FF transparent
        m_placeHolder->setStyleSheet("QLabel{background-color:transparent;color:rgba(255,255,255,0.57);font-size:14px;margin: 2 0 0 0 px;}");
        this->setObjectName("SearchBtn");
        this->setStyleSheet("QFrame#SearchBtn{background:rgba(255,255,255,0.08);border-radius:4px;}");
    }
}
