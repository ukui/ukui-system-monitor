#include "myunderlinebutton.h"

#include <QMouseEvent>
#include <QEvent>
#include <QDebug>
#include <QVBoxLayout>


MyUnderLineButton::MyUnderLineButton(QWidget *parent)
    : QWidget(parent)
    ,m_state(Normal)
    ,m_isChecked(false)
{
    const QByteArray idd(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(idd))
    {
        qtSettings = new QGSettings(idd);
    }

    this->setFixedSize(100, 32);    
    m_textLabel = new QLabel;

    initThemeMode();

//    m_underlineLabel = new QLabel;
//    m_underlineLabel->setFixedSize(52, 2);
//    m_underlineLabel->setStyleSheet("QLabel{background-color:#ffffff;}");
//    m_underlineLabel->hide();

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0,5,0,10);

    m_layout->addWidget(m_textLabel, 0, Qt::AlignTop | Qt::AlignHCenter);
    //m_layout->addWidget(m_underlineLabel, 0, Qt::AlignBottom | Qt::AlignHCenter);
}

void MyUnderLineButton::initThemeMode()
{
    //监听主题改变
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key)
    {

        if (key == "styleName")
        {
            auto style = qtSettings->get(key).toString();
            qApp->setStyle(new InternalStyle(style));
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            qDebug()<<"监听主题改变-------------------->"<<currentThemeMode<<endl;
            qApp->setStyle(new InternalStyle(currentThemeMode));
            //repaint();
            if (currentThemeMode == "ukui-white")
            {
                m_textLabel->setStyleSheet("QLabel{background-color:transparent;color:rgba(0,0,0,0.57); font-size:14px;text-align:center;}"); //ffffff
            }

            if (currentThemeMode == "ukui-black")
            {
                m_textLabel->setStyleSheet("QLabel{background-color:transparent;color:rgba(255,255,255,0.57); font-size:14px;text-align:center;}"); //ffffff
            }

        }
        repaint();
        updateStyleSheet();
    });
    currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if (currentThemeMode == "ukui-white")
    {
        m_textLabel->setStyleSheet("QLabel{background-color:transparent;color:rgba(0,0,0,0.57); font-size:14px;text-align:center;}"); //ffffff
    }

    if (currentThemeMode == "ukui-black")
    {
        m_textLabel->setStyleSheet("QLabel{background-color:transparent;color:rgba(255,255,255,0.57); font-size:14px;text-align:center;}"); //ffffff
    }
}

MyUnderLineButton::~MyUnderLineButton()
{
    delete m_textLabel;
    //delete m_underlineLabel;
}

void MyUnderLineButton::enterEvent(QEvent *event)
{
    setCursor(Qt::PointingHandCursor);

    if (!m_isChecked){
        setState(Normal);
    }

    event->accept();
    //QWidget::enterEvent(event);
}

void MyUnderLineButton::leaveEvent(QEvent *event)
{
    if (!m_isChecked){
        setState(Normal);
    }

    event->accept();
    //QWidget::leaveEvent(event);
}

void MyUnderLineButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    setState(Press);

    event->accept();
    //QWidget::mousePressEvent(event);
}

void MyUnderLineButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (!rect().contains(event->pos()))
        return;

    m_isChecked = !m_isChecked;
    if (m_isChecked){
        setState(Checked);
    } else {
        setState(Normal);
    }

    event->accept();
    //QWidget::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
        emit clicked();
}

void MyUnderLineButton::mouseMoveEvent(QMouseEvent *event)
{
    if (!rect().contains(event->pos())) {
        setState(Normal);
    }
}

void MyUnderLineButton::updateStyleSheet()
{
    switch (m_state) {
//    case Hover:
//        m_textLabel->setStyleSheet("QLabel{background-color:transparent;color:#778899; font-size:14px;text-align:center;font-weight:bold;}");
//        //m_underlineLabel->hide();
//        break;
    case Press:
        this->setObjectName("OnlyBackground");
        this->setStyleSheet("onlyBackground#QPushButton{background:#3d6be5;}");
        m_textLabel->setStyleSheet("QLabel{background-color:#3d6be5;color:#ffffff; font-size:14px;text-align:center;font-weight:bold;}");
        //m_underlineLabel->hide();
        break;
    case Normal:
        if(currentThemeMode == "ukui-white")
        {
            m_textLabel->setStyleSheet("QLabel{back-color:transparent;color:rgba(0,0,0,0.57); font-size:14px;text-align:center;}");
        }
        else
        {
            m_textLabel->setStyleSheet("QLabel{back-color:transparent;color:rgba(255,255,255,0.57); font-size:14px;text-align:center;}");
        }
//    case Normal:

//    case Checked:
//        m_textLabel->setStyleSheet("QLabel{background-color:transparent;color:778899; font-size:14px;text-align:center;font-weight:bold;}");
//        //m_underlineLabel->show();
//        break;
//    default:
//        m_textLabel->setStyleSheet("QLabel{background-color:transparent;color:778899; font-size:14px;text-align:center;}");
//        //m_underlineLabel->hide();
//        break;
    }
//   ////////////// m_textLabel->setStyleSheet("QLabel{color:white};");
}

void MyUnderLineButton::setState(MyUnderLineButton::ButtonState state)
{
    if (m_state == state)
        return;

    m_state = state;

        updateStyleSheet();
}

void MyUnderLineButton::setChecked(bool flag)          /////////////////face the difficulties，then resolve them
{
    m_isChecked = flag;
    if (m_isChecked){
        setState(Press);
    } else {
        setState(Normal);
    }
    repaint();
}

bool MyUnderLineButton::isChecked()
{
    return m_isChecked;
}

void MyUnderLineButton::setName(const QString &name)
{
    this->m_textLabel->setText(name);
}

MyUnderLineButton::ButtonState MyUnderLineButton::getButtonState() const
{
    return m_state;
}

void MyUnderLineButton::paintEvent(QPaintEvent *event)
{
#if 0
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
////    ////////set the background color
//    QPainterPath path;
//    path.addRect(QRectF(0, 0, width(), height()));
////    painter.setOpacity(1);
////    painter.fillPath(path, QColor("#0d87ca"));

    ////////Draw fillet
    QStyleOption opt;
    opt.init(this);CC00FF
    painter.setBrush(QBrush(QColor(0x3D,0x6B,0xE5,0xFF)));
    painter.setPen(Qt::NoPen);
    opt.rect.adjust(0,0,0,0);
//    path.addRoundedRect(opt.rect,4,4);
    painter.drawRoundedRect(opt.rect,4,4);
//    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget,&opt,&painter,this);

//    painter.setPen(QPen(QColor("#e9eef0"), 0));//边框颜色
//    painter.setBrush(QColor("#0d87ca"));//背景色
//    //painter.setOpacity(1);
//    QRectF r(1, 1, width() - 2, height() - 2);//左边 上边 右边 下边
//    painter.drawRoundedRect(r, 6, 6);

#endif
#if 1
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    switch(getButtonState())
    {
    case Normal:
    {
//        p.setOpacity(0.57);
        if(currentThemeMode == "ukui-white")
        {
            p.setBrush(QBrush(QColor(0x13,0x14,0x14,0x19)));
        }

        else
        {
            p.setBrush(QBrush(QColor(0xff,0xff,0xff,0x19)));
        }
        p.setPen(Qt::NoPen);
 //       QPainterPath path;
        opt.rect.adjust(0,0,0,0);
 //       path.addRoundedRect(opt.rect,6,6);
        p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
 //       p.drawRoundedRect(opt.rect,6,6);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 //       p.setPen(QPen(QColor("#e9eef0"), 0));//边框颜色
 //       p.setBrush(QColor("#0d87ca"));//背景色
 //       QRectF r(1, 1, width() - 2, height() - 2);//左边 上边 右边 下边
        p.drawRoundedRect(opt.rect, 6, 6);
        break;
    }
    case Press:
    {
        p.setBrush(QBrush(QColor(0x3D,0x6B,0xE5,0xFF)));   //0x33,0x42,0x33,0x33  0x3D,0x6B,0xE5,0xFF
        p.setPen(Qt::NoPen);
//        p.setPen(QPen(QColor("#CC00FF")));
 //       QPainterPath path;
        opt.rect.adjust(0,0,0,0);
 //       path.addRoundedRect(opt.rect,6,6);
        p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
 //       p.drawRoundedRect(opt.rect,6,6);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 //       p.setPen(QPen(QColor("#e9eef0"), 0));//边框颜色
 //       p.setBrush(QColor("#0d87ca"));//背景色
 //       QRectF r(1, 1, width() - 2, height() - 2);//左边 上边 右边 下边
        p.drawRoundedRect(opt.rect, 6, 6);
        break;
    }
//    case Checked:
//    {
//        p.setBrush(QBrush(QColor(0x3D,0x6B,0xE5,0xFF)));   //0x33,0x42,0x33,0x33  0x3D,0x6B,0xE5,0xFF
////        p.setPen(Qt::NoPen);

// //       QPainterPath path;
//        opt.rect.adjust(0,0,0,0);
// //       path.addRoundedRect(opt.rect,6,6);
//        p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
// //       p.drawRoundedRect(opt.rect,6,6);
//        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
// //       p.setPen(QPen(QColor("#e9eef0"), 0));//边框颜色
// //       p.setBrush(QColor("#0d87ca"));//背景色
// //       QRectF r(1, 1, width() - 2, height() - 2);//左边 上边 右边 下边
//        p.drawRoundedRect(opt.rect, 6, 6);
//        p.setPen(QPen(QColor("#CC00FF")));
//        break;
//    }
    }
#endif
}
