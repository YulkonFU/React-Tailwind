import { useRef, useEffect, useState, forwardRef, useImperativeHandle } from "react";
import * as PIXI from "pixi.js";

const OverlayCanvas = forwardRef(({ width, height, visible = true }, ref) => {
  const pixiRef = useRef(null);
  const [app, setApp] = useState(null);

  // shapes 用于记录已绘制完成的图形
  const [shapes, setShapes] = useState([]);
  // 当前绘制中
  const [isDrawing, setIsDrawing] = useState(false);
  // 起点
  const [startPoint, setStartPoint] = useState({ x: 0, y: 0 });
  // 当前工具
  const [currentTool, setCurrentTool] = useState("arrow");
  // 预览用 Graphics
  const previewRef = useRef(null);

  // 初始化 PixiJS (v8)
  useEffect(() => {
    let pixiApp;
    (async () => {
      if (!pixiRef.current) return;

      // 1) 创建 Application 对象
      pixiApp = new PIXI.Application();
      // 2) 异步 init，传入配置
      await pixiApp.init({
        width,
        height,
        antialias: true,
        transparent: true,
        clearBeforeRender: true,
        backgroundColor: 0x000000, // 可根据需要调整
      });
      // 3) 将创建好的 canvas 加入 DOM
      if (pixiRef.current) {
        pixiRef.current.appendChild(pixiApp.canvas);
        setApp(pixiApp);

        // 开启舞台交互，注册事件
        pixiApp.stage.interactive = true;
        pixiApp.stage.on("pointerdown", handlePointerDown);
        pixiApp.stage.on("pointermove", handlePointerMove);
        pixiApp.stage.on("pointerup", handlePointerUp);
        pixiApp.stage.on("pointerupoutside", handlePointerUp);

        // 创建预览用图形
        const previewGraphics = new PIXI.Graphics();
        pixiApp.stage.addChild(previewGraphics);
        previewRef.current = previewGraphics;
      }
    })();

    // 卸载时清理
    return () => {
      if (pixiApp) {
        pixiApp.destroy(true, true);
        setApp(null);
      }
    };
  }, [width, height]);

  // 暴露给父组件的方法
  useImperativeHandle(ref, () => ({
    getCanvas: () => app?.canvas,
    clearCanvas: () => {
      setShapes([]);
      if (!app) return;
      app.stage.removeChildren(); // 会移除 preview，也要重新添加
      const newPreview = new PIXI.Graphics();
      app.stage.addChild(newPreview);
      previewRef.current = newPreview;
    },
    setTool: (tool) => setCurrentTool(tool),
  }));

  useEffect(() => {
    if (!isDrawing && app) {
      redrawAll();
    }
  }, [shapes, app, isDrawing]);

  function handlePointerDown(e) {
    if (!visible) return;
    setIsDrawing(true);
    const { x, y } = e.data.getLocalPosition(app.stage);
    setStartPoint({ x, y });
  }

  function handlePointerMove(e) {
    if (!isDrawing || !visible) return;
    const { x, y } = e.data.getLocalPosition(app.stage);
    drawPreview({ x, y });
  }

  function handlePointerUp(e) {
    if (!isDrawing || !visible) return;
    setIsDrawing(false);
    const { x, y } = e.data.getLocalPosition(app.stage);
    const newShape = { tool: currentTool, start: startPoint, end: { x, y } };
    setShapes((prev) => [...prev, newShape]);

    // 清空预览
    if (previewRef.current) {
      previewRef.current.clear();
    }
  }

  // 重绘所有已完成的图形
  function redrawAll() {
    if (!app) return;
    app.stage.children.forEach((c) => {
      if (c !== previewRef.current) c.clear?.();
    });
    shapes.forEach((shape) => {
      const g = new PIXI.Graphics();
      drawShape(g, shape);
      app.stage.addChild(g);
    });
  }

  // 每次鼠标移动时的实时预览
  function drawPreview(currentEnd) {
    if (!previewRef.current) return;
    previewRef.current.clear();
    drawShape(previewRef.current, {
      tool: currentTool,
      start: startPoint,
      end: currentEnd,
    });
  }

  // 根据 tool 绘制图形
  function drawShape(g, { tool, start, end }) {
    g.lineStyle(2, 0xff0000, 1);
    switch (tool) {
      case "arrow":
        drawArrow(g, start, end);
        break;
      case "line":
        drawLine(g, start, end);
        break;
      case "rect":
        drawRect(g, start, end);
        break;
      case "circle":
        drawCircle(g, start, end);
        break;
      default:
        drawArrow(g, start, end);
        break;
    }
  }

  function drawLine(g, start, end) {
    g.moveTo(start.x, start.y);
    g.lineTo(end.x, end.y);
  }

  function drawArrow(g, start, end) {
    const headlen = 10;
    const dx = end.x - start.x;
    const dy = end.y - start.y;
    const angle = Math.atan2(dy, dx);
    // 主干
    g.moveTo(start.x, start.y);
    g.lineTo(end.x, end.y);
    // 箭头两侧
    g.moveTo(end.x, end.y);
    g.lineTo(
      end.x - headlen * Math.cos(angle - Math.PI / 6),
      end.y - headlen * Math.sin(angle - Math.PI / 6)
    );
    g.moveTo(end.x, end.y);
    g.lineTo(
      end.x - headlen * Math.cos(angle + Math.PI / 6),
      end.y - headlen * Math.sin(angle + Math.PI / 6)
    );
  }

  function drawRect(g, start, end) {
    const x = Math.min(start.x, end.x);
    const y = Math.min(start.y, end.y);
    const w = Math.abs(end.x - start.x);
    const h = Math.abs(end.y - start.y);
    g.drawRect(x, y, w, h);
  }

  function drawCircle(g, start, end) {
    const radius = Math.sqrt(
      (end.x - start.x) ** 2 + (end.y - start.y) ** 2
    );
    g.drawCircle(start.x, start.y, radius);
  }

  return (
    <div
      ref={pixiRef}
      style={{
        position: "absolute",
        top: 0,
        left: 0,
        width,
        height,
        pointerEvents: visible ? "auto" : "none",
        opacity: visible ? 1 : 0,
        overflow: "hidden",
        cursor: isDrawing ? "crosshair" : "default",
      }}
    />
  );
});

OverlayCanvas.displayName = "OverlayCanvas";
export default OverlayCanvas;