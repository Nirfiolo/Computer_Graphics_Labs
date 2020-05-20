#include "Application.h"
#include "two_d_tree.h"

#include "imgui\imgui.h"


int main()
{
    frm::vvve::VVVE vvve{};

    frm::vvve::load_from_file("Vvse_1.dat", vvve);

    std::shared_ptr<TwoDTreeNode> root = create_tree(vvve);

    frm::Application application{};

    frm::Point left_top_point{ -10000.f, -10000.f };
    frm::Point right_bottom_point{ 10000.f, 10000.f };

    application.set_on_event([&left_top_point, &right_bottom_point](sf::Event current_event) noexcept
        {
            ImGuiIO const & imgui_io = ImGui::GetIO();

            if (!imgui_io.WantCaptureMouse && current_event.type == sf::Event::MouseButtonPressed)
            {
                int x = current_event.mouseButton.x;
                int y = current_event.mouseButton.y;

                frm::Point const point = { static_cast<float>(x), static_cast<float>(y) };

                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
                {
                    left_top_point = point;
                }
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
                {
                    right_bottom_point = point;
                }
            }
        });

    application.set_on_update([&vvve, &root, &left_top_point, &right_bottom_point](float dt, sf::RenderWindow & window) noexcept
        {
            frm::vvve::draw(vvve, window);

            bool is_dirty_vvve = false;

            is_dirty_vvve |= frm::vvve::spawn_ui(vvve, window, "Vvse_1.dat");

            if (is_dirty_vvve)
            {
                root = create_tree(vvve);
            }

            sf::Vector2f size = { right_bottom_point.x - left_top_point.x, right_bottom_point.y - left_top_point.y };
            sf::RectangleShape rectangle{ size };
            if (size.x < frm::epsilon || size.y < frm::epsilon)
            {
                rectangle.setFillColor(sf::Color{ 200, 0, 0, 50 });
            }
            else
            {
                rectangle.setFillColor(sf::Color{ 0, 0, 200, 50 });
            }
            rectangle.setPosition(left_top_point.x, left_top_point.y);
            window.draw(rectangle);

            if (ImGui::Begin("Points count into reactangle"))
            {
                ImGui::Text("Count: %d", static_cast<int>(get_points_count_into_range(root, left_top_point, right_bottom_point)));
            }
            ImGui::End();
        });

    application.run();
}