#include "ui/imgui_renderer.h"
#include <imgui_impl_win32.h>
#include <Windows.h>

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (ImGui::GetCurrentContext() == nullptr) return 0;

  ImGuiIO& io = ImGui::GetIO();
  switch (msg) {
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK: {
      int button = 0;
      if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
      if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
      if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
      if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
      if (!ImGui::IsAnyMouseDown() && GetCapture() == nullptr)
        SetCapture(hWnd);
      io.AddMouseButtonEvent(button, true);
      return 0;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP: {
      int button = 0;
      if (msg == WM_LBUTTONUP) { button = 0; }
      if (msg == WM_RBUTTONUP) { button = 1; }
      if (msg == WM_MBUTTONUP) { button = 2; }
      if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
      io.AddMouseButtonEvent(button, false);
      return 0;
    }
    case WM_MOUSEWHEEL:
      io.AddMouseWheelEvent(0.0f, static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA));
      return 0;
    case WM_MOUSEHWHEEL:
      io.AddMouseWheelEvent(static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA), 0.0f);
      return 0;
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP: {
      bool down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
      if (wParam < 256) {
        io.AddKeyEvent(ImGuiKey_ModCtrl, (GetKeyState(VK_CONTROL) & 0x8000) != 0);
        io.AddKeyEvent(ImGuiKey_ModShift, (GetKeyState(VK_SHIFT) & 0x8000) != 0);
        io.AddKeyEvent(ImGuiKey_ModAlt, (GetKeyState(VK_MENU) & 0x8000) != 0);
        io.AddKeyEvent(ImGuiKey_ModSuper, (GetKeyState(VK_LWIN) & 0x8000 || GetKeyState(VK_RWIN) & 0x8000) != 0);
      }
      return 0;
    }
    case WM_CHAR:
      if (wParam > 0 && wParam < 0x10000)
        io.AddInputCharacterUTF16(static_cast<unsigned short>(wParam));
      return 0;
    case WM_SETCURSOR:
      if (LOWORD(lParam) == HTCLIENT && ImGui::GetIO().WantCaptureMouse)
        return 1;
      break;
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
      return 0;
    default:
      break;
  }

  return 0;
}