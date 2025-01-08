import { useState } from "react";
import {
  Gamepad2,
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
    currentPositions: [],
  });

  const [showAxisPanel, setShowAxisPanel] = useState(false);
  const [tempAxisCount, setTempAxisCount] = useState("");
  const [editingAxis, setEditingAxis] = useState(null);
  const [moveValue, setMoveValue] = useState("");

  const handleSetAxisCount = () => {
    const count = parseInt(tempAxisCount);
    if (count > 0 && count <= 12) {
      setManipulatorState((prev) => ({
        ...prev,
        axisCount: count,
        currentPositions: Array(count).fill(0),
      }));
      setTempAxisCount("");
    }
  };

  const handleAxisClick = (index) => {
    setEditingAxis(index);
    setMoveValue("");
  };

  const handleMoveValueChange = (value) => {
    if (/^[+-]?\d*\.?\d*$/.test(value)) {
      setMoveValue(value);
    }
  };

  const handleMove = () => {
    if (editingAxis === null || moveValue === "") return;

    const moveVal = parseFloat(moveValue);
    if (isNaN(moveVal)) return;

    setManipulatorState((prev) => {
      const newPositions = [...prev.currentPositions];
      newPositions[editingAxis] = parseFloat(
        (newPositions[editingAxis] + moveVal).toFixed(3)
      );

      return {
        ...prev,
        status: "CNC_REACHED",
        currentPositions: newPositions,
      };
    });

    setEditingAxis(null);
    setMoveValue("");
  };

  const handleStop = () => {
    setEditingAxis(null);
    setMoveValue("");
    setManipulatorState((prev) => ({
      ...prev,
      status: "CNC_IS_READY",
    }));
  };

  return (
    <div className="p-4 bg-gray-100 rounded-lg space-y-4">
      {/* Header */}
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-4">
          <h3 className="text-sm font-semibold">Manipulator Control</h3>
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
          </div>
          {manipulatorState.axisCount === 0 && (
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
                onClick={handleSetAxisCount}
                className="px-3 py-1 text-sm bg-blue-500 text-white rounded hover:bg-blue-600"
              >
                Configure
              </button>
            </div>
          )}
        </div>

        {/* Axis Position Display */}
        {manipulatorState.axisCount > 0 && (
          <div className="p-3 border-t">
            <div className="grid grid-cols-3 gap-4">
              {manipulatorState.currentPositions.map((pos, index) => (
                <div key={index} className="flex bg-gray-50 rounded border">
                  <div className="flex items-center px-2 font-medium">
                    {String.fromCharCode(65 + index)}
                  </div>
                  <div className="border-l border-gray-200 bg-white rounded-r flex-1">
                    {editingAxis === index ? (
                      <input
                        type="text"
                        value={moveValue}
                        onChange={(e) => handleMoveValueChange(e.target.value)}
                        className="w-full px-2 py-1 focus:outline-none text-sm"
                        placeholder="Enter move value"
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
                disabled={editingAxis === null}
                className={`px-4 py-2 rounded ${
                  editingAxis !== null
                    ? "bg-blue-500 text-white hover:bg-blue-600"
                    : "bg-gray-200 text-gray-500"
                }`}
              >
                Move To Position
              </button>
              <button
                onClick={handleStop}
                className="px-4 py-2 bg-red-500 text-white rounded hover:bg-red-600"
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
          onClick={() =>
            setManipulatorState((prev) => ({
              ...prev,
              isJoyEnabled: !prev.isJoyEnabled,
            }))
          }
          className={`flex items-center gap-2 px-3 py-1.5 rounded ${
            manipulatorState.isJoyEnabled
              ? "bg-green-500 text-white"
              : "bg-gray-200 hover:bg-gray-300"
          }`}
        >
          <Gamepad2 className="w-6 h-6" />
          <span className="text-sm">
            {manipulatorState.isJoyEnabled ? "Enabled" : "Disabled"}
          </span>
        </button>

        <button
          onClick={() =>
            setManipulatorState((prev) => ({
              ...prev,
              isCollisionDetected: !prev.isCollisionDetected,
            }))
          }
          className={`p-2 rounded hover:bg-gray-100 ${
            manipulatorState.isCollisionDetected
              ? "text-red-500"
              : "text-green-500"
          }`}
        >
          <Radar className="w-6 h-6" />
        </button>

        <div
          className={`p-2 rounded ${
            manipulatorState.isDoorOpen ? "text-red-500" : "text-green-500"
          }`}
        >
          <DoorClosed className="w-6 h-6" />
        </div>
      </div>
    </div>
  );
};

export default ManipulatorControl;
