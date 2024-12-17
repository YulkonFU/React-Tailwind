import { useState, useEffect } from "react";
import PropTypes from "prop-types";
import { Radiation, AlertTriangle, ChevronUp, ChevronDown } from "lucide-react";

const XrayControl = ({ onStatusChange }) => {
  const [xrayState, setXrayState] = useState({
    isPowered: false,
    isWarmedUp: false,
    voltage: 130,
    current: 300,
    focus: 0,
    status: "XR_NOT_READY",
  });

  const [showWarning, setShowWarning] = useState(false);
  const [warningMessage, setWarningMessage] = useState("");
  const [position, setPosition] = useState({ x: 20, y: 20 });
  const [isDragging, setIsDragging] = useState(false);
  const [dragStart, setDragStart] = useState({ x: 0, y: 0 });
  const [showWarmupDialog, setShowWarmupDialog] = useState(false);
  const [isCollapsed, setIsCollapsed] = useState(false);

  // 处理暖机确认
  const handleWarmupConfirm = () => {
    setShowWarmupDialog(false);
    setXrayState((prev) => {
      const newState = {
        ...prev,
        isPowered: true,
        isWarmedUp: true,
        status: "XR_IS_ON",
      };
      onStatusChange(newState.status);
      return newState;
    });
  };

  // 处理开关
  const handlePowerToggle = () => {
    if (!xrayState.isPowered) {
      setShowWarmupDialog(true);
    } else {
      setXrayState((prev) => {
        const newState = {
          ...prev,
          isPowered: false,
          isWarmedUp: false,
          status: "XR_IS_OFF",
        };
        onStatusChange(newState.status);
        return newState;
      });
    }
  };
  // 处理拖动开始
  const handleDragStart = (e) => {
    setIsDragging(true);
    setDragStart({
      x: e.clientX - position.x,
      y: e.clientY - position.y,
    });
  };

  // 处理拖动
  const handleDrag = (e) => {
    if (isDragging) {
      requestAnimationFrame(() => {
        setPosition({
          x: e.clientX - dragStart.x,
          y: e.clientY - dragStart.y,
        });
      });
    }
  };

  // 处理拖动结束
  const handleDragEnd = () => {
    setIsDragging(false);
  };

  // 计算功率
  const calculatePower = () => {
    return ((xrayState.voltage * xrayState.current) / 1000).toFixed(2);
  };

  useEffect(() => {
    if (isDragging) {
      window.addEventListener("mousemove", handleDrag);
      window.addEventListener("mouseup", handleDragEnd);
    }
    return () => {
      window.removeEventListener("mousemove", handleDrag);
      window.removeEventListener("mouseup", handleDragEnd);
    };
  }, [isDragging]);

  return (
    <>
      <div
        className={`fixed bg-white rounded-lg shadow-lg will-change-transform`}
        style={{
          transform: `translate(${position.x}px, ${position.y}px)`,
          transition: isDragging ? "none" : "transform 0.3s",
          width: isCollapsed ? "40px" : "320px", // 增加宽度
        }}
      >
        {/* 主控件头部 */}
        <div
          className="cursor-move py-3 px-4 bg-gray-100 rounded-t-lg flex justify-between items-center"
          onMouseDown={handleDragStart}
        >
          {!isCollapsed && (
            <h3 className="text-sm font-semibold">X-ray Control</h3>
          )}
          <button
            onClick={() => setIsCollapsed(!isCollapsed)}
            className="p-1.5 hover:bg-gray-200 rounded"
          >
            {isCollapsed ? (
              <ChevronDown className="w-4 h-4" />
            ) : (
              <ChevronUp className="w-4 h-4" />
            )}
          </button>
        </div>

        {/* 展开时显示的内容 */}
        {!isCollapsed && (
          <div className="p-5">
            {/* 状态显示 */}
            <div className="mb-4 p-3 bg-gray-100 rounded-lg">
              <div className="flex justify-between items-center">
                <span className="text-sm font-medium">Status</span>
                <span className="text-sm">{xrayState.status}</span>
              </div>
            </div>

            {/* Power Control */}
            <div className="flex items-center justify-between p-3 bg-white rounded-lg shadow-sm mb-4">
              <div className="flex items-center space-x-2">
                <Radiation
                  className={`w-5 h-5 ${
                    xrayState.isPowered ? "text-red-500" : "text-gray-400"
                  }`}
                />
                <span className="text-sm font-medium">X-ray Power</span>
              </div>
              <button
                onClick={handlePowerToggle}
                className={`px-4 py-2 rounded text-white ${
                  xrayState.isPowered
                    ? "bg-red-500 hover:bg-red-600"
                    : "bg-gray-400 hover:bg-gray-500"
                }`}
              >
                {xrayState.isPowered ? "ON" : "OFF"}
              </button>
            </div>

            {/* 控制面板 */}
            <div className="space-y-4">
              {/* Voltage Control */}
              <div className="space-y-2">
                <div className="flex justify-between">
                  <label className="text-sm font-medium">Voltage (kV)</label>
                  <span className="text-sm">{xrayState.voltage} kV</span>
                </div>
                <input
                  type="range"
                  min="0"
                  max="200"
                  value={xrayState.voltage}
                  onChange={(e) =>
                    setXrayState((prev) => ({
                      ...prev,
                      voltage: parseInt(e.target.value),
                    }))
                  }
                  className="w-full"
                  disabled={!xrayState.isPowered}
                />
              </div>

              {/* Current Control */}
              <div className="space-y-2">
                <div className="flex justify-between">
                  <label className="text-sm font-medium">Current (µA)</label>
                  <span className="text-sm">{xrayState.current} µA</span>
                </div>
                <input
                  type="range"
                  min="0"
                  max="500"
                  value={xrayState.current}
                  onChange={(e) =>
                    setXrayState((prev) => ({
                      ...prev,
                      current: parseInt(e.target.value),
                    }))
                  }
                  className="w-full"
                  disabled={!xrayState.isPowered}
                />
              </div>

              {/* Power Display */}
              <div className="space-y-2">
                <div className="flex justify-between">
                  <label className="text-sm font-medium">Power (W)</label>
                  <span className="text-sm">{calculatePower()} W</span>
                </div>
              </div>

              {/* Focus Control */}
              <div className="space-y-2">
                <div className="flex justify-between items-center">
                  <label className="text-sm font-medium">Focus Mode</label>
                  <span className="text-sm">Mode {xrayState.focus}</span>
                </div>
                <div className="grid grid-cols-4 gap-2">
                  {[0, 1, 2, 3].map((mode) => (
                    <button
                      key={mode}
                      onClick={() =>
                        setXrayState((prev) => ({ ...prev, focus: mode }))
                      }
                      disabled={!xrayState.isPowered}
                      className={`p-2 rounded ${
                        xrayState.focus === mode
                          ? "bg-blue-500 text-white"
                          : "bg-gray-100 text-gray-700 hover:bg-gray-200"
                      } ${
                        !xrayState.isPowered && "opacity-50 cursor-not-allowed"
                      }`}
                    >
                      F{mode}
                    </button>
                  ))}
                </div>
              </div>
            </div>

            {/* Warning Alert */}
            {showWarning && (
              <div className="flex items-center gap-2 p-3 mt-4 bg-red-100 border border-red-300 text-red-700 rounded-lg">
                <AlertTriangle className="h-4 w-4" />
                <p className="text-sm">{warningMessage}</p>
                <button
                  onClick={() => setShowWarning(false)}
                  className="ml-auto text-red-700 hover:text-red-900"
                >
                  ×
                </button>
              </div>
            )}
          </div>
        )}

        {/* Warmup Dialog - 相对于组件定位 */}
        {showWarmupDialog && (
          <div className="absolute inset-0 bg-black bg-opacity-50 flex items-center justify-center rounded-lg">
            <div className="bg-white p-4 rounded-lg shadow-lg w-[80%]">
              <h3 className="text-lg font-medium mb-2">Warm-up Required</h3>
              <p className="text-sm text-gray-600 mb-4">
                Do you want to proceed with warm-up?
              </p>
              <div className="flex justify-end space-x-2">
                <button
                  onClick={() => setShowWarmupDialog(false)}
                  className="px-4 py-2 text-sm text-gray-600 hover:bg-gray-100 rounded"
                >
                  Cancel
                </button>
                <button
                  onClick={handleWarmupConfirm}
                  className="px-4 py-2 text-sm text-white bg-blue-500 hover:bg-blue-600 rounded"
                >
                  Yes, Proceed
                </button>
              </div>
            </div>
          </div>
        )}
      </div>
    </>
  );
};
XrayControl.propTypes = {
  onStatusChange: PropTypes.func.isRequired,
};

export default XrayControl;
