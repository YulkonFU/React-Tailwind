import { Grid, Map, Box, Filter } from 'lucide-react';

const ModulesMenu = () => {
  return (
    <div className="flex gap-8">
      <div className="space-y-4">
        <h3 className="text-sm font-semibold text-gray-600">Available Modules</h3>
        <div className="flex gap-4">
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Grid className="w-5 h-5" />
            <span className="text-xs">Measurement</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Map className="w-5 h-5" />
            <span className="text-xs">Sample Map</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Box className="w-5 h-5" />
            <span className="text-xs">Pseudo 3D</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Filter className="w-5 h-5" />
            <span className="text-xs">Image Filters</span>
          </button>
        </div>
      </div>
    </div>
  );
};

export default ModulesMenu;