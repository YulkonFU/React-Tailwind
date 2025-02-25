



Detector的主要流程：<img src="C:\Users\fuyuk\Downloads\Detector流程图.png" alt="Detector流程图" style="zoom: 25%;" />

1. #### 初始化流程（InitializeDetector）：
   
   - 获取DigGrabber实例：使用GetAndInitDigGrabber()
   - 获取图像尺寸：GetActGrabbedImageSize()
   - 创建共享内存：CreateSharedBuffer()用于图像数据传输
   - 定义目标缓冲区：DefineDestBuffer()设置图像数据存储区
- 设置回调：SetCallbacksAndMessages()注册帧回调函数
   
2. #### 实时采集流程（StartLive）：
   
   - 设置上下文：SetAcqData()传递DeviceHandler实例
   - 开始采集：AcquireImage()以连续模式采集
   - 帧回调处理：
     - EndFrameCallback在新帧到达时被调用
     - 将图像数据复制到共享内存
  - 通过WebView2发送消息通知前端
   
3. #### 数据流：
   
   ```
   Detector硬件 -> DigGrabber -> 目标缓冲区 -> 共享内存 -> 前端UI
```
   
4. #### 前端与后端交互：
   
   - 前端通过WebView2调用后端方法
   - 后端通过回调和消息机制通知前端
- 图像数据通过共享内存传输
   
   
   
