#include "Window.h"

Window* createWindow(UINT width, UINT height, char* title)
{
    Window* window = malloc(sizeof(Window));

    if (window == NULL)
    {
        MessageBox(NULL, "Failed to create window!", "Error", MB_OK);
        exit(FAILED_TO_CREATE_WINDOW);
    }

    window->hInstance = GetModuleHandle(NULL);
    window->width     = width;
    window->height    = height;
    window->title     = title;

    ZeroMemory(&window->windowClass, sizeof(WNDCLASSEX));
    char* className = "DirectX Window";

    window->windowClass.cbSize        = sizeof(WNDCLASSEX);
    window->windowClass.style         = 0;
    window->windowClass.lpfnWndProc   = windowProcedure;
    window->windowClass.hInstance     = window->hInstance;
    window->windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    window->windowClass.lpszClassName = className;

    if (!RegisterClassEx(&window->windowClass))
    {
        MessageBox(NULL, "Failed to register window class!", "Error", MB_OK);
        exit(FAILED_TO_REGISTER_CLASS);
    }
    
    RECT windowRectangle = { 0, 0, window->width, window->height };
    AdjustWindowRect(&windowRectangle, WS_OVERLAPPED, FALSE);

    GetWindowRect(window->handle, &windowRectangle);

    UINT x = (GetSystemMetrics(SM_CXSCREEN) - windowRectangle.right) / 2;
    UINT y = (GetSystemMetrics(SM_CYSCREEN) - windowRectangle.bottom) / 2;

    window->handle = CreateWindowEx(
        0,
        window->windowClass.lpszClassName,
        window->title,
        WS_OVERLAPPEDWINDOW,
        x, y, // Window position
        window->width, // Window width
        window->height, // Window height
        NULL,
        NULL,
        window->hInstance,
        NULL
    );

    ShowWindow(window->handle, SW_SHOW);
    
    window->isOpen = TRUE;

    return window;
}

void initializeD3D(Window* window)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc;

    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    swapChainDesc.BufferCount       = 1;                                        // 1 back buffer
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;               // 32-bit color
    swapChainDesc.BufferDesc.Width  = window->width;
    swapChainDesc.BufferDesc.Height = window->height;
    swapChainDesc.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;          // Renders to the window
    swapChainDesc.OutputWindow      = window->handle;                           // Uses the current window
    swapChainDesc.SampleDesc.Count  = 4;                                        // Multisample count
    swapChainDesc.Windowed          = TRUE;                                     // Not fullscreen
    swapChainDesc.Flags             = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;   // Allows fullscreen switching

    D3D11CreateDeviceAndSwapChain(
        NULL, // Default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        0, // Flags D3D11_CREATE_DEVICE_DEBUG
        NULL,
        0,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &window->swapChain,
        &window->device,
        NULL,
        &window->deviceContext
    );

    ID3D11Resource* pBackBuffer; // ID3D11Texture2D

    if (window->swapChain == NULL)
    {
        MessageBox(NULL, "Couldn't create swap chain!", "Error", MB_OK);
        exit(FAILED_TO_CREATE_SWAP_CHAIN);
    }

    IDXGISwapChain_GetBuffer(window->swapChain, 0, &IID_ID3D11Texture2D, (LPVOID*)& pBackBuffer); // Finds the back buffer in the swap chain to create the texture object
    ID3D11Device_CreateRenderTargetView(window->device, pBackBuffer, NULL, &window->backBuffer);
    ID3D11Texture2D_Release(pBackBuffer);

    ID3D11DeviceContext_OMSetRenderTargets(window->deviceContext, 1, &window->backBuffer, NULL);

    D3D11_VIEWPORT viewport;

    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = (float)window->width;
    viewport.Height   = (float)window->height;

    ID3D11DeviceContext_RSSetViewports(window->deviceContext, 1, &viewport);

    initializePipeline(window);
    initializeGraphics(window);
}

void initializePipeline(Window* window)
{
    ID3D10Blob* compiledVertexShader;
    ID3D10Blob* compiledPixelShader;

    if (FAILED(D3DCompileFromFile(L"Shaders.fx", NULL, NULL, "VShader", "vs_5_0", 0, 0, &compiledVertexShader, NULL)))
        MessageBox(NULL, "Failed to compile vertex shader!", "Error", MB_OK);

    if (FAILED(D3DCompileFromFile(L"Shaders.fx", NULL, NULL, "PShader", "ps_5_0", 0, 0, &compiledPixelShader, NULL)))
        MessageBox(NULL, "Failed to compile pixel shader!", "Error", MB_OK);

    ID3D11Device_CreateVertexShader(window->device, ID3D10Blob_GetBufferPointer(compiledVertexShader), ID3D10Blob_GetBufferSize(compiledVertexShader), NULL, &vertexShader);
    ID3D11Device_CreatePixelShader(window->device, ID3D10Blob_GetBufferPointer(compiledPixelShader), ID3D10Blob_GetBufferSize(compiledPixelShader), NULL, &pixelShader);

    ID3D11DeviceContext_VSSetShader(window->deviceContext, vertexShader, 0, 0);
    ID3D11DeviceContext_PSSetShader(window->deviceContext, pixelShader, 0, 0);

    D3D11_INPUT_ELEMENT_DESC inputElementDesc[2] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ID3D11Device_CreateInputLayout(window->device, inputElementDesc, 2, ID3D10Blob_GetBufferPointer(compiledVertexShader), ID3D10Blob_GetBufferSize(compiledVertexShader), &inputLayout);
    ID3D11DeviceContext_IASetInputLayout(window->deviceContext, inputLayout);
}

void initializeGraphics(Window* window)
{
    Vertex vertices[] =
    {
        {  0.0f,  0.5f, 0.0f, { 1.0f, 0.0f, 0.0f, 1.0f } },
        {  0.5f, -0.5f, 0.0f, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { -0.5f, -0.5f, 0.0f, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

    D3D11_BUFFER_DESC bufferDesc;

    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

    bufferDesc.Usage          = D3D11_USAGE_DYNAMIC;      // Write access by the CPU and GPU
    bufferDesc.ByteWidth      = sizeof(Vertex) * 3;       // 3 vertices for a triangle
    bufferDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER; // This buffer is used as a vertex buffer
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;   // Allows the CPU to write in the vertex buffer

    ID3D11Device_CreateBuffer(window->device, &bufferDesc, NULL, &vertexBuffer);

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;

    ID3D11DeviceContext_Map(window->deviceContext, vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
    memcpy(mappedSubresource.pData, vertices, sizeof(vertices));
    ID3D11DeviceContext_Unmap(window->deviceContext, vertexBuffer, 0);
}

LRESULT CALLBACK windowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        exit(0);
    }
    break;
    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void updateWindow(Window* window)
{
    if (PeekMessage(&window->message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&window->message);
        DispatchMessage(&window->message);

        if (window->message.message == WM_QUIT)
            window->isOpen = FALSE;
    }
}

void clearWindow(const Window* window, float red, float green, float blue, float alpha)
{
    float color[4] = { red, green, blue, alpha };
    ID3D11DeviceContext_ClearRenderTargetView(window->deviceContext, window->backBuffer, color);
}

void draw(const Window* window)
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    ID3D11DeviceContext_IASetVertexBuffers(window->deviceContext, 0, 1, &vertexBuffer, &stride, &offset);
    ID3D11DeviceContext_IASetPrimitiveTopology(window->deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11DeviceContext_Draw(window->deviceContext, 3, 0);
}

void swapBuffers(const Window* window)
{
    IDXGISwapChain_Present(window->swapChain, 0, 0);
}

void cleanD3D(const Window* window)
{
    IDXGISwapChain_SetFullscreenState(window->swapChain, FALSE, NULL);

    ID3D11VertexShader_Release(vertexShader);
    ID3D11PixelShader_Release(pixelShader);

    ID3D11Buffer_Release(vertexBuffer);
    ID3D11InputLayout_Release(inputLayout);

    ID3D11Device_Release(window->device);
    ID3D11DeviceContext_Release(window->deviceContext);
    IDXGISwapChain_Release(window->swapChain);
    ID3D11RenderTargetView_Release(window->backBuffer);
}

void destroyWindow(Window* window)
{
    UnregisterClass(window->windowClass.lpszClassName, window->hInstance);
    DestroyWindow(window->handle);

    free(window);
}