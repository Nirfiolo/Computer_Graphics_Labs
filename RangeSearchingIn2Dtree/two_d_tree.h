#pragma once


#include "vvve.h"


struct TwoDTreeNode
{
    enum class NodeType : uint8_t
    {
        Leaf,
        Vertical,
        Horizontal
    };

    static constexpr size_t Dimention{ 2 };

    frm::Point point;
    NodeType node_type{ NodeType::Leaf };

    std::shared_ptr<TwoDTreeNode> left{};
    std::shared_ptr<TwoDTreeNode> right{};
};

std::shared_ptr<TwoDTreeNode> create_tree(frm::vvve::VVVE const & vvve) noexcept;

size_t get_points_count_into_range(std::shared_ptr<TwoDTreeNode> root, frm::Point left_top_point, frm::Point right_bottom_point) noexcept;