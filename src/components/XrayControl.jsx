import { useState } from "react";
import { Radiation, AlertTriangle, Plug2 } from "lucide-react";

const XrayControl = () => {
  const [xrayState, setXrayState] = useState({
    connected: false,
    isPowered: false,
    isWarmedUp: false,
    voltage: 130,
    current: 300,
    focus: 0, // 0-3 之间的离散值
    status: "XR_NOT_READY",
  });

  const [showWarning, setShowWarning] = useState(false);
  const [warningMessage, setWarningMessage] = useState("");
  const [showWarmupDialog, setShowWarmupDialog] = useState(false);

  const handleConnect = () => {
    if (xrayState.connected) {
      // 检查 X-ray 是否开启
      if (xrayState.isPowered) {
        setWarningMessage("Please turn off X-ray before disconnecting.");
        setShowWarning(true);
        return;
      }
      // 断开连接逻辑
      setXrayState((prev) => ({
        ...prev,
        connected: false,
        status: "XR_NOT_READY",
      }));
    } else {
      // 连接逻辑
      setXrayState((prev) => ({
        ...prev,
        connected: true,
        status: "XR_READY",
      }));
    }
  };

  const handlePowerToggle = () => {
    if (!xrayState.connected) {
      setWarningMessage("X-ray not connected. Please check connection.");
      setShowWarning(true);
      return;
    }

    if (!xrayState.isPowered) {
      setXrayState((prev) => ({
        ...prev,
        isPowered: true,
        status: "XR_IS_COLD",
      }));
      setShowWarmupDialog(true);
    } else {
      setXrayState((prev) => ({
        ...prev,
        isPowered: false,
        isWarmedUp: false,
        status: "XR_IS_OFF",
      }));
    }
  };

  const handleWarmupConfirm = () => {
    setShowWarmupDialog(false);
    // Start warmup process
    setTimeout(() => {
      setXrayState((prev) => ({
        ...prev,
        isWarmedUp: true,
        status: "XR_IS_ON",
      }));
    }, 2000);
  };

  const handleWarmupCancel = () => {
    setShowWarmupDialog(false);
    // Reset to OFF state
    setXrayState((prev) => ({
      ...prev,
      isPowered: false,
      status: "XR_IS_OFF",
    }));
  };

  // 添加新的处理函数
  const adjustValue = (type, amount) => {
    setXrayState((prev) => {
      let newValue;
      if (type === "voltage") {
        newValue = Math.min(200, Math.max(0, prev.voltage + amount));
        return { ...prev, voltage: newValue };
      } else if (type === "current") {
        newValue = Math.min(500, Math.max(0, prev.current + amount));
        return { ...prev, current: newValue };
      }
      return prev;
    });
  };

  return (
    <div className="p-4 bg-gray-100 rounded-lg space-y-4">
      {/* Warmup Dialog */}
      {showWarmupDialog && (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
          <div className="bg-white rounded-lg p-6 max-w-sm w-full mx-4">
            <p className="text-gray-800 mb-6">
              Current status is XR_IS_COLD. Do you want to proceed with warmup?
            </p>
            <div className="flex justify-end space-x-4">
              <button
                onClick={handleWarmupCancel}
                className="px-4 py-2 text-gray-600 hover:bg-gray-100 rounded"
              >
                No
              </button>
              <button
                onClick={handleWarmupConfirm}
                className="px-4 py-2 bg-blue-500 text-white hover:bg-blue-600 rounded"
              >
                Yes
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Header */}
      <div className="flex items-center justify-between mb-4">
        <h3 className="text-sm font-semibold">X-ray Control</h3>
        <button
          onClick={handleConnect}
          className={`flex items-center space-x-2 px-3 py-1 rounded ${
            xrayState.isPowered
              ? "bg-gray-100 cursor-not-allowed"
              : "bg-white hover:bg-gray-50"
          } border`}
        >
          <Plug2
            className={`w-4 h-4 ${
              xrayState.connected ? "text-green-500" : "text-gray-400"
            }`}
          />
          <span className="text-xs">
            {xrayState.connected ? "Disconnect" : "Connect"}
          </span>
        </button>
      </div>

      {/* Connection Status */}
      <div className="flex items-center space-x-2 text-xs">
        <div
          className={`w-2 h-2 rounded-full ${
            xrayState.connected ? "bg-green-500" : "bg-red-500"
          }`}
        />
        <span className="text-gray-600">
          {xrayState.connected ? "Connected" : "Not Connected"}
        </span>
      </div>

      {/* Warning Alert */}
      {showWarning && (
        <div className="flex items-center gap-2 p-3 mb-4 bg-red-100 border border-red-300 text-red-700 rounded-lg">
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

      {/* Power Control */}
      <div className="flex items-center justify-between p-3 bg-white rounded-lg shadow-sm">
        <div className="flex items-center space-x-2">
          <Radiation
            className={`w-5 h-5 ${
              xrayState.isPowered ? "text-red-600" : "text-gray-300"
            }`}
          />
          <span className="text-sm font-medium">X-ray Power</span>
        </div>
        <button
          onClick={handlePowerToggle}
          className={`px-4 py-2 rounded text-white ${
            xrayState.isPowered
              ? "bg-red-600 hover:bg-red-700"
              : "bg-gray-300 hover:bg-gray-400"
          }`}
        >
          {xrayState.isPowered ? "ON" : "OFF"}
        </button>
      </div>

      {/* Controls Section */}
      <div className={`space-y-4 ${!xrayState.isWarmedUp && "opacity-50"}`}>
        {/* Voltage Control */}
        <div className="space-y-2">
          <div className="flex justify-between">
            <label className="text-sm font-medium">Voltage (kV)</label>
            <div className="flex items-center gap-1">
              <span className="text-sm">{xrayState.voltage} kV</span>
              <div className="flex flex-col">
                <button
                  className="text-gray-600 hover:text-gray-800 disabled:text-gray-300"
                  onClick={() => adjustValue("voltage", 10)}
                  disabled={!xrayState.isWarmedUp}
                >
                  ⇧
                </button>
                <button
                  className="text-gray-600 hover:text-gray-800 disabled:text-gray-300"
                  onClick={() => adjustValue("voltage", -10)}
                  disabled={!xrayState.isWarmedUp}
                >
                  ⇩
                </button>
              </div>
              <div className="flex flex-col">
                <button
                  className="text-gray-600 hover:text-gray-800 disabled:text-gray-300"
                  onClick={() => adjustValue("voltage", 1)}
                  disabled={!xrayState.isWarmedUp}
                >
                  ↑
                </button>
                <button
                  className="text-gray-600 hover:text-gray-800 disabled:text-gray-300"
                  onClick={() => adjustValue("voltage", -1)}
                  disabled={!xrayState.isWarmedUp}
                >
                  ↓
                </button>
              </div>
            </div>
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
            disabled={!xrayState.isWarmedUp}
          />
        </div>

        {/* Current Control */}
        <div className="space-y-2">
          <div className="flex justify-between">
            <label className="text-sm font-medium">Current (µA)</label>
            <div className="flex items-center gap-1">
              <span className="text-sm">{xrayState.current} µA</span>
              <div className="flex flex-col">
                <button
                  className="text-gray-600 hover:text-gray-800 disabled:text-gray-300"
                  onClick={() => adjustValue("current", 10)}
                  disabled={!xrayState.isWarmedUp}
                >
                  ⇧
                </button>
                <button
                  className="text-gray-600 hover:text-gray-800 disabled:text-gray-300"
                  onClick={() => adjustValue("current", -10)}
                  disabled={!xrayState.isWarmedUp}
                >
                  ⇩
                </button>
              </div>
              <div className="flex flex-col">
                <button
                  className="text-gray-600 hover:text-gray-800 disabled:text-gray-300"
                  onClick={() => adjustValue("current", 1)}
                  disabled={!xrayState.isWarmedUp}
                >
                  ↑
                </button>
                <button
                  className="text-gray-600 hover:text-gray-800 disabled:text-gray-300"
                  onClick={() => adjustValue("current", -1)}
                  disabled={!xrayState.isWarmedUp}
                >
                  ↓
                </button>
              </div>
            </div>
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
            disabled={!xrayState.isWarmedUp}
          />
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
                disabled={!xrayState.isWarmedUp}
                className={`p-2 rounded ${
                  xrayState.focus === mode
                    ? "bg-blue-500 text-white"
                    : "bg-gray-100 text-gray-700 hover:bg-gray-200"
                } ${!xrayState.isWarmedUp && "opacity-50 cursor-not-allowed"}`}
              >
                F{mode}
              </button>
            ))}
          </div>
        </div>
      </div>

      {/* Status Display */}
      <div className="mt-4 p-3 bg-gray-200 rounded-lg">
        <div className="flex justify-between items-center">
          <span className="text-sm font-medium">Status</span>
          <span className="text-sm">{xrayState.status}</span>
        </div>
      </div>
    </div>
  );
};

export default XrayControl;
