import { useState, useEffect, useRef } from "react";
import {
  Image as ImageIcon,
  Save,
  Grid,
  Filter,
  MessageSquare,
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
  const [isDrawingEnabled, setIsDrawingEnabled] = useState(false); // 新增状态
  const [isGridView, setIsGridView] = useState(false);
  const [activeGridIndex, setActiveGridIndex] = useState(0);

  const menuItems = [
    { key: "file", title: "File" },
    { key: "edit", title: "Edit" },
    { key: "view", title: "View" },
    { key: "modules", title: "Modules" },
    { key: "setup", title: "Setup" },
    { key: "help", title: "Help" },
  ];

  // 修改处理画笔工具按钮点击的函数
  const handleDrawingToolsToggle = () => {
    setShowDrawingTools(!showDrawingTools);
    setIsDrawingEnabled(!showDrawingTools); // 同步更新绘图启用状态
    if (!showDrawingTools) {
      setDrawingTool("arrow"); // 默认选择箭头工具
    }
  };

  // 添加切换Grid布局函数
  const toggleGridView = () => {
    setIsGridView(!isGridView);
    setActiveGridIndex(0); // Reset active grid when toggling
  };

  // 添加处理双击事件函数
  const handleGridDoubleClick = (index) => {
    setIsGridView(false);
    setActiveGridIndex(index);
  };

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
    <div className="flex flex-col h-screen">
      {/* Main Content Area */}
      <div className="ml-16 flex-1 flex flex-col">
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
        <div className="bg-white border-b">
          {/* Menu Icons */}
          <div className="h-12 flex items-center px-4">
            {menuItems.map(({ key, title }) => (
              <button
                key={key}
                className={`px-4 py-2 text-sm hover:bg-gray-100 transition-colors ${
                  activeMenu === key ? "bg-gray-100" : ""
                }`}
                onClick={() => toggleMenu(key)}
              >
                {title}
              </button>
            ))}
          </div>

          {/* Menu Content */}
          <div
            className={`overflow-hidden transition-all duration-300 ${
              activeMenu ? "h-[80px]" : "h-0" // 减小高度
            }`}
          >
            <div className="bg-white">
              {" "}
              {/* 移除 p-4 和 shadow-lg */}
              {activeMenu === "file" && <FileMenu />}
              {activeMenu === "edit" && <EditMenu />}
              {activeMenu === "view" && <ViewMenu />}
              {activeMenu === "modules" && <ModulesMenu />}
              {activeMenu === "setup" && <SetupMenu />}
              {activeMenu === "help" && <HelpMenu />}
            </div>
          </div>
        </div>

        {/* Main Workspace */}
        <div
          className={`flex-1 flex ${isExpanded ? "fixed inset-0 z-40" : ""}`}
        >
          {/* Left Sidebar - Tool Buttons */}
          <div className="w-16 bg-gray-800 p-2 flex flex-col justify-between items-center">
            {/* 上部分按钮组 */}
            <div className="flex flex-col gap-4">
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
              <button
                className="p-2 text-black hover:bg-gray-700 rounded"
                onClick={toggleGridView}
              >
                <Grid
                  className={`w-6 h-6 ${isGridView ? "text-blue-500" : ""}`}
                />
              </button>
              <button className="p-2 text-black hover:bg-gray-700 rounded">
                <Filter className="w-6 h-6" />
              </button>
              <button
                onClick={handleDrawingToolsToggle} // 更新这里
                className={`p-2 rounded transition-colors ${
                  showDrawingTools
                    ? "bg-blue-500 text-black"
                    : "text-black hover:bg-gray-700"
                }`}
                title="绘图工具"
              >
                <Pencil className="w-5 h-5" />
              </button>
            </div>

            {/* 底部按钮组 */}
            <div className="flex flex-col gap-4">{/* 预留底部按钮位置 */}</div>
          </div>

          {/* Image View Area */}
          <div
            className={`flex-1 bg-black relative ${
              isExpanded ? "w-full h-full" : ""
            }`}
            style={{
              width: "800px",
              height: "800px",
              position: "relative",
              zIndex: 1,
            }}
          >
            {isGridView ? (
              <div className="grid grid-cols-2 grid-rows-2 gap-1 w-full h-full">
                {[0, 1, 2, 3].map((index) => (
                  <div
                    key={index}
                    className="relative bg-black"
                    onDoubleClick={() => handleGridDoubleClick(index)}
                    style={{ cursor: "pointer" }}
                  >
                    <ImageViewer
                      ref={index === 0 ? imageViewerRef : null}
                      style={{ width: "100%", height: "100%" }}
                      showOverlay={showOverlay}
                      currentTool={drawingTool}
                    />
                  </div>
                ))}
              </div>
            ) : (
              <ImageViewer
                ref={imageViewerRef}
                style={{ width: "100%", height: "100%" }}
                showOverlay={showOverlay}
                currentTool={drawingTool}
                isDrawingEnabled={isDrawingEnabled} // 新增属性
              />
            )}

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
                  spriteRef.current.scale.set(1);
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
