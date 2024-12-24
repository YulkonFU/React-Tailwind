import { useRef, useEffect, useState, forwardRef, useImperativeHandle } from 'react';
import PropTypes from 'prop-types';

const OverlayCanvas = forwardRef(({ width, height, visible = true }, ref) => {
  const canvasRef = useRef(null);
  const [isDrawing, setIsDrawing] = useState(false);
  const [startPoint, setStartPoint] = useState({ x: 0, y: 0 });
  const [currentTool, setCurrentTool] = useState('arrow');

  useEffect(() => {
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    ctx.strokeStyle = 'red';
    ctx.lineWidth = 2;
  }, []);

  useImperativeHandle(ref, () => ({
    getCanvas: () => canvasRef.current,
    clearCanvas: () => {
      const canvas = canvasRef.current;
      const ctx = canvas.getContext('2d');
      ctx.clearRect(0, 0, canvas.width, canvas.height);
    },
    setTool: (tool) => setCurrentTool(tool)
  }));

  const drawArrow = (start, end, ctx) => {
    const headlen = 10;
    const dx = end.x - start.x;
    const dy = end.y - start.y;
    const angle = Math.atan2(dy, dx);
    
    ctx.beginPath();
    ctx.moveTo(start.x, start.y);
    ctx.lineTo(end.x, end.y);
    ctx.lineTo(end.x - headlen * Math.cos(angle - Math.PI / 6), 
               end.y - headlen * Math.sin(angle - Math.PI / 6));
    ctx.moveTo(end.x, end.y);
    ctx.lineTo(end.x - headlen * Math.cos(angle + Math.PI / 6),
               end.y - headlen * Math.sin(angle + Math.PI / 6));
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

  const handleMouseDown = (e) => {
    setIsDrawing(true);
    const rect = canvasRef.current.getBoundingClientRect();
    setStartPoint({
      x: e.clientX - rect.left,
      y: e.clientY - rect.top
    });
  };

  const handleMouseMove = (e) => {
    if (!isDrawing) return;
    
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    const rect = canvas.getBoundingClientRect();
    const currentPoint = {
      x: e.clientX - rect.left,
      y: e.clientY - rect.top
    };

    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    if (currentTool === 'arrow') {
      drawArrow(startPoint, currentPoint, ctx);
    } else if (currentTool === 'circle') {
      drawCircle(startPoint, currentPoint, ctx);
    }
  };

  const handleMouseUp = () => {
    setIsDrawing(false);
  };

  return (
    <canvas
      ref={canvasRef}
      width={width}
      height={height}
      style={{
        position: 'absolute',
        top: 0,
        left: 0,
        pointerEvents: visible ? 'auto' : 'none',
        opacity: visible ? 1 : 0,
        cursor: isDrawing ? 'crosshair' : 'default'
      }}
      onMouseDown={handleMouseDown}
      onMouseMove={handleMouseMove}
      onMouseUp={handleMouseUp}
      onMouseLeave={handleMouseUp}
    />
  );
});
OverlayCanvas.displayName = 'OverlayCanvas';

OverlayCanvas.propTypes = {
  width: PropTypes.number.isRequired,
  height: PropTypes.number.isRequired,
  visible: PropTypes.bool
};
OverlayCanvas.displayName = 'OverlayCanvas';

export default OverlayCanvas;