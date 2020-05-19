#include "Application.h"
#include "divide_and_conquer.h"

#include "imgui/imgui.h"


int main()
{
    frm::vvve::VVVE vvve{};

    frm::vvve::load_from_file("Vvse_1.dat", vvve);

    frm::Application application{};

    application.set_on_update([&vvve](float dt, sf::RenderWindow & window) noexcept
        {
            frm::vvve::draw(vvve, window);

            bool is_dirty_vvve = false;

            is_dirty_vvve |= frm::vvve::spawn_ui(vvve, window, "Vvse_1.dat");

            if (ImGui::Begin("Divide and conquer"))
            {
                if (ImGui::Button("Run"))
                {
                    divide_and_conquer(vvve);
                }
            }
            ImGui::End();
        });

    application.run();
}