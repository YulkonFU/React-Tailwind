import { useState, useEffect, useRef } from "react";
import {
  Image as ImageIcon,
  Save,
  Grid,
  Filter,
  MessageSquare,
} from "lucide-react";
import FileMenu from "../components/FileMenu";
import EditMenu from "../components/EditMenu";
import ViewMenu from "../components/ViewMenu";
import ModulesMenu from "../components/ModulesMenu";
import SetupMenu from "../components/SetupMenu";
import HelpMenu from "../components/HelpMenu";
import ImageViewer from "../components/ImageViewer";
import ImageControls from "../components/ImageControls";
import XrayControl from "../components/XrayControl";
import ManipulatorControl from "../components/ManipulatorControl";
import ImageProcessingControl from "../components/ImageProcessingControl";

const XactLayout = () => {
  const [activeMenu, setActiveMenu] = useState(null);
  const [isExpanded, setIsExpanded] = useState(false);
  const menuRef = useRef(null);
  const spriteRef = useRef(null);
  const imageViewerRef = useRef(null);

  // 处理加载图片
  const handleLoadImage = () => {
    // 创建文件输入元素
    const input = document.createElement("input");
    input.type = "file";
    input.accept = "image/*";
    input.onchange = (event) => {
      const file = event.target.files[0];
      if (file && imageViewerRef.current) {
        const url = URL.createObjectURL(file);
        imageViewerRef.current.loadImage(url);
      }
    };
    input.click();
  };

  // 处理保存图片
  const handleSaveImage = () => {
    if (imageViewerRef.current) {
      imageViewerRef.current.saveImage();
    }
  };

  // 切换菜单
  const toggleMenu = (menu) => {
    setActiveMenu(activeMenu === menu ? null : menu);
  };

  // 处理点击外部关闭菜单
  const handleClickOutside = (event) => {
    if (menuRef.current && !menuRef.current.contains(event.target)) {
      setActiveMenu(null);
    }
  };

  // 添加点击外部关闭菜单的事件监听
  useEffect(() => {
    document.addEventListener("mousedown", handleClickOutside);
    return () => {
      document.removeEventListener("mousedown", handleClickOutside);
    };
  }, []);

  // 处理图像加载完成后的逻辑
  const handleImageLoad = (sprite) => {
    spriteRef.current = sprite;

    // 设置拖拽功能
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

  // 处理全屏切换
  const handleToggleExpand = async () => {
    try {
      if (!isExpanded) {
        await document.documentElement.requestFullscreen();
      } else {
        if (document.fullscreenElement) {
          await document.exitFullscreen();
        }
      }
      setIsExpanded(!isExpanded);
    } catch (error) {
      console.error("Error toggling fullscreen:", error);
    }
  };

  return (
    <div className="flex h-screen bg-gray-100">
      {/* Left Sidebar - Tool Buttons */}
      <div className="w-16 bg-gray-800 p-2 flex flex-col gap-4">
        <button
          className="p-2 text-black hover:bg-gray-700 rounded"
          onClick={handleLoadImage}
        >
          <ImageIcon className="w-6 h-6" />
        </button>
        <button
          className="p-2 text-black hover:bg-gray-700 rounded"
          onClick={handleSaveImage}
        >
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

          {/* 下拉菜单 */}
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
              zIndex: 1,
            }}
          >
            <ImageViewer
              ref={imageViewerRef}
              key={isExpanded ? "expanded" : "normal"}
              onImageLoad={handleImageLoad}
            />
            <ImageControls
              isExpanded={isExpanded}
              onToggleExpand={handleToggleExpand}
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
                  const initialScale = spriteRef.current.initialScale || 1;
                  spriteRef.current.scale.set(initialScale);
                  if (imageViewerRef.current?.pixiAppRef.current) {
                    const app = imageViewerRef.current.pixiAppRef.current;
                    spriteRef.current.position.set(
                      app.screen.width / 2,
                      app.screen.height / 2
                    );
                  }
                }
              }}
            />
          </div>

          {/* Right Control Panel */}
          {!isExpanded && (
            <div className="w-full lg:w-96 bg-white border-t lg:border-t-0 lg:border-l flex flex-col">
              {/* X-ray Controls */}
              <XrayControl />

              {/* Manipulator Controls */}
              <ManipulatorControl />

              {/* Image Processing Controls */}
              <ImageProcessingControl />
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
