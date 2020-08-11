#include "Window.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    Window* window = createWindow(800, 600, "Learning DirectX in C!");
    initializeD3D(window);

    while (window->isOpen)
    {
        updateWindow(window);
        clearWindow(window, 0.f, .2f, .4f, 1.f);
        draw(window);
        swapBuffers(window);
    }

    cleanD3D(window);
    destroyWindow(window);

    return EXIT_SUCCESS;
}