import React, { useState, useEffect, useRef } from 'react';
import { Image as ImageIcon, Save, Grid, Filter, MessageSquare, Maximize2, Minimize2 } from 'lucide-react';
import * as PIXI from 'pixi.js';
import FileMenu from '../components/FileMenu';
import EditMenu from '../components/EditMenu';
import ViewMenu from '../components/ViewMenu';
import ModulesMenu from '../components/ModulesMenu';
import SetupMenu from '../components/SetupMenu';
import HelpMenu from '../components/HelpMenu';

const XactLayout = () => {
  const [activeMenu, setActiveMenu] = useState(null);
  const [xrayOn, setXrayOn] = useState(false); // 定义 xrayOn 状态变量
  const [isExpanded, setIsExpanded] = useState(false); // 定义 isExpanded 状态变量
  const menuRef = useRef(null);
  const pixiContainerRef = useRef(null);
  const pixiAppRef = useRef(null);

  const toggleMenu = (menu) => {
    setActiveMenu(activeMenu === menu ? null : menu);
  };

  const handleClickOutside = (event) => {
    if (menuRef.current && !menuRef.current.contains(event.target)) {
      setActiveMenu(null);
    }
  };

  useEffect(() => {
    document.addEventListener('mousedown', handleClickOutside);
    return () => {
      document.removeEventListener('mousedown', handleClickOutside);
    };
  }, []);

  useEffect(() => {
    const initPixiApp = async () => {
      if (pixiContainerRef.current) {
        // 创建应用实例
        const app = new PIXI.Application();
        
        // 使用 init() 方法初始化
        await app.init({
          width: pixiContainerRef.current.clientWidth,
          height: pixiContainerRef.current.clientHeight,
          backgroundColor: 0x000000,
          antialias: true,
        });

        // 使用 canvas 替代 view
        pixiContainerRef.current.appendChild(app.canvas);
        pixiAppRef.current = app;

        // 加载并显示图像
        const loadAndDisplayImage = async (imageUrl) => {
          await PIXI.Assets.load(imageUrl);
          const sprite = PIXI.Sprite.from(imageUrl);
          sprite.width = 384; // 设置精灵宽度
          sprite.height = 384; // 设置精灵高度
          sprite.x = (app.screen.width - sprite.width) / 2; // 居中显示
          sprite.y = (app.screen.height - sprite.height) / 2; // 居中显示
          app.stage.addChild(sprite);
        };

        // 示例图像 URL
        const imageUrl = 'https://pixijs.com/assets/files/sample-747abf529b135a1f549dff3ec846afbc.png';
        await loadAndDisplayImage(imageUrl);
      }
    };

    initPixiApp().catch(console.error);

    return () => {
      if (pixiAppRef.current) {
        pixiAppRef.current.destroy(true, { children: true, texture: true, baseTexture: true });
      }
    };
  }, []);

  return (
    <div className="flex h-screen bg-gray-100">
      {/* Left Sidebar - Tool Buttons */}
      <div className="w-16 bg-gray-800 p-2 flex flex-col gap-4">
        <button className="p-2 text-black hover:bg-gray-700 rounded">
          <ImageIcon className="w-6 h-6" />
        </button>
        <button className="p-2 text-black hover:bg-gray-700 rounded">
          <Save className="w-6 h-6" />
        </button>
        <button className="p-2 text-black hover:bg-gray-700 rounded">
          <Grid className="w-6 h-6" />
        </button>
        <button className="p-2 text-black hover:bg-gray-700 rounded">
          <Filter className="w-6 h-6" />
        </button>
        <button className="p-2 text-black hover:bg-gray-700 rounded">
          <MessageSquare className="w-6 h-6" />
        </button>
      </div>

      {/* Main Content Area */}
      <div className="flex-1 flex flex-col">
        {/* Top Menu Bar */}
        <div
          className="bg-white h-12 border-b flex items-center px-4 gap-4 relative"
          ref={menuRef}
        >
          <button
            className="px-3 py-1 text-sm hover:bg-gray-200 rounded"
            onClick={() => toggleMenu('file')}
          >
            File
          </button>
          <button
            className="px-3 py-1 text-sm hover:bg-gray-200 rounded"
            onClick={() => toggleMenu('edit')}
          >
            Edit
          </button>
          <button
            className="px-3 py-1 text-sm hover:bg-gray-200 rounded"
            onClick={() => toggleMenu('view')}
          >
            View
          </button>
          <button
            className="px-3 py-1 text-sm hover:bg-gray-200 rounded"
            onClick={() => toggleMenu('modules')}
          >
            Modules
          </button>
          <button
            className="px-3 py-1 text-sm hover:bg-gray-200 rounded"
            onClick={() => toggleMenu('setup')}
          >
            Setup
          </button>
          <button
            className="px-3 py-1 text-sm hover:bg-gray-200 rounded"
            onClick={() => toggleMenu('help')}
          >
            Help
          </button>

          {activeMenu === 'file' && <FileMenu />}
          {activeMenu === 'edit' && <EditMenu />}
          {activeMenu === 'view' && <ViewMenu />}
          {activeMenu === 'modules' && <ModulesMenu />}
          {activeMenu === 'setup' && <SetupMenu />}
          {activeMenu === 'help' && <HelpMenu />}
        </div>

        {/* Main Workspace */}
        <div className={`flex-1 flex flex-col lg:flex-row ${isExpanded ? 'fixed inset-0 z-40' : ''}`}>
          {/* Image View Area */}                    
          <div 
            className={`flex-1 bg-black relative ${isExpanded ? 'w-full h-full' : ''}`} 
            style={{ 
              width: '500px',  // 设置固定宽度
              height: '500px', // 设置固定高度
              position: 'relative'
            }} 
          >
            <div className="absolute top-4 right-4 flex gap-2">
              <button 
                className="p-2 bg-gray-800 text-white rounded hover:bg-gray-700"
                onClick={() => setIsExpanded(!isExpanded)}
              >
                {isExpanded ? <Minimize2 className="w-4 h-4" /> : <Maximize2 className="w-4 h-4" />}
              </button>
            </div>
            {/* PIXI 容器 */}
            <div 
              ref={pixiContainerRef}
              style={{ 
                width: '100%',
                height: '100%',
                position: 'absolute',
                top: 0,
                left: 0
              }}
            />
          </div>

          {/* Right Control Panel */}
          {!isExpanded && (
            <div className="w-full lg:w-80 bg-white border-t lg:border-t-0 lg:border-l flex flex-col">
              {/* X-ray Controls */}
              <div className="p-4 border-b">
                <h3 className="text-sm font-semibold mb-4">X-ray Control</h3>
                <div className="space-y-4">
                  <div className="flex items-center justify-between">
                    <span className="text-sm">X-ray Power</span>
                    <button 
                      className={`px-4 py-2 rounded ${xrayOn ? 'bg-red-600' : 'bg-gray-200'} text-white`}
                      onClick={() => setXrayOn(!xrayOn)}
                    >
                      {xrayOn ? 'ON' : 'OFF'}
                    </button>
                  </div>
                  <div className="space-y-2">
                    <label className="text-sm">kV</label>
                    <input type="range" className="w-full" />
                  </div>
                  <div className="space-y-2">
                    <label className="text-sm">µA</label>
                    <input type="range" className="w-full" />
                  </div>
                </div>
              </div>

              {/* Navigation Controls */}
              <div className="p-4 border-b">
                <h3 className="text-sm font-semibold mb-4">Navigation</h3>
                <div className="grid grid-cols-3 gap-2">
                  <button className="p-2 border rounded hover:bg-gray-50">X+</button>
                  <button className="p-2 border rounded hover:bg-gray-50">Y+</button>
                  <button className="p-2 border rounded hover:bg-gray-50">Z+</button>
                  <button className="p-2 border rounded hover:bg-gray-50">X-</button>
                  <button className="p-2 border rounded hover:bg-gray-50">Y-</button>
                  <button className="p-2 border rounded hover:bg-gray-50">Z-</button>
                </div>
              </div>

              {/* Image Processing */}
              <div className="p-4 border-b">
                <h3 className="text-sm font-semibold mb-4">Image Processing</h3>
                <div className="space-y-4">
                  <button className="w-full py-2 bg-blue-600 text-white rounded hover:bg-blue-700">
                    Capture Image
                  </button>
                  <button className="w-full py-2 border rounded hover:bg-gray-50">
                    Apply Filter
                  </button>
                </div>
              </div>
            </div>
          )}
        </div>

        {/* Bottom Status Bar */}
        {!isExpanded && (
          <div className="h-8 bg-gray-800 text-white px-4 flex items-center text-sm">
            <span>Ready</span>
          </div>
        )}
      </div>
    </div>
  );
};

export default XactLayout;