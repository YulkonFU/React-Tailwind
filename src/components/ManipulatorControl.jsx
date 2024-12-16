import { useState } from 'react';
import { Joystick, StopCircle, Lock, Radar, DoorClosed } from 'lucide-react';

const ManipulatorControl = () => {
  const [manipulatorState, setManipulatorState] = useState({
    status: 'CNC_NOT_READY',
    isJoyEnabled: false,
    position: { x: 0, y: 0, z: 0 },
    isDoorOpen: false,
    isCollisionDetected: false,
    isReferencing: false
  });

  // 处理参考点设置
  const handleReference = () => {
    setManipulatorState(prev => ({
      ...prev,
      status: 'CNC_DRIVING_REF',
      isReferencing: true
    }));
    
    // 模拟参考点设置过程
    setTimeout(() => {
      setManipulatorState(prev => ({
        ...prev,
        status: 'CNC_STAND_STILL',
        isReferencing: false
      }));
    }, 2000);
  };

  // 处理紧急停止
  const handleStop = () => {
    setManipulatorState(prev => ({
      ...prev,
      status: 'CNC_STAND_STILL',
      isReferencing: false
    }));
  };

  // 处理摇杆启用/禁用
  const toggleJoystick = () => {
    setManipulatorState(prev => ({
      ...prev,
      isJoyEnabled: !prev.isJoyEnabled
    }));
  };

  // 位置调整函数
  const adjustPosition = (axis, value) => {
    setManipulatorState(prev => ({
      ...prev,
      position: {
        ...prev.position,
        [axis]: prev.position[axis] + value
      }
    }));
  };

  return (
    <div className="p-4 bg-gray-100 rounded-lg space-y-4">
      {/* Header */}
      <div className="flex items-center justify-between mb-4">
        <h3 className="text-sm font-semibold">Manipulator Control</h3>
        <div className="flex items-center space-x-2">
          <span className={`inline-block w-2 h-2 rounded-full ${
            manipulatorState.status === 'CNC_STAND_STILL' ? 'bg-green-500' : 'bg-yellow-500'
          }`} />
          <span className="text-xs text-gray-600">{manipulatorState.status}</span>
        </div>
      </div>

      {/* Position Display */}
      <div className="grid grid-cols-3 gap-2 bg-white p-3 rounded-lg">
        <div className="space-y-1">
          <label className="text-xs text-gray-600">X Position</label>
          <div className="flex items-center justify-between">
            <span className="text-sm font-medium">{manipulatorState.position.x.toFixed(3)}</span>
            <div className="flex flex-col">
              <button 
                className="px-2 py-1 text-xs hover:bg-gray-100 rounded"
                onClick={() => adjustPosition('x', 0.001)}
              >+</button>
              <button 
                className="px-2 py-1 text-xs hover:bg-gray-100 rounded"
                onClick={() => adjustPosition('x', -0.001)}
              >-</button>
            </div>
          </div>
        </div>
        <div className="space-y-1">
          <label className="text-xs text-gray-600">Y Position</label>
          <div className="flex items-center justify-between">
            <span className="text-sm font-medium">{manipulatorState.position.y.toFixed(3)}</span>
            <div className="flex flex-col">
              <button 
                className="px-2 py-1 text-xs hover:bg-gray-100 rounded"
                onClick={() => adjustPosition('y', 0.001)}
              >+</button>
              <button 
                className="px-2 py-1 text-xs hover:bg-gray-100 rounded"
                onClick={() => adjustPosition('y', -0.001)}
              >-</button>
            </div>
          </div>
        </div>
        <div className="space-y-1">
          <label className="text-xs text-gray-600">Z Position</label>
          <div className="flex items-center justify-between">
            <span className="text-sm font-medium">{manipulatorState.position.z.toFixed(3)}</span>
            <div className="flex flex-col">
              <button 
                className="px-2 py-1 text-xs hover:bg-gray-100 rounded"
                onClick={() => adjustPosition('z', 0.001)}
              >+</button>
              <button 
                className="px-2 py-1 text-xs hover:bg-gray-100 rounded"
                onClick={() => adjustPosition('z', -0.001)}
              >-</button>
            </div>
          </div>
        </div>
      </div>

      {/* Control Buttons */}
      <div className="grid grid-cols-2 gap-2">
        <button
          onClick={handleReference}
          className={`p-2 rounded flex items-center justify-center gap-2 ${
            manipulatorState.isReferencing
              ? 'bg-blue-500 text-white'
              : 'bg-white hover:bg-gray-50'
          }`}
        >
          <Lock className="w-4 h-4" />
          <span>Reference</span>
        </button>
        <button
          onClick={handleStop}
          className="p-2 bg-red-500 text-white rounded flex items-center justify-center gap-2 hover:bg-red-600"
        >
          <StopCircle className="w-4 h-4" />
          <span>Stop</span>
        </button>
      </div>

      {/* Status and Controls */}
      <div className="space-y-2">
        <div className="flex items-center justify-between p-2 bg-white rounded">
          <div className="flex items-center gap-2">
            <Joystick className="w-4 h-4" />
            <span className="text-sm">Joystick Control</span>
          </div>
          <button
            onClick={toggleJoystick}
            className={`px-3 py-1 rounded text-sm ${
              manipulatorState.isJoyEnabled
                ? 'bg-green-500 text-white'
                : 'bg-gray-200 hover:bg-gray-300'
            }`}
          >
            {manipulatorState.isJoyEnabled ? 'Enabled' : 'Disabled'}
          </button>
        </div>

        <div className="flex items-center justify-between p-2 bg-white rounded">
          <div className="flex items-center gap-2">
            <Radar className="w-4 h-4" />
            <span className="text-sm">Collision Detection</span>
          </div>
          <span className={`px-2 py-1 rounded text-xs ${
            manipulatorState.isCollisionDetected
              ? 'bg-red-100 text-red-600'
              : 'bg-green-100 text-green-600'
          }`}>
            {manipulatorState.isCollisionDetected ? 'Detected' : 'Clear'}
          </span>
        </div>

        <div className="flex items-center justify-between p-2 bg-white rounded">
          <div className="flex items-center gap-2">
            <DoorClosed  className="w-4 h-4" />
            <span className="text-sm">Door Status</span>
          </div>
          <span className={`px-2 py-1 rounded text-xs ${
            manipulatorState.isDoorOpen
              ? 'bg-red-100 text-red-600'
              : 'bg-green-100 text-green-600'
          }`}>
            {manipulatorState.isDoorOpen ? 'Open' : 'Closed'}
          </span>
        </div>
      </div>
    </div>
  );
};

export default ManipulatorControl;