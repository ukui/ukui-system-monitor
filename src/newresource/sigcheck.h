#ifndef _SIGCHECK_H_
#define _SIGCHECK_H_

#include <QWidget>
#include <QStyleOption>
#include <QPainter>
#include "../shell/macro.h"

class sigCheck : public QWidget{
public:
    sigCheck(QWidget * parent=0,int whichSig = 0);
    ~sigCheck();
protected:
    void paintEvent(QPaintEvent *event) override;
private :
    int  sig;
};

#endif //_SIGCHECK_H_
