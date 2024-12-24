import {
  useEffect,
  useRef,
  forwardRef,
  useState,
  useImperativeHandle,
} from "react";
import * as PIXI from "pixi.js";
import { imageDataService } from "../services/ImageDataService";
import OverlayCanvas from "./OverlayCanvas";

const ImageViewer = forwardRef((props, ref) => {
  const pixiContainerRef = useRef(null);
  const pixiAppRef = useRef(null);
  const spriteRef = useRef(null);
  const overlayRef = useRef(null);
  const initializationTimeout = useRef(null);
  const [showOverlay, setShowOverlay] = useState(true);

  useImperativeHandle(ref, () => ({
    saveImage: (options = { includeOverlay: true, filename: "image.png" }) => {
      const canvas = document.createElement("canvas");
      canvas.width = pixiAppRef.current.screen.width;
      canvas.height = pixiAppRef.current.screen.height;
      const ctx = canvas.getContext("2d");

      // 绘制基础图像
      ctx.drawImage(pixiAppRef.current.view, 0, 0);

      // 如果需要包含 overlay
      if (options.includeOverlay && showOverlay) {
        ctx.drawImage(overlayRef.current.getCanvas(), 0, 0);
      }

      // 创建下载链接
      const link = document.createElement("a");
      link.download = options.filename;
      link.href = canvas.toDataURL();
      link.click();
    },
    setDrawingTool: (tool) => {
      if (overlayRef.current) {
        overlayRef.current.setTool(tool);
      }
    },
    toggleOverlay: (show) => {
      setShowOverlay(show);
    },
    clearOverlay: () => {
      if (overlayRef.current) {
        overlayRef.current.clearCanvas();
      }
    },
  }));

  useEffect(() => {
    const initPixiApp = async () => {
      try {
        if (pixiAppRef.current) {
          pixiAppRef.current.destroy(true);
          pixiAppRef.current = null;
        }

        if (pixiContainerRef.current) {
          console.log("Initializing PIXI application...");

          const app = new PIXI.Application();
          await app.init({
            width: pixiContainerRef.current.clientWidth,
            height: pixiContainerRef.current.clientHeight,
            backgroundColor: 0x000000,
            antialias: true,
            clearBeforeRender: true,
            powerPreference: "high-performance",
            hello: true,
            resizeTo: pixiContainerRef.current,
          });

          pixiAppRef.current = app;
          pixiContainerRef.current.appendChild(app.canvas);

          console.log("Getting image data...");
          const [imageData, width, height] =
            await imageDataService.getImageData();
          console.log(
            `Received image data: ${width}x${height}, data length: ${imageData.length}`
          );

          if (imageData && width && height) {
            const canvas = document.createElement("canvas");
            canvas.width = width;
            canvas.height = height;
            const ctx = canvas.getContext("2d");

            // 创建正确的ImageData对象
            const imgData = new ImageData(
              new Uint8ClampedArray(imageData.buffer),
              width,
              height
            );

            ctx.putImageData(imgData, 0, 0);

            // 创建PIXI纹理和精灵
            const texture = PIXI.Texture.from(canvas);
            const sprite = new PIXI.Sprite(texture);

            // 计算缩放以适应容器
            const scaleX = app.screen.width / width;
            const scaleY = app.screen.height / height;
            const scale = Math.min(scaleX, scaleY) * 0.9;

            sprite.scale.set(scale);
            sprite.x = app.screen.width / 2;
            sprite.y = app.screen.height / 2;
            sprite.anchor.set(0.5);

            // 启用交互
            sprite.interactive = true;
            sprite.cursor = "pointer";

            app.stage.addChild(sprite);
            spriteRef.current = sprite;
            console.log("Sprite successfully added to stage");
          }
        }
      } catch (error) {
        console.error("Error initializing PixiJS:", error);
        if (!initializationTimeout.current) {
          console.log("Scheduling retry...");
          initializationTimeout.current = setTimeout(() => {
            console.log("Retrying initialization...");
            initializationTimeout.current = null;
            initPixiApp();
          }, 1000);
        }
      }
    };

    initPixiApp();

    return () => {
      if (initializationTimeout.current) {
        clearTimeout(initializationTimeout.current);
      }
      if (pixiAppRef.current) {
        pixiAppRef.current.destroy(true, {
          children: true,
          texture: true,
          baseTexture: true,
        });
        pixiAppRef.current = null;
      }
    };
  }, []);

  return (
    <div
      ref={pixiContainerRef}
      style={{
        width: "100%",
        height: "100%",
        position: "absolute",
        top: 0,
        left: 0,
        overflow: "hidden",
        zIndex: 2,
        backgroundColor: "#000",
      }}
    >
      <OverlayCanvas
        ref={overlayRef}
        width={pixiContainerRef.current?.clientWidth}
        height={pixiContainerRef.current?.clientHeight}
        visible={showOverlay}
      />
    </div>
  );
});

ImageViewer.displayName = "ImageViewer";

export default ImageViewer;
