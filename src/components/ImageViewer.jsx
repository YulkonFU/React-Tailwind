import React, { useEffect, useRef, forwardRef } from 'react';
import * as PIXI from 'pixi.js';
import { imageDataService } from '../services/ImageDataService';

const ImageViewer = forwardRef((props, ref) => {
  const pixiContainerRef = useRef(null);
  const pixiAppRef = useRef(null);
  const spriteRef = useRef(null);
  const initializationTimeout = useRef(null);

  useEffect(() => {
    const initPixiApp = async () => {
      try {
        if (pixiAppRef.current) {
          pixiAppRef.current.destroy(true);
          pixiAppRef.current = null;
        }

        if (pixiContainerRef.current) {
          console.log('Initializing PIXI application...');  // 调试日志
          
          const app = new PIXI.Application();
          await app.init({
            width: pixiContainerRef.current.clientWidth,
            height: pixiContainerRef.current.clientHeight,
            backgroundColor: 0x000000,
            antialias: true,
            clearBeforeRender: true,
            powerPreference: 'high-performance',
            hello: true,
            resizeTo: pixiContainerRef.current,
          });

          pixiAppRef.current = app;
          pixiContainerRef.current.appendChild(app.canvas);

          console.log('Getting image data...');  // 调试日志
          const imageResult = await imageDataService.getImageData();
          console.log('Got image result:', imageResult);  // 调试日志

          if (imageResult && Array.isArray(imageResult) && imageResult.length >= 3) {
            const imageData = new Uint8Array(imageResult[0]);
            const width = imageResult[1];
            const height = imageResult[2];

            console.log(`Creating canvas with dimensions: ${width}x${height}`);  // 调试日志

            const canvas = document.createElement('canvas');
            canvas.width = width;
            canvas.height = height;
            const ctx = canvas.getContext('2d');
            const imgData = new ImageData(
              new Uint8ClampedArray(imageData),
              width,
              height
            );
            ctx.putImageData(imgData, 0, 0);

            const texture = PIXI.Texture.from(canvas);
            const sprite = new PIXI.Sprite(texture);

            const scaleX = app.screen.width / width;
            const scaleY = app.screen.height / height;
            const scale = Math.min(scaleX, scaleY) * 0.9;

            sprite.scale.set(scale);
            sprite.x = app.screen.width / 2;
            sprite.y = app.screen.height / 2;
            sprite.anchor.set(0.5);

            app.stage.addChild(sprite);
            spriteRef.current = sprite;
            console.log('Sprite added to stage');  // 调试日志
          }
        }
      } catch (error) {
        console.error('Error initializing PixiJS:', error);
        // 如果失败，尝试重新初始化
        if (!initializationTimeout.current) {
          console.log('Scheduling retry...');  // 调试日志
          initializationTimeout.current = setTimeout(() => {
            console.log('Retrying initialization...');  // 调试日志
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
        width: '100%',
        height: '100%',
        position: 'absolute',
        top: 0,
        left: 0,
        overflow: 'hidden',
        zIndex: 2,
      }}
    />
  );
});

ImageViewer.displayName = 'ImageViewer';

export default ImageViewer;