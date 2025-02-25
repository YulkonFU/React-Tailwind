```jsx
import React, { useState } from 'react';
import { Camera, Settings, Layers, Image as ImageIcon, Play, Save, Filter, Grid, Maximize2, MessageSquare } from 'lucide-react';

const XactLayout = () => {
  const [xrayOn, setXrayOn] = useState(false);

  return (
    <div className="flex h-screen bg-gray-100">
      {/* Left Sidebar - Tool Buttons */}
      <div className="w-16 bg-gray-800 p-2 flex flex-col gap-4">
        <button className="p-2 text-white hover:bg-gray-700 rounded">
          <ImageIcon className="w-6 h-6" />
        </button>
        <button className="p-2 text-white hover:bg-gray-700 rounded">
          <Save className="w-6 h-6" />
        </button>
        <button className="p-2 text-white hover:bg-gray-700 rounded">
          <Grid className="w-6 h-6" />
        </button>
        <button className="p-2 text-white hover:bg-gray-700 rounded">
          <Filter className="w-6 h-6" />
        </button>
        <button className="p-2 text-white hover:bg-gray-700 rounded">
          <MessageSquare className="w-6 h-6" />
        </button>
      </div>

      {/* Main Content Area */}
      <div className="flex-1 flex flex-col">
        {/* Top Menu Bar */}
        <div className="bg-white h-12 border-b flex items-center px-4 gap-4">
          <button className="px-3 py-1 text-sm hover:bg-gray-100 rounded">File</button>
          <button className="px-3 py-1 text-sm hover:bg-gray-100 rounded">Edit</button>
          <button className="px-3 py-1 text-sm hover:bg-gray-100 rounded">View</button>
          <button className="px-3 py-1 text-sm hover:bg-gray-100 rounded">Modules</button>
          <button className="px-3 py-1 text-sm hover:bg-gray-100 rounded">Setup</button>
          <button className="px-3 py-1 text-sm hover:bg-gray-100 rounded">Help</button>
        </div>

        {/* Main Workspace */}
        <div className="flex-1 flex">
          {/* Image View Area */}
          <div className="flex-1 bg-black p-4 relative">
            <div className="absolute top-4 right-4 flex gap-2">
              <button className="p-2 bg-gray-800 text-white rounded hover:bg-gray-700">
                <Maximize2 className="w-4 h-4" />
              </button>
            </div>
            {/* Placeholder for X-ray image */}
            <div className="w-full h-full border-2 border-dashed border-gray-600 flex items-center justify-center">
              <span className="text-gray-500">X-ray Image View</span>
            </div>
          </div>

          {/* Right Control Panel */}
          <div className="w-80 bg-white border-l flex flex-col">
            {/* X-ray Controls */}
            <div className="p-4 border-b">
              <h3 className="text-sm font-semibold mb-4">X-ray Control</h3>
              <div className="space-y-4">
                <div className="flex items-center justify-between">
                  <span className="text-sm">X-ray Power</span>
                  <button 
                    className={`px-4 py-2 rounded ${xrayOn ? 'bg-red-600' : 'bg-gray-200'} text-white`}
                    onClick={() => setXrayOn(!xrayOn)}
                  >
                    {xrayOn ? 'ON' : 'OFF'}
                  </button>
                </div>
                <div className="space-y-2">
                  <label className="text-sm">kV</label>
                  <input type="range" className="w-full" />
                </div>
                <div className="space-y-2">
                  <label className="text-sm">µA</label>
                  <input type="range" className="w-full" />
                </div>
              </div>
            </div>

            {/* Navigation Controls */}
            <div className="p-4 border-b">
              <h3 className="text-sm font-semibold mb-4">Navigation</h3>
              <div className="grid grid-cols-3 gap-2">
                <button className="p-2 border rounded hover:bg-gray-50">X+</button>
                <button className="p-2 border rounded hover:bg-gray-50">Y+</button>
                <button className="p-2 border rounded hover:bg-gray-50">Z+</button>
                <button className="p-2 border rounded hover:bg-gray-50">X-</button>
                <button className="p-2 border rounded hover:bg-gray-50">Y-</button>
                <button className="p-2 border rounded hover:bg-gray-50">Z-</button>
              </div>
            </div>

            {/* Image Processing */}
            <div className="p-4 border-b">
              <h3 className="text-sm font-semibold mb-4">Image Processing</h3>
              <div className="space-y-4">
                <button className="w-full py-2 bg-blue-600 text-white rounded hover:bg-blue-700">
                  Capture Image
                </button>
                <button className="w-full py-2 border rounded hover:bg-gray-50">
                  Apply Filter
                </button>
              </div>
            </div>
          </div>
        </div>

        {/* Bottom Status Bar */}
        <div className="h-8 bg-gray-800 text-white px-4 flex items-center text-sm">
          <span>Ready</span>
        </div>
      </div>
    </div>
  );
};

export default XactLayout;
```

