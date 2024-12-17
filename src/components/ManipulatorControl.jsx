import { useState, useEffect } from "react";
import {
  Gamepad2,
  StopCircle,
  Lock,
  Radar,
  DoorClosed,
  ChevronDown,
  ChevronUp,
} from "lucide-react";

const ManipulatorControl = () => {
  const [manipulatorState, setManipulatorState] = useState({
    status: "CNC_NOT_READY",
    isJoyEnabled: false,
    isDoorOpen: false,
    isCollisionDetected: false,
    isReferencing: false,
    axisCount: 0,
    positions: [],
  });

  const [showAxisPanel, setShowAxisPanel] = useState(false);
  const [tempAxisCount, setTempAxisCount] = useState("");
  // 添加状态
  const [moveTimeout, setMoveTimeout] = useState(null);

  // 设置轴数并初始化位置数据
  const handleSetAxisCount = () => {
    const count = parseInt(tempAxisCount);
    if (count > 0 && count <= 12) {
      const newPositions = Array(count)
        .fill(0)
        .map(() => parseFloat("0.000")); // 确保初始值为数字
      setManipulatorState((prev) => ({
        ...prev,
        axisCount: count,
        positions: newPositions,
      }));
      setTempAxisCount("");
    }
  };

  // 更新位置值
  const handlePositionChange = (index, value) => {
    const newPositions = [...manipulatorState.positions];
    newPositions[index] = parseFloat(value) || 0;
    setManipulatorState((prev) => ({
      ...prev,
      positions: newPositions,
    }));
  };

  // 移动到指定位置
  // 修改 handleMove 函数
  const handleMove = () => {
    console.log("Moving to positions:", manipulatorState.positions);
    setManipulatorState((prev) => ({
      ...prev,
      status: "CNC_MOVING",
    }));

    const timeout = setTimeout(() => {
      setManipulatorState((prev) => ({
        ...prev,
        status: "CNC_REACHED",
      }));
      setMoveTimeout(null);
    }, 3000);

    setMoveTimeout(timeout);
  };

  // 添加 handleStop 函数
  const handleStop = () => {
    if (moveTimeout) {
      clearTimeout(moveTimeout);
      setMoveTimeout(null);
      setManipulatorState((prev) => ({
        ...prev,
        status: "CNC_IS_READY",
      }));
    }
  };

  // 添加清除定时器的副作用
  useEffect(() => {
    return () => {
      if (moveTimeout) {
        clearTimeout(moveTimeout);
      }
    };
  }, [moveTimeout]);

  return (
    <div className="p-4 bg-gray-100 rounded-lg space-y-4">
      {/* Header */}
      <div className="flex items-center justify-between mb-4">
        <h3 className="text-sm font-semibold">Manipulator Control</h3>
        {/* 更新状态指示器 */}
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

      {/* Position Configuration */}
      <div className="bg-white rounded-lg">
        <div
          className="p-3 flex items-center justify-between cursor-pointer"
          onClick={() =>
            manipulatorState.axisCount > 0 && setShowAxisPanel(!showAxisPanel)
          }
        >
          <div className="flex items-center gap-2">
            <span className="text-sm font-medium">Position Control</span>
            {manipulatorState.axisCount > 0 &&
              (showAxisPanel ? (
                <ChevronUp className="w-4 h-4" />
              ) : (
                <ChevronDown className="w-4 h-4" />
              ))}
          </div>
          <div className="flex items-center gap-2">
            {manipulatorState.axisCount === 0 ? (
              <div className="flex gap-2">
                <input
                  type="number"
                  min="1"
                  max="12"
                  value={tempAxisCount}
                  onChange={(e) => setTempAxisCount(e.target.value)}
                  className="w-20 p-1 border rounded text-sm"
                  placeholder="1-12"
                />
                <button
                  onClick={(e) => {
                    e.stopPropagation();
                    handleSetAxisCount();
                  }}
                  className="px-3 py-1 text-sm bg-blue-500 text-white rounded hover:bg-blue-600"
                >
                  Configure
                </button>
              </div>
            ) : (
              <span className="text-sm text-gray-500">
                {manipulatorState.axisCount} axes configured
              </span>
            )}
          </div>
        </div>

        {/* Axis Position Inputs */}
        {showAxisPanel && manipulatorState.axisCount > 0 && (
          <div className="p-3 border-t">
            <div className="grid grid-cols-3 gap-4">
              {manipulatorState.positions.map((pos, index) => (
                <div
                  key={index}
                  className="flex bg-gray-100 rounded border p-0.5"
                >
                  <div className="flex items-center px-2 font-medium">
                    {String.fromCharCode(65 + index)}
                  </div>
                  <div className="border-l border-gray-300 bg-white rounded-r flex-1">
                    <input
                      type="text"
                      value={`${pos >= 0 ? "+" : ""}${pos.toFixed(3)}`}
                      onChange={(e) => {
                        const inputValue = e.target.value;
                        // 允许输入负号、小数点和数字
                        if (/^[+-]?\d*\.?\d*$/.test(inputValue)) {
                          const numValue = parseFloat(inputValue);
                          if (!isNaN(numValue)) {
                            handlePositionChange(index, numValue);
                          }
                        }
                      }}
                      className="w-full px-2 py-1 focus:outline-none"
                      style={{ fontFamily: "monospace" }}
                    />
                  </div>
                </div>
              ))}
            </div>
            <div className="mt-4 flex justify-end space-x-2">
              <button
                onClick={handleMove}
                disabled={moveTimeout !== null}
                className={`px-4 py-2 bg-blue-500 text-white rounded hover:bg-blue-600 ${
                  moveTimeout !== null ? "opacity-50 cursor-not-allowed" : ""
                }`}
              >
                Move To Position
              </button>
              <button
                onClick={handleStop}
                disabled={moveTimeout === null}
                className={`px-4 py-2 bg-red-500 text-white rounded hover:bg-red-600 ${
                  moveTimeout === null ? "opacity-50 cursor-not-allowed" : ""
                }`}
              >
                Stop
              </button>
            </div>
          </div>
        )}
      </div>

      {/* Control Buttons */}
      <div className="grid grid-cols-2 gap-2">
        <button
          onClick={() => {
            setManipulatorState((prev) => ({
              ...prev,
              isReferencing: true,
              status: "CNC_DRIVING_REF",
            }));
            setTimeout(() => {
              setManipulatorState((prev) => ({
                ...prev,
                isReferencing: false,
                status: "CNC_STAND_STILL",
              }));
            }, 2000);
          }}
          className={`p-2 rounded flex items-center justify-center gap-2 ${
            manipulatorState.isReferencing
              ? "bg-blue-500 text-white"
              : "bg-white hover:bg-gray-50"
          }`}
        >
          <Lock className="w-4 h-4" />
          <span>Reference</span>
        </button>
      </div>

      {/* Status and Controls */}
      <div className="space-y-2">
        <div className="flex items-center justify-between p-2 bg-white rounded">
          <div className="flex items-center gap-2">
            <Gamepad2 className="w-4 h-4" />
            <span className="text-sm">Joystick Control</span>
          </div>
          <button
            onClick={() =>
              setManipulatorState((prev) => ({
                ...prev,
                isJoyEnabled: !prev.isJoyEnabled,
              }))
            }
            className={`px-3 py-1 rounded text-sm ${
              manipulatorState.isJoyEnabled
                ? "bg-green-500 text-white"
                : "bg-gray-200 hover:bg-gray-300"
            }`}
          >
            {manipulatorState.isJoyEnabled ? "Enabled" : "Disabled"}
          </button>
        </div>

        <div className="flex items-center justify-between p-2 bg-white rounded">
          <div className="flex items-center gap-2">
            <Radar className="w-4 h-4" />
            <span className="text-sm">Collision Detection</span>
          </div>
          <span
            className={`px-2 py-1 rounded text-xs ${
              manipulatorState.isCollisionDetected
                ? "bg-red-100 text-red-600"
                : "bg-green-100 text-green-600"
            }`}
          >
            {manipulatorState.isCollisionDetected ? "Detected" : "Clear"}
          </span>
        </div>

        <div className="flex items-center justify-between p-2 bg-white rounded">
          <div className="flex items-center gap-2">
            <DoorClosed className="w-4 h-4" />
            <span className="text-sm">Door Status</span>
          </div>
          <span
            className={`px-2 py-1 rounded text-xs ${
              manipulatorState.isDoorOpen
                ? "bg-red-100 text-red-600"
                : "bg-green-100 text-green-600"
            }`}
          >
            {manipulatorState.isDoorOpen ? "Open" : "Closed"}
          </span>
        </div>
      </div>
    </div>
  );
};

export default ManipulatorControl;
