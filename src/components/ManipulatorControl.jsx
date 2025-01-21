import { useState, useEffect } from "react";
import {
  Gamepad2,
  Lock,
  Radar,
  DoorClosed,
  ChevronDown,
  ChevronUp,
} from "lucide-react";

const DEBUG = true;

function log(...args) {
  if (DEBUG) {
    console.log("[ManipulatorControl]", ...args);
  }
}

const ALL_AXIS = -1; // Common constant for all axes

const ManipulatorControl = () => {
  // State management
  const [manipulatorState, setManipulatorState] = useState({
    status: "CNC_NOT_READY",
    isJoyEnabled: false,
    isDoorOpen: false,
    isCollisionDetected: false,
    isReferencing: false,
    axisCount: 0,
    currentPositions: [],
    axisNames: [],
  });

  const [showAxisPanel, setShowAxisPanel] = useState(true);
  const [editingAxis, setEditingAxis] = useState(null);
  const [moveValue, setMoveValue] = useState("");
  const [error, setError] = useState(null);

  // Get handler reference
  const getHandler = () => {
    return window.chrome?.webview?.hostObjects?.deviceHandler;
  };

  // Initialize CNC and get axis configuration
  useEffect(() => {
    const initializeCnc = async () => {
      try {
        const handler = getHandler();
        if (!handler) throw new Error("Device handler not available");

        // Initialize CNC - dispId 101
        await handler.initializeCnc;
        log("CNC initialized");
        
        // Get axis info - dispId 108
        const axesInfo = JSON.parse(await handler.getAxesInfo);
        log("Axes info:", axesInfo);

        // Update state
        setManipulatorState(prev => ({
          ...prev,
          axisCount: axesInfo.axisCount,
          currentPositions: Array(axesInfo.axisCount).fill(0),
          axisNames: axesInfo.axes.map(axis => axis.name)
        }));

        await updateStatus();

      } catch (err) {
        console.error("Failed to initialize CNC:", err);
        setError(err.message);
      }
    };

    initializeCnc();
  }, []);

  // Update status
  const updateStatus = async () => {
    try {
      const handler = getHandler();
      if (!handler) throw new Error("Device handler not available");

      // Get status - dispId 107
      const status = JSON.parse(await handler.getCncStatus);
      log("CNC status:", status);

      // Get positions - dispId 109
      const positions = JSON.parse(await handler.getPositions);
      log("Current positions:", positions);

      setManipulatorState(prev => ({
        ...prev,
        status: status.status,
        isJoyEnabled: status.isJoyEnabled,
        isDoorOpen: status.isDoorOpen,
        isCollisionDetected: status.isCollisionDetected,
        currentPositions: positions
      }));

      setError(null);

    } catch (err) {
      console.error("Failed to update status:", err);
      setError(err.message);
    }
  };

  // Handle reference operation
  const handleReference = async () => {
    try {
      const handler = getHandler();
      if (!handler) throw new Error("Device handler not available");

      setManipulatorState(prev => ({
        ...prev,
        isReferencing: true,
        status: "CNC_DRIVING_REF"
      }));

      // Start reference - dispId 102
      await handler.startReference;
      await updateStatus();

      setError(null);

    } catch (err) {
      console.error("Reference operation failed:", err);
      setError(err.message);
    } finally {
      setManipulatorState(prev => ({
        ...prev,
        isReferencing: false
      }));
    }
  };

  // Handle axis click
  const handleAxisClick = (index) => {
    setEditingAxis(index);
    setMoveValue("");
    setError(null);
  };

  // Handle move operation
  const handleMove = async () => {
    if (editingAxis === null || moveValue === "") return;

    try {
      const handler = getHandler();
      if (!handler) throw new Error("Device handler not available");

      const moveVal = parseFloat(moveValue);
      if (isNaN(moveVal)) {
        throw new Error("Invalid move value");
      }

      // Move axis - dispId 103
      await handler.moveAxis(editingAxis, moveVal);
      await updateStatus();

      setEditingAxis(null);
      setMoveValue("");
      setError(null);

    } catch (err) {
      console.error("Move operation failed:", err);
      setError(err.message);
    }
  };

  // Handle stop operation
  const handleStop = async () => {
    try {
      const handler = getHandler();
      if (!handler) throw new Error("Device handler not available");

      // Stop - dispId 105
      await handler.stop(ALL_AXIS);
      await updateStatus();

      setEditingAxis(null);
      setMoveValue("");
      setError(null);

    } catch (err) {
      console.error("Stop operation failed:", err);
      setError(err.message);
    }
  };

  // Handle joystick toggle
  const handleJoystickToggle = async () => {
    try {
      const handler = getHandler();
      if (!handler) throw new Error("Device handler not available");

      // Enable/disable joy - dispId 106
      await handler.enableJoy(ALL_AXIS, !manipulatorState.isJoyEnabled);
      await updateStatus();
      
      setError(null);

    } catch (err) {
      console.error("Joystick toggle failed:", err);
      setError(err.message);
    }
  };

  // Validate input value
  const handleMoveValueChange = (value) => {
    if (/^[+-]?\d*\.?\d*$/.test(value)) {
      setMoveValue(value);
      setError(null);
    }
  };

  // // 定期更新状态
  // useEffect(() => {
  //   const interval = setInterval(updateStatus, 1000);
  //   return () => clearInterval(interval);
  // }, []);

  return (
    <div className="p-4 bg-gray-100 rounded-lg space-y-4">
      {/* Header */}
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-4">
          <h3 className="text-sm font-semibold">Manipulator Control</h3>
          <button
            onClick={handleReference}
            disabled={manipulatorState.status === "CNC_NOT_READY"}
            className={`p-2 rounded flex items-center justify-center gap-2 ${
              manipulatorState.isReferencing
                ? "bg-blue-500 text-white"
                : manipulatorState.status === "CNC_NOT_READY"
                ? "bg-gray-300 cursor-not-allowed"
                : "bg-white hover:bg-gray-50"
            }`}
          >
            <Lock className="w-4 h-4" />
            <span>Reference</span>
          </button>
        </div>

        {/* Status Indicator */}
        <div className="flex items-center space-x-2">
          <span
            className={`inline-block w-2 h-2 rounded-full ${
              manipulatorState.status === "CNC_REACHED"
                ? "bg-green-500"
                : manipulatorState.status === "CNC_MOVING"
                ? "bg-blue-500"
                : manipulatorState.status === "CNC_IS_READY"
                ? "bg-yellow-500"
                : "bg-red-500"
            }`}
          />
          <span className="text-xs text-gray-600">
            {manipulatorState.status}
          </span>
        </div>
      </div>

      {/* Position Control */}
      <div className="bg-white rounded-lg">
        <div className="p-3 flex items-center justify-between">
          <div className="flex items-center gap-2">
            <span className="text-sm font-medium">Position Control</span>
            <button
              onClick={() => setShowAxisPanel(!showAxisPanel)}
              className="p-1 hover:bg-gray-100 rounded"
            >
              {showAxisPanel ? (
                <ChevronUp className="w-4 h-4" />
              ) : (
                <ChevronDown className="w-4 h-4" />
              )}
            </button>
          </div>
        </div>

        {/* Axis Position Display */}
        {showAxisPanel && manipulatorState.axisCount > 0 && (
          <div className="p-3 border-t">
            <div className="grid grid-cols-3 gap-4">
              {manipulatorState.currentPositions.map((pos, index) => (
                <div key={index} className="flex bg-gray-50 rounded border">
                  <div className="flex items-center px-2 font-medium">
                    {manipulatorState.axisNames[index] || String.fromCharCode(65 + index)}
                  </div>
                  <div className="border-l border-gray-200 bg-white rounded-r flex-1">
                    {editingAxis === index ? (
                      <input
                        type="text"
                        value={moveValue}
                        onChange={(e) => handleMoveValueChange(e.target.value)}
                        onKeyDown={(e) => e.key === 'Enter' && handleMove()}
                        className="w-full px-2 py-1 focus:outline-none text-sm"
                        placeholder="Enter position"
                        autoFocus
                      />
                    ) : (
                      <div
                        className="px-2 py-1 cursor-pointer hover:bg-gray-50"
                        onClick={() => handleAxisClick(index)}
                      >
                        {pos.toFixed(3)}
                      </div>
                    )}
                  </div>
                </div>
              ))}
            </div>

            {/* Control Buttons */}
            <div className="mt-4 flex justify-end space-x-2">
              <button
                onClick={handleMove}
                disabled={editingAxis === null || manipulatorState.status === "CNC_NOT_READY"}
                className={`px-4 py-2 rounded ${
                  editingAxis !== null && manipulatorState.status !== "CNC_NOT_READY"
                    ? "bg-blue-500 text-white hover:bg-blue-600"
                    : "bg-gray-200 text-gray-500 cursor-not-allowed"
                }`}
              >
                Move To Position
              </button>
              <button
                onClick={handleStop}
                disabled={manipulatorState.status === "CNC_NOT_READY"}
                className={`px-4 py-2 rounded ${
                  manipulatorState.status === "CNC_NOT_READY"
                    ? "bg-gray-200 text-gray-500 cursor-not-allowed"
                    : "bg-red-500 text-white hover:bg-red-600"
                }`}
              >
                Stop
              </button>
            </div>
          </div>
        )}
      </div>

      {/* Status Controls */}
      <div className="flex items-center justify-between p-2 bg-white rounded gap-2">
        <button
          onClick={handleJoystickToggle}
          disabled={manipulatorState.status === "CNC_NOT_READY"}
          className={`flex items-center gap-2 px-3 py-1.5 rounded ${
            manipulatorState.status === "CNC_NOT_READY"
              ? "bg-gray-200 text-gray-500 cursor-not-allowed"
              : manipulatorState.isJoyEnabled
              ? "bg-green-500 text-white"
              : "bg-gray-200 hover:bg-gray-300"
          }`}
        >
          <Gamepad2 className="w-6 h-6" />
          <span className="text-sm">
            {manipulatorState.isJoyEnabled ? "Enabled" : "Disabled"}
          </span>
        </button>

        <div 
          className={`p-2 rounded ${
            manipulatorState.isCollisionDetected
              ? "text-red-500"
              : "text-green-500"
          }`}
          title="Collision Detection Status"
        >
          <Radar className="w-6 h-6" />
        </div>

        <div
          className={`p-2 rounded ${
            manipulatorState.isDoorOpen ? "text-red-500" : "text-green-500"
          }`}
          title="Door Status"
        >
          <DoorClosed className="w-6 h-6" />
        </div>
      </div>

      {/* Error Display */}
      {error && (
        <div className="p-3 bg-red-100 border border-red-300 text-red-700 rounded-lg">
          <p className="text-sm">{error}</p>
        </div>
      )}
    </div>
  );
};

export default ManipulatorControl;