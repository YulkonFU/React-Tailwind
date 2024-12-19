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
          // 使用完整路径访问 imageHandler
          const handler =
            window.chrome?.webview?.hostObjects?.sync?.imageHandler;
          if (handler) {
            resolve(handler);
          } else if (this.retryCount < this.maxRetries) {
            this.retryCount++;
            setTimeout(checkHandler, this.retryInterval);
          } else {
            reject(
              new Error("WebView2 image handler not available after retries")
            );
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

  async getImageData() {
    try {
      const imageHandler = await this.waitForImageHandler();
      console.log("Image handler found:", imageHandler);

      // 使用特定的调用方式
      const result = await new Promise((resolve, reject) => {
        try {
          // 使用 proxy 对象的方法调用
          const data = imageHandler.getImageData;
          resolve(data);
        } catch (error) {
          reject(error);
        }
      });

      console.log("Got image data:", result);
      return result;
    } catch (error) {
      console.error("Error in getImageData:", error);
      throw error;
    }
  }
}

export const imageDataService = new ImageDataService();
