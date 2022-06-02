//
// Created by Joshua Mulliken on 6/2/22.
//

#import "raster_utils.h"

using namespace std;

/*
 * LineSegment Class
 */

LineSegment::LineSegment(Point one, Point two) {
    this->start_point = one;
    this->end_point = two;

    x_1 = narrow_cast<double, unsigned int>(one.x);
    y_1 = narrow_cast<double, unsigned int>(one.y);
    x_2 = narrow_cast<double, unsigned int>(two.x);
    y_2 = narrow_cast<double, unsigned int>(two.y);

    diff_x = x_2 - x_1;
    diff_y = y_2 - y_1;

    if (diff_x == 0)
        horizontal = true;

    if (diff_y == 0)
        vertical = true;
}

LineSegment *LineSegment::from(Point one, double length, double slope) {
    auto x = narrow_cast<double, unsigned int>(one.x);
    auto y = narrow_cast<double, unsigned int>(one.y);

    double x_2 = x + length * cos(slope);
    double y_2 = y + length * sin(slope);

    Point two{
            narrow_cast<unsigned int, double>(round(x_2)),
            narrow_cast<unsigned int, double>(round(y_2))
    };

    return new LineSegment(one, two);
}

DEPRECATED vector<Point> LineSegment::as_points() const {
    vector<Point> points;

    auto x_min = narrow_cast<unsigned int, double>(min(x_1, x_2));
    auto x_max = narrow_cast<unsigned int, double>(max(x_1, x_2));

    if (horizontal) {
        auto y_uint = narrow_cast<unsigned int, double>(y_1);

        for (unsigned int x = x_min; x <= x_max; x++) {
            points.push_back(Point{x, y_uint});
        }

        return points;
    }

    auto y_min = narrow_cast<unsigned int, double>(min(y_1, y_2));
    auto y_max = narrow_cast<unsigned int, double>(max(y_1, y_2));

    if (vertical) {
        auto x_uint = narrow_cast<unsigned int, double>(x_1);

        for (unsigned int y = y_min; y <= y_max; y++) {
            points.push_back(Point{x_uint, y});
        }

        return points;
    }

    unsigned int y;
    for (unsigned int x = x_min; x <= x_max; x++) {
        y = narrow_cast<unsigned int, double>(y_at_x(x));
        points.push_back(Point{x, y});
    }

    return points;
}

bool LineSegment::contains(Point point) {
    if (horizontal)
        return point.x == x_1 && point.y >= bottommost_point().y && point.y <= topmost_point().y;

    if (vertical)
        return point.y == y_1 && point.x >= leftmost_point().x && point.x <= rightmost_point().x;

    return point.x >= leftmost_point().x && point.x <= rightmost_point().x &&
           point.y >= bottommost_point().y && point.y <= topmost_point().y;
}

unsigned int LineSegment::x_min() {
    return narrow_cast<unsigned int, double>(round(min(x_1, x_2)));
}

unsigned int LineSegment::x_max() {
    return narrow_cast<unsigned int, double>(round(max(x_1, x_2)));
}

unsigned int LineSegment::y_min() {
    return narrow_cast<unsigned int, double>(round(min(y_1, y_2)));
}

unsigned int LineSegment::y_max() {
    return narrow_cast<unsigned int, double>(round(max(y_1, y_2)));
}

double LineSegment::slope() const {
    if (horizontal)
        return 0;

    if (vertical)
        return numeric_limits<double>::infinity();

    return diff_y / diff_x;
}

double LineSegment::y_intercept() const {
    if (horizontal)
        return y_1;

    if (vertical)
        throw runtime_error("Cannot calculate y intercept for vertical line");

    return y_1 - slope() * x_1;
}

double LineSegment::x_intercept() const {
    if (horizontal)
        throw runtime_error("Cannot calculate x intercept for horizontal line");

    if (vertical)
        return x_1;

    return -y_intercept() / slope();
}

unsigned int LineSegment::y_at_x(unsigned int x) const {
    if (horizontal) {
        if (x != x_1)
            throw runtime_error("LineSegment contains no point at x" + to_string(x));
    }

    if (vertical)
        return narrow_cast<unsigned int, double>(y_1);

    return narrow_cast<unsigned int, double>(round(slope() * x + y_intercept()));
}

unsigned int LineSegment::x_at_y(unsigned int y) const {
    if (vertical) {
        if (y != y_1)
            throw runtime_error("LineSegment contains no point at y" + to_string(y));
    }

    if (horizontal)
        return narrow_cast<unsigned int, double>(x_1);

    return narrow_cast<unsigned int, double>(round((y - y_intercept()) / slope()));
}

Point LineSegment::topmost_point() const {
    if (start_point.y > end_point.y)
        return start_point;

    return end_point;
}

Point LineSegment::bottommost_point() const {
    if (start_point.y < end_point.y)
        return start_point;

    return end_point;
}

Point LineSegment::leftmost_point() const {
    if (start_point.x < end_point.x)
        return start_point;

    return end_point;
}

Point LineSegment::rightmost_point() const {
    if (start_point.x > end_point.x)
        return start_point;

    return end_point;
}

/*
 * Circle Class
 */

Circle::Circle(Point center, unsigned int radius) {
    x_center = narrow_cast<double, unsigned int>(center.x);
    y_center = narrow_cast<double, unsigned int>(center.y);

    this->radius = narrow_cast<double, unsigned int>(radius);
}

bool Circle::contains(Point point) {
    auto x = narrow_cast<double, unsigned int>(point.x);
    auto y = narrow_cast<double, unsigned int>(point.y);

    // Pythagorean Theorem
    double diff_x = x - x_center;
    double diff_y = y - y_center;

    double a_squared = pow(diff_x, 2);
    double b_squared = pow(diff_y, 2);
    double c_squared = a_squared + b_squared;

    double dist = sqrt(c_squared);

    return dist <= radius;
}

unsigned int Circle::x_min() {
    return narrow_cast<unsigned int, double>(round(x_center - radius));
}

unsigned int Circle::x_max() {
    return narrow_cast<unsigned int, double>(round(x_center + radius));
}

unsigned int Circle::y_min() {
    return narrow_cast<unsigned int, double>(round(y_center - radius));
}

unsigned int Circle::y_max() {
    return narrow_cast<unsigned int, double>(round(y_center + radius));
}

/*
 * Rectangle Class
 */

Rectangle::Rectangle(const LineSegment *line, unsigned int width) {
    center_line = line;
    this->width = narrow_cast<double, unsigned int>(width);

    LineSegment *left_line_one = LineSegment::from(line->leftmost_point(), this->width / 2,
                                                   perpendicular_slope());
    LineSegment *left_line_two = LineSegment::from(line->leftmost_point(), -this->width / 2,
                                                   perpendicular_slope());

    left_line = LineSegment(left_line_one->end_point, left_line_two->end_point);

    LineSegment *right_line_one = LineSegment::from(line->rightmost_point(), this->width / 2,
                                                    perpendicular_slope());
    LineSegment *right_line_two = LineSegment::from(line->rightmost_point(), -this->width / 2,
                                                    perpendicular_slope());

    right_line = LineSegment(right_line_one->end_point, right_line_two->end_point);

    top_line = LineSegment(left_line.topmost_point(), right_line.topmost_point());
    bottom_line = LineSegment(left_line.bottommost_point(), right_line.bottommost_point());
}

bool Rectangle::contains(Point point) {
    return point.x >= left_line.x_at_y(point.y) && point.x <= right_line.x_at_y(point.y) &&
           point.y >= bottom_line.y_at_x(point.x) && point.y <= top_line.y_at_x(point.x);
}

unsigned int Rectangle::x_min() {
    return left_line.x_min();
}

unsigned int Rectangle::x_max() {
    return right_line.x_max();
}

unsigned int Rectangle::y_min() {
    return bottom_line.y_min();
}

unsigned int Rectangle::y_max() {
    return top_line.y_max();
}

double Rectangle::perpendicular_slope() const {
    return -1 / center_line->slope();
}

