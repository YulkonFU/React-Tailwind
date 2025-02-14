// services/DetectorService.js
class DetectorService {
    constructor() {
        this.isInitialized = false;
        this.frameCallback = null;
        this.sharedBufferSize = 0;
        this.sharedArrayBuffer = null;
        this.setupMessageListener();
    }

    setupMessageListener() {
        window.chrome?.webview?.addEventListener('message', async (event) => {
            const message = JSON.parse(event.data);
            if (message.type === 'newFrame') {
                // 收到新帧通知,从共享内存读取
                const imageData = await this.readSharedBuffer(message.width, message.height);
                if (this.frameCallback && imageData) {
                    this.frameCallback(imageData, message.width, message.height);
                }
            }
        });
    }

    async initialize() {
        if (!this.isInitialized) {
            const handler = window.chrome?.webview?.hostObjects?.deviceHandler;
            if (!handler) return false;
            
            await handler.initializeDetector;
            this.isInitialized = true;
            return true;
        }
        return false;
    }

    async readSharedBuffer(width, height) {
        try {
            // 使用 SharedArrayBuffer 打开共享内存映射
            const size = width * height * 2; // 16-bit = 2 bytes per pixel
            if (size !== this.sharedBufferSize) {
                this.sharedArrayBuffer = new SharedArrayBuffer(size);
                this.sharedBufferSize = size;
            }

            // 从共享内存读取数据
            const view = new Uint16Array(this.sharedArrayBuffer);
            
            // 将数据转换为适合显示的格式
            // 假设是16位灰度图,我们需要转换为8位显示
            const displayData = new Uint8Array(width * height);
            for (let i = 0; i < view.length; i++) {
                // 16位转8位,保持相对比例
                displayData[i] = (view[i] >> 8) & 0xFF;
            }

            return displayData;
        } catch (error) {
            console.error('Failed to read shared buffer:', error);
            return null;
        }
    }

    async startLive() {
        const handler = window.chrome?.webview?.hostObjects?.deviceHandler;
        if (!handler) return false;
        return await handler.startLive;
    }

    async stopLive() {
        const handler = window.chrome?.webview?.hostObjects?.deviceHandler;
        if (!handler) return false;
        return await handler.stopLive;
    }

    async setGain(gainStep) {
        const handler = window.chrome?.webview?.hostObjects?.deviceHandler;
        if (!handler) return false;
        return await handler.setGain(gainStep);
    }

    async setFPS(timing) {
        const handler = window.chrome?.webview?.hostObjects?.deviceHandler;
        if (!handler) return false;
        return await handler.setFPS(timing);
    }

    onNewFrame(callback) {
        this.frameCallback = callback;
    }
}

export const detectorService = new DetectorService();