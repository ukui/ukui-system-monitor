/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *  rxy         renxinyu@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "cursorposcalculator.h"

int CursorPosCalculator::borderWidth = 5;

CursorPosCalculator::CursorPosCalculator()
{
    reset();
}

void CursorPosCalculator::reset(){
    onEdges = false;

    onLeftEdges = false;
    onTopEdges = false;
    onRightEdges = false;
    onBottomEdges = false;

    onLeftTopEdges =false;
    onRightTopEdges = false;
    onRightBottomEdges = false;
    onLeftBottomEdges = false;
}

void CursorPosCalculator::recalculate(const QPoint &globalMousePos, const QRect &frameRect){
    int globalMouseX = globalMousePos.x();
    int globalMouseY = globalMousePos.y();

    int frameX = frameRect.x();
    int frameY = frameRect.y();

    int frameWidth = frameRect.width();
    int frameHeight = frameRect.height();

    onLeftEdges = (globalMouseX >= frameX &&
                   globalMouseX <= frameX + borderWidth);

    onRightEdges = (globalMouseX >= frameX + frameWidth - borderWidth &&
                    globalMouseX <= frameX + frameWidth);

    onTopEdges = (globalMouseY >= frameY &&
                  globalMouseY <= frameY + borderWidth);

    onBottomEdges = (globalMouseY >= frameY + frameHeight - borderWidth &&
                     globalMouseY <= frameY + frameHeight);

    onLeftTopEdges = onTopEdges && onLeftEdges;
    onRightTopEdges = onRightEdges && onTopEdges;
    onRightBottomEdges = onRightEdges && onBottomEdges;
    onLeftBottomEdges = onLeftEdges && onBottomEdges;

    onEdges = onLeftEdges || onRightEdges || onTopEdges || onBottomEdges;
}
