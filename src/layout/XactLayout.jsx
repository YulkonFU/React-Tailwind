import React, { useState, useEffect, useRef } from "react";
import {
  Image as ImageIcon,
  Save,
  Grid,
  Filter,
  MessageSquare,
  Maximize2,
  Minimize2,
} from "lucide-react";
import * as PIXI from "pixi.js";
import FileMenu from "../components/FileMenu";
import EditMenu from "../components/EditMenu";
import ViewMenu from "../components/ViewMenu";
import ModulesMenu from "../components/ModulesMenu";
import SetupMenu from "../components/SetupMenu";
import HelpMenu from "../components/HelpMenu";
import ImageViewer from "../components/ImageViewer";
import ImageControls from "../components/ImageControls";

const XactLayout = () => {
  const [activeMenu, setActiveMenu] = useState(null);
  const [xrayOn, setXrayOn] = useState(false); // 定义 xrayOn 状态变量
  const [isExpanded, setIsExpanded] = useState(false); // 定义 isExpanded 状态变量
  const menuRef = useRef(null);
  const pixiContainerRef = useRef(null);
  const pixiAppRef = useRef(null);
  const spriteRef = useRef(null);
  const [imageSize, setImageSize] = useState({ width: 384, height: 384 });

  const toggleMenu = (menu) => {
    setActiveMenu(activeMenu === menu ? null : menu);
  };

  const handleClickOutside = (event) => {
    if (menuRef.current && !menuRef.current.contains(event.target)) {
      setActiveMenu(null);
    }
  };

  // 处理图像加载完成后的逻辑
  const handleImageLoad = (sprite) => {
    spriteRef.current = sprite;

    // 设置初始属性
    sprite.anchor.set(0.5); // 设置旋转中心点
    sprite.interactive = true; // 启用交互
    sprite.buttonMode = true; // 显示手型光标

    // 添加拖拽功能
    let isDragging = false;
    let dragStart = { x: 0, y: 0 };

    sprite.on("pointerdown", (event) => {
      isDragging = true;
      dragStart = {
        x: event.data.global.x - sprite.x,
        y: event.data.global.y - sprite.y,
      };
    });

    sprite.on("pointermove", (event) => {
      if (isDragging) {
        sprite.x = event.data.global.x - dragStart.x;
        sprite.y = event.data.global.y - dragStart.y;
      }
    });

    sprite.on("pointerup", () => {
      isDragging = false;
    });

    sprite.on("pointerupoutside", () => {
      isDragging = false;
    });
  };

  useEffect(() => {
    document.addEventListener("mousedown", handleClickOutside);
    return () => {
      document.removeEventListener("mousedown", handleClickOutside);
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
            onClick={() => toggleMenu("file")}
          >
            File
          </button>
          <button
            className="px-3 py-1 text-sm hover:bg-gray-200 rounded"
            onClick={() => toggleMenu("edit")}
          >
            Edit
          </button>
          <button
            className="px-3 py-1 text-sm hover:bg-gray-200 rounded"
            onClick={() => toggleMenu("view")}
          >
            View
          </button>
          <button
            className="px-3 py-1 text-sm hover:bg-gray-200 rounded"
            onClick={() => toggleMenu("modules")}
          >
            Modules
          </button>
          <button
            className="px-3 py-1 text-sm hover:bg-gray-200 rounded"
            onClick={() => toggleMenu("setup")}
          >
            Setup
          </button>
          <button
            className="px-3 py-1 text-sm hover:bg-gray-200 rounded"
            onClick={() => toggleMenu("help")}
          >
            Help
          </button>

          {activeMenu === "file" && <FileMenu />}
          {activeMenu === "edit" && <EditMenu />}
          {activeMenu === "view" && <ViewMenu />}
          {activeMenu === "modules" && <ModulesMenu />}
          {activeMenu === "setup" && <SetupMenu />}
          {activeMenu === "help" && <HelpMenu />}
        </div>

        {/* Main Workspace */}
        <div
          className={`flex-1 flex flex-col lg:flex-row ${
            isExpanded ? "fixed inset-0 z-40" : ""
          }`}
        >
          {/* Image View Area */}
          <div
            className={`flex-1 bg-black relative ${
              isExpanded ? "w-full h-full" : ""
            }`}
            style={{
              width: "500px",
              height: "500px",
              position: "relative",
            }}
          >
            <ImageControls
              isExpanded={isExpanded}
              onToggleExpand={() => setIsExpanded(!isExpanded)}
              onRotate={(angle) => {
                if (spriteRef.current) {
                  spriteRef.current.rotation += (angle * Math.PI) / 180;
                }
              }}
              onZoom={(factor) => {
                if (spriteRef.current) {
                  spriteRef.current.scale.x *= factor;
                  spriteRef.current.scale.y *= factor;
                }
              }}
              onReset={() => {
                if (spriteRef.current) {
                  spriteRef.current.rotation = 0;
                  spriteRef.current.scale.set(1);
                  spriteRef.current.position.set(
                    pixiAppRef.current.screen.width / 2,
                    pixiAppRef.current.screen.height / 2
                  );
                }
              }}
              onEnhance={() => {
                if (spriteRef.current) {
                  const filters = spriteRef.current.filters || [];
                  if (filters.length === 0) {
                    // 添加多个滤镜效果
                    const colorMatrix = new PIXI.filters.ColorMatrixFilter();
                    const blur = new PIXI.filters.BlurFilter();
                    blur.blur = 0.5;

                    spriteRef.current.filters = [colorMatrix, blur];

                    // 调整颜色矩阵
                    colorMatrix.brightness(1.2); // 增加亮度
                    colorMatrix.contrast(1.1); // 增加对比度
                  } else {
                    spriteRef.current.filters = [];
                  }
                }
              }}
            />
            <ImageViewer onImageLoad={handleImageLoad} />
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
                      className={`px-4 py-2 rounded ${
                        xrayOn ? "bg-red-600" : "bg-gray-200"
                      } text-white`}
                      onClick={() => setXrayOn(!xrayOn)}
                    >
                      {xrayOn ? "ON" : "OFF"}
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
                  <button className="p-2 border rounded hover:bg-gray-50">
                    X+
                  </button>
                  <button className="p-2 border rounded hover:bg-gray-50">
                    Y+
                  </button>
                  <button className="p-2 border rounded hover:bg-gray-50">
                    Z+
                  </button>
                  <button className="p-2 border rounded hover:bg-gray-50">
                    X-
                  </button>
                  <button className="p-2 border rounded hover:bg-gray-50">
                    Y-
                  </button>
                  <button className="p-2 border rounded hover:bg-gray-50">
                    Z-
                  </button>
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
