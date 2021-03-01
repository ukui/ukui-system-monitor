/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
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

#ifndef __PROCPROPERTIESDLG_H__
#define __PROCPROPERTIESDLG_H__

#include <QLabel>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QPoint>
#include <QTimer>

class QMouseEvent;

//TODO: add timer to refresh

class ProcPropertiesDlg : public QDialog
{
    Q_OBJECT

public:
    ProcPropertiesDlg(pid_t pid = -1,QWidget *parent = nullptr);
    ~ProcPropertiesDlg();

    pid_t getPid();
    QRect getParentGeometry() const;
    void moveToCenter();
    void updateLabelFrameHeight();
    void initProcproperties();

public slots:
    void refreshProcproperties();

protected:
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QPushButton *closeButton = nullptr;
    QLabel *m_logoLabel = nullptr;

    QVBoxLayout *m_layout = nullptr;
    QHBoxLayout *m_topLayout = nullptr;
    QHBoxLayout *m_topLeftLayout = nullptr;
    QHBoxLayout *m_topRightLayout = nullptr;
    QHBoxLayout *m_bottomLayout = nullptr;
    QLabel *m_iconLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QPushButton *m_okBtn = nullptr;

    pid_t pid;
    QPoint dragPosition;
    bool mousePressed;
    QTimer *timer = nullptr;

    QFrame *m_infoFrame = nullptr;
    QList<QLabel *> m_labelList;
};

#endif // PROPERTIESSDIALOG_H
