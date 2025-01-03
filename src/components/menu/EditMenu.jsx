import { Undo2, Redo2, Scissors, Copy, ClipboardCopy } from 'lucide-react';

const EditMenu = () => {
  return (
    <div className="flex gap-8">
      <div className="space-y-4">
        <div className="flex gap-4">
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <ClipboardCopy className="w-5 h-5" />
            <span className="text-xs">Paste</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Scissors className="w-5 h-5" />
            <span className="text-xs">Cut</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Copy className="w-5 h-5" />
            <span className="text-xs">Copy</span>
          </button>
        </div>
      </div>

      <div className="space-y-4">
        <div className="flex gap-4">
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Undo2 className="w-5 h-5" />
            <span className="text-xs">Undo</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Redo2 className="w-5 h-5" />
            <span className="text-xs">Redo</span>
          </button>
        </div>
      </div>
    </div>
  );
};

export default EditMenu;