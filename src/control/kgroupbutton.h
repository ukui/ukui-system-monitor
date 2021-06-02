#ifndef KGROUPBUTTON_H
#define KGROUPBUTTON_H

#include "../style/usmproxystyle.h"
#include <QPushButton>

class KGroupButton : public QPushButton
{
    Q_OBJECT
public:
    explicit KGroupButton(QWidget *parent = nullptr);
    explicit KGroupButton(const QString &text, QWidget *parent = nullptr);
    KGroupButton(const QIcon& icon, const QString &text, QWidget *parent = nullptr);

    inline GroupButtonStyleOption::ButtonPosition position()
    { return mPosition; }
    inline void setPosition(GroupButtonStyleOption::ButtonPosition pos)
    { mPosition = pos; }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    GroupButtonStyleOption::ButtonPosition mPosition;
};

#endif // KGROUPBUTTON_H
