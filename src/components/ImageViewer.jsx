// ImageViewer.jsx
import { useEffect, useRef } from "react";
import PropTypes from "prop-types";
import * as PIXI from "pixi.js";

const ImageViewer = ({ onImageLoad }) => {
  const pixiContainerRef = useRef(null);
  const pixiAppRef = useRef(null);

  useEffect(() => {
    const initPixiApp = async () => {
      // 清理已存在的应用
      if (pixiAppRef.current) {
        pixiAppRef.current.destroy(true);
        pixiAppRef.current = null;
      }

      if (pixiContainerRef.current) {
        // 清理已存在的 canvas 元素
        while (pixiContainerRef.current.firstChild) {
          pixiContainerRef.current.removeChild(pixiContainerRef.current.firstChild);
        }

        // 创建新的 PixiJS 应用，添加关键配置
        const app = new PIXI.Application();
        await app.init({
          width: pixiContainerRef.current.clientWidth,
          height: pixiContainerRef.current.clientHeight,
          backgroundColor: 0x000000,
          antialias: true,
          clearBeforeRender: true, // 确保在渲染前清除
          powerPreference: "high-performance",
          hello: true // 禁用 PixiJS 启动消息
        });

        // 确保只添加一个 canvas
        if (pixiContainerRef.current.children.length === 0) {
          pixiContainerRef.current.appendChild(app.canvas);
        }
        
        pixiAppRef.current = app;

        const loadAndDisplayImage = async (imageUrl) => {
          try {
            // 清理现有的 sprite（如果存在）
            app.stage.removeChildren();
            
            // 加载并创建新的 sprite
            await PIXI.Assets.load(imageUrl);
            const sprite = PIXI.Sprite.from(imageUrl);
            
            // 设置 sprite 属性
            sprite.width = 384;
            sprite.height = 384;
            sprite.x = app.screen.width / 2;
            sprite.y = app.screen.height / 2;
            sprite.anchor.set(0.5);
            sprite.interactive = true;
            
            app.stage.addChild(sprite);
            
            if (onImageLoad) {
              onImageLoad(sprite);
            }
          } catch (error) {
            console.error('Error loading image:', error);
          }
        };

        const imageUrl = "https://pixijs.com/assets/files/sample-747abf529b135a1f549dff3ec846afbc.png";
        await loadAndDisplayImage(imageUrl);
      }
    };

    initPixiApp().catch(console.error);

    // 清理函数
    return () => {
      if (pixiAppRef.current) {
        pixiAppRef.current.destroy(true, { children: true, texture: true, baseTexture: true });
        pixiAppRef.current = null;
      }
    };
  }, []); // 仅在组件挂载时运行

  return (
    <div
      ref={pixiContainerRef}
      style={{
        width: "100%",
        height: "100%",
        position: "absolute",
        top: 0,
        left: 0,
        overflow: "hidden" // 防止可能的滚动条
      }}
    />
  );
};

ImageViewer.propTypes = {
  onImageLoad: PropTypes.func,
};

export default ImageViewer;