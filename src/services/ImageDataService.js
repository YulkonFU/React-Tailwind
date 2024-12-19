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
  
    base64ToUint8Array(base64) {
      const binaryString = atob(base64);
      const len = binaryString.length;
      const bytes = new Uint8Array(len);
      for (let i = 0; i < len; i++) {
        bytes[i] = binaryString.charCodeAt(i);
      }
      return bytes;
    }
  
    async getImageData() {
      try {
        const imageHandler = await this.waitForImageHandler();
        console.log("Image handler found:", imageHandler);
  
        // 正确的WebView2异步调用方式
        const result = await window.chrome.webview.hostObjects.imageHandler.getImageData;
        console.log("Raw image data received:", result);
  
        if (result && Array.isArray(result) && result.length >= 3) {
          // 将Base64字符串转换为Uint8Array
          const imageData = this.base64ToUint8Array(result[0]);
          const width = result[1];
          const height = result[2];
  
          return [imageData, width, height];
        } else {
          throw new Error("Invalid image data format received");
        }
      } catch (error) {
        console.error("Error in getImageData:", error);
        throw error;
      }
    }
  }
  
  export const imageDataService = new ImageDataService();