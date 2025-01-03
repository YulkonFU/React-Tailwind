import { BookOpen, HeadphonesIcon, Info } from 'lucide-react';

const HelpMenu = () => {
  return (
    <div className="flex gap-8">
      <div className="space-y-4">
        <div className="flex gap-4">
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <BookOpen className="w-5 h-5" />
            <span className="text-xs">Documentation</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <HeadphonesIcon className="w-5 h-5" />
            <span className="text-xs">Support</span>
          </button>
          <button className="flex flex-col items-center gap-1 p-2 rounded hover:bg-gray-100">
            <Info className="w-5 h-5" />
            <span className="text-xs">About</span>
          </button>
        </div>
      </div>
    </div>
  );
};

export default HelpMenu;