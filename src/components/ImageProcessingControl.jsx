import { useState } from 'react';
import { Camera, Sliders, RotateCw, Filter } from 'lucide-react';

const ImageProcessingControl = () => {
  const [processingState, setProcessingState] = useState({
    isLive: false,
    contrast: 50,
    brightness: 50,
    integration: 1
  });

  const handleCapture = () => {
    console.log('Capturing image');
  };

  const handleFilterChange = (type, value) => {
    setProcessingState(prev => ({
      ...prev,
      [type]: value
    }));
  };

  const toggleLive = () => {
    setProcessingState(prev => ({
      ...prev,
      isLive: !prev.isLive
    }));
  };

  return (
    <div className="p-4 bg-gray-100 rounded-lg space-y-4">
      {/* Header */}
      <div className="flex items-center justify-between mb-4">
        <h3 className="text-sm font-semibold">Image Processing</h3>
        <button
          onClick={toggleLive}
          className={`px-3 py-1 rounded text-sm ${
            processingState.isLive
              ? 'bg-green-500 text-white'
              : 'bg-gray-200 hover:bg-gray-300'
          }`}
        >
          {processingState.isLive ? 'Live' : 'Off'}
        </button>
      </div>

      {/* Capture Controls */}
      <div className="flex gap-2">
        <button
          onClick={handleCapture}
          className="flex-1 py-2 bg-blue-600 text-white rounded hover:bg-blue-700 flex items-center justify-center gap-2"
        >
          <Camera className="w-4 h-4" />
          <span>Capture</span>
        </button>
      </div>

      {/* Image Adjustments */}
      <div className="space-y-4">
        <div className="space-y-2">
          <label className="text-sm font-medium flex items-center gap-2">
            <Sliders className="w-4 h-4" />
            Contrast
          </label>
          <input
            type="range"
            min="0"
            max="100"
            value={processingState.contrast}
            onChange={(e) => handleFilterChange('contrast', parseInt(e.target.value))}
            className="w-full"
          />
        </div>
        
        <div className="space-y-2">
          <label className="text-sm font-medium flex items-center gap-2">
            <Sliders className="w-4 h-4" />
            Brightness
          </label>
          <input
            type="range"
            min="0"
            max="100"
            value={processingState.brightness}
            onChange={(e) => handleFilterChange('brightness', parseInt(e.target.value))}
            className="w-full"
          />
        </div>

        <div className="space-y-2">
          <label className="text-sm font-medium flex items-center gap-2">
            <RotateCw className="w-4 h-4" />
            Integration
          </label>
          <select
            className="w-full p-2 border rounded"
            value={processingState.integration}
            onChange={(e) => handleFilterChange('integration', parseInt(e.target.value))}
          >
            <option value={1}>1 Frame</option>
            <option value={2}>2 Frames</option>
            <option value={4}>4 Frames</option>
            <option value={8}>8 Frames</option>
          </select>
        </div>
      </div>

      {/* Filter Button */}
      <button className="w-full py-2 border rounded hover:bg-gray-50 flex items-center justify-center gap-2">
        <Filter className="w-4 h-4" />
        <span>Apply Filter</span>
      </button>
    </div>
  );
};

export default ImageProcessingControl;