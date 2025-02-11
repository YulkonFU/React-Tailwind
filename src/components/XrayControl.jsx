import { useState, useEffect, useCallback } from "react";
import { Radiation, AlertTriangle, ChevronUp, ChevronDown } from "lucide-react";
import PropTypes from "prop-types";

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

  // 获取handler引用，异步调用
  const getHandler = () => {
    return window.chrome?.webview?.hostObjects?.deviceHandler;
  };

  // 初始化
  useEffect(() => {
    const initXray = async () => {
      try {
        const handler = getHandler();
        if (!handler) throw new Error("Device handler not available");

        // Initialize Xray - dispId 1
        await handler.initializeXray;

        // 获取状态
        await updateStatus();

        log("Xray initialized successfully");
      } catch (err) {
        console.error("Failed to initialize Xray:", err);
        setWarningMessage(err.message);
        setShowWarning(true);
      }
    };

    initXray();
  }, []);

  // 更新状态
  const updateStatus = async () => {
    try {
      const handler = getHandler();
      if (!handler) throw new Error("Device handler not available");

      // getXrayStatus - dispId 8
      const status = await handler.getXrayStatus;
      const statusData = JSON.parse(status);

      log("Got Xray status:", statusData);

      setXrayState((prev) => ({
        ...prev,
        isPowered: statusData.isPowered,
        isWarmedUp: statusData.isWarmedUp,
        voltage: statusData.voltage,
        current: statusData.current,
        status: statusData.status,
        focus: statusData.focus,
      }));

      onStatusChange?.(statusData.status);
    } catch (err) {
      console.error("Failed to update status:", err);
      setWarningMessage(err.message);
      setShowWarning(true);
    }
  };

  // 处理电压改变
  const handleVoltageChange = async (value) => {
    try {
      const handler = getHandler();
      if (!handler) throw new Error("Device handler not available");

      const newVoltage = parseInt(
        typeof value === "object" ? value.target.value : value
      );

      if (isNaN(newVoltage) || newVoltage < 0 || newVoltage > 180) {
        throw new Error("Invalid voltage value (0-180)");
      }

      // 使用属性设置方式而不是方法调用
      handler.voltage = newVoltage;

      await updateStatus();
    } catch (err) {
      console.error("Failed to set voltage:", err);
      setWarningMessage(err.message);
      setShowWarning(true);
    }
  };

  // 处理电流改变
  const handleCurrentChange = async (value) => {
    try {
      const handler = getHandler();
      if (!handler) throw new Error("Device handler not available");

      const newCurrent = parseInt(
        typeof value === "object" ? value.target.value : value
      );

      if (isNaN(newCurrent) || newCurrent < 0 || newCurrent > 500) {
        throw new Error("Invalid current value (0-500µA)");
      }

      handler.current = newCurrent;


      await updateStatus();
    } catch (err) {
      console.error("Failed to set current:", err);
      setWarningMessage(err.message);
      setShowWarning(true);
    }
  };

  // 处理开关
  const handlePowerToggle = async () => {
    try {
      const handler = getHandler();
      if (!handler) throw new Error("Device handler not available");

      if (!xrayState.isPowered) {
        // turnXrayOn - dispId 5
        await handler.turnXrayOn;
      } else {
        // turnXrayOff - dispId 6
        await handler.turnXrayOff;
      }

      await updateStatus();
    } catch (err) {
      console.error("Power toggle failed:", err);
      setWarningMessage(err.message);
      setShowWarning(true);
    }
  };

  // 处理聚焦模式改变
  const handleFocusChange = async (mode) => {
    try {
      const handler = getHandler();
      if (!handler) throw new Error("Device handler not available");

      // setFocus - dispId 7
      await handler.setFocus(mode);

      await updateStatus();
    } catch (err) {
      console.error("Failed to set focus mode:", err);
      setWarningMessage(err.message);
      setShowWarning(true);
    }
  };

  // 拖动相关处理
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

  // 计算功率
  const calculatePower = () => {
    return ((xrayState.voltage * xrayState.current) / 1000).toFixed(2);
  };

  // 拖动事件监听
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

  // // 定期更新状态
  // useEffect(() => {
  //   const interval = setInterval(updateStatus, 1000);
  //   return () => clearInterval(interval);
  // }, []);

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
                  <span className="text-sm">Mode {xrayState.focus}</span>
                </div>
                <div className="grid grid-cols-4 gap-2">
                  {[0, 1, 2, 3].map((mode) => (
                    <button
                      key={mode}
                      onClick={() => handleFocusChange(mode)}
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
      </div>
    </>
  );
};

XrayControl.propTypes = {
  onStatusChange: PropTypes.func.isRequired,
};

export default XrayControl;
