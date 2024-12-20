#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
#include <string>
#include <vector>
#include <gdiplus.h>
#include <memory>
#include <fstream>

// Add required libs
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "crypt32.lib")

using namespace Microsoft::WRL;
using namespace Gdiplus;

HWND hWnd;
wil::com_ptr<ICoreWebView2Controller> webViewController;
wil::com_ptr<ICoreWebView2> webViewWindow;
wil::com_ptr<ICoreWebView2Environment> webViewEnvironment;

// ImageHandler class definition
class ImageHandler : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IDispatch>
{
private:
    std::vector<BYTE> imageData;
    UINT width;
    UINT height;
    bool loaded;
    // 移除 GdiplusStartupInput 和句柄的重复使用
    // GdiplusStartup/GdiplusShutdown 全局只调用一次
    // 这里仅保留标记即可
    // GdiplusStartupInput gdiplusStartupInput;
    // ULONG_PTR gdiplusToken;

    bool LoadImageFile(const wchar_t* filePath)
    {
        // 不再在此处调用 GdiplusStartup / GdiplusShutdown
        // 仅使用 GDI+ 读取图像

        // 改为使用原始指针，手动管理生命周期
        Bitmap* bitmap = Bitmap::FromFile(filePath);
        if (!bitmap || bitmap->GetLastStatus() != Ok)
        {
            if (bitmap)
            {
                delete bitmap;
            }
            return false;
        }

        width = bitmap->GetWidth();
        height = bitmap->GetHeight();

        BitmapData bmpData;
        Rect rect(0, 0, width, height);
        if (bitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &bmpData) == Ok)
        {
            // Create shared buffer
            size_t bufferSize = width * height * 4;
            std::vector<BYTE> buffer(bufferSize);
            memcpy(buffer.data(), bmpData.Scan0, bufferSize);

            // Convert std::wstring to LPCWSTR
            std::wstring jsonMessage = L"{\"type\":\"image\",\"width\":" + std::to_wstring(width) +
                L",\"height\":" + std::to_wstring(height) + L",\"data\":\"" +
                std::wstring(buffer.begin(), buffer.end()) + L"\"}";

            webViewWindow->PostWebMessageAsJson(jsonMessage.c_str());

            bitmap->UnlockBits(&bmpData);
            loaded = true;
        }
        else
        {
            loaded = false;
        }

        // 手动释放 Bitmap 对象
        delete bitmap;
        return loaded;
    }

public:
    ImageHandler() : loaded(false) {}

    HRESULT LoadDefaultImage()
    {
        // 替换为你的默认图片路径
        const wchar_t* defaultImagePath = L"C:\\Users\\fuyuk\\Downloads\\test.png";

        if (LoadImageFile(defaultImagePath))
        {
            OutputDebugString(L"Default image loaded successfully\n");
            return S_OK;
        }
        else
        {
            OutputDebugString(L"Failed to load default image\n");
            return E_FAIL;
        }
    }

    // IDispatch implementation
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override
    {
        *pctinfo = 0;
        return S_OK;
    }

    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override
    {
        return E_NOTIMPL;
    }

    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgDispId) override
    {
        if (wcscmp(rgszNames[0], L"loadImage") == 0)
        {
            rgDispId[0] = 1;
            return S_OK;
        }
        return DISP_E_UNKNOWNNAME;
    }

    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
        EXCEPINFO* pExcepInfo, UINT* puArgErr) override
    {
        if (dispIdMember == 1) // loadImage
        {
            if (pDispParams->cArgs != 1 || pDispParams->rgvarg[0].vt != VT_BSTR)
                return E_INVALIDARG;

            bool success = LoadImageFile(pDispParams->rgvarg[0].bstrVal);

            if (pVarResult)
            {
                pVarResult->vt = VT_BOOL;
                pVarResult->boolVal = success ? VARIANT_TRUE : VARIANT_FALSE;
            }
            return S_OK;
        }
        return DISP_E_MEMBERNOTFOUND;
    }
};

// WindowProc implementation remains unchanged
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
        if (webViewController != nullptr)
        {
            RECT bounds;
            GetClientRect(hWnd, &bounds);
            webViewController->put_Bounds(bounds);
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void InitializeWebView2()
{
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
            {
                webViewEnvironment = env;  // 保存环境引用
                env->CreateCoreWebView2Controller(hWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            if (controller != nullptr)
                            {
                                webViewController = controller;
                                webViewController->get_CoreWebView2(&webViewWindow);

                                // 配置WebView2设置
                                ICoreWebView2Settings* Settings;
                                webViewWindow->get_Settings(&Settings);
                                Settings->put_IsScriptEnabled(TRUE);
                                Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                                Settings->put_IsWebMessageEnabled(TRUE);
                                Settings->put_AreDevToolsEnabled(TRUE);

                                // 注册图像处理器
                                auto imageHandler = Microsoft::WRL::Make<ImageHandler>();
                                VARIANT var;
                                VariantInit(&var);
                                var.vt = VT_DISPATCH;
                                var.pdispVal = imageHandler.Get();
                                webViewWindow->AddHostObjectToScript(L"imageHandler", &var);

                                // 设置初始窗口大小
                                RECT bounds;
                                GetClientRect(hWnd, &bounds);
                                controller->put_Bounds(bounds);

                                // 导航到本地服务器
                                webViewWindow->Navigate(L"http://localhost:5173/");

                                // 加载默认图片
                                imageHandler->LoadDefaultImage();
                            }
                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    // 在应用启动时初始化 GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken = 0;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    const wchar_t CLASS_NAME[] = L"XRay Viewer Window";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClass(&wc);

    hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"XRay Image Viewer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hWnd == nullptr)
    {
        // 若窗口创建失败，关闭 GDI+
        GdiplusShutdown(gdiplusToken);
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    InitializeWebView2();

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 在应用结束前一次性关闭 GDI+
    GdiplusShutdown(gdiplusToken);
    return 0;
}