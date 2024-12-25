#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>
#include <string>
#include <vector>
#include <atlenc.h>
#include <wincrypt.h>
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")

// 添加库链接
#pragma comment(lib, "crypt32.lib")

using namespace Microsoft::WRL;

HWND hWnd;
wil::com_ptr<ICoreWebView2Controller> webViewController;
wil::com_ptr<ICoreWebView2> webViewWindow;

// 图像数据结构
struct ImageData {
    std::vector<BYTE> data;
    UINT width;
    UINT height;

    ImageData() : width(0), height(0) {} // Initialize width and height
};

// ImageHandler 类定义
class ImageHandler : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IDispatch> {
private:
    ImageData imageData;

    // 将 std::string 转换为 std::wstring
    std::wstring ConvertToWString(const std::string& str) {
        if (str.empty()) return std::wstring();
        int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size);
        return result;
    }

    // 初始化图像数据
    void InitializeImageData() {
        imageData.width = 640;
        imageData.height = 480;
        UINT dataSize = imageData.width * imageData.height * 4;  // RGBA
        imageData.data.resize(dataSize);

        // 生成渐变图像
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

    // 使用 ATL 的 Base64 编码替代 CryptBinaryToString
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
        // 使用 ATL Base64 编码
        std::string base64Str;
        HRESULT hr = EncodeBase64(data, base64Str);
        if (FAILED(hr)) return hr;

        // 创建SAFEARRAY
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

            // 设置返回值
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
        InitializeImageData();
    }

    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override {
        *pctinfo = 0;
        return S_OK;
    }

    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override {
        return E_NOTIMPL;
    }

    // 获取图像数据
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgDispId) override {
        if (!rgszNames || !rgDispId) return E_INVALIDARG;

        if (cNames == 1 && wcscmp(rgszNames[0], L"getImageData") == 0) {
            rgDispId[0] = 1;
            return S_OK;
        }

        return DISP_E_UNKNOWNNAME;
    }

    // 实现 getImageData 方法
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
        EXCEPINFO* pExcepInfo, UINT* puArgErr) override
    {
        if (dispIdMember == 1) { // getImageData
            if (!pVarResult) return E_POINTER;
            return CreateSafeArray(imageData.data, imageData.width, imageData.height, pVarResult);
        }
        return DISP_E_MEMBERNOTFOUND;
    }
};

// WindowProc 实现保持不变
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

// 初始化WebView2
void InitializeWebView2()
{
    // 创建 WebView2 环境选项
    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();

    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, options.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                env->CreateCoreWebView2Controller(hWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            if (controller != nullptr) {
                                webViewController = controller;
                                webViewController->get_CoreWebView2(&webViewWindow);

                                // 配置WebView2设置
                                Microsoft::WRL::ComPtr<ICoreWebView2Settings> settings;
                                webViewWindow->get_Settings(&settings);
                                settings->put_IsScriptEnabled(TRUE);
                                settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                                settings->put_IsWebMessageEnabled(TRUE);
                                settings->put_AreDevToolsEnabled(TRUE);

                                // 配置高级设置
                                Microsoft::WRL::ComPtr<ICoreWebView2Settings6> settings6;
                                if (SUCCEEDED(settings->QueryInterface(IID_PPV_ARGS(&settings6)))) {
                                    settings6->put_IsPinchZoomEnabled(FALSE);
                                }

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

// 创建窗口
// 修改主函数，添加 DPI 感知
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    // 设置 DPI 感知
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // 初始化 COM
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    const wchar_t CLASS_NAME[] = L"XRay Viewer Window";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClass(&wc);

    // 获取当前显示器的 DPI
    UINT dpi = GetDpiForSystem();
    float scale = static_cast<float>(dpi) / 96.0f;

    // 创建窗口时考虑 DPI 缩放
    hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"XRay Image Viewer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        static_cast<int>(1024 * scale),
        static_cast<int>(768 * scale),
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

    // 清理 COM
    CoUninitialize();

    return 0;
}
