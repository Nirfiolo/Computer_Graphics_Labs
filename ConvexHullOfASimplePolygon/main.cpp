#include "Application.h"
#include "convex_hull_of_a_simple_polygon.h"
#include "dcel.h"

#include "imgui/imgui.h"


int main()
{
    frm::dcel::DCEL dcel{};

    frm::dcel::load_from_file("Dcel_1.dat", dcel);


    frm::vvve::VVVE vvve{};

    frm::vvve::load_from_file("Vvse_1.dat", vvve);

    frm::Application application{};

    application.set_on_update([&dcel, &vvve](float dt, sf::RenderWindow & window) noexcept
        {
            frm::dcel::draw(dcel, window);
            frm::vvve::draw(vvve, window, sf::Color::Green);

            frm::dcel::spawn_ui(dcel, window, "Dcel_1.dat");
            frm::vvve::spawn_ui(vvve, window, "Vvse_1.dat");

            if (ImGui::Begin("Convex Hull Of A Simple Polygon"))
            {
                if (ImGui::Button("Run"))
                {
                    convex_hull_of_a_simple_polygon(dcel, vvve);
                }
            }
            ImGui::End();
        });

    application.run();
}