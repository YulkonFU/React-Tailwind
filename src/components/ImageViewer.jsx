// ImageViewer.jsx
import {
  useEffect,
  useRef,
  forwardRef,
  useImperativeHandle,
  useCallback,
} from "react";
import PropTypes from "prop-types";
import * as PIXI from "pixi.js";

const ImageViewer = forwardRef(function ImageViewer({ onImageLoad }, ref) {
  const pixiContainerRef = useRef(null);
  const pixiAppRef = useRef(null);
  const spriteRef = useRef(null);
  const currentImageDataRef = useRef(null);

  const calculateFitScale = (
    imageWidth,
    imageHeight,
    containerWidth,
    containerHeight
  ) => {
    const scaleX = containerWidth / imageWidth;
    const scaleY = containerHeight / imageHeight;
    return Math.min(scaleX, scaleY) * 0.9;
  };

  // 加载图像方法
  const loadImage = useCallback(
    async (imageFile) => {
      if (!pixiAppRef.current) return;

      try {
        // 清理现有的 sprite
        pixiAppRef.current.stage.removeChildren();

        // 创建图片 URL
        let imageUrl;
        if (imageFile instanceof File) {
          imageUrl = URL.createObjectURL(imageFile);
        } else if (imageFile instanceof Blob) {
          imageUrl = URL.createObjectURL(imageFile);
        } else if (typeof imageFile === "string") {
          imageUrl = imageFile;
        } else {
          console.error("Invalid image input");
          return;
        }

        // 加载图片
        const image = new Image();
        await new Promise((resolve, reject) => {
          image.onload = resolve;
          image.onerror = reject;
          image.src = imageUrl;
        });

        // 创建纹理和精灵
        const texture = PIXI.Texture.from(image);
        const sprite = new PIXI.Sprite(texture);

        // 等待纹理加载完成
        if (!texture.valid) {
          await new Promise((resolve) => texture.once("update", resolve));
        }

        // 计算初始缩放
        const initialScale = calculateFitScale(
          texture.width,
          texture.height,
          pixiAppRef.current.screen.width,
          pixiAppRef.current.screen.height
        );

        sprite.scale.set(initialScale);
        sprite.x = pixiAppRef.current.screen.width / 2;
        sprite.y = pixiAppRef.current.screen.height / 2;
        sprite.anchor.set(0.5);
        sprite.eventMode = "static";
        sprite.initialScale = initialScale;

        pixiAppRef.current.stage.addChild(sprite);
        spriteRef.current = sprite;

        // 强制渲染
        pixiAppRef.current.render();

        if (onImageLoad) {
          onImageLoad(sprite);
        }

        // 清理 URL
        if (imageUrl.startsWith("blob:")) {
          URL.revokeObjectURL(imageUrl);
        }

        // 保存当前图片数据
        currentImageDataRef.current = imageFile;
      } catch (error) {
        console.error("Error loading image:", error);
      }
    },
    [onImageLoad, calculateFitScale]
  );
  
  // 添加保存图像方法
  const saveImage = useCallback(() => {
    if (!pixiAppRef.current || !pixiAppRef.current.canvas) {
      console.error("No canvas available to save");
      return;
    }

    try {
      // 创建一个临时 canvas 来保存当前视图
      const canvas = document.createElement("canvas");
      canvas.width = pixiAppRef.current.canvas.width;
      canvas.height = pixiAppRef.current.canvas.height;
      const ctx = canvas.getContext("2d");

      // 设置黑色背景
      ctx.fillStyle = "#000000";
      ctx.fillRect(0, 0, canvas.width, canvas.height);

      // 将 WebGL canvas 内容绘制到 2D canvas
      ctx.drawImage(pixiAppRef.current.canvas, 0, 0);

      // 创建下载链接
      const link = document.createElement("a");
      canvas.toBlob((blob) => {
        const url = URL.createObjectURL(blob);
        link.href = url;
        link.download = "image.png";
        link.click();

        // 清理
        setTimeout(() => {
          URL.revokeObjectURL(url);
          canvas.remove();
        }, 100);
      }, "image/png");
    } catch (error) {
      console.error("Error saving image:", error);
    }
  }, []);

  // 更新 useImperativeHandle，添加 saveImage 方法
  useImperativeHandle(ref, () => ({
    loadImage,
    getCurrentImageData,
    saveImage, // 添加保存方法
    pixiAppRef,
  }));

  // 获取当前图片数据
  const getCurrentImageData = useCallback(() => {
    return currentImageDataRef.current;
  }, []);

  // 初始化 PIXI 应用
  useEffect(() => {
    const initPixiApp = async () => {
      if (pixiAppRef.current) {
        pixiAppRef.current.destroy(true);
        pixiAppRef.current = null;
      }

      if (pixiContainerRef.current) {
        while (pixiContainerRef.current.firstChild) {
          pixiContainerRef.current.removeChild(
            pixiContainerRef.current.firstChild
          );
        }

        const app = new PIXI.Application();
        await app.init({
          width: pixiContainerRef.current.clientWidth,
          height: pixiContainerRef.current.clientHeight,
          backgroundColor: 0x000000,
          antialias: true,
          clearBeforeRender: true,
          powerPreference: "high-performance",
          hello: true,
        });

        pixiContainerRef.current.appendChild(app.canvas);
        pixiAppRef.current = app;
      }
    };

    initPixiApp();

    return () => {
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
      }}
    />
  );
});

ImageViewer.propTypes = {
  onImageLoad: PropTypes.func,
};

export default ImageViewer;
