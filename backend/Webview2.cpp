#include "framework.h"
#include "WebviewReact.h"
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
#include "DeviceHandler.h"
#include <ShellScalingApi.h>
#include <string>
#include <vector>
#include <atlenc.h>

// 添加库链接
#pragma comment(lib, "crypt32.lib")

using namespace Microsoft::WRL;

#define MAX_LOADSTRING 100

// 图像数据结构
struct ImageData {
    std::vector<BYTE> data;
    UINT width;
    UINT height;
    ImageData() : width(640), height(480) {
        data.resize(width * height * 4);
    }
};

// ImageHandler 类定义
class ImageHandler : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IDispatch> {
private:
    ImageData imageData;

    std::wstring ConvertToWString(const std::string& str) {
        if (str.empty()) return std::wstring();
        int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size);
        return result;
    }

    void GenerateGradientImage() {
        for (UINT y = 0; y < imageData.height; y++) {
            for (UINT x = 0; x < imageData.width; x++) {
                UINT index = (y * imageData.width + x) * 4;
                float normalizedX = (float)x / imageData.width;
                float normalizedY = (float)y / imageData.height;

                imageData.data[index] = (BYTE)(255 * normalizedX);     // R
                imageData.data[index + 1] = (BYTE)(255 * normalizedY); // G
                imageData.data[index + 2] = (BYTE)(255 * (1 - normalizedX)); // B
                imageData.data[index + 3] = 255;                       // A
            }
        }
    }

    HRESULT EncodeBase64(const std::vector<BYTE>& data, std::string& base64Str) {
        int base64Len = Base64EncodeGetRequiredLength(data.size());
        base64Str.resize(base64Len);

        BOOL result = Base64Encode(
            data.data(),
            data.size(),
            &base64Str[0],
            &base64Len,
            ATL_BASE64_FLAG_NOCRLF
        );

        if (!result) {
            return E_FAIL;
        }

        base64Str.resize(base64Len);
        return S_OK;
    }

    HRESULT CreateSafeArray(VARIANT* pResult) {
        std::string base64Str;
        HRESULT hr = EncodeBase64(imageData.data, base64Str);
        if (FAILED(hr)) return hr;

        SAFEARRAYBOUND bounds[1];
        bounds[0].lLbound = 0;
        bounds[0].cElements = 3;
        SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 1, bounds);
        if (!psa) return E_OUTOFMEMORY;

        try {
            LONG index = 0;
            VARIANT var;
            VariantInit(&var);
            var.vt = VT_BSTR;
            var.bstrVal = SysAllocString(ConvertToWString(base64Str).c_str());
            hr = SafeArrayPutElement(psa, &index, &var);
            if (FAILED(hr)) throw hr;
            VariantClear(&var);

            index = 1;
            var.vt = VT_UI4;
            var.ulVal = imageData.width;
            hr = SafeArrayPutElement(psa, &index, &var);
            if (FAILED(hr)) throw hr;

            index = 2;
            var.vt = VT_UI4;
            var.ulVal = imageData.height;
            hr = SafeArrayPutElement(psa, &index, &var);
            if (FAILED(hr)) throw hr;

            pResult->vt = VT_ARRAY | VT_VARIANT;
            pResult->parray = psa;
            return S_OK;
        }
        catch (HRESULT hr) {
            SafeArrayDestroy(psa);
            return hr;
        }
    }

public:
    ImageHandler() {
        GenerateGradientImage();
    }

    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override {
        *pctinfo = 0;
        return S_OK;
    }

    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override {
        return E_NOTIMPL;
    }

    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgDispId) override {
        if (!rgszNames || !rgDispId) return E_INVALIDARG;

        if (cNames == 1 && wcscmp(rgszNames[0], L"getImageData") == 0) {
            rgDispId[0] = 1;
            return S_OK;
        }

        return DISP_E_UNKNOWNNAME;
    }

    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
        EXCEPINFO* pExcepInfo, UINT* puArgErr) override
    {
        if (dispIdMember == 1) {
            if (!pVarResult) return E_POINTER;
            return CreateSafeArray(pVarResult);
        }
        return DISP_E_MEMBERNOTFOUND;
    }
};

// Global Variables
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// WebView2 Globals
wil::com_ptr<ICoreWebView2Controller> webViewController;
wil::com_ptr<ICoreWebView2> webViewWindow;
ComPtr<DeviceHandler> deviceHandler;
ComPtr<ImageHandler> imageHandler;

// Function declarations
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                InitializeWebView2(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return FALSE;

    // Set DPI awareness
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Set DLL directory
    SetDllDirectoryW(L"bin");

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WEBVIEWREACT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    //// 添加这个检查
    //if (!webViewController || !webViewWindow) {
    //    OutputDebugString(L"WebView2 初始化失败。\n");
    //    return FALSE;
    //}

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WEBVIEWREACT));

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // 修改cleanup部分
    webViewController = nullptr;
    webViewWindow = nullptr;
    deviceHandler = nullptr;  // 替换原有的handlers

    CoUninitialize();
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WEBVIEWREACT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WEBVIEWREACT);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    // Get DPI for current monitor
    UINT dpi = GetDpiForSystem();
    float scale = static_cast<float>(dpi) / 96.0f;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0,
        static_cast<int>(1024 * scale),  // Scale window size based on DPI
        static_cast<int>(768 * scale),
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Initialize WebView2 after window creation
    InitializeWebView2(hWnd);

    return TRUE;
}

void InitializeWebView2(HWND hWnd) {
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
            {
                if (FAILED(result) || !env) {
                    OutputDebugString(L"Failed to create WebView2 environment\n");
                    return result;
                }
                env->CreateCoreWebView2Controller(hWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            if (FAILED(result) || !controller) {
                                OutputDebugString(L"Failed to create WebView2 controller\n");
                                return result;
                            }
                            webViewController = controller;
                            webViewController->get_CoreWebView2(&webViewWindow);

                            // 设置 WebView2 选项 
                            ICoreWebView2Settings* settings;
                            webViewWindow->get_Settings(&settings);
                            settings->put_IsScriptEnabled(TRUE);
                            settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                            settings->put_IsWebMessageEnabled(TRUE);
                            settings->put_AreDevToolsEnabled(TRUE);

                            try {
                                // 创建并注册 DeviceHandler
                                deviceHandler = Microsoft::WRL::Make<DeviceHandler>();
                                if (!deviceHandler) {
                                    OutputDebugString(L"Failed to create DeviceHandler\n");
                                    return E_FAIL;
                                }

                                VARIANT var;
                                VariantInit(&var);
                                var.vt = VT_DISPATCH;
                                var.pdispVal = deviceHandler.Get();
                                HRESULT hr = webViewWindow->AddHostObjectToScript(L"deviceHandler", &var);
                                VariantClear(&var);

                                if (FAILED(hr)) {
                                    OutputDebugString(L"Failed to register deviceHandler\n");
                                    return hr;
                                }

                                // 创建并注册 ImageHandler
                                imageHandler = Microsoft::WRL::Make<ImageHandler>();
                                if (!imageHandler) {
                                    OutputDebugString(L"Failed to create ImageHandler\n");
                                    return E_FAIL;
                                }

                                VARIANT varImage;
                                VariantInit(&varImage);
                                varImage.vt = VT_DISPATCH;
                                varImage.pdispVal = imageHandler.Get();
                                hr = webViewWindow->AddHostObjectToScript(L"imageHandler", &varImage);
                                VariantClear(&varImage);

                                if (FAILED(hr)) {
                                    OutputDebugString(L"Failed to register imageHandler\n");
                                    return hr;
                                }

                                // 启动设备监控
                                deviceHandler->StartMonitoring(webViewWindow.get());
                                OutputDebugString(L"Successfully started monitoring\n");

                                // 设置窗口大小
                                RECT bounds;
                                GetClientRect(hWnd, &bounds);
                                controller->put_Bounds(bounds);

                                // 导航到本地页面
                                webViewWindow->Navigate(L"http://localhost:5173/");
                                return S_OK;
                            }
                            catch (const std::exception& e) {
                                OutputDebugStringA(e.what());
                                return E_FAIL;
                            }
                            catch (...) {
                                OutputDebugString(L"Unknown exception during handlers creation\n");
                                return E_FAIL;
                            }
                        }).Get());
                return S_OK;
            }).Get());

    if (FAILED(hr)) {
        OutputDebugString(L"CreateCoreWebView2EnvironmentWithOptions failed\n");
    }
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        if (webViewController != nullptr) {
            RECT bounds;
            GetClientRect(hWnd, &bounds);
            webViewController->put_Bounds(bounds);
        }
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}