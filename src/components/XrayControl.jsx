import { useState, useEffect, useCallback } from "react";
import {
  Radiation,
  AlertTriangle,
  ChevronUp,
  ChevronDown,
  ChevronLeft,
  ChevronRight,
} from "lucide-react";

const DEBUG = true;

function log(...args) {
  if (DEBUG) {
    console.log("[XrayControl]", ...args);
  }
}

const XrayControl = ({ onStatusChange }) => {
  // 状态管理
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
  const [isCollapsed, setIsCollapsed] = useState(false);
  const [spotSizes, setSpotSizes] = useState([]);

  // 初始化时获取焦点模式
  useEffect(() => {
    const initSpotSizes = async () => {
      try {
        const handler = window.chrome?.webview?.hostObjects?.sync?.xrayHandler;
        if (!handler) throw new Error("XrayHandler not available");
        const count = await handler.getSpotsizeCount;
        setSpotSizes(Array.from({ length: count }, (_, i) => i));
      } catch (err) {
        console.error("Failed to get spot sizes:", err);
        setWarningMessage("Failed to initialize spot sizes");
        setShowWarning(true);
      }
    };
    initSpotSizes();
    updateStatus(); // 初始化时更新状态
  }, []);

  const handlePowerToggle = async () => {
    try {
      const handler = window.chrome?.webview?.hostObjects?.sync?.xrayHandler;
      if (!handler) throw new Error("XrayHandler not available");

      if (!xrayState.isPowered) {
        await handler.turnOn;
      } else {
        await handler.turnOff;
      }
      await updateStatus();
    } catch (err) {
      console.error("Power toggle failed:", err);
      setWarningMessage(err.message);
      setShowWarning(true);
    }
  };

  // updateStatus 方法，添加回调
  const updateStatus = async () => {
    try {
      const handler = window.chrome?.webview?.hostObjects?.sync?.xrayHandler;
      if (!handler) throw new Error("XrayHandler not available");
      const status = await handler.getStatus;
      const statusData = JSON.parse(status);
      
      setXrayState((prev) => ({
        ...prev,
        isPowered: statusData.isPowered,
        isWarmedUp: statusData.isWarmedUp,
        status: statusData.status,
        voltage: statusData.voltage,
        current: statusData.current,
      }));
      
      // 通知父组件状态变化
      onStatusChange?.(statusData.status);
      
    } catch (err) {
      console.error("Failed to update status:", err);
      setWarningMessage("Failed to update status");
      setShowWarning(true);
    }
  };

  const handleVoltageChange = async (value) => {
    try {
      const handler = window.chrome?.webview?.hostObjects?.sync?.xrayHandler;
      if (!handler) throw new Error("XrayHandler not available");

      const newVoltage = parseInt(
        typeof value === "object" ? value.target.value : value
      );
      if (isNaN(newVoltage) || newVoltage < 0 || newVoltage > 200) {
        throw new Error("Invalid voltage value (0-200kV)");
      }

      console.log("Setting voltage to:", newVoltage);

      // 改为设置属性（无括号），sync hostObjects 可传递一个参数
      handler.Voltage = newVoltage;

      await updateStatus();
      console.log("Voltage set successfully:", newVoltage);
    } catch (err) {
      console.error("Failed to set voltage:", err);
      setWarningMessage(`Failed to set voltage: ${err.message}`);
      setShowWarning(true);
    }
  };

  const handleCurrentChange = async (value) => {
    try {
      const handler = window.chrome?.webview?.hostObjects?.sync?.xrayHandler;
      if (!handler) throw new Error("XrayHandler not available");

      const newCurrent = parseInt(
        typeof value === "object" ? value.target.value : value
      );
      if (isNaN(newCurrent) || newCurrent < 0 || newCurrent > 500) {
        throw new Error("Invalid current value (0-500µA)");
      }

      // 同理改为属性赋值
      handler.Current = newCurrent;

      await updateStatus();
    } catch (err) {
      console.error("Failed to set current:", err);
      setWarningMessage(`Failed to set current: ${err.message}`);
      setShowWarning(true);
    }
  };

  const handleFocusChange = async (mode) => {
    try {
      const handler = window.chrome?.webview?.hostObjects?.sync?.xrayHandler;
      if (!handler) throw new Error("XrayHandler not available");
      await handler.setFocus(mode);
      setXrayState((prev) => ({ ...prev, focus: mode }));
      await updateStatus();
    } catch (err) {
      console.error("Failed to set focus mode:", err);
      setWarningMessage(err.message);
      setShowWarning(true);
    }
  };

  const handleDragStart = (e) => {
    setIsDragging(true);
    setDragStart({
      x: e.clientX - position.x,
      y: e.clientY - position.y,
    });
  };

  const handleDrag = useCallback(
    (e) => {
      if (isDragging) {
        requestAnimationFrame(() => {
          setPosition({
            x: e.clientX - dragStart.x,
            y: e.clientY - dragStart.y,
          });
        });
      }
    },
    [isDragging, dragStart]
  );

  const handleDragEnd = () => {
    setIsDragging(false);
  };

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
  }, [isDragging, handleDrag]);

  return (
    <>
      <div
        className={`fixed bg-white rounded-lg shadow-lg will-change-transform`}
        style={{
          transform: `translate(${position.x}px, ${position.y}px)`,
          transition: isDragging ? "none" : "transform 0.3s",
          width: isCollapsed ? "160px" : "320px",
          backgroundColor: "white",
          zIndex: 1000,
        }}
      >
        {/* 主控件头部 */}
        <div
          className="cursor-move py-3 px-4 bg-gray-100 rounded-t-lg flex justify-between items-center"
          onMouseDown={handleDragStart}
        >
          {isCollapsed ? (
            <div className="flex items-center space-x-3 w-full">
              <Radiation
                className={`w-5 h-5 ${
                  xrayState.isPowered ? "text-red-500" : "text-gray-400"
                }`}
              />
              <div className="text-xs">
                <div>{xrayState.voltage}kV</div>
                <div>{xrayState.current}µA</div>
              </div>
              <button
                onClick={() => setIsCollapsed(!isCollapsed)}
                className="p-1.5 hover:bg-gray-200 rounded ml-auto"
              >
                <ChevronDown className="w-4 h-4" />
              </button>
            </div>
          ) : (
            <>
              <h3 className="text-sm font-semibold">X-ray Control</h3>
              <button
                onClick={() => setIsCollapsed(!isCollapsed)}
                className="p-1.5 hover:bg-gray-200 rounded"
              >
                <ChevronUp className="w-4 h-4" />
              </button>
            </>
          )}
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
              {/* Voltage Display */}
              <div className="space-y-2">
                <div className="flex justify-between items-center">
                  <label className="text-sm font-medium">Voltage (kV)</label>
                  <div className="flex items-center space-x-2">
                    <input
                      type="number"
                      min="0"
                      max="180"
                      value={xrayState.voltage}
                      onChange={(e) => {
                        const value = parseInt(e.target.value);
                        if (!isNaN(value)) {
                          handleVoltageChange(value);
                        }
                      }}
                      disabled={!xrayState.isPowered}
                      className={`w-20 p-1 border rounded ${
                        xrayState.isPowered
                          ? "border-gray-300 bg-white"
                          : "border-gray-200 bg-gray-100 text-gray-500 cursor-not-allowed"
                      }`}
                    />
                  </div>
                </div>
              </div>

              {/* Current Display */}
              <div className="space-y-2">
                <div className="flex justify-between items-center">
                  <label className="text-sm font-medium">Current (µA)</label>
                  <div className="flex items-center space-x-2">
                    <input
                      type="number"
                      value={xrayState.current}
                      onChange={(e) =>
                        handleCurrentChange(parseInt(e.target.value))
                      }
                      disabled={!xrayState.isPowered}
                      className={`w-20 p-1 border rounded ${
                        xrayState.isPowered
                          ? "border-gray-300 bg-white"
                          : "border-gray-200 bg-gray-100 text-gray-500 cursor-not-allowed"
                      }`}
                    />
                  </div>
                </div>
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
                  <select
                    value={xrayState.focus}
                    onChange={(e) =>
                      handleFocusChange(parseInt(e.target.value))
                    }
                    disabled={!xrayState.isPowered}
                    className="bg-gray-50 border border-gray-300 rounded-md"
                  >
                    {spotSizes.map((mode) => (
                      <option key={mode} value={mode}>
                        F{mode}
                      </option>
                    ))}
                  </select>
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
      </div>
    </>
  );
};

export default XrayControl;
