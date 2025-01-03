import { Maximize2, Grid, Monitor, RotateCcw } from 'lucide-react';

const ViewMenu = () => {
  return (
    <div className="flex gap-8">
      <div className="space-y-4">
        <h3 className="text-sm font-semibold text-gray-600">View Mode</h3>
        <div className="flex gap-4">
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Monitor className="w-5 h-5" />
            <span className="text-xs">Single View</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Grid className="w-5 h-5" />
            <span className="text-xs">Quad View</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Maximize2 className="w-5 h-5" />
            <span className="text-xs">Fullscreen</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <RotateCcw className="w-5 h-5" />
            <span className="text-xs">Restore</span>
          </button>
        </div>
      </div>
    </div>
  );
};

export default ViewMenu;