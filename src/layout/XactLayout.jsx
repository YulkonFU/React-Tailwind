import { useState, useEffect, useRef } from "react";
import {
  Image as ImageIcon,
  Save,
  Grid,
  Filter,
  MessageSquare,
  Download,
  Circle,
  ArrowRight,
  Eye,
  EyeOff,
  Pencil,
} from "lucide-react";
import FileMenu from "../components/menu/FileMenu";
import EditMenu from "../components/menu/EditMenu";
import ViewMenu from "../components/menu/ViewMenu";
import ModulesMenu from "../components/menu/ModulesMenu";
import SetupMenu from "../components/menu/SetupMenu";
import HelpMenu from "../components/menu/HelpMenu";
import ImageViewer from "../components/ImageViewer";
import ImageControls from "../components/ImageControls";
import XrayControl from "../components/XrayControl";
import ManipulatorControl from "../components/ManipulatorControl";
import DetectorControl from "../components/DetectorControl";
import DrawingToolbar from "../components/DrawingToolbar";

const XactLayout = () => {
  const [activeMenu, setActiveMenu] = useState(null);
  const [isExpanded, setIsExpanded] = useState(false);
  const [xrayStatus, setXrayStatus] = useState("XR_NOT_READY");
  const menuRef = useRef(null);
  const spriteRef = useRef(null);
  const imageViewerRef = useRef(null);
  const [drawingTool, setDrawingTool] = useState("arrow"); // 'arrow' | 'circle'
  const [showOverlay, setShowOverlay] = useState(true);
  const [showDrawingTools, setShowDrawingTools] = useState(false);

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
  const handleSaveImage = (includeOverlay = true) => {
    if (imageViewerRef.current) {
      const saveOptions = {
        includeOverlay,
        filename: `xray_image_${new Date().getTime()}.png`,
      };
      imageViewerRef.current.saveImage(saveOptions);
    }
  };

  // 处理工具选择
  const handleToolSelect = (tool) => {
    setDrawingTool(tool);
    if (imageViewerRef.current) {
      imageViewerRef.current.setDrawingTool(tool);
    }
  };

  // 清除标注
  const handleClearOverlay = () => {
    if (imageViewerRef.current) {
      imageViewerRef.current.clearOverlay();
    }
  };

  // 切换 overlay 显示/隐藏
  const handleToggleOverlay = () => {
    setShowOverlay(!showOverlay);
    if (imageViewerRef.current) {
      imageViewerRef.current.toggleOverlay(!showOverlay);
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
        <button
          onClick={() => setShowDrawingTools(!showDrawingTools)}
          className={`p-2 rounded-lg transition-colors ${
            showDrawingTools
              ? "bg-blue-500 text-white"
              : "bg-white hover:bg-gray-100"
          }`}
          title="绘图工具"
        >
          <Pencil className="w-5 h-5" />
        </button>
      </div>

      {/* Main Content Area */}
      <div className="flex-1 flex flex-col">
        {/* 绘图工具栏 */}
        {showDrawingTools && (
          <DrawingToolbar
            currentTool={drawingTool}
            showOverlay={showOverlay}
            onToolSelect={handleToolSelect}
            onToggleOverlay={handleToggleOverlay}
            onClear={handleClearOverlay}
          />
        )}

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
              width: "700px",
              height: "700px",
              position: "relative",
              zIndex: 1,
            }}
          >
            <ImageViewer
              ref={imageViewerRef}
              style={{ width: "100%", height: "100%" }}
              showOverlay={showOverlay}
              currentTool={drawingTool}
            />

            {/* Drawing Tools Panel */}
            <div className="absolute top-4 right-4 bg-white/80 backdrop-blur-sm rounded-lg shadow-lg p-2">
              <div className="flex items-center gap-2">
                <button
                  onClick={() => handleToolSelect("arrow")}
                  className={`p-2 rounded-lg transition-colors ${
                    drawingTool === "arrow"
                      ? "bg-blue-500 text-white"
                      : "hover:bg-gray-200"
                  }`}
                  title="箭头工具"
                >
                  <ArrowRight className="w-5 h-5" />
                </button>

                <button
                  onClick={() => handleToolSelect("circle")}
                  className={`p-2 rounded-lg transition-colors ${
                    drawingTool === "circle"
                      ? "bg-blue-500 text-white"
                      : "hover:bg-gray-200"
                  }`}
                  title="圆形工具"
                >
                  <Circle className="w-5 h-5" />
                </button>

                <div className="w-px h-6 bg-gray-300" />

                <button
                  onClick={handleToggleOverlay}
                  className="p-2 rounded-lg hover:bg-gray-200 transition-colors"
                  title={showOverlay ? "隐藏标注" : "显示标注"}
                >
                  {showOverlay ? (
                    <EyeOff className="w-5 h-5" />
                  ) : (
                    <Eye className="w-5 h-5" />
                  )}
                </button>

                <div className="w-px h-6 bg-gray-300" />

                <button
                  onClick={() => handleSaveImage(true)}
                  className="p-2 rounded-lg hover:bg-gray-200 transition-colors"
                  title="保存带标注的图像"
                >
                  <Download className="w-5 h-5" />
                </button>
              </div>
            </div>

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
              <XrayControl onStatusChange={(status) => setXrayStatus(status)} />

              {/* 使用三元运算符处理条件类名 */}
              <div
                className={
                  xrayStatus !== "XR_IS_ON"
                    ? "opacity-50 pointer-events-none"
                    : ""
                }
              >
                <ManipulatorControl />
                <DetectorControl />
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
