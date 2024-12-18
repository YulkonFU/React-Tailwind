import { useState } from 'react';
import { Play, Square, Clock, Sliders, RotateCcw, ChevronDown, BarChart2, Settings, Maximize2 } from 'lucide-react';

const IMAGE_FILTERS = [
  'Average', 'Blur', 'Difference', 'ENR', 'Gauss', 'HighLight',
  'HighLight_weak', 'Horizontal-edge', 'Kirsch 360°', 'Median',
  'NDT Filter', 'New Macro', 'Prewitt', 'Relief', 'Roberts',
  'Save As', 'Sharpen', 'Smooth', 'Sobel'
];

const DetectorControl = () => {
  // 图像采集状态
  const [acquisitionState, setAcquisitionState] = useState({
    isLive: false,
    isIntegrating: false,
    fps: 30,
    integrationFrames: 1,
    gain: 1.0,
    histogram: false,
    timing: {
      exposure: 100,
      delay: 0
    }
  });

  // 图像处理状态
  const [selectedFilters, setSelectedFilters] = useState(['none', 'none', 'none', 'none']);
  const [filterHistory, setFilterHistory] = useState([]);

  // Acquire Live 控制
  const toggleLive = () => {
    setAcquisitionState(prev => ({
      ...prev,
      isLive: !prev.isLive
    }));
  };

  // 积分控制
  const handleIntegrationChange = (frames) => {
    setAcquisitionState(prev => ({
      ...prev,
      integrationFrames: frames
    }));
  };

  // FPS 控制
  const handleFpsChange = (value) => {
    setAcquisitionState(prev => ({
      ...prev,
      fps: value
    }));
  };

  // Camera Gain 控制
  const handleGainChange = (value) => {
    setAcquisitionState(prev => ({
      ...prev,
      gain: value
    }));
  };

  // 图像滤镜控制
  const handleFilterChange = (index, value) => {
    const newFilters = [...selectedFilters];
    newFilters[index] = value;
    setSelectedFilters(newFilters);
    setFilterHistory([...filterHistory, selectedFilters]);
  };

  // 重置/撤销
  const handleUndo = () => {
    if (filterHistory.length > 0) {
      const previousState = filterHistory[filterHistory.length - 1];
      setSelectedFilters(previousState);
      setFilterHistory(filterHistory.slice(0, -1));
    } else {
      setSelectedFilters(['none', 'none', 'none', 'none']);
    }
  };

  return (
    <div className="space-y-4">
      {/* Acquire Control Section */}
      <div className="bg-gray-100 rounded-lg p-4">
        <h3 className="text-sm font-medium mb-4">Acquisition Control</h3>
        
        <div className="space-y-4">
          {/* Live/Integration Controls */}
          <div className="flex gap-4">
            <button
              onClick={toggleLive}
              className={`flex items-center gap-2 px-4 py-2 rounded ${
                acquisitionState.isLive
                  ? 'bg-green-500 text-white'
                  : 'bg-gray-200 hover:bg-gray-300'
              }`}
            >
              {acquisitionState.isLive ? <Square className="w-4 h-4" /> : <Play className="w-4 h-4" />}
              {acquisitionState.isLive ? 'Stop Live' : 'Start Live'}
            </button>

            <select
              value={acquisitionState.integrationFrames}
              onChange={(e) => handleIntegrationChange(parseInt(e.target.value))}
              className="bg-gray-200 rounded px-3 py-2"
            >
              <option value={1}>No Integration</option>
              <option value={2}>2 Frames</option>
              <option value={4}>4 Frames</option>
              <option value={8}>8 Frames</option>
            </select>
          </div>

          {/* Timing/FPS Control */}
          <div className="space-y-2">
            <div className="flex items-center gap-2">
              <Clock className="w-4 h-4" />
              <span className="text-sm">FPS Control</span>
            </div>
            <input
              type="range"
              min="1"
              max="60"
              value={acquisitionState.fps}
              onChange={(e) => handleFpsChange(parseInt(e.target.value))}
              className="w-full"
            />
            <div className="text-right text-sm">{acquisitionState.fps} fps</div>
          </div>

          {/* Camera Gain Control */}
          <div className="space-y-2">
            <div className="flex items-center gap-2">
              <Sliders className="w-4 h-4" />
              <span className="text-sm">Camera Gain</span>
            </div>
            <input
              type="range"
              min="1"
              max="10"
              step="0.1"
              value={acquisitionState.gain}
              onChange={(e) => handleGainChange(parseFloat(e.target.value))}
              className="w-full"
            />
            <div className="text-right text-sm">{acquisitionState.gain.toFixed(1)}x</div>
          </div>
        </div>
      </div>

      {/* Image Processing Section */}
      <div className="bg-gray-100 rounded-lg p-4">
        <h3 className="text-sm font-medium mb-4">Image Processing</h3>

        <div className="space-y-2">
          {/* Histogram Toggle */}
          <button
            onClick={() => setAcquisitionState(prev => ({ ...prev, histogram: !prev.histogram }))}
            className={`w-full flex items-center justify-between px-3 py-2 rounded ${
              acquisitionState.histogram ? 'bg-blue-500 text-white' : 'bg-gray-200'
            }`}
          >
            <div className="flex items-center gap-2">
              <BarChart2 className="w-4 h-4" />
              <span>Histogram</span>
            </div>
          </button>

          {/* Filter Controls */}
          {[0, 1, 2, 3].map((index) => (
            <div key={index} className="flex items-center gap-2">
              <div className="w-4 h-4 flex items-center">▶</div>
              <div className="relative w-full">
                <select
                  value={selectedFilters[index]}
                  onChange={(e) => handleFilterChange(index, e.target.value)}
                  className="w-full appearance-none bg-gray-200 text-gray-700 py-2 px-3 pr-8 rounded leading-tight focus:outline-none cursor-pointer"
                >
                  <option value="none">&lt;none&gt;</option>
                  {IMAGE_FILTERS.map((filter) => (
                    <option key={filter} value={filter}>{filter}</option>
                  ))}
                </select>
                <div className="pointer-events-none absolute inset-y-0 right-0 flex items-center px-2 text-gray-700">
                  <ChevronDown className="h-4 w-4" />
                </div>
              </div>
            </div>
          ))}

          {/* Undo Button */}
          <button
            onClick={handleUndo}
            className="w-full flex items-center justify-start gap-2 bg-gray-200 text-gray-700 py-2 px-3 rounded hover:bg-gray-300"
          >
            <RotateCcw className="h-4 w-4" />
            <span>Undo</span>
          </button>
        </div>
      </div>

      {/* Display Controls */}
      <div className="bg-gray-100 rounded-lg p-4">
        <h3 className="text-sm font-medium mb-4">Display Controls</h3>
        <div className="space-y-2">
          <button className="w-full flex items-center justify-between px-3 py-2 bg-gray-200 rounded hover:bg-gray-300">
            <span>Multi-View</span>
            <Maximize2 className="w-4 h-4" />
          </button>
          
          <button className="w-full flex items-center justify-between px-3 py-2 bg-gray-200 rounded hover:bg-gray-300">
            <span>Overlay</span>
            <Settings className="w-4 h-4" />
          </button>
        </div>
      </div>
    </div>
  );
};

export default DetectorControl;