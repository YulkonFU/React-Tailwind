import React from 'react';

const HelpMenu = () => {
  return (
    <div className="absolute bg-white shadow-lg rounded p-2 mt-20 z-50">
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Documentation</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Support</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">About</button>
    </div>
  );
};

export default HelpMenu;