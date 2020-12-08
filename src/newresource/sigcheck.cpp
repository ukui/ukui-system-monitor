#include "sigcheck.h"

sigCheck::sigCheck(QWidget *parent,int whichSig)
    : QWidget(parent),sig(whichSig)
{
    this->setFixedSize(14,14);
}

sigCheck::~sigCheck()
{
    
}

void sigCheck::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);

    if(sig == REDTYPE)
    {
        p.setBrush(QBrush(QColor(246,79,61)));
    }

    if(sig == PURPLETYPE)
    {
        p.setBrush(QBrush(QColor(204,72,255)));
    }

    if(sig == GREENTYPE)
    {
        p.setBrush(QBrush(QColor(26,195,161)));
    }

    if(sig == BLUETYPE)
    {
        p.setBrush(QBrush(QColor(42,177,232)));
    }

    if(sig == YELLOWTYPE)
    {
        p.setBrush(QBrush(QColor(241,191,48)));
    }

    p.setPen(Qt::NoPen);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);

    path.addRoundedRect(opt.rect,2,2);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.drawRoundedRect(opt.rect,2,2);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
