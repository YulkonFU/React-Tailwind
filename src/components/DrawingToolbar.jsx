import {
  ArrowRight,
  Circle,
  Square,
  Minus,
  Eraser,
  Eye,
  EyeOff,
} from "lucide-react";
import PropTypes from "prop-types";

const DrawingToolbar = ({
  currentTool,
  showOverlay,
  onToolSelect,
  onToggleOverlay,
  onClear,
}) => {
  return (
    <div
      className="fixed left-1 top-1/2 -translate-y-1/2 bg-white/80 backdrop-blur-sm rounded-lg shadow-lg p-2"
      style={{ zIndex: 1000 }}
    >
      <div className="flex flex-col gap-2">
        {/* 直线工具 */}
        <button
          onClick={() => onToolSelect("line")}
          className={`p-2 rounded-lg transition-colors ${
            currentTool === "line"
              ? "bg-blue-500 text-white"
              : "hover:bg-gray-200"
          }`}
          title="直线工具"
        >
          <Minus className="w-5 h-5" />
        </button>

        {/* 箭头工具 */}
        <button
          onClick={() => onToolSelect("arrow")}
          className={`p-2 rounded-lg transition-colors ${
            currentTool === "arrow"
              ? "bg-blue-500 text-white"
              : "hover:bg-gray-200"
          }`}
          title="箭头工具"
        >
          <ArrowRight className="w-5 h-5" />
        </button>

        {/* 矩形工具 */}
        <button
          onClick={() => onToolSelect("rect")}
          className={`p-2 rounded-lg transition-colors ${
            currentTool === "rect"
              ? "bg-blue-500 text-white"
              : "hover:bg-gray-200"
          }`}
          title="矩形工具"
        >
          <Square className="w-5 h-5" />
        </button>

        {/* 圆形工具 */}
        <button
          onClick={() => onToolSelect("circle")}
          className={`p-2 rounded-lg transition-colors ${
            currentTool === "circle"
              ? "bg-blue-500 text-white"
              : "hover:bg-gray-200"
          }`}
          title="圆形工具"
        >
          <Circle className="w-5 h-5" />
        </button>

        <div className="w-full h-px bg-gray-300" />

        <button
          onClick={() => onToggleOverlay()}
          className="p-2 rounded-lg hover:bg-gray-200 transition-colors"
          title={showOverlay ? "隐藏标注" : "显示标注"}
        >
          {showOverlay ? (
            <EyeOff className="w-5 h-5" />
          ) : (
            <Eye className="w-5 h-5" />
          )}
        </button>

        <button
          onClick={onClear}
          className="p-2 rounded-lg hover:bg-gray-200 transition-colors"
          title="清除标注"
        >
          <Eraser className="w-5 h-5" />
        </button>
      </div>
    </div>
  );
};
DrawingToolbar.propTypes = {
  currentTool: PropTypes.string.isRequired,
  showOverlay: PropTypes.bool.isRequired,
  onToolSelect: PropTypes.func.isRequired,
  onToggleOverlay: PropTypes.func.isRequired,
  onClear: PropTypes.func.isRequired,
};

export default DrawingToolbar;
