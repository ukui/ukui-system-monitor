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

#include "mysearchedit.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QPushButton>
#include <QEvent>
#include <QFocusEvent>
#include <QResizeEvent>

MySearchEdit::MySearchEdit(QWidget *parent)
    : QFrame(parent)
    ,m_showCurve(QEasingCurve::OutCubic)    
    ,m_hideCurve(QEasingCurve::InCubic)
    ,fontSettings(nullptr)
    ,qtSettings(nullptr)
{
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

    this->setWindowFlags(Qt::FramelessWindowHint);

    m_searchBtn = new QLabel;
    m_searchBtn->setStyleSheet("QLabel{background-color:transparent;border:none;background-image:url(:/img/search.png);}");
    m_searchBtn->setFixedSize(SEARCHBUTTON, SEARCHBUTTON);

    m_pClearTextButton = new QPushButton;
    m_pClearTextButton->setFixedSize(19, 21);
    m_pClearTextButton->setIconSize(QSize(19, 19));
    m_pClearTextButton->setObjectName("ClearTextButton");

    m_edit = new QLineEdit;

    QIcon ClearTextEditIcon;
    m_pClearTextButton->setCursor(Qt::ArrowCursor);

    m_edit->setAttribute(Qt::WA_Hover, true);
    m_edit->setTextMargins(8,0,0,0);

    connect(m_edit, &QLineEdit::textChanged, this, &MySearchEdit::textChageSlots);

    initFontSize();

    QFont SearchLine;
    SearchLine = m_edit->font();
    SearchLine.setPixelSize(fontSize);
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

    m_placeHolder = new QLabel;  //about the font
    QFont font;
    font.setPointSize(fontSize-2);
    m_placeHolder->setFont(font);

    initThemeMode();

    m_animation = new QPropertyAnimation(m_edit, "minimumWidth");

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
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    setFocusPolicy(Qt::StrongFocus);

    connect(m_edit, &QLineEdit::textChanged, this, &MySearchEdit::textChanged, Qt::DirectConnection);
}


QPixmap MySearchEdit::drawSymbolicColoredPixmap(const QPixmap &source)
{
    if(currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
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
    } else if(currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black") {
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
    } else {
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
}

MySearchEdit::~MySearchEdit()
{
    m_animation->deleteLater();
    delete m_edit;
    delete m_searchBtn;
    delete m_placeHolder;
    if(fontSettings)
    {
        delete fontSettings;
    }
    if(qtSettings)
    {
        delete qtSettings;
    }
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

void MySearchEdit::initThemeMode()
{
    if (!qtSettings) {
//        qWarning() << "Failed to load the gsettings: " << THEME_QT_SCHEMA;
        return;
    }

    //监听主题改变
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key){

        if (key == "styleName") {
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            m_pClearTextButton->setIcon(drawSymbolicColoredPixmap(QPixmap::fromImage(QIcon::fromTheme(":/img/button-close-default-add-background-three.svg").pixmap(24,24).toImage())));

            if(currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
            {
                m_edit->setStyleSheet("QLineEdit{background:transparent;border-radius:4px;color:#000000;padding-right:12px;padding-bottom: 2px;}"); //#CC00FF transparent
                m_placeHolder->setStyleSheet("QLabel{background-color:transparent;color:rgba(0,0,0,0.57);margin: 2 0 0 0 px;}");
                this->setObjectName("SearchBtn");
                this->setStyleSheet("QFrame#SearchBtn{background:rgba(13,14,14,0.08);border-radius:4px;}");
            }

            if(currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black")
            {
                m_edit->setStyleSheet("QLineEdit{background:transparent;border-radius:4px;color:#FFFFFF;padding-right:12px;padding-bottom: 2px;}"); //#CC00FF transparent
                m_placeHolder->setStyleSheet("QLabel{background-color:transparent;color:rgba(255,255,255,0.57);margin: 2 0 0 0 px;}");
                this->setObjectName("SearchBtn");
                this->setStyleSheet("QFrame#SearchBtn{background:rgba(255,255,255,0.08);border-radius:4px;}");
            }
        }

    });
    //获取当前主题
    currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    m_pClearTextButton->setIcon(drawSymbolicColoredPixmap(QPixmap::fromImage(QIcon::fromTheme(":/img/button-close-default-add-background-three.svg").pixmap(24,24).toImage())));
    if(currentThemeMode == "ukui-light" || currentThemeMode == "ukui-default" || currentThemeMode == "ukui-white")
    {
        m_edit->setStyleSheet("QLineEdit{background:transparent;border-radius:4px;color:#000000;padding-right:12px;padding-bottom: 2px;}"); //#CC00FF transparent
        m_placeHolder->setStyleSheet("QLabel{background-color:transparent;color:rgba(0,0,0,0.57);margin: 2 0 0 0 px;}");
        this->setObjectName("SearchBtn");
        this->setStyleSheet("QFrame#SearchBtn{background:rgba(13,14,14,0.08);border-radius:4px;}");
    }

    if(currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black")
    {
        m_edit->setStyleSheet("QLineEdit{background:transparent;border-radius:4px;color:#FFFFFF;padding-right:12px;padding-bottom: 2px;}"); //#CC00FF transparent
        m_placeHolder->setStyleSheet("QLabel{background-color:transparent;color:rgba(255,255,255,0.57);margin: 2 0 0 0 px;}");
        this->setObjectName("SearchBtn");
        this->setStyleSheet("QFrame#SearchBtn{background:rgba(255,255,255,0.08);border-radius:4px;}");
    }
}

void MySearchEdit::initFontSize()
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
        font.setPointSize(fontSize - 2);
        m_placeHolder->setFont(font);
        QFont searchLineFont;
        searchLineFont.setPointSize(fontSize);
        m_edit->setFont(searchLineFont);
    });
    fontSize = fontSettings->get(FONT_SIZE).toString().toFloat();
}
