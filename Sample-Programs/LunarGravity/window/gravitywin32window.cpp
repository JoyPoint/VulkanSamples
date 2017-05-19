/*
 * LunarGravity - gravitywin32window.cpp
 *
 * Copyright (C) 2017 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef VK_USE_PLATFORM_WIN32_KHR

#include <iostream>

#include "gravitywin32window.hpp"
#include "gravitylogger.hpp"
#include "gravityevent.hpp"


GravityWin32Window::GravityWin32Window(const char *win_name, const uint32_t width, const uint32_t height, bool fullscreen) :
    GravityWindow(win_name, width, height, fullscreen) {

    m_instance = GetModuleHandle(NULL);
}

GravityWin32Window::~GravityWin32Window() {
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    GravityLogger &logger = GravityLogger::getInstance();
    GravityEventList &event_list = GravityEventList::getInstance();
    GravityWin32Window *window = reinterpret_cast<GravityWin32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (msg) {
    case WM_CLOSE:
    {
        logger.LogInfo("GravityWin32Window::WndProc -Received close event");
        GravityEvent event(GravityEvent::GRAVITY_EVENT_WINDOW_CLOSE);
        if (event_list.SpaceAvailable()) {
            event_list.InsertEvent(event);
        }
        else {
            logger.LogError("GravityWin32Window::WndProc No space in event list to add key press");
        }
        window->TriggerQuit();
        break;
    }
    case WM_GETMINMAXINFO:     // set window's minimum size
        // TODO: Brainpain - Do we need to handle this?
        //((MINMAXINFO*)lParam)->ptMinTrackSize = m_minsize;
        return 0;
    case WM_SIZE:
        // TODO: Brainpain - Post re-size message to app
        // Resize the application to the new window size, except when
        // it was minimized. Vulkan doesn't support images or swapchains
        // with width=0 and height=0.
        //if (wParam != SIZE_MINIMIZED) {
        //    demo.width = lParam & 0xffff;
        //    demo.height = (lParam & 0xffff0000) >> 16;
        //    demo_resize(&demo);
        //}
        break;
    case WM_KEYDOWN: {
        GravityEvent event(GravityEvent::GRAVITY_EVENT_KEY_PRESS);
        bool add_key = false;
        switch (wparam) {
        case VK_ESCAPE:
            event.data.key = KEYNAME_ESCAPE;
            add_key = true;
logger.LogWarning("Hit ESCAPE");
            break;
        case VK_LEFT:
            event.data.key = KEYNAME_ARROW_LEFT;
            add_key = true;
logger.LogWarning("Hit LEFT");
            break;
        case VK_RIGHT:
            event.data.key = KEYNAME_ARROW_RIGHT;
            add_key = true;
logger.LogWarning("Hit RIGHT");
            break;
        case VK_SPACE:
            event.data.key = KEYNAME_SPACE;
            window->TogglePause();
            add_key = true;
logger.LogWarning("Hit SPACE");
            break;
        default:
logger.LogWarning("Unknown Key");
            break;
        }
        if (add_key) {
            if (event_list.SpaceAvailable()) {
                event_list.InsertEvent(event);
            } else {
                logger.LogError("GravityXcbWindow::handle_xcb_event No space in event "
                    "list to add key press");
            }
        }
        break;
    }

    default:
        printf("%d\n", msg);
        break;
    }
    return (DefWindowProc(hwnd, msg, wparam, lparam));
}

static void window_thread(GravityWin32Window *window) {
    GravityLogger &logger = GravityLogger::getInstance();
    GravityEventList &event_list = GravityEventList::getInstance();
    MSG msg;
    bool quit = false;

    logger.LogInfo("window_thread starting window thread");
    while (!quit) {
#if 0 // Brainpain
        BOOL bRet;
        while ((bRet = GetMessage(&msg, window->GetHwnd(), 0, 0)) != 0)
        {
            if (bRet == -1)
            {
                // handle the error and possibly exit
            }
            else
            {
                if (msg.message == WM_CLOSE || msg.message == WM_QUIT) {
                    logger.LogInfo("window_thread - Received close event");
                    GravityEvent event(GravityEvent::GRAVITY_EVENT_WINDOW_CLOSE);
                    if (event_list.SpaceAvailable()) {
                        event_list.InsertEvent(event);
                    }
                    else {
                        logger.LogError("window_thread No space in event list to add key press");
                    }
                    window->TriggerQuit();
                    quit = true;
                } else if (msg.message == WM_KEYDOWN) {
                    logger.LogError("KEY DOWN!!");
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
#else
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            switch (msg.message) {
                case WM_CLOSE:
                case WM_QUIT:
                {
                    logger.LogInfo("window_thread - Received close event");
                    GravityEvent event(GravityEvent::GRAVITY_EVENT_WINDOW_CLOSE);
                    if (event_list.SpaceAvailable()) {
                        event_list.InsertEvent(event);
                    }
                    else {
                        logger.LogError("window_thread No space in event list to add key press");
                    }
                    window->TriggerQuit();
                    quit = true;
                    break;
                }
                case WM_KEYDOWN: {
                    logger.LogError("KEY DOWN!!");
                    break;
                }
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
#endif
    }

    logger.LogInfo("window_thread window thread finished");
}

bool GravityWin32Window::CreateGfxWindow(VkInstance &instance) {
    GravityLogger &logger = GravityLogger::getInstance();
    RECT wr = {0, 0, (LONG)m_width, (LONG)m_height};
    DWORD ext_win_style = WS_EX_APPWINDOW;
    DWORD win_style = WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    WNDCLASSEX win_class_ex = {};

    win_class_ex.cbSize = sizeof(WNDCLASSEX);
    win_class_ex.style = CS_HREDRAW | CS_VREDRAW;
    win_class_ex.lpfnWndProc = WndProc;
    win_class_ex.cbClsExtra = 0;
    win_class_ex.cbWndExtra = 0;
    win_class_ex.hInstance = m_instance;
    win_class_ex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    win_class_ex.hCursor = LoadCursor(NULL, IDC_ARROW);
    win_class_ex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    win_class_ex.lpszMenuName = NULL;
    win_class_ex.lpszClassName = m_win_name;
    win_class_ex.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
    if (!RegisterClassEx(&win_class_ex)) {
        logger.LogError("GravityWin32Window::CreateGfxWindow - Failed attempting"
                        "to Register window class!");
        return false;
    }

    if (m_fullscreen) {
        // Determine if we can go fullscreen with the provided settings.
        DEVMODE dev_mode_settings = {};
        dev_mode_settings.dmSize = sizeof(DEVMODE);
        dev_mode_settings.dmPelsWidth = m_width;
        dev_mode_settings.dmPelsHeight = m_height;
        dev_mode_settings.dmBitsPerPel = 32;
        dev_mode_settings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
        if (DISP_CHANGE_SUCCESSFUL != ChangeDisplaySettings(&dev_mode_settings, CDS_FULLSCREEN))
        {
            if (IDYES == MessageBox(nullptr, "Failed fullscreen at provided resolution.  Go Windowed?", m_win_name,
                           MB_YESNO | MB_ICONEXCLAMATION))
            {
                // Fallback to windowed mode
                m_fullscreen = FALSE;
            } else {
                // Give up
                logger.LogError("GravityWin32Window::CreateGfxWindow - Failed to create"
                                "fullscreen window at provided resolution.");
                return false;
            }
        }
    }

    if (m_fullscreen) {
        win_style |= WS_POPUP;
        ShowCursor(FALSE);
    } else {
        win_style |= WS_OVERLAPPEDWINDOW;
        ext_win_style |= WS_EX_WINDOWEDGE;
    }

    // Create window with the registered class:
    AdjustWindowRectEx(&wr, win_style, FALSE, ext_win_style);
    m_window = CreateWindowEx(ext_win_style, m_win_name, m_win_name, win_style,
                              100, 100, wr.right - wr.left,
                              wr.bottom - wr.top, nullptr, nullptr,
                              m_instance, nullptr);
    if (!m_window) {
        logger.LogError("GravityWin32Window::CreateGfxWindow - Failed attempting"
                        "to create window class!");
        fflush(stdout);
        return false;
    }

    SetForegroundWindow(m_window);
    SetWindowLongPtr(m_window, GWLP_USERDATA, (LONG_PTR)this);

    // Window client area size must be at least 1 pixel high, to prevent crash.
    m_minsize.x = GetSystemMetrics(SM_CXMINTRACK);
    m_minsize.y = GetSystemMetrics(SM_CYMINTRACK) + 1;

    VkWin32SurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.hinstance = m_instance;
    createInfo.hwnd = m_window;
    VkResult vk_result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &m_vk_surface);
    if (VK_SUCCESS != vk_result) {
        std::string error_msg = "GravityWin32Window::CreateGfxWindow - vkCreateWin32SurfaceKHR failed "
                                "with error ";
        error_msg += vk_result;
        logger.LogError(error_msg);
        return false;
    }
    m_window_thread = new std::thread(window_thread, this);
    if (NULL == m_window_thread) {
        logger.LogError("GravityXcbWindow::CreateGfxWindow failed to create window thread");
        return false;
    }

    return true;
}

bool GravityWin32Window::CloseGfxWindow() {
    if (NULL != m_window_thread) {
        m_window_thread->join();
        delete m_window_thread;
    }

    if (m_fullscreen) {
        ChangeDisplaySettings(nullptr, 0);
        ShowCursor(TRUE);
    }
    DestroyWindow(m_window);
    return true;
}

void GravityWin32Window::TriggerQuit() {
    PostQuitMessage(0);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
