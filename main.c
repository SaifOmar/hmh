#include <stdio.h>
#include <windows.h>

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message, WPARAM wParam,
                                    LPARAM lParam) {
  LRESULT Result = 0;
  switch (Message) {
  case WM_SIZE: {
    OutputDebugString("WM_SIZE\n");
  } break;

  case WM_DESTROY: {
    OutputDebugString("WM_DESTROY\n");
  } break;

  case WM_CLOSE: {
    OutputDebugString("WM_CLOSE\n");
  } break;

  case WM_ACTIVATEAPP: {
    OutputDebugString("WM_ACTIVATEAPP\n");
  } break;
  case WM_PAINT: {
    PAINTSTRUCT Paint;
    HDC DeviceContext = BeginPaint(Window, &Paint);
    int X = Paint.rcPaint.left;
    int Y = Paint.rcPaint.top;
    int Width = Paint.rcPaint.right - Paint.rcPaint.left;
    int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
    PatBlt(DeviceContext, X, Y, Width, Height, BLACKNESS);
    EndPaint(Window, &Paint);
  } break;
  default: {
    // OutputDebugString("Default\n");
    Result = DefWindowProcA(Window, Message, wParam, lParam);
  } break;
  }
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow) {

  WNDCLASSA WindowClass = {};

  WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = hInstance;
  WindowClass.lpszClassName = "HandMadeHeroWindowClass";

  if (RegisterClassA(&WindowClass)) {

    HWND WindowHandle = CreateWindowEx(
        0, WindowClass.lpszClassName, "HandMadeHero",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);
    if (WindowHandle) {
      for (;;) {
        MSG Message;
        BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
        if (MessageResult > 0) {
          TranslateMessage(&Message);
          DispatchMessage(&Message);
        } else {
          break;
        }
      }
    } else {
    }

  } else {
  }

  return (0);
};
