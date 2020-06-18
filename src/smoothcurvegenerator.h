/*
 * Copyright (C) 2013 ~ 2018 National University of Defense Technology(NUDT) & Tianjin Kylin Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
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

#ifndef SMOOTHCURVEGENERATOR_H
#define SMOOTHCURVEGENERATOR_H

#include <QList>
#include <QPainterPath>
#include <QPointF>

class SmoothCurveGenerator {
public:
    /**
     * 传入曲线上的点的 list，创建平滑曲线
     *
     * @param points - 曲线上的点
     * @return - 返回使用给定的点创建的 QPainterPath 表示的平滑曲线
     * 生成的平滑曲线，会发现平滑曲线段之间会有很多小段的抖动
     */
    static QPainterPath generateSmoothCurve1(const QList<QPointF> &points);

    /**
     * 传入曲线上的点的 list，创建平滑曲线
     * @param points - 曲线上的点
     * @return - 返回使用给定的点创建的 QPainterPath 表示的平滑曲线
     * 可以解决平滑曲线段之间的抖动问题，但是曲线段在比较陡时，曲线上的值可能会超出曲线原来值的范围
     */
    static QPainterPath generateSmoothCurve(const QList<QPointF> &points);

private:
    /**
     * Solves a tridiagonal system for one of coordinates (x or y)
     * of first Bezier control points.
     * @param result - Solution vector.
     * @param rhs - Right hand side vector.
     * @param n - Size of rhs.
     */
    static void calculateFirstControlPoints(double * &result, const double *rhs, int n);
    /**
     * Calculate control points of the smooth curve using the given knots.
     * @param knots - Points of the given curve.
     * @param firstControlPoints - Store the generated first control points.
     * @param secondControlPoints - Store the generated second control points.
     */
    static void calculateControlPoints(const QList<QPointF> &knots,
                                       QList<QPointF> *firstControlPoints,
                                       QList<QPointF> *secondControlPoints);
};
#endif // SMOOTHCURVEGENERATOR_H
