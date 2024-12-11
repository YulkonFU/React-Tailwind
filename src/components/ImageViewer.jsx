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
        });

        pixiAppRef.current = app;
        pixiContainerRef.current.appendChild(app.canvas);
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
    if (pixiAppRef.current) {
      const canvas = pixiAppRef.current.canvas;
      const dataURL = canvas.toDataURL("image/png");

      // 创建一个链接，触发下载
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
      }}
    />
  );
});

ImageViewer.propTypes = {
  onImageLoad: PropTypes.func,
};

export default ImageViewer;
