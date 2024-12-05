import React from 'react';

const FileMenu = () => {
  return (
    <div className="absolute bg-white shadow-lg rounded p-2 mt-20 z-50">
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">New</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Open</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Save</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Save As</button>
    </div>
  );
};

export default FileMenu;