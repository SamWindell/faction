
#define GL_LITE_IMPLEMENTATION
#include "gl_lite.h"

#include <stdio.h>
#include <windows.h>
#include <algorithm>

#include "faction_platform.h"
#include "faction.h"

struct FactionWin32Layer {
    HWND window;
    HDC deviceContext;
    HGLRC renderingContext;
};

LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    // more convenient to handle this separately
    if (message == WM_CREATE) {
        // take the win32Layer struct that we passed in and assign it to the lparam so that
        // we can access it more conveniently
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        SetWindowLongPtr(window, GWL_USERDATA, (LPARAM)(lpcs->lpCreateParams));
        FactionWin32Layer *win32Layer = (FactionWin32Layer *)GetWindowLongPtr(window, GWL_USERDATA);

        // init openGL
        win32Layer->deviceContext = GetDC(window);
        if (win32Layer->deviceContext == NULL) {
            OutputDebugStringA("ERROR: Failed getting the device context\n");
        }

        PIXELFORMATDESCRIPTOR pixelFormatDesc = {};
        pixelFormatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pixelFormatDesc.nVersion = 1;
        pixelFormatDesc.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
        pixelFormatDesc.cColorBits = 32;
        pixelFormatDesc.cDepthBits = 32;
        pixelFormatDesc.dwLayerMask = PFD_MAIN_PLANE;
        int pixelFormat = ChoosePixelFormat(win32Layer->deviceContext, &pixelFormatDesc);
        SetPixelFormat(win32Layer->deviceContext, pixelFormat, &pixelFormatDesc);

        win32Layer->renderingContext = wglCreateContext(win32Layer->deviceContext);
        if (win32Layer->renderingContext == NULL) {
            OutputDebugStringA("ERROR: Failed creating a rendering context\n");                
        }
        wglMakeCurrent(win32Layer->deviceContext, win32Layer->renderingContext);
        gl_lite_init(); // init the function pointers because opengl is not necessarily included with windows
        return 0;
    }

    LRESULT result = 0;
    switch (message) {
        case WM_MOUSEMOVE: {
            return 1;
        }

        default: {
            result = DefWindowProcA(window, message, wParam, lParam); 
            break;
        }
    }

    return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCode) {
    FactionWin32Layer win32Layer = {};
    FactionMain();

    { // create and show window
        WNDCLASSA wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = instance;
        wc.lpszClassName = "FactionWindowClass";
        wc.style = CS_HREDRAW | CS_VREDRAW;
        RegisterClassA(&wc);

        // pass in the win32Layer struct as the last param so that we can use it in other win32 functions
        win32Layer.window = CreateWindowExA(0, wc.lpszClassName, "Faction", WS_OVERLAPPEDWINDOW,
                                            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                                            NULL, NULL, instance, &win32Layer); 
        if (win32Layer.window == NULL) {
            OutputDebugStringA("Window failed to open\n");
            return 0;
        }
        ShowWindow(win32Layer.window, showCode);
    }

    { // message loop
        MSG msg = { };
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            RenderFrame();            
            SwapBuffers(win32Layer.deviceContext);
            Sleep(16); // make a more robust system for timing
        }
    }

    return 0;
}

std::string PlatformGetExeDir() { // don't actaully need this function yet...
    TCHAR filePath[MAX_PATH];
    GetModuleFileName(NULL, filePath, MAX_PATH);
    std::string fullExe(filePath);
    std::string result = fullExe.substr(0, fullExe.find_last_of('\\')+1);
    std::replace(result.begin(), result.end(), '\\', '/');
    return result;
}

void PlatformDMG(const char *msg) {
    OutputDebugStringA(msg);
}