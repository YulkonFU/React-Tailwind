#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
#include <string>
#include <vector>

using namespace Microsoft::WRL;

HWND hWnd;
wil::com_ptr<ICoreWebView2Controller> webViewController;
wil::com_ptr<ICoreWebView2> webViewWindow;

// ImageHandler 类定义
class ImageHandler : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IDispatch> {
private:
    std::vector<BYTE> imageBuffer;

public:
    ImageHandler() {}

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
        else if (wcscmp(name, L"sendImageData") == 0) {
            rgDispId[0] = 2;
            return S_OK;
        }

        return DISP_E_UNKNOWNNAME;
    }

    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
        EXCEPINFO* pExcepInfo, UINT* puArgErr) override
    {
        switch (dispIdMember) {
        case 1: // getImageData
            if (pDispParams->cArgs == 2) { // width, height
                UINT width = pDispParams->rgvarg[1].ulVal;
                UINT height = pDispParams->rgvarg[0].ulVal;

                // 生成测试图像数据（红色渐变示例）
                UINT dataSize = width * height * 4;  // RGBA
                imageBuffer.resize(dataSize);
                for (UINT y = 0; y < height; y++) {
                    for (UINT x = 0; x < width; x++) {
                        UINT index = (y * width + x) * 4;
                        float gradientX = (float)x / width;
                        float gradientY = (float)y / height;
                        imageBuffer[index] = (BYTE)(255 * gradientX);    // R
                        imageBuffer[index + 1] = 0;                      // G
                        imageBuffer[index + 2] = (BYTE)(255 * gradientY);// B
                        imageBuffer[index + 3] = 255;                    // A
                    }
                }

                // 创建返回的数组
                SAFEARRAY* psa = SafeArrayCreateVector(VT_UI1, 0, dataSize);
                if (psa == NULL) return E_OUTOFMEMORY;

                void* pData;
                HRESULT hr = SafeArrayAccessData(psa, &pData);
                if (SUCCEEDED(hr)) {
                    memcpy(pData, imageBuffer.data(), dataSize);
                    SafeArrayUnaccessData(psa);

                    if (pVarResult) {
                        pVarResult->vt = VT_ARRAY | VT_UI1;
                        pVarResult->parray = psa;
                    }
                    return S_OK;
                }
                else {
                    SafeArrayDestroy(psa);
                    return hr;
                }
            }
            return E_INVALIDARG;

        case 2: // sendImageData
            if (pDispParams->cArgs >= 1 &&
                pDispParams->rgvarg[0].vt == (VT_ARRAY | VT_UI1)) {
                SAFEARRAY* psa = pDispParams->rgvarg[0].parray;
                LONG lbound, ubound;
                SafeArrayGetLBound(psa, 1, &lbound);
                SafeArrayGetUBound(psa, 1, &ubound);
                UINT size = ubound - lbound + 1;

                void* pData;
                HRESULT hr = SafeArrayAccessData(psa, &pData);
                if (SUCCEEDED(hr)) {
                    imageBuffer.resize(size);
                    memcpy(imageBuffer.data(), pData, size);
                    SafeArrayUnaccessData(psa);
                    return S_OK;
                }
                return hr;
            }
            return E_INVALIDARG;

        default:
            return DISP_E_MEMBERNOTFOUND;
        }
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
                            var.vt = VT_DISPATCH;
                            var.pdispVal = imageHandler.Get();
                            webViewWindow->AddHostObjectToScript(L"imageHandler", &var);

                            // 导航到本地服务器
                            webViewWindow->Navigate(L"http://localhost:5173/");

                            // 注册消息处理
                            EventRegistrationToken token;
                            webViewWindow->add_WebMessageReceived(
                                Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                        wil::unique_cotaskmem_string message;
                                        HRESULT hr = args->TryGetWebMessageAsString(&message);
                                        if (SUCCEEDED(hr)) {
                                            std::wstring messageW(message.get());
                                            std::string messageA(messageW.begin(), messageW.end());
                                            MessageBoxA(nullptr, messageA.c_str(), "Message from React", MB_OK);
                                        }
                                        return S_OK;
                                    }).Get(), &token);

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