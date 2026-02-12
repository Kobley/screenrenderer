#include "main.hh"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg)
  {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Render(HWND hWnd, HDC screenDC, HDC memDC, void* bits) {

  ZeroMemory(bits, context::width * context::height * 4);

  // Setup
  HBRUSH bBlack = CreateSolidBrush(RGB(0, 0, 0)); // BLACK brush
  HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, bBlack);

  HPEN nullPen = (HPEN)GetStockObject(NULL_PEN); // no outline
  HPEN oldPen = (HPEN)SelectObject(memDC, nullPen);

  HPEN pWhite = CreatePen(PS_SOLID, 1, RGB(255, 255, 255)); // WHITE outline
  HPEN pBlack = CreatePen(PS_SOLID, 4, RGB(1, 1, 1)); // BLACK outline
  HPEN pBlackThin = CreatePen(PS_SOLID, 1, RGB(1, 1, 1));

  // Draw
  POINT p;
  GetCursorPos(&p);

  int centerX = p.x; //context.width/2;
  int centerY = p.y; //context.height/2;
  int eWidth = 50;
  int eHeight = 50;

  //draw espbox with outline
  SelectObject(memDC, pBlack);
  Rectangle(memDC, 
    centerX - eWidth/2+1, 
    centerY - eHeight/2+1,
    centerX + eWidth/2, 
    centerY + eHeight/2
  );

  SelectObject(memDC, pWhite);
  Rectangle(memDC, 
    centerX - eWidth/2, 
    centerY - eHeight/2,
    centerX + eWidth/2, 
    centerY + eHeight/2
  );

  SelectObject(memDC, pBlackThin);
  Rectangle(memDC, 
    centerX - eWidth/2+1, 
    centerY - eHeight/2+1,
    centerX + eWidth/2-1, 
    centerY + eHeight/2-1
  );

  //Clear tools
  SelectObject(memDC, oldBrush);
  SelectObject(memDC, oldPen);
  DeleteObject(bBlack);
  DeleteObject(pWhite);
  DeleteObject(pBlack);
  DeleteObject(pBlackThin);

  // force pixels alpha
  DWORD* pixel = (DWORD*)bits;
  for (int i = 0; i < context::width * context::height; i++)
  {
    if (pixel[i] != 0) // if pixel is not fully transparent
        pixel[i] |= 0xFF000000; // force alpha = 255
  }

  POINT ptPos = { 0, 0 };
  SIZE sizeWnd = { context::width, context::height };
  POINT ptSrc = { 0, 0 };

  BLENDFUNCTION blend = {0};
  blend.BlendOp = AC_SRC_OVER;
  blend.SourceConstantAlpha = 255;
  blend.AlphaFormat = AC_SRC_ALPHA;

  UpdateLayeredWindow(
      hWnd,
      screenDC,
      &ptPos,
      &sizeWnd,
      memDC,
      &ptSrc,
      0,
      &blend,
      ULW_ALPHA
  );

  ReleaseDC(NULL, screenDC);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE retarded, LPSTR lpCmdLine, int nCmdShow) {

  const char CLASS_NAME[] = "gdiexample";

  WNDCLASSA wc = {0};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  RegisterClassA(&wc);

  HWND hWnd = CreateWindowExA(
    WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
    CLASS_NAME,
    "GDI Example",
    WS_POPUP,
    1, 1,
    context::width, context::height,
    NULL,
    NULL,
    hInstance,
    NULL
  );

  ShowWindow(hWnd, nCmdShow);

  HDC screenDC = GetDC(NULL);
  HDC memDC = CreateCompatibleDC(screenDC);

  BITMAPINFO bmi = {0};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = context::width;
  bmi.bmiHeader.biHeight = -context::height; // top-down DIB
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  void* bits = NULL;
  HBITMAP hBitmap = CreateDIBSection(screenDC, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
  SelectObject(memDC, hBitmap);

  // Clear entire bitmap (fully transparent)
  ZeroMemory(bits, context::width * context::height * 4);

  MSG msg = {0};

  //Draw here
  while (TRUE) {
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) 
        return 0;
      
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    Render(hWnd, screenDC, memDC, bits);
    Sleep(6);
  }

  return 0;
}