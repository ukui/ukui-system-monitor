#include "kgroupbutton.h"

#include <QStylePainter>
#include <QDebug>
#include <QStyle>

KGroupButton::KGroupButton(QWidget *parent)
    : QPushButton(parent)
{
    mPosition = GroupButtonStyleOption::OnlyOne;
}

KGroupButton::KGroupButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    mPosition = GroupButtonStyleOption::OnlyOne;
}

KGroupButton::KGroupButton(const QIcon& icon, const QString &text, QWidget *parent)
    : QPushButton(icon, text, parent)
{
    mPosition = GroupButtonStyleOption::OnlyOne;
}

void KGroupButton::paintEvent(QPaintEvent *paintEvent)
{
    QStylePainter p(this);
    GroupButtonStyleOption option;
    option.position = mPosition;
    initStyleOption(&option);
    p.drawControl(QStyle::CE_PushButton, option);
}
