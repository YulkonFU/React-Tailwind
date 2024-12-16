import { useEffect } from "react";
import "./App.css";
import XactLayout from "./layout/XactLayout";

const App = () => {
  useEffect(() => {
    // 检查 WebView2 环境
    if (window.chrome && window.chrome.webview) {
      // 发送消息到后端
      window.chrome.webview.postMessage("Hello from React!");

      // 接收来自后端的消息
      window.chrome.webview.addEventListener("message", (event) => {
        console.log("Message from backend:", event.data);
      });
    } else {
      console.warn("WebView2 环境未初始化");
    }
  }, []);

  return (
    <div className="App">
      <XactLayout />
    </div>
  );
};

export default App;
