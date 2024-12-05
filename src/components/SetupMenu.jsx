import React from 'react';

const SetupMenu = () => {
  return (
    <div className="absolute bg-white shadow-lg rounded p-2 mt-20 z-50">
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Preferences</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">System Settings</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">User Management</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Backup & Restore</button>
    </div>
  );
};

export default SetupMenu;