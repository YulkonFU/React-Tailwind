#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
#include <string>
#include <vector>
#include <atlenc.h>
#include <wincrypt.h>

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

    void InitializeImageData() {
        imageData.width = 640;
        imageData.height = 480;
        UINT dataSize = imageData.width * imageData.height * 4;  // RGBA
        imageData.data.resize(dataSize);

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