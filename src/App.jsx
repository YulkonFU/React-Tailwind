// App.jsx
import { useEffect } from "react";
import { BrowserRouter as Router, Routes, Route } from "react-router-dom";
import XactLayout from "./layout/XactLayout";
import GridPage from "./layout/GridPage";
import { ImageProvider } from "./components/ImageContext"; // 引入 ImageProvider

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
    <ImageProvider> {/* 包裹 ImageProvider */}
      <Router>
        <div className="App">
          <Routes>
            <Route path="/" element={<XactLayout />} />
            <Route path="/grid" element={<GridPage />} />
          </Routes>
          <h1>Hello, WebView2!</h1>
        </div>
      </Router>
    </ImageProvider>
  );
};

export default App;