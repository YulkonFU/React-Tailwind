import { useEffect, useRef, forwardRef, useState, useImperativeHandle } from "react";
import * as PIXI from "pixi.js";
import PropTypes from "prop-types";
import OverlayCanvas from "./OverlayCanvas";

const ImageViewer = forwardRef(({ 
  showOverlay: initialShowOverlay = true, 
  isDrawingEnabled = false,
  imageData,
  width,
  height 
}, ref) => {
    const pixiContainerRef = useRef(null);
    const pixiAppRef = useRef(null);
    const spriteRef = useRef(null);
    const overlayRef = useRef(null);
    const [showOverlay, setShowOverlay] = useState(initialShowOverlay);

    useImperativeHandle(ref, () => ({
        saveImage: (options = { includeOverlay: true, filename: "image.png" }) => {
            const canvas = document.createElement("canvas");
            canvas.width = pixiAppRef.current.screen.width;
            canvas.height = pixiAppRef.current.screen.height;
            const ctx = canvas.getContext("2d");

            ctx.drawImage(pixiAppRef.current.view, 0, 0);

            if (options.includeOverlay && showOverlay) {
                ctx.drawImage(overlayRef.current.getCanvas(), 0, 0);
            }

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
        const updatePixiApp = async () => {
            try {
                if (!pixiAppRef.current) {
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

                if (imageData && width && height) {
                    const canvas = document.createElement("canvas");
                    canvas.width = width;
                    canvas.height = height;
                    const ctx = canvas.getContext("2d");

                    // 创建灰度图像数据
                    const imgData = ctx.createImageData(width, height);
                    for (let i = 0; i < imageData.length; i++) {
                        const value = imageData[i];
                        const idx = i * 4;
                        imgData.data[idx] = value;     // R
                        imgData.data[idx + 1] = value; // G
                        imgData.data[idx + 2] = value; // B
                        imgData.data[idx + 3] = 255;   // A
                    }

                    ctx.putImageData(imgData, 0, 0);

                    // 更新或创建精灵
                    const texture = PIXI.Texture.from(canvas);
                    
                    if (spriteRef.current) {
                        spriteRef.current.texture = texture;
                    } else {
                        const sprite = new PIXI.Sprite(texture);
                        const scaleX = pixiAppRef.current.screen.width / width;
                        const scaleY = pixiAppRef.current.screen.height / height;
                        const scale = Math.min(scaleX, scaleY) * 0.9;

                        sprite.scale.set(scale);
                        sprite.x = pixiAppRef.current.screen.width / 2;
                        sprite.y = pixiAppRef.current.screen.height / 2;
                        sprite.anchor.set(0.5);
                        sprite.interactive = true;
                        sprite.cursor = "pointer";

                        pixiAppRef.current.stage.addChild(sprite);
                        spriteRef.current = sprite;
                    }
                }
            } catch (error) {
                console.error("Error updating PixiJS:", error);
            }
        };

        updatePixiApp();

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
    }, [imageData, width, height]);

    return (
        <div
            ref={pixiContainerRef}
            className="w-full h-full absolute top-0 left-0 overflow-hidden z-10 bg-black"
        >
            <OverlayCanvas
                ref={overlayRef}
                width={pixiContainerRef.current?.clientWidth}
                height={pixiContainerRef.current?.clientHeight}
                visible={showOverlay}
                isDrawingEnabled={isDrawingEnabled}
            />
        </div>
    );
});

ImageViewer.displayName = "ImageViewer";
ImageViewer.propTypes = {
    showOverlay: PropTypes.bool,
    isDrawingEnabled: PropTypes.bool,
    imageData: PropTypes.instanceOf(Uint8Array),
    width: PropTypes.number,
    height: PropTypes.number,
};

export default ImageViewer;