#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
#include <string>
#include <vector>
#include <atlenc.h>  // 添加用于 Base64 编码

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

    void InitializeImageData() {
        // 设置图像尺寸
        imageData.width = 640;   // 示例尺寸
        imageData.height = 480;  // 示例尺寸

        // 初始化图像数据
        UINT dataSize = imageData.width * imageData.height * 4;  // RGBA
        imageData.data.resize(dataSize);

        // 生成测试图像
        for (UINT y = 0; y < imageData.height; y++) {
            for (UINT x = 0; x < imageData.width; x++) {
                UINT index = (y * imageData.width + x) * 4;
                float normalizedX = (float)x / imageData.width;
                float normalizedY = (float)y / imageData.height;

                // 创建渐变图案
                imageData.data[index] = (BYTE)(255 * normalizedX);     // R
                imageData.data[index + 1] = (BYTE)(255 * normalizedY); // G
                imageData.data[index + 2] = (BYTE)(255 * (1 - normalizedX)); // B
                imageData.data[index + 3] = 255;                       // A
            }
        }
    }

public:
    ImageHandler() {
        InitializeImageData();
    }

    // IDispatch 方法实现
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override {
        *pctinfo = 0;
        return S_OK;
    }

    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override {
        return E_NOTIMPL;
    }

    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgDispId) override {
        if (cNames == 0 || rgszNames == 0 || rgDispId == 0) {
            return E_INVALIDARG;
        }

        const WCHAR* name = rgszNames[0];
        if (wcscmp(name, L"getImageData") == 0) {
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

            // 将图像数据编码为 Base64 字符串
            int base64Len = Base64EncodeGetRequiredLength(imageData.data.size());
            if (base64Len > 0) {
                std::string base64Str(base64Len, '\0');
                if (Base64Encode(
                    imageData.data.data(),
                    imageData.data.size(),
                    &base64Str[0],
                    &base64Len,
                    ATL_BASE64_FLAG_NOCRLF))
                {
                    // 将 base64Str 转换为 BSTR
                    int wchars_num = MultiByteToWideChar(CP_UTF8, 0, base64Str.c_str(), base64Len, NULL, 0);
                    BSTR bstr = SysAllocStringLen(0, wchars_num);
                    MultiByteToWideChar(CP_UTF8, 0, base64Str.c_str(), base64Len, bstr, wchars_num);

                    // 创建返回对象的 SAFEARRAY
                    SAFEARRAYBOUND bounds[1];
                    bounds[0].lLbound = 0;
                    bounds[0].cElements = 3;  // 三个属性：data, width, height
                    SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 1, bounds);
                    if (!psa) {
                        SysFreeString(bstr);
                        return E_OUTOFMEMORY;
                    }

                    LONG index = 0;
                    VARIANT var;

                    // 设置图像数据（Base64 字符串）
                    VariantInit(&var);
                    var.vt = VT_BSTR;
                    var.bstrVal = bstr;
                    SafeArrayPutElement(psa, &index, &var);

                    // 设置宽度
                    index = 1;
                    VariantInit(&var);
                    var.vt = VT_UI4;
                    var.ulVal = imageData.width;
                    SafeArrayPutElement(psa, &index, &var);

                    // 设置高度
                    index = 2;
                    VariantInit(&var);
                    var.vt = VT_UI4;
                    var.ulVal = imageData.height;
                    SafeArrayPutElement(psa, &index, &var);

                    // 返回结果
                    pVarResult->vt = VT_ARRAY | VT_VARIANT;
                    pVarResult->parray = psa;

                    return S_OK;
                }
            }
            return E_FAIL;
        }

        return DISP_E_MEMBERNOTFOUND;
    }
};

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
            [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                env->CreateCoreWebView2Controller(hWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            if (controller != nullptr)
                            {
                                webViewController = controller;
                                webViewController->get_CoreWebView2(&webViewWindow);
                            }

                            ICoreWebView2Settings* Settings;
                            webViewWindow->get_Settings(&Settings);
                            Settings->put_IsScriptEnabled(TRUE);
                            Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                            Settings->put_IsWebMessageEnabled(TRUE);

                            // 注册图像处理器
                            ComPtr<IDispatch> imageHandler = Microsoft::WRL::Make<ImageHandler>();
                            VARIANT var;
                            VariantInit(&var);
                            var.vt = VT_DISPATCH;
                            var.pdispVal = imageHandler.Get();
                            webViewWindow->AddHostObjectToScript(L"imageHandler", &var);

                            // 导航到本地服务器
                            webViewWindow->Navigate(L"http://localhost:5173/");

                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"WebView2 Example",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    ShowWindow(hWnd, nCmdShow);

    InitializeWebView2();

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}