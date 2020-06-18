#ifndef MYUNDERLINEBUTTON_H
#define MYUNDERLINEBUTTON_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QStyleOption>
#include <QGSettings/QGSettings>
#include <QApplication>

#include "shell/customstyle.h"
#include "../shell/macro.h"

class QVBoxLayout;

class MyUnderLineButton : public QWidget
{
    Q_OBJECT

    enum ButtonState {Normal, Hover, Press, Checked};

public:
    MyUnderLineButton(QWidget * parent=0);
    ~MyUnderLineButton();

    void setChecked(bool flag);
    bool isChecked();
    void setName(const QString &name);
    ButtonState getButtonState() const;
    void initThemeMode();

signals:
    void clicked();

protected:
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) override;

private:
    void updateStyleSheet();
    void setState(ButtonState state);

private:
    ButtonState m_state;
    ButtonState m_button_state;
    bool m_isChecked;
    QLabel *m_textLabel = nullptr;
    //QLabel *m_underlineLabel = nullptr;
    QVBoxLayout *m_layout = nullptr;

    QGSettings * qtSettings;
    QString currentThemeMode;

};

#endif // MYUNDERLINEBUTTON_H
