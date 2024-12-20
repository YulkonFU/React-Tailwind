import React, { useEffect, useRef, forwardRef } from "react";
import * as PIXI from "pixi.js";
import { imageDataService } from "../services/ImageDataService";

const ImageViewer = forwardRef((props, ref) => {
  const pixiContainerRef = useRef(null);
  const pixiAppRef = useRef(null);
  const spriteRef = useRef(null);
  const initializationTimeout = useRef(null);

  const loadAndDisplayImage = async () => {
    try {
      console.log("Getting image data...");
      // 等待图像数据
      const [imageData, width, height] = await imageDataService.getImageData();
      console.log(`Received image data: ${width}x${height}, length: ${imageData.length}`);

      if (!pixiAppRef.current || !imageData || !width || !height) {
        throw new Error("Invalid image data or PIXI app not initialized");
      }

      // 创建临时 canvas 并绘制图像数据
      const canvas = document.createElement("canvas");
      canvas.width = width;
      canvas.height = height;
      const ctx = canvas.getContext("2d");

      // 创建 ImageData 对象并绘制到 canvas
      const imgData = new ImageData(new Uint8ClampedArray(imageData), width, height);
      ctx.putImageData(imgData, 0, 0);

      // 如果存在旧的精灵，先移除它
      if (spriteRef.current) {
        pixiAppRef.current.stage.removeChild(spriteRef.current);
        spriteRef.current.destroy({ children: true, texture: true, baseTexture: true });
      }

      // 创建新的纹理和精灵
      const texture = PIXI.Texture.from(canvas);
      const sprite = new PIXI.Sprite(texture);

      // 计算适应屏幕的缩放比例
      const scaleX = pixiAppRef.current.screen.width / width;
      const scaleY = pixiAppRef.current.screen.height / height;
      const scale = Math.min(scaleX, scaleY) * 0.9;

      // 设置精灵属性
      sprite.scale.set(scale);
      sprite.x = pixiAppRef.current.screen.width / 2;
      sprite.y = pixiAppRef.current.screen.height / 2;
      sprite.anchor.set(0.5);
      sprite.interactive = true;
      sprite.cursor = "pointer";

      // 添加精灵到舞台
      pixiAppRef.current.stage.addChild(sprite);
      spriteRef.current = sprite;

      console.log("Image successfully loaded and displayed");
      return true;
    } catch (error) {
      console.error("Error loading image:", error);
      return false;
    }
  };

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

          // 添加延迟等待初始化完成
          setTimeout(async () => {
            const success = await loadAndDisplayImage();
            if (!success && !initializationTimeout.current) {
              console.log("Scheduling retry...");
              initializationTimeout.current = setTimeout(() => {
                console.log("Retrying initialization...");
                initializationTimeout.current = null;
                initPixiApp();
              }, 1000);
            }
          }, 500);
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

  // 暴露加载新图像的方法
  React.useImperativeHandle(ref, () => ({
    loadImage: async (filePath) => {
      try {
        const result = await imageDataService.loadImage(filePath);
        if (result) {
          return await loadAndDisplayImage();
        }
        return false;
      } catch (error) {
        console.error("Error loading new image:", error);
        return false;
      }
    }
  }));

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
    />
  );
});

ImageViewer.displayName = "ImageViewer";

export default ImageViewer;