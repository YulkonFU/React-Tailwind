import { FilePlus, FolderOpen, Save, SaveAll } from "lucide-react";

const FileMenu = () => {
  return (
    <div className="flex gap-4 p-2">
      <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
        <FilePlus className="w-5 h-5" />
        <span className="text-xs">New</span>
      </button>
      <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
        <FolderOpen className="w-5 h-5" />
        <span className="text-xs">Open</span>
      </button>
      <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
        <Save className="w-5 h-5" />
        <span className="text-xs">Save</span>
      </button>
      <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
        <SaveAll className="w-5 h-5" />
        <span className="text-xs">Save As</span>
      </button>
    </div>
  );
};

export default FileMenu;
