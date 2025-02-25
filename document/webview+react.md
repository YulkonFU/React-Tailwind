要在前端使用 React 嵌入 WebView2，并在后端使用 C++ 实现前后端通信，可以使用 WebView2 提供的 `postMessage` 和 `addEventListener` 方法来实现。以下是详细步骤：

### 步骤 1: 创建 React 项目

首先，确保你的 React 项目已经构建好，并生成了静态文件。你可以使用以下命令来构建项目：

```bash
npm run build
```

这将在 `dist` 目录中生成构建后的静态文件。

### 步骤 2: 创建 WebView2 项目

#### 使用 C++ 和 Win32 创建 WebView2 项目

1. 打开 Visual Studio，创建一个新的 Win32 应用程序项目。
2. 安装 WebView2 NuGet 包：

```bash
Install-Package Microsoft.Web.WebView2
```

#### 修改 C++ 项目文件

1. 在项目的头文件中包含 WebView2 的头文件：

```cpp
#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
```

2. 初始化 WebView2 控件并加载 React 应用：

```cpp
#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
#include <string>

using namespace Microsoft::WRL;

HWND hWnd;
wil::com_ptr<ICoreWebView2Controller> webViewController;
wil::com_ptr<ICoreWebView2> webViewWindow;

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
                env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
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

                        webViewWindow->AddScriptToExecuteOnDocumentCreated(L"window.chrome.webview.addEventListener('message', event => { console.log(event.data); });", nullptr);
                        webViewWindow->Navigate(L"file:///C:/path/to/your/react-app/dist/index.html");

                        EventRegistrationToken token;
                        webViewWindow->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                            [](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                wil::unique_cotaskmem_string message;
                                args->get_WebMessageAsString(&message);
                                std::wstring messageW(message.get());
                                std::string messageA(messageW.begin(), messageW.end());
                                // 处理来自前端的消息
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

    hWnd = CreateWindowEx(0, CLASS_NAME, L"WebView2 Example", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInstance, nullptr);

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
```

### 步骤 3: 前端 React 项目中实现通信

在 React 项目中，可以使用 `window.chrome.webview.postMessage` 向后端发送消息，并使用 `window.chrome.webview.addEventListener` 接收来自后端的消息。

#### 发送消息到后端

```jsx
import React, { useEffect } from 'react';

const App = () => {
  useEffect(() => {
    // 发送消息到后端
    window.chrome.webview.postMessage('Hello from React!');
  }, []);

  return (
    <div>
      <h1>Hello, WebView2!</h1>
    </div>
  );
};

export default App;
```

#### 接收来自后端的消息

```jsx
import React, { useEffect } from 'react';

const App = () => {
  useEffect(() => {
    // 接收来自后端的消息
    window.chrome.webview.addEventListener('message', event => {
      console.log('Message from backend:', event.data);
    });
  }, []);

  return (
    <div>
      <h1>Hello, WebView2!</h1>
    </div>
  );
};

export default App;
```

### 注意事项

1. **路径问题**：确保 

index.html

 文件的路径正确。你可以将 React 项目的构建输出复制到 WebView2 项目的目录中，或者使用绝对路径。
2. **消息格式**：确保前后端发送和接收的消息格式一致，可以使用 JSON 格式进行消息传递。

通过这些步骤，你可以实现前端 React 嵌入 WebView2，并通过 `postMessage` 和 `addEventListener` 方法实现与后端 C++ 的通信。

找到具有 1 个许可证类型的类似代码