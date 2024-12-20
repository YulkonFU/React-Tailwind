#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
#include <string>
#include <vector>
#include <atlenc.h>
#include <wincrypt.h>
#include <gdiplus.h>
#include <memory>
#include <fstream>

// 添加库链接
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Microsoft::WRL;
using namespace Gdiplus;

HWND hWnd;
wil::com_ptr<ICoreWebView2Controller> webViewController;
wil::com_ptr<ICoreWebView2> webViewWindow;

// 图像数据结构
struct ImageData {
    std::vector<BYTE> data;
    UINT width;
    UINT height;
    bool loaded;

    ImageData() : width(0), height(0), loaded(false) {}
};

// ImageHandler 类定义
class ImageHandler : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IDispatch> {
private:
    ImageData imageData;
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    bool isGdiplusInitialized;

    std::wstring ConvertToWString(const std::string& str) {
        if (str.empty()) return std::wstring();
        int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size);
        return result;
    }

    bool InitializeGdiplus() {
        if (!isGdiplusInitialized) {
            if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) != Ok) {
                return false;
            }
            isGdiplusInitialized = true;
        }
        return true;
    }

    bool LoadImageFile(const wchar_t* filePath) {
        OutputDebugString(L"Starting LoadImageFile\n");

        if (!InitializeGdiplus()) {
            OutputDebugString(L"Failed to initialize GDI+\n");
            return false;
        }

        // 使用智能指针管理Bitmap对象
        std::unique_ptr<Bitmap> bitmap(Bitmap::FromFile(filePath));
        if (!bitmap || bitmap->GetLastStatus() != Ok) {
            OutputDebugString(L"Failed to load bitmap\n");
            return false;
        }

        // 获取图像尺寸
        imageData.width = bitmap->GetWidth();
        imageData.height = bitmap->GetHeight();

        // 分配内存
        try {
            imageData.data.resize(imageData.width * imageData.height * 4);
        }
        catch (const std::exception& e) {
            OutputDebugString(L"Failed to allocate memory for image data\n");
            return false;
        }

        // 锁定位图数据
        BitmapData bitmapData;
        Rect rect(0, 0, imageData.width, imageData.height);
        Status status = bitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);

        if (status == Ok) {
            // 复制数据
            memcpy(imageData.data.data(), bitmapData.Scan0, imageData.data.size());
            bitmap->UnlockBits(&bitmapData);
            imageData.loaded = true;

            OutputDebugString(L"Image loaded successfully\n");
            return true;
        }

        OutputDebugString(L"Failed to lock bitmap bits\n");
        return false;
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

    HRESULT CreateSafeArray(const std::vector<BYTE>& data, UINT width, UINT height, VARIANT* pResult) {
        std::string base64Str;
        HRESULT hr = EncodeBase64(data, base64Str);
        if (FAILED(hr)) return hr;

        SAFEARRAYBOUND bounds[1];
        bounds[0].lLbound = 0;
        bounds[0].cElements = 3;
        SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 1, bounds);
        if (!psa) return E_OUTOFMEMORY;

        try {
            // 添加Base64数据
            LONG index = 0;
            VARIANT var;
            VariantInit(&var);
            var.vt = VT_BSTR;
            var.bstrVal = SysAllocString(ConvertToWString(base64Str).c_str());
            hr = SafeArrayPutElement(psa, &index, &var);
            if (FAILED(hr)) throw hr;
            VariantClear(&var);

            // 添加宽度
            index = 1;
            VariantInit(&var);
            var.vt = VT_UI4;
            var.ulVal = width;
            hr = SafeArrayPutElement(psa, &index, &var);
            if (FAILED(hr)) throw hr;

            // 添加高度
            index = 2;
            VariantInit(&var);
            var.vt = VT_UI4;
            var.ulVal = height;
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
    ImageHandler() : isGdiplusInitialized(false) {}

    ~ImageHandler() {
        if (isGdiplusInitialized) {
            GdiplusShutdown(gdiplusToken);
            isGdiplusInitialized = false;
        }
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

        if (cNames == 1) {
            if (wcscmp(rgszNames[0], L"getImageData") == 0) {
                rgDispId[0] = 1;
                return S_OK;
            }
            else if (wcscmp(rgszNames[0], L"loadImage") == 0) {
                rgDispId[0] = 2;
                return S_OK;
            }
        }

        return DISP_E_UNKNOWNNAME;
    }

    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
        EXCEPINFO* pExcepInfo, UINT* puArgErr) override
    {
        try {
            OutputDebugString(L"Invoke called\n");

            switch (dispIdMember) {
            case 1: { // getImageData
                OutputDebugString(L"getImageData called\n");
                if (!pVarResult) {
                    OutputDebugString(L"pVarResult is null\n");
                    return E_POINTER;
                }
                if (!imageData.loaded) {
                    OutputDebugString(L"No image data loaded\n");
                    return E_FAIL;
                }
                OutputDebugString(L"Creating safe array\n");
                HRESULT hr = CreateSafeArray(imageData.data, imageData.width, imageData.height, pVarResult);
                if (FAILED(hr)) {
                    OutputDebugString(L"Failed to create safe array\n");
                }
                return hr;
            }

            case 2: { // loadImage
                OutputDebugString(L"loadImage called\n");
                if (pDispParams->cArgs != 1) {
                    OutputDebugString(L"Invalid argument count\n");
                    return E_INVALIDARG;
                }
                if (pDispParams->rgvarg[0].vt != VT_BSTR) {
                    OutputDebugString(L"Invalid argument type\n");
                    return E_INVALIDARG;
                }

                BSTR filePath = pDispParams->rgvarg[0].bstrVal;
                OutputDebugString(L"Loading image from: ");
                OutputDebugString(filePath);
                OutputDebugString(L"\n");

                bool success = LoadImageFile(filePath);

                if (pVarResult) {
                    pVarResult->vt = VT_BOOL;
                    pVarResult->boolVal = success ? VARIANT_TRUE : VARIANT_FALSE;
                }
                OutputDebugString(success ? L"Image loaded successfully\n" : L"Failed to load image\n");
                return S_OK;
            }

            default:
                OutputDebugString(L"Unknown method called\n");
                return DISP_E_MEMBERNOTFOUND;
            }
        }
        catch (...) {
            OutputDebugString(L"Unhandled exception in Invoke\n");
            return E_FAIL;
        }
    }

};

// WindowProc 实现
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
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void InitializeWebView2()
{
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                env->CreateCoreWebView2Controller(hWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            if (controller != nullptr) {
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
                                ComPtr<IDispatch> imageHandler = Microsoft::WRL::Make<ImageHandler>();
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
                            }
                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
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

    if (hWnd == nullptr) {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    InitializeWebView2();

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}