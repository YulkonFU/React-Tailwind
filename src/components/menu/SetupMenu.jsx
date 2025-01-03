import { Settings2, Database, Users, Save } from 'lucide-react';

const SetupMenu = () => {
  return (
    <div className="flex gap-8">
      <div className="space-y-4">
        <h3 className="text-sm font-semibold text-gray-600">System Setup</h3>
        <div className="flex gap-4">
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Settings2 className="w-5 h-5" />
            <span className="text-xs">Preferences</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Database className="w-5 h-5" />
            <span className="text-xs">System Settings</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Users className="w-5 h-5" />
            <span className="text-xs">User Management</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Save className="w-5 h-5" />
            <span className="text-xs">Backup & Restore</span>
          </button>
        </div>
      </div>
    </div>
  );
};

export default SetupMenu;