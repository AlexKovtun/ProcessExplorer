#include "ExplorerGui.h"

int main(int, char**)
{
    gui::CreateHWindow("bob", "alice");
    gui::CreateImGui();

    while (!gui::done)
    {
        gui::BeginRender();
        gui::Render();
        gui::EndRender();
    }
    gui::DestroyImGui();

    gui::CleanupDevice();
    gui::DestroyHWindow();
    return 0;
}