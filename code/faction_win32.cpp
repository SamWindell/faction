
#define GL_LITE_IMPLEMENTATION
#include "gl_lite.h"

#include <stdio.h>
#include <windows.h>
#include <Windowsx.h>
#include <algorithm>
#include <assert.h>

#include "faction_platform.h"
#include "faction.h"

struct FactionWin32Layer {
    HWND window;
    HDC deviceContext;
    HGLRC renderingContext;
    UserInput userInput;
};

LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    // more convenient to handle this separately
    if (message == WM_CREATE) {
        // take the win32Layer struct that we passed in and assign it to the lparam so that
        // we can access it more conveniently
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        SetWindowLongPtr(window, GWL_USERDATA, (LPARAM)(lpcs->lpCreateParams));
        FactionWin32Layer *win32Layer = (FactionWin32Layer *)GetWindowLongPtr(window, GWL_USERDATA);

        { // init openGL
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
            gl_lite_init(); // init the function pointers because opengl is not neces6sarily included with windows
        }
        return 0;
    }

    FactionWin32Layer *win32Layer = (FactionWin32Layer *)GetWindowLongPtr(window, GWL_USERDATA);
    if (win32Layer == NULL) {
        return DefWindowProcA(window, message, wParam, lParam);
    }
    win32Layer->userInput.scroll = 0;

    LRESULT result = 0;
    switch (message) {
        case WM_MOUSEMOVE: {
            win32Layer->userInput.cursorPos.x = (float)((lParam << 16) >> 16);
            win32Layer->userInput.cursorPos.y = (float)(lParam >> 16);
            return 1;
        }
        case WM_MOUSEWHEEL: {
			auto zDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            win32Layer->userInput.scroll = (int)zDelta;
            return 1;
        }
        case WM_LBUTTONDOWN: { win32Layer->userInput.mouseState[1] = 1; return 1; }
        case WM_LBUTTONUP:   { win32Layer->userInput.mouseState[1] = 0; return 1; }
        case WM_RBUTTONDOWN: { win32Layer->userInput.mouseState[1] = 1; return 1; }
        case WM_RBUTTONUP:   { win32Layer->userInput.mouseState[1] = 0; return 1; }
        case WM_MBUTTONDOWN: { win32Layer->userInput.mouseState[2] = 1; return 1; }
        case WM_MBUTTONUP:   { win32Layer->userInput.mouseState[2] = 0; return 1; }
        case WM_KEYDOWN: {
            if (wParam < 256) {
                win32Layer->userInput.keysDown[wParam] = true;
            }
            return 1;
        }
        case WM_KEYUP: {
            if (wParam < 256) {
                win32Layer->userInput.keysDown[wParam] = false;
            }
            return 1;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        case WM_CLOSE: {
            PostQuitMessage(0);
            return 0;
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
    const int defaultWindowWidth = 1280;
    const int defaultWindowHeight = 720;

    { // create and show window
        WNDCLASSA wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = instance;
        wc.lpszClassName = "FactionWindowClass";
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClassA(&wc);

        // pass in the win32Layer struct as the last param so that we can use it in other win32 functions
        win32Layer.window = CreateWindowExA(0, wc.lpszClassName, "Faction", WS_OVERLAPPEDWINDOW,
                                            CW_USEDEFAULT, CW_USEDEFAULT, defaultWindowWidth, defaultWindowHeight,
                                            NULL, NULL, instance, &win32Layer); 
        if (win32Layer.window == NULL) {
            OutputDebugStringA("Window failed to open\n");
            return 0;
        }
        ShowWindow(win32Layer.window, showCode);
    }
    
    GameState *gameState = FactionMain(defaultWindowWidth, defaultWindowHeight);
    win32Layer.userInput.keyMap[keyCode_A] = 'A';
    win32Layer.userInput.keyMap[keyCode_D] = 'D';
    win32Layer.userInput.keyMap[keyCode_S] = 'S';
    win32Layer.userInput.keyMap[keyCode_W] = 'W';
    win32Layer.userInput.keyMap[keyCode_Tab] = VK_TAB;
    win32Layer.userInput.keyMap[keyCode_LeftArrow] = VK_LEFT;
    win32Layer.userInput.keyMap[keyCode_RightArrow] = VK_RIGHT;
    win32Layer.userInput.keyMap[keyCode_UpArrow] = VK_UP;
    win32Layer.userInput.keyMap[keyCode_DownArrow] = VK_DOWN;
    win32Layer.userInput.keyMap[keyCode_Enter] = VK_RETURN;
    win32Layer.userInput.keyMap[keyCode_Escape] = VK_ESCAPE;

    LARGE_INTEGER frequency; // how many counts per second for the performance counter
    QueryPerformanceFrequency(&frequency); 

    { // message loop
        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0)) {

            // start timing
            LARGE_INTEGER startingTime, endingTime;
            int64_t elapsedMicroseconds;
            QueryPerformanceCounter(&startingTime);

            TranslateMessage(&msg);
            DispatchMessage(&msg);

            RECT clientRect;
            GetClientRect(win32Layer.window, &clientRect);
            float windowWidth = (float)clientRect.right;
            float windowHeight = (float)clientRect.bottom;
            GameRenderAndUpdate(gameState, windowWidth, windowHeight, &win32Layer.userInput);
            SwapBuffers(win32Layer.deviceContext);

            // end timing
            QueryPerformanceCounter(&endingTime);
            elapsedMicroseconds = endingTime.QuadPart - startingTime.QuadPart;
            elapsedMicroseconds *= 1000000;
            elapsedMicroseconds /= frequency.QuadPart;

            float targetFramesPerSec = 30.0f;
            int64_t targetMicroseconds = (int64_t)(1000000.0f / targetFramesPerSec);
            int64_t microsecondsToWait = targetMicroseconds - elapsedMicroseconds;
            if (microsecondsToWait > 0) {
                gameState->deltaT = targetMicroseconds / 1000.f;
                Sleep((DWORD)(microsecondsToWait / 1000.f)); // TODO: still no good?? Sleep() only has millisecond resultion
            } else {
                // missed the frame rate
                assert(false);
            }
        }
    }

    return 0;
}

// std::string PlatformGetExeDir() { // don't actaully need this function yet...
//     TCHAR filePath[MAX_PATH];
//     GetModuleFileName(NULL, filePath, MAX_PATH);
//     std::string fullExe(filePath);
//     std::string result = fullExe.substr(0, fullExe.find_last_of('\\')+1);
//     std::replace(result.begin(), result.end(), '\\', '/');
//     return result;
// }

void PlatformDBG(const char *msg) {
    OutputDebugStringA(msg);
}
