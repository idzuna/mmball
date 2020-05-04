
#include "InputSender.hpp"
#include "Parameters.hpp"

#include <Windows.h>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <iostream>

constexpr char CLASS_NAME[] = "MMBALL DEFAULT CLASS";
constexpr char WINDOW_NAME[] = "MMBALL DEFAULT WINDOW";
constexpr char MUTEX_NAME[] = "MMBALL RUNNING MUTEX";
constexpr char USAGE_STRING[] =
"マウスボタンの入れ替えとスクロールエミュレートを行います\n"
"\n"
"MMBALL [/?] [/V] [/K] [/Lx] [/Rx] [/Mx] [/1x] [/2x] [/A 加速度] [/S 感度]\n"
"  [[/D デバイス] [/Lx] [/Rx] [/Mx] [/1x] [/2x] [/A 感度] [/S 感度]]\n"
"\n"
"オプション:\n"
"  /?            この使い方を表示します。\n"
"  /V            コンソールへマウス入力の情報を出力します\n"
"  /K            現在起動している MMBALL をすべて終了し、自身も終了します\n"
"  /Lx           マウスの左ボタンに役割 x を割り当てます\n"
"  /Rx           マウスの右ボタンに役割 x を割り当てます\n"
"  /Mx           マウスの中ボタンに役割 x を割り当てます\n"
"  /1x           マウスの X1 ボタンに役割 x を割り当てます\n"
"  /2x           マウスの X2 ボタンに役割 x を割り当てます\n"
"  /A加速度      マウスの加速度を変更します。加速度は以下の形式で指定します。\n"
"                  v0,t1:v1,t2:v2,...\n"
"                  マウスの移動量が t1 未満のときの速度の変化量 v0\n"
"                  マウスの移動量が tx 以上 t(x+1) 未満ときの速度の変化量 vx\n"
"                例:\n"
"                  /A1,1:1.4,5:1.8,10:2.2,20:3.2\n"
"                  移動量が          1 未満のときの速度の変化量 1\n"
"                  移動量が  1 以上  5 未満のときの速度の変化量 1.4\n"
"                  移動量が  5 以上 10 未満のときの速度の変化量 1.8\n"
"                  移動量が 10 以上 20 未満のときの速度の変化量 2.2\n"
"                  移動量が 20 以上        のときの速度の変化量 3.2\n"
"  /S感度        スクロールエミュレートの感度を正の小数で指定します。値が小さいほ\n"
"                ど感度が高くなります。既定値は 8 です。\n"
"  /Dデバイス名  特定のデバイスに対して個別の設定を適用します。デバイス名には /V\n"
"                オプション指定時に出力される文字列を指定します。\n"
"\n"
"役割:\n"
"  L 左ボタンを割り当てます\n"
"  R 右ボタンを割り当てます\n"
"  M 中ボタンを割り当てます\n"
"  1 X1 ボタンを割り当てます\n"
"  2 X2 ボタンを割り当てます\n"
"  S 押している間スクロールエミュレートを行います\n"
"  D ボタンを無効にします\n"
"\n"
"例:\n"
"  MMBALL /MD /1M /2S\n"
"  すべてのデバイスに対して中ボタンを無効にし、X1 ボタンに中ボタンを割り当てて、\n"
"  X2 ボタンが押されている間スクロールエミュレートを行います\n"
"\n"
"  MMBALL /MD /Dデバイス１ /Dデバイス２ /2S\n"
"  すべてのデバイスに対して中ボタンを無効にしますが、デバイス１とデバイス２では無\n"
"  効にせず、デバイス２の X2 ボタンに中ボタンを割り当てます。\n"
"\n";

static Parameters g_parameters;
static InputSender g_sender;

LRESULT CALLBACK LowLevelMouseProc(
    _In_ int    nCode,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    LPMSLLHOOKSTRUCT lpHook = reinterpret_cast<LPMSLLHOOKSTRUCT>(lParam);

    if (nCode >= 0 && lpHook->flags == 0) {
        switch (wParam) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEMOVE:
            return 1;
        }
    }
    return CallNextHookEx(0, nCode, wParam, lParam);
}

void PrintInput(const RAWINPUT& ri)
{
    if (ri.header.dwType == RIM_TYPEMOUSE && ri.header.hDevice) {
        char buf[128] = {};
        UINT size = sizeof(buf) - 1;
        GetRawInputDeviceInfo(ri.header.hDevice, RIDI_DEVICENAME, buf, &size);
        std::cout
            << "Device: " << buf
            << ", Motion: (" << ri.data.mouse.lLastX << ", " << ri.data.mouse.lLastY << ")";
        if (ri.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
            std::cout << ", Left button down";
        }
        if (ri.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
            std::cout << ", Left button up";
        }
        if (ri.data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
            std::cout << ", Right button down";
        }
        if (ri.data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
            std::cout << ", Right button up";
        }
        if (ri.data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
            std::cout << ", Middle button down";
        }
        if (ri.data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
            std::cout << ", Middle button up";
        }
        if (ri.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) {
            std::cout << ", X1 button down";
        }
        if (ri.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) {
            std::cout << ", X1 button up";
        }
        if (ri.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) {
            std::cout << ", X2 button down";
        }
        if (ri.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) {
            std::cout << ", X2 button up";
        }
        if (ri.data.mouse.usButtonFlags & RI_MOUSE_WHEEL) {
            std::cout << ", Wheel";
        }
        std::cout << std::endl;
    }
}

void ProcessInput(const Parameter& parameter, const RAWMOUSE& mouse)
{
    static double dx = 0;
    static double dy = 0;
    static bool scrolling = false;

    INPUT input = { INPUT_MOUSE };

    auto process = [&](Role role, Role whenNoChange, bool up) {
        DWORD flags = 0;
        switch (role == Role::NOCHANGE ? whenNoChange : role) {
        case Role::LEFT:
            flags = MOUSEEVENTF_LEFTDOWN;
            break;
        case Role::RIGHT:
            flags = MOUSEEVENTF_RIGHTDOWN;
            break;
        case Role::MIDDLE:
            flags = MOUSEEVENTF_MIDDLEDOWN;
            break;
        case Role::X1:
            flags = MOUSEEVENTF_XDOWN;
            input.mi.mouseData |= XBUTTON1;
            break;
        case Role::X2:
            flags = MOUSEEVENTF_XDOWN;
            input.mi.mouseData |= XBUTTON2;
            break;
        case Role::SCROLL:
            scrolling = !up;
            break;
        }
        if (up) {
            flags <<= 1;
        }
        input.mi.dwFlags |= flags;
    };

    if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
        process(parameter.left, Role::LEFT, false);
    }
    if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
        process(parameter.right, Role::RIGHT, false);
    }
    if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
        process(parameter.middle, Role::MIDDLE, false);
    }
    if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) {
        process(parameter.x1, Role::X1, false);
    }
    if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) {
        process(parameter.x2, Role::X2, false);
    }
    if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
        process(parameter.left, Role::LEFT, true);
    }
    if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
        process(parameter.right, Role::RIGHT, true);
    }
    if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
        process(parameter.middle, Role::MIDDLE, true);
    }
    if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) {
        process(parameter.x1, Role::X1, true);
    }
    if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) {
        process(parameter.x2, Role::X2, true);
    }

    if (mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
        input.mi.dx = mouse.lLastX;
        input.mi.dy = mouse.lLastY;
        input.mi.dwFlags |= MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
        g_sender.send(input);
    } else if (scrolling) {
        g_sender.send(input);
        if (mouse.lLastX != 0 || mouse.lLastY != 0) {
            const double k = parameter.acceleration.get(mouse.lLastX, mouse.lLastY);
            dx += mouse.lLastX * k / parameter.sensitivity;
            dy += mouse.lLastY * k / parameter.sensitivity;
            while (true) {
                if (dy >= 1.) {
                    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
                    input.mi.mouseData = static_cast<DWORD>(-WHEEL_DELTA);
                    g_sender.send(input);
                    dy -= 1;
                } else if (dy <= -1) {
                    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
                    input.mi.mouseData = static_cast<DWORD>(WHEEL_DELTA);
                    g_sender.send(input);
                    dy += 1;
                } else {
                    break;
                }
            }
        }
    } else {
        if (mouse.lLastX != 0 || mouse.lLastY != 0) {
            const double k = parameter.acceleration.get(mouse.lLastX, mouse.lLastY);
            input.mi.dx = static_cast<LONG>(std::round(mouse.lLastX * k));
            input.mi.dy = static_cast<LONG>(std::round(mouse.lLastY * k));
            input.mi.dwFlags |= MOUSEEVENTF_MOVE;
        }
        g_sender.send(input);
        dx = 0;
        dy = 0;
    }
}

LRESULT CALLBACK WindowProc(
    _In_ HWND   hwnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    constexpr double threshold = 10.;
    if (uMsg == WM_INPUT) {
        RAWINPUT ri;
        UINT size = sizeof(ri);
        GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
                        RID_INPUT, &ri, &size, sizeof(RAWINPUTHEADER));

        if (g_parameters.global().verbose) {
            PrintInput(ri);
        }

        if (ri.header.dwType == RIM_TYPEMOUSE && ri.header.hDevice != NULL) {
            char buf[128] = {};
            UINT size = sizeof(buf) - 1;
            GetRawInputDeviceInfo(ri.header.hDevice, RIDI_DEVICENAME, buf, &size);

            const Parameter& parameter = g_parameters.get(buf);
            ProcessInput(parameter, ri.data.mouse);
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nShowCmd
)
{
    const BOOL console = AttachConsole(ATTACH_PARENT_PROCESS);
    if (console) {
        FILE* fpOut = NULL;
        freopen_s(&fpOut, "CONOUT$", "w", stdout);
        std::cout << std::endl;
    }

    try {
        g_parameters.parse(__argc, __argv);
    } catch (const std::string& str) {
        if (console) {
            std::cout << str.c_str();
        } else {
            MessageBoxA(NULL, str.c_str(), __argv[0], MB_OK);
        }
        return -1;
    }

    if (g_parameters.global().help) {
        if (console) {
            std::cout << USAGE_STRING;
        } else {
            MessageBoxA(NULL, USAGE_STRING, __argv[0], MB_OK);
        }
        return 0;
    }

    if (g_parameters.global().kill) {
        HWND hwnd = NULL;
        std::vector<HWND> windows;
        while (hwnd = FindWindowEx(NULL, hwnd, CLASS_NAME, WINDOW_NAME)) {
            windows.push_back(hwnd);
        }
        for (auto w : windows) {
            PostMessage(w, WM_QUIT, 0, 0);
        }
        return 0;
    }

    HANDLE hMutex = CreateMutex(NULL, TRUE, MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        constexpr char message[] =
            "多重起動はできません。すでに起動中のプロセスを終了する場合は /K オプションを指定します。";
        if (console) {
            std::cout << message << std::endl;
        } else {
            MessageBoxA(NULL, message, __argv[0], MB_OK);
        }
        return -1;
    }

    SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);

    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW,
                             100, 100, 100, 100, NULL, NULL, hInstance, NULL);
    
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 1;
    rid.usUsage = 2;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hwnd;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));

    MSG uMsg;
    while (GetMessage(&uMsg, NULL, 0, 0))
    {
        DispatchMessage(&uMsg);
    }
    return 0;
}
