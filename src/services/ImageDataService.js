// services/ImageDataService.js
class ImageDataService {
    constructor() {
      this.retryCount = 0;
      this.maxRetries = 10;
      this.retryInterval = 500;
    }
  
    async waitForImageHandler() {
      return new Promise((resolve, reject) => {
        const checkHandler = () => {
          try {
            // 获取同步代理对象
            const handler = window.chrome?.webview?.hostObjects?.sync?.imageHandler;
            if (handler) {
              resolve(handler);
            } else if (this.retryCount < this.maxRetries) {
              this.retryCount++;
              setTimeout(checkHandler, this.retryInterval);
            } else {
              reject(new Error("WebView2 image handler not available after retries"));
            }
          } catch (error) {
            if (this.retryCount < this.maxRetries) {
              this.retryCount++;
              setTimeout(checkHandler, this.retryInterval);
            } else {
              reject(error);
            }
          }
        };
        checkHandler();
      });
    }
  
    async loadImage(filePath) {
      try {
        const imageHandler = await this.waitForImageHandler();
        console.log("Calling loadImage with path:", filePath);
        return await imageHandler.loadImage(filePath);
      } catch (error) {
        console.error("Error loading image:", error);
        throw error;
      }
    }
  
    async getImageData() {
      try {
        const imageHandler = await this.waitForImageHandler();
        console.log("Image handler found:", imageHandler);
  
        // 直接调用 getImageData 方法
        const rawResult = await imageHandler.getImageData();
        const result = await rawResult;  // 确保解析完成
        
        console.log("Raw image data received:", result);
  
        if (result && Array.isArray(result) && result.length >= 3) {
          const base64Str = result[0];
          const binary = atob(base64Str);
          const bytes = new Uint8Array(binary.length);
          for (let i = 0; i < binary.length; i++) {
            bytes[i] = binary.charCodeAt(i);
          }
  
          const width = result[1];
          const height = result[2];
  
          console.log(`Decoded image data: ${width}x${height}, length: ${bytes.length}`);
          return [bytes, width, height];
        }
        
        throw new Error("Invalid image data format received");
      } catch (error) {
        console.error("Error in getImageData:", error);
        throw error;
      }
    }
  }
  
  export const imageDataService = new ImageDataService();