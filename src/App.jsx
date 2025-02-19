import { useEffect } from "react";
import "./App.css";
import XactLayout from "./layout/XactLayout";

const App = () => {
  useEffect(() => {
    // 检查 WebView2 环境
    if (window.chrome && window.chrome.webview) {
      console.log("WebView2 环境已初始化");

      // 发送消息到后端
      window.chrome.webview.postMessage("Hello from React!");

      // 接收来自后端的消息
      window.chrome.webview.addEventListener("message", (event) => {
        console.log("Message from backend:", event.data);
      });
    } else {
      console.warn("WebView2 环境未初始化");
      // 显示错误信息
      alert("WebView2 环境未初始化，请确保应用运行在 WebView2 环境中。");
    }
  }, []);

  return (
    <div className="App">
      <XactLayout /> {/* 确保 DetectorControl 被正确渲染 */}
    </div>
  );
};

export default App;