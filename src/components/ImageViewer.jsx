import React, { useEffect, useRef } from 'react';
import * as PIXI from 'pixi.js';

const ImageViewer = ({ onImageLoad }) => {
  const pixiContainerRef = useRef(null);
  const pixiAppRef = useRef(null);

  useEffect(() => {
    const initPixiApp = async () => {
      if (pixiContainerRef.current) {
        const app = new PIXI.Application();
        await app.init({
          width: pixiContainerRef.current.clientWidth,
          height: pixiContainerRef.current.clientHeight,
          backgroundColor: 0x000000,
          antialias: true,
        });

        pixiContainerRef.current.appendChild(app.canvas);
        pixiAppRef.current = app;

        // 加载并显示图像
        const loadAndDisplayImage = async (imageUrl) => {
          await PIXI.Assets.load(imageUrl);
          const sprite = PIXI.Sprite.from(imageUrl);
          sprite.width = 384;
          sprite.height = 384;
          sprite.x = (app.screen.width - sprite.width) / 2;
          sprite.y = (app.screen.height - sprite.height) / 2;
          app.stage.addChild(sprite);
          if (onImageLoad) onImageLoad(sprite);
        };

        const imageUrl = 'https://pixijs.com/assets/files/sample-747abf529b135a1f549dff3ec846afbc.png';
        await loadAndDisplayImage(imageUrl);
      }
    };

    initPixiApp().catch(console.error);

    return () => {
      if (pixiAppRef.current) {
        pixiAppRef.current.destroy(true);
      }
    };
  }, [onImageLoad]);

  return (
    <div 
      ref={pixiContainerRef}
      style={{ 
        width: '100%',
        height: '100%',
        position: 'absolute',
        top: 0,
        left: 0
      }}
    />
  );
};

export default ImageViewer;

