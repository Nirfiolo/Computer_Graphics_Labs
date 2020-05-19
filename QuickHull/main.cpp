#include "Application.h"
#include "quick_hull.h"

#include "imgui/imgui.h"


int main()
{
    frm::vvse::VVSE vvse{};

    frm::vvse::load_from_file("Vvse_1.dat", vvse);

    frm::Application application{};

    application.set_on_update([&vvse](float dt, sf::RenderWindow & window) noexcept
        {
            frm::vvse::draw(vvse, window);

            bool is_dirty_vvse = false;

            is_dirty_vvse |= frm::vvse::spawn_ui(vvse, window, "Vvse_1.dat");

            if (ImGui::Begin("QuickHull"))
            {
                if (ImGui::Button("Run"))
                {
                    quick_hull(vvse);
                }
            }
            ImGui::End();
        });

    application.run();
}