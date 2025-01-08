#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
#include <string>
#include <ShellScalingApi.h>
#include "XrayHandler.h"

// Link required library
#pragma comment(lib, "Shcore.lib")

using namespace Microsoft::WRL;

HWND hWnd;
wil::com_ptr<ICoreWebView2Controller> webViewController;
wil::com_ptr<ICoreWebView2> webViewWindow;
ComPtr<XrayHandler> xrayHandler;

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
        if (webViewController != nullptr) {
            RECT bounds;
            GetClientRect(hWnd, &bounds);
            webViewController->put_Bounds(bounds);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_DPICHANGED:
        // Handle DPI changes
        if (webViewController) {
            RECT* const newWindowSize = reinterpret_cast<RECT*>(lParam);
            SetWindowPos(
                hwnd,
                nullptr,
                newWindowSize->left,
                newWindowSize->top,
                newWindowSize->right - newWindowSize->left,
                newWindowSize->bottom - newWindowSize->top,
                SWP_NOZORDER | SWP_NOACTIVATE
            );
        }
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Initialize WebView2 environment and create webview
void InitializeWebView2()
{
    // Create WebView2 Environment
    auto options = Make<CoreWebView2EnvironmentOptions>();
    options->put_AllowSingleSignOnUsingOSPrimaryAccount(FALSE);

    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, options.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

                // Create a WebView2 Controller
                env->CreateCoreWebView2Controller(hWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            if (controller != nullptr) {
                                webViewController = controller;
                                webViewController->get_CoreWebView2(&webViewWindow);

                                // Configure WebView2 Settings
                                ICoreWebView2Settings* Settings;
                                webViewWindow->get_Settings(&Settings);
                                Settings->put_IsScriptEnabled(TRUE);
                                Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                                Settings->put_IsWebMessageEnabled(TRUE);
                                Settings->put_AreDevToolsEnabled(TRUE);

                                // Create and register XrayHandler
                                xrayHandler = Microsoft::WRL::Make<XrayHandler>();
                                
                                // Add XrayHandler as a host object
                                VARIANT var;
                                VariantInit(&var);
                                var.vt = VT_DISPATCH;
                                var.pdispVal = xrayHandler.Get();
                                
                                // Register the handler
                                webViewWindow->AddHostObjectToScript(L"xrayHandler", &var);
                                VariantClear(&var);

                                // Set initial window size
                                RECT bounds;
                                GetClientRect(hWnd, &bounds);
                                controller->put_Bounds(bounds);

                                // Navigate to local development server
                                webViewWindow->Navigate(L"http://localhost:5173/");

                                // Add event handlers
                                // WebMessage received from JavaScript
                                webViewWindow->add_WebMessageReceived(
                                    Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                        [](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                            LPWSTR message;
                                            args->TryGetWebMessageAsString(&message);
                                            // Handle message if needed
                                            CoTaskMemFree(message);
                                            return S_OK;
                                        }).Get(), nullptr);

                                // Navigation completed
                                webViewWindow->add_NavigationCompleted(
                                    Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                        [](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                            BOOL success;
                                            args->get_IsSuccess(&success);
                                            if (!success) {
                                                // Handle navigation failure
                                                MessageBox(hWnd, L"Navigation failed.", L"Error", MB_OK);
                                            }
                                            return S_OK;
                                        }).Get(), nullptr);
                            }
                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return 1;

    // Register window class
    const wchar_t CLASS_NAME[] = L"XRay Control Window";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    
    RegisterClass(&wc);

    // Get DPI for current monitor
    UINT dpi = GetDpiForSystem();
    float scale = static_cast<float>(dpi) / 96.0f;

    // Create main window
    hWnd = CreateWindowEx(
        0,                          // Optional window styles
        CLASS_NAME,                 // Window class
        L"XRay Control System",     // Window text
        WS_OVERLAPPEDWINDOW,        // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, // Position
        static_cast<int>(1024 * scale), // Width
        static_cast<int>(768 * scale),  // Height
        nullptr,       // Parent window    
        nullptr,       // Menu
        hInstance,     // Instance handle
        nullptr        // Additional application data
    );

    if (hWnd == nullptr) {
        return 0;
    }

    // Make sure XRay2.dll can be found
    SetDllDirectory(L"bin");

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Initialize WebView2
    InitializeWebView2();

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    webViewController = nullptr;
    webViewWindow = nullptr;
    xrayHandler = nullptr;

    CoUninitialize();

    return 0;
}