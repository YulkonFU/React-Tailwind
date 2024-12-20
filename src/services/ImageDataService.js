class ImageDataService {
  constructor() {
    this.retryCount = 0;
    this.maxRetries = 10;
    this.retryInterval = 500;
    this.setupSharedBufferHandler();
  }

  setupSharedBufferHandler() {
    window.chrome.webview.addEventListener('sharedbufferreceived', async (e) => {
      try {
        if (e.additionalData?.type === 'image') {
          const buffer = e.getBuffer();
          const imageData = new Uint8Array(buffer);
          const width = e.additionalData.width;
          const height = e.additionalData.height;
          
          // Store the received image data
          this.lastReceivedImage = {
            data: imageData,
            width: width,
            height: height
          };

          // Notify any listeners that new image data is available
          if(this.onImageDataReceived) {
            this.onImageDataReceived(imageData, width, height);
          }
        }
      } catch (error) {
        console.error('Error processing shared buffer:', error);
      }
    });
  }

  async waitForImageHandler() {
    return new Promise((resolve, reject) => {
      const checkHandler = () => {
        try {
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
      return await imageHandler.loadImage(filePath);
    } catch (error) {
      console.error("Error loading image:", error);
      throw error;
    }
  }

  async getImageData() {
    // Return the last received image data if available
    if (this.lastReceivedImage) {
      const {data, width, height} = this.lastReceivedImage;
      return [data, width, height];
    }
    throw new Error("No image data available");
  }

  // Register a callback to be notified when new image data is received
  onImageReceived(callback) {
    this.onImageDataReceived = callback;
  }
}

export const imageDataService = new ImageDataService();