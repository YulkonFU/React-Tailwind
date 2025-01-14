import { useState, useEffect, useCallback } from "react";
import { Radiation, AlertTriangle, ChevronUp, ChevronDown } from "lucide-react";

const DEBUG = true;

function log(...args) {
  if (DEBUG) {
    console.log("[XrayControl]", ...args);
  }
}

const XrayControl = () => {
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
  // useEffect(() => {
  //   const initSpotSizes = async () => {
  //     const count = await callXrayHandler("getSpotsizeCount");
  //     setSpotSizes(Array.from({ length: count }, (_, i) => i));
  //   };
  //   initSpotSizes();
  // }, []);

  // XrayControl.jsx 修改 callXrayHandler:
  // ...existing code...

  const callXrayHandler = async (method, ...args) => {
    try {
        const handler = window.chrome?.webview?.hostObjects?.sync?.xrayHandler;
        if (!handler) {
            throw new Error("XrayHandler not available");
        }

        // 直接调用方法
        const result = await handler[method];
        console.log(`${method} result:`, result);
        return result;
    } catch (err) {
        console.error(`Error in callXrayHandler(${method}):`, err);
        throw err;
    }
};

const handlePowerToggle = async () => {
    try {
        const handler = window.chrome?.webview?.hostObjects?.sync?.xrayHandler;
        if (!handler) {
            throw new Error("XrayHandler not available");
        }

        if (!xrayState.isPowered) {
            await handler.turnOn;  // 直接访问属性
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

  // 电压和电流从状态更新中获取
  const updateStatus = async () => {
    try {
      const status = await callXrayHandler("getStatus");
      const statusData = JSON.parse(status);
      setXrayState((prev) => ({
        ...prev,
        isPowered: statusData.isPowered,
        isWarmedUp: statusData.isWarmedUp,
        status: statusData.status,
        voltage: statusData.voltage, // 从状态更新中获取实际值
        current: statusData.current, // 从状态更新中获取实际值
      }));
    } catch (err) {
      console.error("Failed to update status:", err);
    }
  };

  // 处理电压改变
  const handleVoltageChange = async (e) => {
    const newVoltage = parseInt(e.target.value);
    try {
      await callXrayHandler("setVoltage", newVoltage);
      setXrayState((prev) => ({ ...prev, voltage: newVoltage }));
    } catch (err) {
      console.error("Failed to set voltage:", err);
    }
  };

  // 处理电流改变
  const handleCurrentChange = async (e) => {
    const newCurrent = parseInt(e.target.value);
    try {
      await callXrayHandler("setCurrent", newCurrent);
      setXrayState((prev) => ({ ...prev, current: newCurrent }));
    } catch (err) {
      console.error("Failed to set current:", err);
    }
  };

  // 处理聚焦模式改变
  const handleFocusChange = async (mode) => {
    try {
      await callXrayHandler("setFocus", mode);
      setXrayState((prev) => ({ ...prev, focus: mode }));
    } catch (err) {
      console.error("Failed to set focus mode:", err);
    }
  };

  // 拖拽相关处理函数
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

  // 处理拖拽事件监听
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
                <div className="flex justify-between">
                  <label className="text-sm font-medium">Voltage (kV)</label>
                  <span className="text-sm">{xrayState.voltage} kV</span>
                </div>
                <div className="w-full h-2 bg-gray-200 rounded">
                  <div
                    className="h-full bg-blue-500 rounded"
                    style={{ width: `${(xrayState.voltage / 200) * 100}%` }}
                  />
                </div>
              </div>

              {/* Current Display */}
              <div className="space-y-2">
                <div className="flex justify-between">
                  <label className="text-sm font-medium">Current (µA)</label>
                  <span className="text-sm">{xrayState.current} µA</span>
                </div>
                <div className="w-full h-2 bg-gray-200 rounded">
                  <div
                    className="h-full bg-blue-500 rounded"
                    style={{ width: `${(xrayState.current / 500) * 100}%` }}
                  />
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
