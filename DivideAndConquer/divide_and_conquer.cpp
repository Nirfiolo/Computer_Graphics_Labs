#include "divide_and_conquer.h"

#include <cassert>


enum class Orientation : uint8_t
{
    Counterclockwise,
    Clockwise,
    Colinear
};

Orientation get_orientation(frm::Point p, frm::Point q, frm::Point r) noexcept
{
    float const orientation = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

    if (abs(orientation) < frm::epsilon)
    {
        return Orientation::Colinear;
    }
    if (orientation > frm::epsilon)
    {
        return Orientation::Clockwise;
    }
    return Orientation::Counterclockwise;
}

std::vector<size_t> jarvis_algorithm(frm::vvve::VVVE const & vvve, size_t from_index, size_t to_index) noexcept
{
    std::vector<size_t> hull{};

    size_t const size = to_index - from_index;

    size_t leftmost_index = from_index;
    for (size_t i = from_index + 1; i < to_index; ++i)
    {
        if (vvve.vertices[i].coordinate.x < vvve.vertices[leftmost_index].coordinate.x)
        {
            leftmost_index = i;
        }
    }

    size_t p = leftmost_index;
    do
    {
        hull.push_back(p);

        size_t q = ((p + 1 - from_index) % size) + from_index;
        for (size_t i = from_index; i < to_index; ++i)
        {
            if (get_orientation(
                vvve.vertices[p].coordinate,
                vvve.vertices[i].coordinate,
                vvve.vertices[q].coordinate
            ) == Orientation::Counterclockwise)
            {
                q = i;
            }
        }
        p = q;
    } while (p != leftmost_index);

    return hull;
}

enum class Quad : uint8_t
{
    First,
    Second,
    Third,
    Force
};

Quad quad(frm::Point point) noexcept
{
    if (point.x >= 0 && point.y >= 0)
    {
        return Quad::First;
    }
    if (point.x <= 0 && point.y >= 0)
    {
        return Quad::Second;
    }
    if (point.x <= 0 && point.y <= 0)
    {
        return Quad::Third;
    }
    return Quad::Force;
}

enum class SideByLine : uint8_t
{
    Left,
    Right,
    OnLine
};

SideByLine get_side(frm::Point begin, frm::Point end, frm::Point point) noexcept
{
    float const side = (end.y - begin.y) * (point.x - end.x) - (point.y - end.y) * (end.x - begin.x);

    if (abs(side) < frm::epsilon)
    {
        return SideByLine::OnLine;
    }
    if (side > frm::epsilon)
    {
        return SideByLine::Right;
    }
    return SideByLine::Left;
}

constexpr SideByLine invert_side(SideByLine side) noexcept
{
    return (side == SideByLine::OnLine ? SideByLine::OnLine :
        (side == SideByLine::Left ? SideByLine::Right : SideByLine::Left)
        );
}

std::vector<size_t> merge(frm::vvve::VVVE const & vvve, std::vector<size_t> const & a, std::vector<size_t> const & b) noexcept
{
    size_t const a_size = a.size();
    size_t const b_size = b.size();

    size_t leftmost_b_index = 0;
    size_t rightmost_a_index = 0;

    for (size_t i = 1; i < a_size; ++i)
    {
        if (vvve.vertices[a[i]].coordinate.x > vvve.vertices[a[rightmost_a_index]].coordinate.x)
        {
            rightmost_a_index = i;
        }
    }

    for (size_t i = 1; i < b_size; ++i)
    {
        if (vvve.vertices[b[i]].coordinate.x < vvve.vertices[b[leftmost_b_index]].coordinate.x)
        {
            leftmost_b_index = i;
        }
    }

    size_t index_a = rightmost_a_index;
    size_t index_b = leftmost_b_index;
    bool done = false;
    while (!done)
    {
        done = true;

        while (get_side(
            vvve.vertices[b[index_b]].coordinate,
            vvve.vertices[a[index_a]].coordinate,
            vvve.vertices[a[(index_a + 1) % a_size]].coordinate
        ) != SideByLine::Left)
        {
            index_a = (index_a + 1) % a_size;
        }

        while (get_side(
            vvve.vertices[a[index_a]].coordinate,
            vvve.vertices[b[index_b]].coordinate,
            vvve.vertices[b[(index_b + b_size - 1) % b_size]].coordinate
        ) != SideByLine::Right)
        {
            index_b = (index_b + b_size - 1) % b_size;
            done = false;
        }
    }

    size_t upper_a = index_a;
    size_t upper_b = index_b;
    index_a = rightmost_a_index;
    index_b = leftmost_b_index;

    done = false;
    while (!done)
    {
        done = true;

        while (get_side(
            vvve.vertices[a[index_a]].coordinate,
            vvve.vertices[b[index_b]].coordinate,
            vvve.vertices[b[(index_b + 1) % b_size]].coordinate
        ) != SideByLine::Left)
        {
            index_b = (index_b + 1) % b_size;
        }

        while (get_side(
            vvve.vertices[b[index_b]].coordinate,
            vvve.vertices[a[index_a]].coordinate,
            vvve.vertices[a[(index_a + a_size - 1) % a_size]].coordinate
        ) != SideByLine::Right)
        {
            index_a = (index_a + a_size - 1) % a_size;
            done = false;
        }
    }

    size_t lower_a = index_a;
    size_t lower_b = index_b;

    std::vector<size_t> result;

    size_t index = upper_a;

    result.push_back(a[upper_a]);
    while (index != lower_a)
    {
        index = (index + 1) % a_size;
        result.push_back(a[index]);
    }

    index = lower_b;
    result.push_back(b[index]);
    while (index != upper_b)
    {
        index = (index + 1) % b_size;
        result.push_back(b[index]);
    }

    return result;
}


std::vector<size_t> divide(frm::vvve::VVVE const & vvve, size_t from_index, size_t to_index) noexcept
{
    size_t const size = to_index - from_index;

    if (size < 6)
    {
        return jarvis_algorithm(vvve, from_index, to_index);
    }

    size_t const middle = (to_index + from_index) / 2;

    std::vector<size_t> const hull_first = divide(vvve, from_index, middle);
    std::vector<size_t> const hull_second = divide(vvve, middle, to_index);

    return merge(vvve, hull_first, hull_second);
}


void divide_and_conquer(frm::vvve::VVVE & vvve) noexcept(!IS_DEBUG)
{
    assert(vvve.vertices.size() >= 3);

    std::sort(vvve.vertices.begin(), vvve.vertices.end(), [](frm::vvve::VVVE::Vertex a, frm::vvve::VVVE::Vertex b) noexcept -> bool
        {
            frm::Point point_a = a.coordinate;
            frm::Point point_b = b.coordinate;

            if (abs(point_a.x - point_b.x) < frm::epsilon)
            {
                return point_a.y < point_b.y;
            }

            return point_a.x < point_b.x;
        });

    std::vector<size_t> const hull = divide(vvve, 0, vvve.vertices.size());

    size_t previous_index = hull.size() - 1;
    for (size_t i = 0; i < hull.size(); ++i)
    {
        frm::vvve::add_edge_between_two_vertices(vvve, hull[previous_index], hull[i]);
        previous_index = i;
    }
}