// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef UTIL_POINT_H
#define UTIL_POINT_H

#include "util/util.h"

#include <QtCore>
#include <cmath>
#include <iostream>

#define POINT_ABS_ZERO 1e-12
#define POINT_REL_ZERO 1e-7

struct Point;

// return center
AGROS_LIBRARY_API Point centerPoint(const Point &pointStart, const Point &pointEnd, double angle);

// intersection of two lines
AGROS_LIBRARY_API bool intersectionLines(const Point &p1s, const Point &p1e, const Point &p2s, const Point &p2e, Point &out);

// intersection of two lines or line and arc
AGROS_LIBRARY_API QList<Point> intersection(const Point &p1s, const Point &p1e, const Point &center1, double radius1, double angle1,
  const Point &p2s, const Point &p2e, const Point &center2, double radius2, double angle2);

// find a point on an arc which lies on a line between the given point and center
AGROS_LIBRARY_API Point prolong_point_to_arc(const Point &candidate, const Point &center, double radius);


struct AGROS_LIBRARY_API  Point
{
    double x, y;

    Point() : x(0), y(0) {}
    Point(double x, double y) : x(x), y(y) {}

    inline Point operator+(const Point &vec) const { return Point(x + vec.x, y + vec.y); }
    inline Point operator-(const Point &vec) const { return Point(x - vec.x, y - vec.y); }
    inline Point operator*(double num) const { return Point(x * num, y * num); }
    inline Point operator/(double num) const { return Point(x / num, y / num); }
    inline double operator&(const Point &vec) const { return x*vec.x + y*vec.y; } // dot product
    inline double operator%(const Point &vec) const { return x*vec.y - y*vec.x; } // cross product
    bool operator!=(const Point &vec) const;
    bool operator==(const Point &vec) const;

    inline double magnitude() const { return sqrt(x * x + y * y); }
    inline double magnitudeSquared() const { return (x * x + y * y); }
    inline double angle() const { return atan2(y, x); }

    Point normalizePoint() const
    {
        double m = magnitude();

        double mx = x/m;
        double my = y/m;

        return Point(mx, my);
    }

    QString toString() const
    {
        return QString("x = %1, y = %2, magnitude = %3").
                arg(x).
                arg(y).
                arg(magnitude());
    }
};

AGROS_LIBRARY_API QDebug& operator<<(QDebug &output, const Point& pt);

struct Point3
{
    double x, y, z;

    Point3() : x(0), y(0), z(0) {}
    Point3(double x, double y, double z) : x(x), y(y), z(z) {}

    inline Point3 operator+(const Point3 &vec) const { return Point3(x + vec.x, y + vec.y, z + vec.z); }
    inline Point3 operator-(const Point3 &vec) const { return Point3(x - vec.x, y - vec.y, z - vec.z); }
    inline Point3 operator*(double num) const { return Point3(x * num, y * num, z * num); }
    inline Point3 operator/(double num) const { return Point3(x / num, y / num, z / num); }
    inline double operator&(const Point3 &vec) const { return x*vec.x + y*vec.y + z*vec.z; } // dot product
    inline Point3 operator%(const Point3 &vec) const { return Point3(- z*vec.y, z*vec.x, x*vec.y - y*vec.x); } // cross product

    inline double magnitude() const { return sqrt(x * x + y * y + z * z); }
    inline double anglexy() const { return atan2(y, x); }
    inline double angleyz() const { return atan2(z, y); }
    inline double anglezx() const { return atan2(x, z); }

    Point3 normalizePoint() const
    {
        double m = magnitude();

        double mx = x/m;
        double my = y/m;
        double mz = z/m;

        return Point3(mx, my, mz);
    }

    QString toString()
    {
        return QString("x = %1, y = %2, z = %3, magnitude = %4").
                arg(x).
                arg(y).
                arg(z).
                arg(magnitude());
    }
};

struct RectPoint
{
    Point start;
    Point end;

    inline RectPoint(const Point &start, const Point &end) : start(start), end(end) {}
    inline RectPoint() : start(Point()), end(Point()) {}

    inline void set(const Point &start, const Point &end) { this->start = start; this->end = end; }
    inline double width() const { return fabs(end.x - start.x); }
    inline double height() const { return fabs(end.y - start.y); }
};

#endif // UTIL_POINT_H

