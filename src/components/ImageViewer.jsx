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

  // 加载图像方法
  const loadImage = useCallback(
    async (imageUrl) => {
      if (pixiAppRef.current) {
        try {
          // 清理现有的 sprite
          pixiAppRef.current.stage.removeChildren();

          // 如果是 Blob URL，先获取图片数据
          const response = await fetch(imageUrl);
          const blob = await response.blob();

          // 使用 createImageBitmap 创建位图
          const imageBitmap = await createImageBitmap(blob);

          // 从位图创建 Texture
          const texture = PIXI.Texture.from(imageBitmap);

          // 创建 sprite
          const sprite = new PIXI.Sprite(texture);

          // 设置 sprite 属性
          sprite.width = 384;
          sprite.height = 384;
          sprite.x = pixiAppRef.current.screen.width / 2;
          sprite.y = pixiAppRef.current.screen.height / 2;
          sprite.anchor.set(0.5);
          sprite.eventMode = "static";

          pixiAppRef.current.stage.addChild(sprite);
          spriteRef.current = sprite;

          if (onImageLoad) {
            onImageLoad(sprite);
          }

          // 清理 Blob URL
          URL.revokeObjectURL(imageUrl);
        } catch (error) {
          console.error("Error loading image:", error);
        }
      }
    },
    [onImageLoad]
  );

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
          resizeTo: pixiContainerRef.current,
          backgroundAlpha: 1, // 确保背景不透明
        });

        pixiAppRef.current = app;
        pixiContainerRef.current.appendChild(app.canvas);
        app.canvas.style.position = "absolute";
        app.canvas.style.top = "0";
        app.canvas.style.left = "0";
        app.canvas.style.zIndex = "2";
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
  }, [loadImage]);

  // 保存图像方法
  const saveImage = () => {
    if (pixiAppRef.current && spriteRef.current) {
      // 确保在保存之前进行一次渲染
      pixiAppRef.current.render();

      const canvas = pixiAppRef.current.canvas;

      // 创建一个临时的画布来保存当前视图
      const tempCanvas = document.createElement("canvas");
      tempCanvas.width = canvas.width;
      tempCanvas.height = canvas.height;

      const ctx = tempCanvas.getContext("2d");

      // 填充黑色背景
      ctx.fillStyle = "#000000";
      ctx.fillRect(0, 0, tempCanvas.width, tempCanvas.height);

      // 将 WebGL canvas 内容绘制到 2D canvas
      ctx.drawImage(canvas, 0, 0);

      // 获取图片数据并触发下载
      const dataURL = tempCanvas.toDataURL("image/png");

      const link = document.createElement("a");
      link.href = dataURL;
      link.download = "image.png";
      link.click();
    }
  };

  // 暴露方法给父组件
  useImperativeHandle(ref, () => ({
    loadImage,
    saveImage,
    pixiAppRef,
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
        zIndex: 2, // 确保 canvas 在最上层
      }}
    />
  );
});

ImageViewer.propTypes = {
  onImageLoad: PropTypes.func,
};

export default ImageViewer;
