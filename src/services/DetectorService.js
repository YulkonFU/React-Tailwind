// services/DetectorService.js
class DetectorService {
  constructor() {
    console.log("DetectorService constructor called"); // 添加调试信息

    this.isInitialized = false;
    this.frameCallback = null;
    this.isProcessingFrame = false;

    // 共享内存相关
    this.sharedMemoryName = "Local\\DetectorImageBuffer";
    this.imageBuffer = null;
    this.displayBuffer = null;

    this.setupMessageListener();
  }

  // DetectorService.js 中的 setupMessageListener 方法
  setupMessageListener() {
    console.log("setupMessageListener called"); // 添加调试信息

    window.chrome?.webview?.addEventListener("message", async (event) => {
      try {
        console.log("Received message:", event.data);
        let message;
        try {
          message = JSON.parse(event.data);
        } catch (e) {
          console.error("Failed to parse message:", event.data, e);
          return;
        }

        if (message.type === "newFrame" && !this.isProcessingFrame) {
          this.isProcessingFrame = true;
          const { width, height, size } = message;

          console.log(`Processing new frame: ${width}x${height}, size=${size}`);

          // 确保缓冲区大小正确
          this.ensureBuffers(width, height);

          try {
            // 读取共享内存数据
            const imageData = await this.readSharedBuffer(width, height);
            if (imageData && this.frameCallback) {
              this.frameCallback(imageData, width, height);
            }
          } catch (error) {
            console.error("Error processing frame:", error);
          } finally {
            this.isProcessingFrame = false;
          }
        }
      } catch (error) {
        console.error("Error in message listener:", error);
        this.isProcessingFrame = false;
      }
    });
  }

  ensureBuffers(width, height) {
    const requiredSize = width * height;

    // 如果缓冲区不存在或大小不对，重新创建
    if (!this.displayBuffer || this.displayBuffer.length !== requiredSize) {
      this.displayBuffer = new Uint8Array(requiredSize);
    }

    // 16位图像数据缓冲区
    if (!this.imageBuffer || this.imageBuffer.length !== requiredSize) {
      this.imageBuffer = new Uint16Array(requiredSize);
    }
  }

  async readSharedBuffer(width, height) {
    try {
      // 检查参数有效性
      if (!width || !height || width <= 0 || height <= 0) {
        throw new Error("Invalid dimensions");
      }

      // 从共享内存读取原始16位数据
      const rawData = await this.fetchSharedMemoryData(width, height);
      if (!rawData) {
        throw new Error("No data received from shared memory");
      }

      // 处理图像数据
      this.processImageData(rawData, this.displayBuffer);

      return this.displayBuffer;
    } catch (error) {
      console.error("Failed to read shared buffer:", error);
      return null;
    }
  }

  async fetchSharedMemoryData(width, height) {
    const size = width * height * 2; // 16-bit = 2 bytes per pixel
    console.log(
      `Attempting to read shared memory: size=${size}, width=${width}, height=${height}`
    );

    try {
      const handler = window.chrome?.webview?.hostObjects?.deviceHandler;
      if (!handler) {
        throw new Error("DeviceHandler not available");
      }

      // 修改为使用属性方式调用，并传递正确的参数
      const name = "Local\\DetectorImageBuffer";
      // 使用 COM 调用语法，不使用 invoke
      handler.ReadSharedMemory = [name, size];

      // 等待异步操作完成
      const arrayBuffer = await handler.ReadSharedMemory;
      if (!arrayBuffer) {
        throw new Error("No data received from shared memory");
      }

      // 转换为 Uint16Array
      console.log("Received array buffer:", arrayBuffer);
      const imageArray = new Uint16Array(arrayBuffer);

      // 输出一些统计信息进行验证
      const min = Math.min(...imageArray);
      const max = Math.max(...imageArray);
      const mean = imageArray.reduce((a, b) => a + b) / imageArray.length;
      console.log(
        `Image statistics: min=${min}, max=${max}, mean=${mean.toFixed(2)}`
      );

      return imageArray;
    } catch (error) {
      console.error("Error in fetchSharedMemoryData:", error);
      throw error; // 重新抛出错误以便上层处理
    }
  }

  processImageData(sourceData, targetBuffer) {
    if (
      !sourceData ||
      !targetBuffer ||
      sourceData.length !== targetBuffer.length
    ) {
      return false;
    }

    // 16位转8位，使用更高效的方法
    for (let i = 0; i < sourceData.length; i++) {
      // 使用位移操作 + 查找表可以进一步优化
      targetBuffer[i] = sourceData[i] >> 8;
    }

    return true;
  }

  async initialize() {
    if (this.isInitialized) {
      return true;
    }

    try {
      const handler = window.chrome?.webview?.hostObjects?.deviceHandler;
      if (!handler) {
        throw new Error("DeviceHandler not available");
      }

      const success = await handler.initializeDetector;
      this.isInitialized = success;
      return success;
    } catch (error) {
      console.error("Failed to initialize detector:", error);
      return false;
    }
  }

  // 控制方法
  async startLive() {
    try {
      const handler = window.chrome?.webview?.hostObjects?.deviceHandler;
      if (!handler) return false;
      return await handler.startLive;
    } catch (error) {
      console.error("Failed to start live:", error);
      return false;
    }
  }

  async stopLive() {
    try {
      const handler = window.chrome?.webview?.hostObjects?.deviceHandler;
      if (!handler) return false;
      return await handler.stopLive;
    } catch (error) {
      console.error("Failed to stop live:", error);
      return false;
    }
  }

  async setGain(gainStep) {
    try {
      const handler = window.chrome?.webview?.hostObjects?.deviceHandler;
      if (!handler || !this.isInitialized) return false;
      handler.gain = gainStep;
      return true;
    } catch (error) {
      console.error("Failed to set gain:", error);
      return false;
    }
  }

  async setFPS(timing) {
    try {
      const handler = window.chrome?.webview?.hostObjects?.deviceHandler;
      if (!handler || !this.isInitialized) return false;
      handler.fps = timing;
      return true;
    } catch (error) {
      console.error("Failed to set FPS:", error);
      return false;
    }
  }

  onNewFrame(callback) {
    this.frameCallback = callback;
  }

  // 清理资源
  dispose() {
    this.frameCallback = null;
    this.imageBuffer = null;
    this.displayBuffer = null;
    this.isInitialized = false;
  }
}

export default DetectorService;