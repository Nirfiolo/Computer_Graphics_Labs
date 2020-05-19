#include "convex_hull_of_a_simple_polygon.h"

#include <cassert>


enum class SideByLine : uint8_t
{
    Left,
    Right,
    OnLine
};

SideByLine get_side(frm::Point begin, frm::Point end, frm::Point point) noexcept
{
    float const side = (point.y - begin.y) * (end.x - begin.x) - (end.y - begin.y) * (point.x - begin.x);

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
    return (side == SideByLine::OnLine ? SideByLine::OnLine : (
        side == SideByLine::Left ? SideByLine::Right : SideByLine::Left
        ));
}

float distance_to_line(frm::Point begin, frm::Point end, frm::Point point) noexcept
{
    return abs((point.y - begin.y) * (end.x - begin.x) - (end.y - begin.y) * (point.x - begin.x));
}

void quick_hull(frm::vvve::VVVE & vvve, size_t begin_index, size_t end_index, SideByLine side) noexcept
{
    size_t index = std::numeric_limits<size_t>::max();

    float max_distance = 0.f;

    frm::Point const begin = vvve.vertices[begin_index].coordinate;
    frm::Point const end = vvve.vertices[end_index].coordinate;

    for (size_t i = 0; i < vvve.vertices.size(); ++i)
    {
        frm::Point const current_point = vvve.vertices[i].coordinate;
        float const current_distance = distance_to_line(begin, end, current_point);

        if (get_side(begin, end, current_point) == side && current_distance > max_distance)
        {
            index = i;
            max_distance = current_distance;
        }
    }

    if (index == std::numeric_limits<size_t>::max())
    {
        vvve.edges.push_back({ begin_index, end_index });
        return;
    }

    frm::Point const point_by_index = vvve.vertices[index].coordinate;
    quick_hull(vvve, index, begin_index, invert_side(get_side(point_by_index, begin, end)));
    quick_hull(vvve, index, end_index, invert_side(get_side(point_by_index, end, begin)));
}


std::vector<size_t> convex_hull_of_a_simple_polygon(frm::vvve::VVVE const & vvve) noexcept
{
    size_t const size = vvve.vertices.size();

    std::vector<size_t> deque(2 * size + 1);

    size_t bottom = size - 2;
    size_t top = bottom + 3;

    deque[bottom] = 2;
    deque[top] = 2;

    if (get_side(
        vvve.vertices[0].coordinate,
        vvve.vertices[1].coordinate,
        vvve.vertices[2].coordinate
    ) == SideByLine::Right)
    {
        deque[bottom + 1] = 0;
        deque[bottom + 2] = 1;
    }
    else
    {
        deque[bottom + 1] = 1;
        deque[bottom + 2] = 0;
    }

    for (size_t i = 3; i < size; ++i)
    {
        if (get_side(
            vvve.vertices[deque[bottom]].coordinate,
            vvve.vertices[deque[bottom + 1]].coordinate,
            vvve.vertices[i].coordinate
        ) == SideByLine::Right &&
            get_side(
                vvve.vertices[deque[top - 1]].coordinate,
                vvve.vertices[deque[top]].coordinate,
                vvve.vertices[i].coordinate
            ) == SideByLine::Right)
        {
            continue;
        }

        while (get_side(
            vvve.vertices[deque[bottom]].coordinate,
            vvve.vertices[deque[bottom + 1]].coordinate,
            vvve.vertices[i].coordinate
        ) != SideByLine::Right)
        {
            ++bottom;
        }

        deque[--bottom] = i;

        while (get_side(
            vvve.vertices[deque[top - 1]].coordinate,
            vvve.vertices[deque[top]].coordinate,
            vvve.vertices[i].coordinate
        ) != SideByLine::Right)
        {
            --top;
        }

        deque[++top] = i;
    }

    std::vector<size_t> hull(top - bottom + 1);
    for (size_t i = 0; i <= top - bottom; ++i)
    {
        hull[i] = deque[bottom + i];
    }

    return hull;
}


void convex_hull_of_a_simple_polygon(frm::dcel::DCEL const & dcel, frm::vvve::VVVE & vvve) noexcept(!IS_DEBUG)
{
    assert(dcel.vertices.size() >= 3);

    vvve.vertices.clear();
    vvve.edges.clear();

    size_t const outside_face = frm::dcel::get_outside_face_index(dcel);

    size_t const begin = dcel.faces[outside_face].edge;
    size_t current_index = begin;

    do
    {
        vvve.vertices.push_back({ dcel.vertices[dcel.edges[current_index].origin_vertex].coordinate });

        current_index = dcel.edges[current_index].next_edge;

    } while (current_index != begin);

    std::vector<size_t> const hull = convex_hull_of_a_simple_polygon(vvve);

    size_t previous_index = hull.size() - 1;
    for (size_t i = 0; i < hull.size(); ++i)
    {
        frm::vvve::add_edge_between_two_vertices(vvve, hull[previous_index], hull[i]);
        previous_index = i;
    }
}