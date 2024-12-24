
const ModulesMenu = () => {
  return (
    <div className="absolute bg-white shadow-lg rounded p-2 mt-20 z-50">
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Measurement Modules</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Show Sample Map</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Show Pseudo 3D</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Show Image Filters</button>
    </div>
  );
};

export default ModulesMenu;