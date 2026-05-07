#include <stdint.h>
#include <stdio.h>
#include <windows.h>

#define local_persist static
#define global_variable static
#define internal static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


struct win32_offscreen_buffer {
  void *BitmapMemory;
  BITMAPINFO BitmapInfo;
  int BitmapWidth;
  int BitmapHeight;
  int BytesPerPixel;
  int Pitch;
};


struct win32_window_dimensions{
  int Width;
  int Height;
} ;

global_variable bool Running;
global_variable win32_offscreen_buffer OffscreenBuffer;

internal void RenderWeirdGradient(win32_offscreen_buffer *Buffer, int XOffset, int YOffset) {

  Buffer->Pitch = Buffer->BitmapWidth * Buffer->BytesPerPixel;

  uint8 *Row = (uint8 *)Buffer->BitmapMemory;
  for (int y = 0; y < Buffer->BitmapHeight; y++) {
    uint32 *Pixel = (uint32 *)Row;
    for (int x = 0; x < Buffer->BitmapWidth; x++) {
      uint8 Blue = (x + XOffset);
      uint8 Green =(y + YOffset);
      *Pixel++ = ((Green << 8) | Blue);
    }
    Row += Buffer->Pitch;
  }
}

internal void Win32ResizeDIBSection(win32_window_dimensions WindowDimensions, win32_offscreen_buffer *Buffer) {

  if (Buffer->BitmapMemory) {
    VirtualFree(OffscreenBuffer.BitmapMemory, 0, MEM_RELEASE);
  }

  Buffer->BitmapHeight = WindowDimensions.Height;
  Buffer->BitmapWidth = WindowDimensions.Width;
  Buffer->BytesPerPixel = 4;

  Buffer->BitmapInfo.bmiHeader.biSize = sizeof(Buffer->BitmapInfo.bmiHeader);
  Buffer->BitmapInfo.bmiHeader.biWidth = WindowDimensions.Width;
  Buffer->BitmapInfo.bmiHeader.biHeight = -WindowDimensions.Height;
  Buffer->BitmapInfo.bmiHeader.biPlanes = 1;
  Buffer->BitmapInfo.bmiHeader.biBitCount = 32;
  Buffer->BitmapInfo.bmiHeader.biCompression = BI_RGB;

  int BitmapMemorySize = (WindowDimensions.Width* WindowDimensions.Height) * Buffer->BytesPerPixel;
  Buffer->BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32UpdateWindow(HDC DeviceContext, win32_offscreen_buffer *Buffer, win32_window_dimensions WindowDimensions)
{
  StretchDIBits(DeviceContext,
                0, 0, WindowDimensions.Width, WindowDimensions.Height,
                0, 0, Buffer->BitmapWidth, Buffer->BitmapHeight,
                Buffer->BitmapMemory, &Buffer->BitmapInfo,
                DIB_RGB_COLORS, SRCCOPY);
}

win32_window_dimensions Win32GetWindowDimensions(HWND Window) {
  win32_window_dimensions Result;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    return Result;
}

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message, WPARAM wParam,
                                    LPARAM lParam) {
  LRESULT Result = 0;
  switch (Message) {
  case WM_SIZE: {

    win32_window_dimensions WindowDimensions = Win32GetWindowDimensions(Window);
    Win32ResizeDIBSection(WindowDimensions, &OffscreenBuffer);
  } break;

  case WM_DESTROY: {
    Running = false;
  } break;

  case WM_SYSKEYUP: {
  } break;
  case WM_SYSKEYDOWN: {
  } break;
  case WM_KEYDOWN: {
  } break;
  case WM_KEYUP: {
        uint32 VKCode = wParam;
        bool WasDown  = ((lParam & (1 << 30)) != 0);
        bool IsDown = ((lParam & (1 << 31)) == 0);
        if (IsDown != WasDown) {
          if (VKCode == 'W') {}
          else if (VKCode == 'A') {}
          else if (VKCode == 'S') {}
          else if (VKCode == 'D') {}
          else if (VKCode == VK_UP) {}
          else if (VKCode == VK_DOWN) {}
          else if (VKCode == VK_LEFT) {}
          else if (VKCode == VK_RIGHT) {}
          else if (VKCode == VK_ESCAPE) {
            printf("Escape: ");
            if (IsDown) {
              printf("Is down\n");
            }
            if (WasDown) {
              printf("Was down\n");
            }
          }
          else if (VKCode == VK_SPACE) {}
      }
    } break;
  case WM_CLOSE: {
    Running = false;
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

    win32_window_dimensions WindowDimensions = Win32GetWindowDimensions(Window);
    Win32UpdateWindow(DeviceContext,&OffscreenBuffer, WindowDimensions);
    EndPaint(Window, &Paint);
  } break;

  default: {
    Result = DefWindowProcA(Window, Message, wParam, lParam);
  } break;
  }
  return Result;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow) {

  WNDCLASSA WindowClass = {};

  WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = hInstance;
  WindowClass.lpszClassName = "HandMadeHeroWindowClass";

  if (RegisterClassA(&WindowClass)) {

    HWND Window = CreateWindowEx(0, WindowClass.lpszClassName, "HandMadeHero",
                                 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                 CW_USEDEFAULT, 0, 0, hInstance, 0);

    if (Window) {

      int XOffset = 0;
      int YOffset = 0;
      Running = true;

      while (Running) {

        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
          if (Message.message == WM_QUIT) {
            Running = false;
            break;
          }

          TranslateMessage(&Message);
          DispatchMessage(&Message);
        }
        // this is where he handle the xbox stuff (I don't care about that)
        // for () {
        //
        // }

        RenderWeirdGradient(&OffscreenBuffer,XOffset, YOffset);

        HDC DeviceContext = GetDC(Window);

        win32_window_dimensions WindowDimensions = Win32GetWindowDimensions(Window);
        Win32UpdateWindow(DeviceContext,&OffscreenBuffer, WindowDimensions);
        ReleaseDC(Window, DeviceContext);

        XOffset++;
        YOffset++;
      }
    }
  }

  return (0);
}
