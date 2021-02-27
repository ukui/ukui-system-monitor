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

#include "ktableview.h"

#include <QApplication>
#include <QPalette>
#include <QStyle>

#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QHeaderView>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QScroller>
#include <QScrollerProperties>
#include <QScrollBar>
#include <QFocusEvent>

// default constructor
KTableView::KTableView(QWidget *parent)
    : QTreeView(parent)
{
    // enable touch event handling
    setAttribute(Qt::WA_AcceptTouchEvents);

    // set delegate instance
    m_itemDelegate = new KItemDelegate(this);
    setItemDelegate(m_itemDelegate);

    // set header view instance
    m_headerView = new KHeaderView(Qt::Horizontal, this);
    setHeader(m_headerView);
    // section movable
    m_headerView->setSectionsMovable(true);
    // section clickable
    m_headerView->setSectionsClickable(true);
    // section resizable
    m_headerView->setSectionResizeMode(QHeaderView::Interactive);
    // can stretch last section?
    m_headerView->setStretchLastSection(true);
    // show sort indicator?
    m_headerView->setSortIndicatorShown(true);
    // section default text alignment
    m_headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // header context menu policy
    m_headerView->setContextMenuPolicy(Qt::CustomContextMenu);
    // header view focus policy
    m_headerView->setFocusPolicy(Qt::StrongFocus);

    // not allowing expanding/collpasing top-level items
    setRootIsDecorated(false);
    // items are not expandable
    setItemsExpandable(false);
    setFrameStyle(QFrame::NoFrame);
    viewport()->setAutoFillBackground(true);

    setAlternatingRowColors(true);

    setAllColumnsShowFocus(true);
    setFocusPolicy(Qt::StrongFocus);

    // adjust focus order (header -> treeview)
    setTabOrder(m_headerView, this);

    // treeview touch scroll support
    auto *scroller = QScroller::scroller(viewport());
    auto prop = scroller->scrollerProperties();

    prop.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    prop.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    prop.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0);
    prop.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 1);

    prop.setScrollMetric(QScrollerProperties::AxisLockThreshold, 1);
    scroller->setScrollerProperties(prop);
    // enable touch gesture
    QScroller::grabGesture(viewport(), QScroller::TouchGesture);
}

// set view model
void KTableView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);

    // listen on modelReset signal, reset any hovered or pressed index
    if (model) {
        connect(model, &QAbstractItemModel::modelReset, this, [ = ]() {
            m_hover = {};
            m_pressed = {};
        });
    }
}

// paint event handler
void KTableView::paintEvent(QPaintEvent *event)
{
    QTreeView::paintEvent(event);
}

// draw custom styled row
void KTableView::drawRow(QPainter *painter, const QStyleOptionViewItem &options,
                            const QModelIndex &index) const
{
    QTreeView::drawRow(painter, options, index);
}

void KTableView::focusInEvent(QFocusEvent *event)
{
    QTreeView::focusInEvent(event);
    m_focusReason =  event->reason();
}

// current selected item changed handler
void KTableView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);

    // update previous item's paint region
    if (previous.isValid()) {
        QRect previousRect = visualRect(previous);
        previousRect.setX(0);
        previousRect.setWidth(viewport()->width());
        viewport()->update(previousRect);
    }
    // update current item's paint region
    if (current.isValid()) {
        QRect currentRect = visualRect(current);
        currentRect.setX(0);
        currentRect.setWidth(viewport()->width());
        viewport()->update(currentRect);
    }
}

// viewport event handler
bool KTableView::viewportEvent(QEvent *event)
{
    // delegate other events to base event handler
    return QTreeView::viewportEvent(event);
}

// scroll viewport to ensure specified indexed item be visible
void KTableView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    // check index validity
    if (!(index.isValid() && index.row() >= 0 && index.column() >= 0 && index.model() == model())) {
        return;
    }

    // reset last hovered item in case user scroll viewport with up/down key press
    m_hover = {};

    auto area = viewport()->rect();
    // calculate current indexed item's rect
    QRect rect(columnViewportPosition(index.column()),
               indexRowSizeHint(index) * index.row() - verticalScrollBar()->value(),
               columnWidth(index.column()),
               indexRowSizeHint(index));

    if (rect.isEmpty()) {
        // nothing to do
    } else if (hint == EnsureVisible && area.contains(rect)) {
        viewport()->update(QRect {0,
                                  indexRowSizeHint(index) * index.row() - verticalScrollBar()->value(),
                                  viewport()->width(),
                                  indexRowSizeHint(index)});
        // nothing to do
    } else {
        // current item above viewport rect
        bool above = (hint == EnsureVisible
                      && (rect.top() < area.top()
                          || area.height() < rect.height()));
        // current item below viewport rect
        bool below = (hint == EnsureVisible
                      && rect.bottom() > area.bottom()
                      && rect.height() < area.height());

        int verticalValue = verticalScrollBar()->value();
        if (hint == PositionAtTop || above)
            verticalValue += rect.top();
        else if (hint == PositionAtBottom || below)
            verticalValue += rect.bottom() - area.height();
        else if (hint == PositionAtCenter)
            verticalValue += rect.top() - ((area.height() - rect.height()) / 2);
        // adjust scroll bar to a new position to ensure item visible
        verticalScrollBar()->setValue(verticalValue);
    }
}
