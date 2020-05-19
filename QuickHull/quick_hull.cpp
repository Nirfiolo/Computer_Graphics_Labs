#include "quick_hull.h"

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

void quick_hull(frm::vvse::VVSE & vvse, size_t begin_index, size_t end_index, SideByLine side) noexcept
{
    size_t index = std::numeric_limits<size_t>::max();

    float max_distance = 0.f;

    frm::Point const begin = vvse.vertices[begin_index].coordinate;
    frm::Point const end = vvse.vertices[end_index].coordinate;

    for (size_t i = 0; i < vvse.vertices.size(); ++i)
    {
        frm::Point const current_point = vvse.vertices[i].coordinate;
        float const current_distance = distance_to_line(begin, end, current_point);

        if (get_side(begin, end, current_point) == side && current_distance > max_distance)
        {
            index = i;
            max_distance = current_distance;
        }
    }

    if (index == std::numeric_limits<size_t>::max())
    {
        vvse.edges.insert({ begin_index, end_index });
        return;
    }

    frm::Point const point_by_index = vvse.vertices[index].coordinate;
    quick_hull(vvse, index, begin_index, invert_side(get_side(point_by_index, begin, end)));
    quick_hull(vvse, index, end_index, invert_side(get_side(point_by_index, end, begin)));
}

void quick_hull(frm::vvse::VVSE & vvse) noexcept(!IS_DEBUG)
{
    assert(vvse.vertices.size() >= 3);

    size_t left_index = 0;
    size_t right_index = 0;


    for (size_t i = 0; i < vvse.vertices.size(); ++i)
    {
        if (vvse.vertices[i].coordinate.x < vvse.vertices[left_index].coordinate.x)
        {
            left_index = i;
        }
        if (vvse.vertices[i].coordinate.x > vvse.vertices[right_index].coordinate.x)
        {
            right_index = i;
        }
    }

    quick_hull(vvse, left_index, right_index, SideByLine::Right);
    quick_hull(vvse, left_index, right_index, SideByLine::Left);
}