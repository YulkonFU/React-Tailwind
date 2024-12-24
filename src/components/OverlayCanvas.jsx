import { useRef, useEffect, useState, forwardRef, useImperativeHandle } from "react";

/**
 * OverlayCanvas 提供在画布上绘制箭头、圆形等功能。
 * 每次拖拽绘制时，仅显示当前图形预览，松开鼠标后将图形记录到数组中。
 */
const OverlayCanvas = forwardRef(({ width, height, visible = true }, ref) => {
  const canvasRef = useRef(null);

  // 存储所有已完成的图形
  const [shapes, setShapes] = useState([]);

  // 当前是否正在绘制
  const [isDrawing, setIsDrawing] = useState(false);

  // 绘制起点记录
  const [startPoint, setStartPoint] = useState({ x: 0, y: 0 });

  // 当前选中的工具，默认箭头
  const [currentTool, setCurrentTool] = useState("arrow");

  // 在挂载时初始化画笔
  useEffect(() => {
    const canvas = canvasRef.current;
    if (canvas) {
      const ctx = canvas.getContext("2d");
      // 保持红色
      ctx.strokeStyle = "red";
      ctx.lineWidth = 2;
    }
  }, []);

  // shapes 更新后，立刻重绘，以显示最新图形
  useEffect(() => {
    if (!isDrawing) {
      redrawAll(null); 
    }
  }, [shapes, isDrawing]);

  // 在父组件可调用的方法
  useImperativeHandle(ref, () => ({
    getCanvas: () => canvasRef.current,
    clearCanvas: () => {
      setShapes([]);
      const cvs = canvasRef.current;
      cvs.getContext("2d").clearRect(0, 0, cvs.width, cvs.height);
    },
    setTool: (tool) => setCurrentTool(tool),
  }));

  const drawArrow = (start, end, ctx) => {
    const headlen = 10;
    const dx = end.x - start.x;
    const dy = end.y - start.y;
    const angle = Math.atan2(dy, dx);

    ctx.beginPath();
    ctx.moveTo(start.x, start.y);
    ctx.lineTo(end.x, end.y);
    ctx.lineTo(
      end.x - headlen * Math.cos(angle - Math.PI / 6),
      end.y - headlen * Math.sin(angle - Math.PI / 6)
    );
    ctx.moveTo(end.x, end.y);
    ctx.lineTo(
      end.x - headlen * Math.cos(angle + Math.PI / 6),
      end.y - headlen * Math.sin(angle + Math.PI / 6)
    );
    ctx.stroke();
  };

  const drawCircle = (start, end, ctx) => {
    const radius = Math.sqrt(
      Math.pow(end.x - start.x, 2) + Math.pow(end.y - start.y, 2)
    );
    ctx.beginPath();
    ctx.arc(start.x, start.y, radius, 0, 2 * Math.PI);
    ctx.stroke();
  };

  // 将单个图形数据绘制到 ctx
  const drawShape = (shape, ctx) => {
    ctx.strokeStyle = "red";
    ctx.lineWidth = 2;
    const { tool, start, end } = shape;
    if (tool === "arrow") {
      drawArrow(start, end, ctx);
    } else if (tool === "circle") {
      drawCircle(start, end, ctx);
    }
  };

  // 重绘所有图形 + (可选)当前正在绘制的图形
  const redrawAll = (tempEnd) => {
    const cvs = canvasRef.current;
    const ctx = cvs.getContext("2d");
    ctx.clearRect(0, 0, cvs.width, cvs.height);

    // 先绘制已完成的图形
    shapes.forEach((shape) => drawShape(shape, ctx));

    // 正在拖拽时，先绘制之前图形，然后再绘制当前预览图形
    if (tempEnd && isDrawing) {
      drawShape({ tool: currentTool, start: startPoint, end: tempEnd }, ctx);
    }
  };

  // 鼠标按下
  const handleMouseDown = (e) => {
    const rect = canvasRef.current.getBoundingClientRect();
    setStartPoint({
      x: e.clientX - rect.left,
      y: e.clientY - rect.top,
    });
    setIsDrawing(true);
  };

  // 鼠标移动 -> 实时预览
  const handleMouseMove = (e) => {
    if (!isDrawing) return;
    const rect = canvasRef.current.getBoundingClientRect();
    const currentPoint = {
      x: e.clientX - rect.left,
      y: e.clientY - rect.top,
    };
    redrawAll(currentPoint);
  };

  // 鼠标抬起 -> 完成绘制
  const handleMouseUp = (e) => {
    if (!isDrawing) return;
    const rect = canvasRef.current.getBoundingClientRect();
    const endPoint = {
      x: e.clientX - rect.left,
      y: e.clientY - rect.top,
    };
    setShapes((prev) => [
      ...prev,
      { tool: currentTool, start: startPoint, end: endPoint },
    ]);
    setIsDrawing(false);
    // 注意：此时立刻调用 redrawAll 只能重绘旧 shapes，
    // shapes 数组在下一次渲染才更新，所以这里交给 useEffect 监听 shapes 自动重绘
  };

  return (
    <canvas
      ref={canvasRef}
      width={width}
      height={height}
      style={{
        position: "absolute",
        top: 0,
        left: 0,
        pointerEvents: visible ? "auto" : "none",
        opacity: visible ? 1 : 0,
        cursor: isDrawing ? "crosshair" : "default",
      }}
      onMouseDown={handleMouseDown}
      onMouseMove={handleMouseMove}
      onMouseUp={handleMouseUp}
      onMouseLeave={handleMouseUp}
    />
  );
});

OverlayCanvas.displayName = "OverlayCanvas";
export default OverlayCanvas;