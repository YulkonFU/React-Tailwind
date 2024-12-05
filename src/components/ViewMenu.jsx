import React from 'react';

const ViewMenu = () => {
  return (
    <div className="absolute bg-white shadow-lg rounded p-2 mt-20 z-50">
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Single View</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Quad View</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Fullscreen</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Restore</button>
    </div>
  );
};

export default ViewMenu;