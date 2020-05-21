#include "Application.h"
#include "fortune_algorithm.h"
#include "dcel.h"

#include "imgui/imgui.h"


int main()
{
    frm::dcel::DCEL dcel{};

    frm::vvve::VVVE vvve{};

    frm::vvve::load_from_file("Vvse_1.dat", vvve);

    float height = 700.f;
    float width = 600.f;

    frm::Application application{};

    application.set_on_update([&dcel, &vvve, &height, &width](float dt, sf::RenderWindow & window) noexcept
        {
            frm::dcel::draw(dcel, window);
            frm::vvve::draw(vvve, window, sf::Color::Green);

            frm::dcel::spawn_ui(dcel, window, "Dcel_1.dat");
            frm::vvve::spawn_ui(vvve, window, "Vvse_1.dat");

            // TODO: added animatiom

            if (ImGui::Begin("Voronoi diagram"))
            {
                ImGui::SliderFloat("Width", &width, 1.f, 1000.f, "%.0f", 1.f);
                ImGui::SliderFloat("Height", &height, 1.f, 1000.f, "%.0f", 1.f);

                if (ImGui::Button("Run"))
                {
                    fortune_algorithm(vvve, dcel, height, width);
                }
            }
            ImGui::End();
        });

    application.run();
}