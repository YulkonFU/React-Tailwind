// src/components/ImageControls.jsx
import PropTypes from "prop-types";
import {
  Minimize2,
  Maximize2,
  RotateCcw,
  RotateCw,
  ZoomIn,
  ZoomOut,
  RefreshCw,
} from "react-feather";

const ImageControls = ({
  isExpanded,
  onToggleExpand,
  onRotate,
  onZoom,
  onReset,
}) => {
  return (
    <div className="absolute bottom-4 left-4 flex flex-col gap-4">
      {/* 主控制面板 */}
      <div className="bg-gray-800 bg-opacity-75 p-2 rounded-lg flex gap-2">
        {/* 展开/收起按钮 */}
        <button
          className="p-2 bg-gray-700 text-white rounded hover:bg-gray-600"
          onClick={onToggleExpand}
        >
          {isExpanded ? (
            <Minimize2 className="w-4 h-4" />
          ) : (
            <Maximize2 className="w-4 h-4" />
          )}
        </button>

        {/* 旋转控制 */}
        <button
          className="p-2 bg-gray-700 text-white rounded hover:bg-gray-600"
          onClick={() => onRotate(-90)}
        >
          <RotateCcw className="w-4 h-4" />
        </button>
        <button
          className="p-2 bg-gray-700 text-white rounded hover:bg-gray-600"
          onClick={() => onRotate(90)}
        >
          <RotateCw className="w-4 h-4" />
        </button>

        {/* 缩放控制 */}
        <button
          className="p-2 bg-gray-700 text-white rounded hover:bg-gray-600"
          onClick={() => onZoom(1.1)}
        >
          <ZoomIn className="w-4 h-4" />
        </button>
        <button
          className="p-2 bg-gray-700 text-white rounded hover:bg-gray-600"
          onClick={() => onZoom(0.9)}
        >
          <ZoomOut className="w-4 h-4" />
        </button>

        {/* 重置按钮 */}
        <button
          className="p-2 bg-gray-700 text-white rounded hover:bg-gray-600"
          onClick={onReset}
        >
          <RefreshCw className="w-4 h-4" />
        </button>
      </div>
    </div>
  );
};
ImageControls.propTypes = {
  isExpanded: PropTypes.bool.isRequired,
  onToggleExpand: PropTypes.func.isRequired,
  onRotate: PropTypes.func.isRequired,
  onZoom: PropTypes.func.isRequired,
  onReset: PropTypes.func.isRequired,
  onEnhance: PropTypes.func.isRequired,
};

export default ImageControls;
