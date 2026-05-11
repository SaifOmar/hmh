#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <dsound.h>

#define local_persist static
#define global_variable static
#define internal static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;
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
global_variable LPDIRECTSOUNDBUFFER SecondaryBuffer;

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID lpcGuidDevice, LPDIRECTSOUND *lplpDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);


internal void Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize) {

  // NOTE: Load the lib
  HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
  if (!DSoundLibrary) { printf("Could not load dsound.dll\n"); return; }

  // NOTE: Get a direct sound object! - coapriative
   direct_sound_create *DirectSoundCreate = (direct_sound_create *)
      GetProcAddress(DSoundLibrary, "DirectSoundCreate");

    LPDIRECTSOUND DirectSound;
    // if (!DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))){ printf("Could not get DirectSoundCreate\n"); return; }

    if (!DirectSoundCreate || !SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) { printf("Could not get DirectSoundCreate\n"); return; }
    if (!SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) { printf("Could not set cooperative level\n"); return; }

    WAVEFORMATEX WaveFormat = {};
    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormat.nChannels = 2;
    WaveFormat.nSamplesPerSec = SamplesPerSecond;
    WaveFormat.wBitsPerSample = 16;
    WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample)/ 8;
    WaveFormat.nAvgBytesPerSec = SamplesPerSecond * WaveFormat.nBlockAlign;
    WaveFormat.cbSize = 0;

    // NOTE: Create a primary buffer
    DSBUFFERDESC PrimaryBufferDescription = {};
    PrimaryBufferDescription.dwSize = sizeof(PrimaryBufferDescription);
    PrimaryBufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
    LPDIRECTSOUNDBUFFER PrimaryBuffer;
    if(!SUCCEEDED(DirectSound->CreateSoundBuffer(&PrimaryBufferDescription, &PrimaryBuffer, 0))) { printf("Could not create primary buffer\n"); return;}
    

    if(!SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat))) { printf("Could not set format\n"); return; }

    // NOTE: Create a secondary buffer
    // NOTE(saif):this is the buffer that we will be playing
    DSBUFFERDESC SecondaryBufferDescription = {};
    SecondaryBufferDescription.dwSize = sizeof(SecondaryBufferDescription);
    SecondaryBufferDescription.dwFlags = 0;
    SecondaryBufferDescription.dwBufferBytes = BufferSize;
    SecondaryBufferDescription.lpwfxFormat = &WaveFormat;
    if(!SUCCEEDED(DirectSound->CreateSoundBuffer(&SecondaryBufferDescription, &SecondaryBuffer, 0))) { printf("Could not create secondary buffer\n"); return; }
  
    // NOTE: Start playing


    


}

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

  case WM_SYSKEYUP:
  case WM_SYSKEYDOWN:
  case WM_KEYDOWN:
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
      bool32 AltKeyWasDown = (lParam & (1 << 29));
      printf("lparam: %d\n", lParam);
      printf("AltKeyWasDown: %d\n", AltKeyWasDown);
      if ((VKCode == VK_F4) && AltKeyWasDown) {
        printf("F4 was pressed\n");
        Running = false;
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
      int SamplesPerSecond = 48000;
      int XOffset = 0;
      int YOffset = 0;
      int Hz = 256;
      uint32 RunningSampleIndex = 0;
      // int SquareWaveCounter = 0;
      int SquareWavePeriod = SamplesPerSecond/Hz;
      int HalfSquareWavePeriod = SquareWavePeriod/2;
      int BytesPerSample = sizeof(int16)*2;
      int SecondaryBufferSize = SamplesPerSecond*BytesPerSample;

      HDC DeviceContext = GetDC(Window);

      Win32InitDSound(Window, SamplesPerSecond, SecondaryBufferSize);
      SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

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

        // NOTE: Output test
         
        DWORD PlayCursor;
        DWORD WriteCursor;

        if(SUCCEEDED(SecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor))) {

          DWORD ByteToLock= (RunningSampleIndex*BytesPerSample) % SecondaryBufferSize;
          DWORD BytesToWrite;
          if (ByteToLock > PlayCursor) {
            BytesToWrite=  SecondaryBufferSize - ByteToLock;
            BytesToWrite += PlayCursor;
          } else {
            BytesToWrite = PlayCursor - ByteToLock;
          }

          void *Region1;
          DWORD Region1Size;
          void *Region2;
          DWORD Region2Size;
          if(SUCCEEDED( SecondaryBuffer->Lock(ByteToLock,BytesToWrite,&Region1,&Region1Size,&Region2,&Region2Size,0))) {
            int16 *SampleOut = (int16 *)Region1;
            // TODO: Assert that Region1Size and Region2Size are valid
            DWORD Region1SampleCount = Region1Size / BytesPerSample;
            DWORD Region2SampleCount = Region2Size / BytesPerSample;
            for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; SampleIndex++) {


              int16 SampleValue= ((RunningSampleIndex / HalfSquareWavePeriod)) % 2  ? 16000 : -16000;
              *SampleOut++ = SampleValue;
              *SampleOut++ = SampleValue;
              RunningSampleIndex++;
            }

            SampleOut = (int16 *)Region2;
            for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; SampleIndex++) {

              int16 SampleValue= ((RunningSampleIndex / HalfSquareWavePeriod) % 2) ? 16000 : -16000;
              *SampleOut++ = SampleValue;
              *SampleOut++ = SampleValue;
              RunningSampleIndex++;
            }
            SecondaryBuffer->Unlock(
              Region1, Region1Size,
              Region2, Region2Size
            );
          }
        }
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
