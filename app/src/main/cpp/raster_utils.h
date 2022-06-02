#ifndef PINENOTE_LIB_RASTER_UTILS_H
#define PINENOTE_LIB_RASTER_UTILS_H

#include "utils.h"

#include <algorithm>
#include <vector>
#include <string>
#include <limits>
#include <stdexcept>

using namespace std;

struct Point {
    unsigned int x;
    unsigned int y;
};

class Shape {
public:
    virtual bool contains(Point point) = 0;
    virtual unsigned int x_min() = 0;
    virtual unsigned int x_max() = 0;
    virtual unsigned int y_min() = 0;
    virtual unsigned int y_max() = 0;
};

class LineSegment : public Shape {
public:
    LineSegment(Point one, Point two);

    static LineSegment *from(Point one, double length, double slope);

    vector<Point> as_points() const;

    bool contains(Point point) override;

    unsigned int x_min() override;

    unsigned int x_max() override;

    unsigned int y_min() override;

    unsigned int y_max() override;

    double slope() const;

    double y_intercept() const;

    double x_intercept() const;

    unsigned int y_at_x(unsigned int x) const;

    unsigned int x_at_y(unsigned int y) const;

    Point topmost_point() const;

    Point bottommost_point() const;

    Point leftmost_point() const;

    Point rightmost_point() const;

    bool horizontal = false;
    bool vertical = false;

    Point start_point{};
    Point end_point{};
private:
    double x_1;
    double y_1;
    double x_2;
    double y_2;

    double diff_x;
    double diff_y;
};

class Circle : public Shape {
public:
    Circle(Point center, unsigned int radius);

    bool contains(Point point) override;

    unsigned int x_min() override;
    unsigned int x_max() override;
    unsigned int y_min() override;
    unsigned int y_max() override;

private:
    double x_center;
    double y_center;

    double radius;
};

class Rectangle : public Shape {
public:
    Rectangle(const LineSegment *line, unsigned int width);

    bool contains(Point point) override;

    unsigned int x_min() override;
    unsigned int x_max() override;
    unsigned int y_min() override;
    unsigned int y_max() override;

private:
    double perpendicular_slope() const;

    const LineSegment *center_line;
    LineSegment top_line{{}, {}};
    LineSegment bottom_line{{}, {}};
    LineSegment left_line{{}, {}};
    LineSegment right_line{{}, {}};
    double width;
};

#endif // PINENOTE_LIB_RASTER_UTILS_H