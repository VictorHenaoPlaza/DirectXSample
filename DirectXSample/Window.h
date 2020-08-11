#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <d3d11.h>
#include <d3d10.h>
#include <d3dcompiler.h>
#include <math.h>

ID3D11VertexShader* vertexShader;
ID3D11PixelShader*  pixelShader;
ID3D11Buffer*       vertexBuffer;
ID3D11InputLayout*  inputLayout;

enum WindowError
{
    FAILED_TO_CREATE_WINDOW = 1,
    FAILED_TO_REGISTER_CLASS,
    FAILED_TO_CREATE_SWAP_CHAIN
};

typedef struct
{
    float x, y, z;
    float color[4];
} Vertex;

typedef struct
{
    HINSTANCE	            hInstance;
    UINT		            width;
    UINT		            height;
    char*                   title;
    BOOL		            isOpen;
    HWND		            handle;
    WNDCLASSEX	            windowClass;
    MSG			            message;
    ID3D11Device*           device;         // Virtual representation of the video adapter
    ID3D11DeviceContext*    deviceContext;  // Manages the GPU and the rendering pipeline
    IDXGISwapChain*         swapChain;      // Represents the swap chain
    ID3D11RenderTargetView* backBuffer;     // Represents the back buffer
} Window;

/// <summary>
/// Creates a window for DirectX rendering.
/// </summary>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="title"></param>
/// <returns></returns>
Window* createWindow(UINT width, UINT height, char* title);

/// <summary>
/// Initializes Direct3D COM objects.
/// </summary>
/// <param name="window"></param>
void initializeD3D(Window* window);

void initializePipeline(Window* window);

void initializeGraphics(Window* window);

/// <summary>
/// The main window procedure.
/// </summary>
/// <param name="hWnd"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT CALLBACK windowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/// <summary>
/// Processes window messages.
/// </summary>
/// <param name="window"></param>
void updateWindow(Window* window);

/// <summary>
/// Clears the windows background with the desired color.
/// </summary>
/// <param name="window"></param>
/// <param name="red"></param>
/// <param name="green"></param>
/// <param name="blue"></param>
void clearWindow(const Window* window, float red, float green, float blue, float alpha);

void draw(const Window* window);

/// <summary>
/// Swaps the window's buffers.
/// </summary>
/// <param name="window"></param>
void swapBuffers(const Window* window);

/// <summary>
/// Releases Direct3D COM objects.
/// </summary>
/// <param name="window"></param>
void cleanD3D(const Window* window);

/// <summary>
/// Unregisters the window class and destroys the window
/// </summary>
/// <param name="window"></param>
void destroyWindow(Window* window);
