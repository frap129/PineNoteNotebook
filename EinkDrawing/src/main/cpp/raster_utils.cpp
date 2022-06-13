//
// Created by Joshua Mulliken on 6/2/22.
//

#import "raster_utils.h"

using namespace std;

/*
 * LineSegment Class
 */

LineSegment::LineSegment(Point one, Point two) {
    start_point = one;
    end_point = two;

    x_1 = narrow_cast<double, unsigned int>(one.x);
    y_1 = narrow_cast<double, unsigned int>(one.y);
    x_2 = narrow_cast<double, unsigned int>(two.x);
    y_2 = narrow_cast<double, unsigned int>(two.y);

    diff_x = x_2 - x_1;
    diff_y = y_2 - y_1;

    if (diff_x == 0)
        vertical = true;

    if (diff_y == 0)
        horizontal = true;
}

bool LineSegment::contains(Point point) {
    if (horizontal)
        return point.x == x_1 && point.y >= bottommost_point().y && point.y <= topmost_point().y;

    if (vertical)
        return point.y == y_1 && point.x >= leftmost_point().x && point.x <= rightmost_point().x;

    return point.x >= leftmost_point().x && point.x <= rightmost_point().x &&
           point.y >= bottommost_point().y && point.y <= topmost_point().y;
}

vector<Point> LineSegment::as_points() {
    vector<Point> points;

    if (horizontal) {
        auto y_uint = narrow_cast<unsigned int, double>(y_1);
        for (unsigned int x = x_min(); x <= x_max(); x++) {
            points.push_back({x, y_uint});
        }
    } else if (vertical) {
        auto x_uint = narrow_cast<unsigned int, double>(x_1);
        for (unsigned int y = y_min(); y <= y_max(); y++) {
            points.push_back({x_uint, y});
        }
    } else {
        double y = 0;
        unsigned int y_uint = 0;
        for (unsigned int ix = x_min(); ix <= x_max(); ix++) {
            y = y_at_x(ix);
            y_uint = narrow_cast<unsigned int, double>(round(y));
            points.push_back({ix, y_uint});
        }

        double x = 0;
        unsigned int x_uint = 0;
        for (unsigned int iy = y_min(); iy <= y_max(); iy++) {
            x = x_at_y(iy);
            x_uint = narrow_cast<unsigned int, double>(round(x));
            points.push_back({x_uint, iy});
        }
    }

    return points;
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
    double x_min = round(x_center - radius);
    if (x_min < 0)
        return 0;

    return narrow_cast<unsigned int, double>(x_min);
}

unsigned int Circle::x_max() {
    double x_max = round(x_center + radius);
    if (x_max > numeric_limits<unsigned int>::max())
        return numeric_limits<unsigned int>::max();

    return narrow_cast<unsigned int, double>(x_max);
}

unsigned int Circle::y_min() {
    double y_min = round(y_center - radius);
    if (y_min < 0)
        return 0;

    return narrow_cast<unsigned int, double>(y_min);
}

unsigned int Circle::y_max() {
    double y_max = round(y_center + radius);
    if (y_max > numeric_limits<unsigned int>::max())
        return numeric_limits<unsigned int>::max();

    return narrow_cast<unsigned int, double>(y_max);
}
