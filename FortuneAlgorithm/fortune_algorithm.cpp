#include "fortune_algorithm.h"

#include <cassert>
#include <queue>
#include <set>


struct VorEvent;


struct VorPoint
{
    // TODO: swap
    float x{ 0.f };
    float y{ 0.f };
    size_t index_in_vvve{ std::numeric_limits<size_t>::max() };

    VorPoint() noexcept
    {

    }

    VorPoint(float x, float y) noexcept : x{ x }, y{ y }
    {

    }

    VorPoint(float x, float y, size_t index_in_vvve) noexcept
        : x{ x },
        y{ y },
        index_in_vvve{ index_in_vvve }
    {

    }
};

struct VorEdge
{
    VorPoint start;
    VorPoint end;
    // Voronoi place on the left side of edge
    VorPoint left;
    // Voronoi place on the right side of edge
    VorPoint right;
    VorPoint direction;

    size_t neighbour_index = std::numeric_limits<size_t>::max();

    float k;
    float c;

    VorEdge(VorPoint start, VorPoint left, VorPoint right) noexcept
        : start{ start },
        left{ left },
        right{ right }
    {
        if (abs(left.y - right.y) < frm::epsilon)
        {
            k = 10000000.f;
        }
        else
        {
            k = (right.x - left.x) / (left.y - right.y);
        }

        c = start.y - k * start.x;

        direction = { right.y - left.y, -(right.x - left.x) };
    }
};

struct VorParabola
{
    VorPoint site;

    size_t edge_index;

    std::shared_ptr<VorEvent> event{};

    std::shared_ptr<VorParabola> parent{};
    std::shared_ptr<VorParabola> left{};
    std::shared_ptr<VorParabola> right{};

    bool is_leaf{ true };

    VorParabola() noexcept : is_leaf{ false }
    {

    }

    VorParabola(VorPoint site) noexcept : site{ site }
    {

    }
};

struct VorEvent
{
    struct CompareEvent
    {
        bool operator() (VorEvent const & left, VorEvent const & right) const noexcept
        {
            if (abs(left.point.y - right.point.y) < frm::epsilon)
            {
                return left.point.x < right.point.x;
            }
            return left.point.y < right.point.y;
        }

        bool operator() (std::shared_ptr<VorEvent> const & left, std::shared_ptr<VorEvent> const & right) const noexcept
        {
            return CompareEvent{}(*left, *right);
        }
    };

    VorPoint point;

    std::shared_ptr<VorParabola> arch{};

    bool is_site_event{ true };

    VorEvent(VorPoint point) noexcept : point{ point }
    {

    }
};

std::shared_ptr<VorParabola> get_left_parent(std::shared_ptr<VorParabola> parabola) noexcept
{
    std::shared_ptr<VorParabola> parent = parabola->parent;
    std::shared_ptr<VorParabola> last_parabola = parabola;
    while (parent->left == last_parabola)
    {
        if (!parent->parent)
        {
            return {};
        }
        last_parabola = parent;
        parent = parent->parent;
    }
    return parent;
}

std::shared_ptr<VorParabola> get_right_parent(std::shared_ptr<VorParabola> parabola) noexcept
{
    std::shared_ptr<VorParabola> parent = parabola->parent;
    std::shared_ptr<VorParabola> last_parabola = parabola;
    while (parent->right == last_parabola)
    {
        if (!parent->parent)
        {
            return {};
        }
        last_parabola = parent;
        parent = parent->parent;
    }
    return parent;
}

std::shared_ptr<VorParabola> get_left_child(std::shared_ptr<VorParabola> parabola) noexcept
{
    if (!parabola)
    {
        return {};
    }
    std::shared_ptr<VorParabola> current = parabola->left;
    while (!current->is_leaf)
    {
        current = current->right;
    }
    return current;
}

std::shared_ptr<VorParabola> get_right_child(std::shared_ptr<VorParabola> parabola) noexcept
{
    if (!parabola)
    {
        return {};
    }
    std::shared_ptr<VorParabola> current = parabola->right;
    while (!current->is_leaf)
    {
        current = current->left;
    }
    return current;
}


float get_y_by_focus(float last_y, VorPoint focus, float x) noexcept
{
    float dp = 2.f * (focus.y - last_y);
    float a1 = 1.f / dp;
    float b1 = -2.f * focus.x / dp;
    float c1 = last_y + dp / 4.f + focus.x * focus.x / dp;

    return a1 * x * x + b1 * x + c1;
}

float get_x_of_edge(std::shared_ptr<VorParabola> parabola, float y) noexcept
{
    std::shared_ptr<VorParabola> const left = get_left_child(parabola);
    std::shared_ptr<VorParabola> const right = get_right_child(parabola);

    VorPoint const left_point = left->site;
    VorPoint const right_point = right->site;

    float dp = 2.f * (left_point.y - y);
    float const a1 = 1.f / dp;
    float const b1 = -2.f * left_point.x / dp;
    float const c1 = y + dp / 4.f + left_point.x * left_point.x / dp;

    dp = 2.f * (right_point.y - y);
    float const a2 = 1.f / dp;
    float const b2 = -2.f * right_point.x / dp;
    float const c2 = y + dp / 4.f + right_point.x * right_point.x / dp;

    float const a = a1 - a2;
    float const b = b1 - b2;
    float const c = c1 - c2;

    float const disc = b * b - 4.f * a * c;
    float const x1 = (-b + std::sqrt(disc)) / (2.f * a);
    float const x2 = (-b - std::sqrt(disc)) / (2.f * a);

    float result_x;
    if (left_point.y < right_point.y)
    {
        result_x = std::max(x1, x2);
    }
    else
    {
        result_x = std::min(x1, x2);
    }

    return result_x;
}

std::shared_ptr<VorParabola> get_parabola_by_x(std::shared_ptr<VorParabola> root, float last_y, float x) noexcept
{
    std::shared_ptr<VorParabola> current_parabola = root;
    float current_x = 0.f;

    while (!current_parabola->is_leaf)
    {
        current_x = get_x_of_edge(current_parabola, last_y);
        if (current_x > x)
        {
            current_parabola = current_parabola->left;
        }
        else
        {
            current_parabola = current_parabola->right;
        }
    }
    return current_parabola;
}



// returned value:
// first intersection point
// second is_intersect
std::pair<VorPoint, bool> get_edge_intersection(std::vector<VorEdge> & edges, size_t a_index, size_t b_index) noexcept
{
    VorEdge const a = edges[a_index];
    VorEdge const b = edges[b_index];

    float const x = (b.c - a.c) / (a.k - b.k);
    float const y = a.k * x + a.c;

    if ((x - a.start.x) / a.direction.x < frm::epsilon)
    {
        return { {}, false };
    }
    if ((y - a.start.y) / a.direction.y < frm::epsilon)
    {
        return { {}, false };
    }

    if ((x - b.start.x) / b.direction.x < frm::epsilon)
    {
        return { {}, false };
    }
    if ((y - b.start.y) / b.direction.y < frm::epsilon)
    {
        return { {}, false };
    }
    return { { x, y }, true };
}


void handle_circle_event(
    std::priority_queue<std::shared_ptr<VorEvent>, std::vector<std::shared_ptr<VorEvent>>, VorEvent::CompareEvent> & queue,
    std::vector<VorEdge> & edges,
    float last_y,
    std::shared_ptr<VorParabola> parabola
) noexcept
{
    std::shared_ptr<VorParabola> left_parent = get_left_parent(parabola);
    std::shared_ptr<VorParabola> right_parent = get_right_parent(parabola);

    std::shared_ptr<VorParabola> a = get_left_child(left_parent);
    std::shared_ptr<VorParabola> c = get_left_child(right_parent);

    if (!a || !c || a->site.index_in_vvve == c->site.index_in_vvve)
    {
        return;
    }

    std::pair<VorPoint, bool> const edge_intersection = get_edge_intersection(edges, left_parent->edge_index, right_parent->edge_index);
    if (!edge_intersection.second)
    {
        return;
    }

    VorPoint const s = edge_intersection.first;

    float const dx = a->site.x - s.x;
    float const dy = a->site.y - s.y;

    float const d = std::sqrt((dx * dx) + (dy * dy));

    if (s.y - d >= last_y)
    {
        return;
    }

    std::shared_ptr<VorEvent> new_event = std::make_shared<VorEvent>(VorPoint{ s.x, s.y - d });
    new_event->is_site_event = false;
    new_event->arch = parabola;
    parabola->event = new_event;

    queue.push(new_event);
}


void insert_parabola(
    std::priority_queue<std::shared_ptr<VorEvent>, std::vector<std::shared_ptr<VorEvent>>, VorEvent::CompareEvent> & queue,
    std::shared_ptr<VorParabola> & root,
    std::vector<VorEdge> & edges,
    std::set<std::shared_ptr<VorEvent>, VorEvent::CompareEvent> & deleted,
    float last_y,
    float height,
    VorPoint point
) noexcept
{
    if (!root)
    {
        root = std::make_shared<VorParabola>(point);
        return;
    }

    if (root->is_leaf && abs(root->site.y - point.y) < frm::epsilon)
    {
        VorPoint const root_site = root->site;
        root->is_leaf = false;
        root->left = std::make_shared<VorParabola>(root_site);
        root->left->parent = root;
        root->right = std::make_shared<VorParabola>(point);
        root->right->parent = root;

        VorPoint start = { (point.x + root_site.x) / 2.f, height };

        root->edge_index = edges.size();
        if (point.x > root_site.x)
        {
            edges.push_back({ start, root_site, point });
        }
        else
        {
            edges.push_back({ start, point, root_site });
        }

        return;
    }

    std::shared_ptr<VorParabola> parabola = get_parabola_by_x(root, last_y, point.x);

    if (parabola->event)
    {
        deleted.insert(parabola->event);
        parabola->event = {};
    }

    VorPoint start{ point.x, get_y_by_focus(last_y, parabola->site, point.x) };

    size_t const left_edge_index = edges.size();
    edges.push_back({ start, parabola->site, point });
    size_t const right_edge_index = edges.size();
    edges.push_back({ start, point, parabola->site });

    VorEdge & left_edge = edges[left_edge_index];
    left_edge.neighbour_index = right_edge_index;
    VorEdge & right_edge = edges[right_edge_index];
    right_edge.neighbour_index = left_edge_index;

    parabola->edge_index = right_edge_index;
    parabola->is_leaf = false;

    std::shared_ptr<VorParabola> parabola_0 = std::make_shared<VorParabola>(parabola->site);
    std::shared_ptr<VorParabola> parabola_1 = std::make_shared<VorParabola>(point);
    std::shared_ptr<VorParabola> parabola_2 = std::make_shared<VorParabola>(parabola->site);

    parabola->right = parabola_2;
    parabola->right->parent = parabola;

    parabola->left = std::make_shared<VorParabola>();
    parabola->left->parent = parabola;

    parabola->left->edge_index = left_edge_index;

    parabola->left->left = parabola_0;
    parabola->left->left->parent = parabola->left;

    parabola->left->right = parabola_1;
    parabola->left->right->parent = parabola->left;

    handle_circle_event(queue, edges, last_y, parabola_0);
    handle_circle_event(queue, edges, last_y, parabola_2);
}


void remove_parabola(
    std::priority_queue<std::shared_ptr<VorEvent>, std::vector<std::shared_ptr<VorEvent>>, VorEvent::CompareEvent> & queue,
    std::shared_ptr<VorParabola> & root,
    std::vector<VorEdge> & edges,
    std::set<std::shared_ptr<VorEvent>, VorEvent::CompareEvent> & deleted,
    float last_y,
    std::shared_ptr<VorEvent> current_event
) noexcept
{
    std::shared_ptr<VorParabola> parabola_1 = current_event->arch;

    std::shared_ptr<VorParabola> left_parent = get_left_parent(parabola_1);
    std::shared_ptr<VorParabola> right_parent = get_right_parent(parabola_1);

    std::shared_ptr<VorParabola> parabola_0 = get_left_child(left_parent);
    std::shared_ptr<VorParabola> parabola_2 = get_right_child(right_parent);

    assert(parabola_0 != parabola_2 && "The right and left parabolas have the same focus!");

    if (parabola_0->event)
    {
        deleted.insert(parabola_0->event);
        parabola_0->event = {};
    }
    if (parabola_2->event)
    {
        deleted.insert(parabola_2->event);
        parabola_2->event = {};
    }

    VorPoint p{ current_event->point.x, get_y_by_focus(last_y, parabola_1->site, current_event->point.x) };

    edges[left_parent->edge_index].end = p;
    edges[left_parent->edge_index].end = p;

    std::shared_ptr<VorParabola> higher;
    std::shared_ptr<VorParabola> parabola = parabola_1;

    while (parabola != root)
    {
        parabola = parabola->parent;

        if (parabola == left_parent)
        {
            higher = left_parent;
        }
        if (parabola == right_parent)
        {
            higher = right_parent;
        }
    }

    assert(higher);

    higher->edge_index = edges.size();
    edges.push_back({ p, parabola->site, parabola_2->site });

    std::shared_ptr<VorParabola> grandparent = parabola_1->parent->parent;
    if (parabola_1->parent->left == parabola_1)
    {
        if (grandparent->left == parabola_1->parent)
        {
            grandparent->left = parabola_1->parent->right;
            grandparent->left->parent = grandparent;
        }
        if (grandparent->right == parabola_1->parent)
        {
            grandparent->right = parabola_1->parent->right;
            grandparent->right->parent = grandparent;
        }
    }
    else
    {
        if (grandparent->left == parabola_1->parent)
        {
            grandparent->left = parabola_1->parent->left;
            grandparent->left->parent = grandparent;
        }
        if (grandparent->right == parabola_1->parent)
        {
            grandparent->right = parabola_1->parent->left;
            grandparent->right->parent = grandparent;
        }
    }

    parabola_1->parent->parent = {};
    parabola_1->parent->left = {};
    parabola_1->parent->right = {};

    parabola_1->parent = {};
    parabola_1->left = {};
    parabola_1->right = {};

    handle_circle_event(queue, edges, last_y, parabola_0);
    handle_circle_event(queue, edges, last_y, parabola_2);
}


void finish_edge(
    std::vector<VorEdge> & edges,
    float width,
    std::shared_ptr<VorParabola> parabola
) noexcept
{
    if (parabola->is_leaf)
    {
        parabola->parent = {};
        parabola->left = {};
        parabola->right = {};
        return;
    }

    float mx;

    VorEdge & edge = edges[parabola->edge_index];

    if (edge.direction.x > frm::epsilon)
    {
        mx = std::max(width, edge.start.x + 10.f);
    }
    else
    {
        mx = std::min(0.f, edge.start.x - 10.f);
    }

    VorPoint const end{ mx, mx * edge.k + edge.c };
    edge.end = end;

    finish_edge(edges, width, parabola->left);
    finish_edge(edges, width, parabola->right);

    parabola->parent = {};
    parabola->left = {};
    parabola->right = {};
}


void create_outside_wall(frm::vvve::VVVE const & vvve, frm::dcel::DCEL & dcel, float height, float width) noexcept
{
    frm::dcel::add_vertex(dcel, { 0.f, 0.f });
    frm::dcel::add_vertex(dcel, { width, 0.f });
    frm::dcel::add_vertex(dcel, { 0.f, height });

    dcel.faces.push_back({});

    frm::dcel::add_face_from_three_points(dcel, 0, 1, 2, 0);
    dcel.faces[0].edge = dcel.edges[dcel.faces[1].edge].twin_edge;

    frm::dcel::add_vertex_and_split_edge(dcel, { width, height }, 2);
}

void fortune_algorithm(frm::vvve::VVVE & vvve, frm::dcel::DCEL & dcel, float height, float width) noexcept(!IS_DEBUG)
{
    assert(vvve.vertices.size() >= 2);

    dcel.vertices.clear();
    dcel.faces.clear();
    dcel.edges.clear();
    dcel.free_vertices.clear();
    dcel.free_faces.clear();
    dcel.free_edges.clear();

    //create_outside_wall(vvve, dcel, height, width);

    std::priority_queue<std::shared_ptr<VorEvent>, std::vector<std::shared_ptr<VorEvent>>, VorEvent::CompareEvent> queue{};
    std::set<std::shared_ptr<VorEvent>, VorEvent::CompareEvent> deleted{};
    std::shared_ptr<VorParabola> root{};
    std::vector<VorEdge> edges{};
    float last_y;

    for (size_t i = 0; i < vvve.vertices.size(); ++i)
    {
        queue.push(std::make_shared<VorEvent>(VorPoint{ vvve.vertices[i].coordinate.x, vvve.vertices[i].coordinate.y, i }));
    }

    while (!queue.empty())
    {
        std::shared_ptr<VorEvent> current_event = queue.top();
        queue.pop();

        last_y = current_event->point.y;

        if (deleted.find(current_event) != deleted.end())
        {
            deleted.erase(current_event);
            continue;
        }

        if (current_event->is_site_event)
        {
            insert_parabola(queue, root, edges, deleted, last_y, height, current_event->point);
        }
        else
        {
            remove_parabola(queue, root, edges, deleted, last_y, current_event);
        }
    }

    finish_edge(edges, width, root);




    for (size_t i = 0; i < edges.size(); ++i)
    {
        size_t const vertex_begin_index = dcel.vertices.size();
        dcel.vertices.push_back({ edges[i].start.x, edges[i].start.y });
        size_t const vertex_end_index = dcel.vertices.size();
        dcel.vertices.push_back({ edges[i].end.x, edges[i].end.y });

        size_t const forward_edge_index = dcel.edges.size();
        dcel.edges.push_back({});
        size_t const backward_edge_index = dcel.edges.size();
        dcel.edges.push_back({});

        frm::dcel::DCEL::Edge & forward_edge = dcel.edges[forward_edge_index];
        frm::dcel::DCEL::Edge & backward_edge = dcel.edges[backward_edge_index];

        forward_edge.origin_vertex = vertex_begin_index;
        forward_edge.twin_edge = backward_edge_index;

        backward_edge.origin_vertex = vertex_end_index;
        backward_edge.twin_edge = forward_edge_index;
    }
}