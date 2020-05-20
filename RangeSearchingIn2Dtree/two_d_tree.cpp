#include "two_d_tree.h"

#include <cassert>


std::shared_ptr<TwoDTreeNode> insert(std::shared_ptr<TwoDTreeNode> root, frm::Point point, size_t depth) noexcept
{
    if (!root)
    {
        std::shared_ptr<TwoDTreeNode> new_node = std::make_shared<TwoDTreeNode>();
        new_node->point = point;
        return new_node;
    }

    size_t const current_dimention = depth % TwoDTreeNode::Dimention;

    root->node_type = (current_dimention == 0 ? TwoDTreeNode::NodeType::Vertical : TwoDTreeNode::NodeType::Horizontal);

    if ((current_dimention == 0 && point.x < root->point.x) ||
        (current_dimention == 1 && point.y < root->point.y))
    {
        root->left = insert(root->left, point, depth + 1);
    }
    else
    {
        root->right = insert(root->right, point, depth + 1);
    }

    return root;
}


std::shared_ptr<TwoDTreeNode> insert(std::shared_ptr<TwoDTreeNode> root, frm::Point point) noexcept
{
    return insert(root, point, 0);
}


std::shared_ptr<TwoDTreeNode> create_tree(frm::vvve::VVVE const & vvve) noexcept
{
    std::shared_ptr<TwoDTreeNode> root{};

    for (size_t i = 0; i < vvve.vertices.size(); ++i)
    {
        root = insert(root, vvve.vertices[i].coordinate);
    }

    return root;
}

bool is_point_into_reactangle(frm::Point left_top_point, frm::Point right_bottom_point, frm::Point point) noexcept
{
    return left_top_point.x <= point.x && left_top_point.y <= point.y && point.x <= right_bottom_point.x && point.y <= right_bottom_point.y;
}

size_t get_points_count_into_range(std::shared_ptr<TwoDTreeNode> root, frm::Point left_top_point, frm::Point right_bottom_point) noexcept
{
    if (!root)
    {
        return 0;
    }

    size_t result = 0;

    float left;
    float right;
    float current_value;

    if (root->node_type == TwoDTreeNode::NodeType::Vertical)
    {
        left = left_top_point.x;
        right = right_bottom_point.x;
        current_value = root->point.x;
    }
    else
    {
        left = left_top_point.y;
        right = right_bottom_point.y;
        current_value = root->point.y;
    }

    if (left <= current_value && current_value <= right)
    {
        if (is_point_into_reactangle(left_top_point, right_bottom_point, root->point))
        {
            ++result;
        }
    }

    if (root->node_type != TwoDTreeNode::NodeType::Leaf)
    {
        if (left < current_value)
        {
            result += get_points_count_into_range(root->left, left_top_point, right_bottom_point);
        }
        if (current_value < right)
        {
            result += get_points_count_into_range(root->right, left_top_point, right_bottom_point);
        }
    }

    return result;
}
