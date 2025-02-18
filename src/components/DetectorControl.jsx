import { useState, useEffect } from "react";
import { detectorService } from "../services/DetectorService";
import {
  Play,
  Square,
  Clock,
  Sliders,
  RotateCcw,
  ChevronDown,
  BarChart2,
} from "lucide-react";

const IMAGE_FILTERS = [
  "Average",
  "Blur",
  "Difference",
  "ENR",
  "Gauss",
  "HighLight",
  "HighLight_weak",
  "Horizontal-edge",
  "Kirsch 360°",
  "Median",
  "NDT Filter",
  "New Macro",
  "Prewitt",
  "Relief",
  "Roberts",
  "Save As",
  "Sharpen",
  "Smooth",
  "Sobel",
];

const DetectorControl = () => {
  // 探测器状态
  const [acquisitionState, setAcquisitionState] = useState({
    isLive: false,
    isIntegrating: false,
    fps: 30,
    integrationFrames: 1,
    gain: 1.0,
    histogram: false,
    timing: {
      exposure: 100,
      delay: 0,
    },
    brightness: 100,
    contrast: 100,
  });

  // 图像状态
  const [imageState, setImageState] = useState({
    data: null,
    width: 0,
    height: 0,
  });

  // UI状态
  const [selectedFilters, setSelectedFilters] = useState([
    "none",
    "none",
    "none",
    "none",
  ]);
  const [filterHistory, setFilterHistory] = useState([]);
  const [expandedSections, setExpandedSections] = useState({
    acquire: true,
    display: true,
    processing: true,
  });

  //初始化探测器的代码
  useEffect(() => {
    const initDetector = async () => {
      try {
        const handler = window.chrome?.webview?.hostObjects?.deviceHandler;
        if (!handler) return;

        // 直接调用方法
        await handler.initializeDetector;

        // 监听新帧消息
        window.chrome?.webview?.addEventListener("message", (event) => {
          const message = JSON.parse(event.data);
          if (message.type === "newFrame") {
            // 发布新帧事件给主窗口
            window.dispatchEvent(
              new CustomEvent("detectorFrame", {
                detail: {
                  data: message.imageData,
                  width: message.width,
                  height: message.height,
                },
              })
            );
          }
        });
      } catch (error) {
        console.error("Failed to initialize detector:", error);
      }
    };

    initDetector();
  }, []);

  // 实时采集控制
  const toggleLive = async () => {
    try {
      if (!acquisitionState.isLive) {
        // 使用 detectorService 而不是直接调用 handler
        try {
          // 设置 FPS
          await detectorService.setFPS(acquisitionState.fps);
          // 开始采集
          const result = await detectorService.startLive();
          console.log("startLive result:", result);
        } catch (error) {
          console.error("startLive error:", error);
          throw error;
        }
      } else {
        await detectorService.stopLive();
      }

      setAcquisitionState((prev) => ({
        ...prev,
        isLive: !prev.isLive,
      }));
    } catch (error) {
      console.error("Failed to toggle live mode:", error);
    }
  };

  // 同样修改 FPS 控制
  const handleFpsChange = async (value) => {
    try {
      await detectorService.setFPS(value);
      setAcquisitionState((prev) => ({
        ...prev,
        fps: value,
      }));
    } catch (error) {
      console.error("Failed to set FPS:", error);
    }
  };

  // 修改增益控制
  const handleGainChange = async (value) => {
    try {
      await detectorService.setGain(value);
      setAcquisitionState((prev) => ({
        ...prev,
        gain: value,
      }));
    } catch (error) {
      console.error("Failed to set gain:", error);
    }
  };

  // 积分控制
  const handleIntegrationChange = (frames) => {
    setAcquisitionState((prev) => ({
      ...prev,
      integrationFrames: frames,
    }));
  };

  // 滤镜控制
  const handleFilterChange = (index, value) => {
    const newFilters = [...selectedFilters];
    newFilters[index] = value;
    setSelectedFilters(newFilters);
    setFilterHistory([...filterHistory, selectedFilters]);
  };

  // 撤销滤镜
  const handleUndo = () => {
    if (filterHistory.length > 0) {
      const previousState = filterHistory[filterHistory.length - 1];
      setSelectedFilters(previousState);
      setFilterHistory(filterHistory.slice(0, -1));
    } else {
      setSelectedFilters(["none", "none", "none", "none"]);
    }
  };

  // 面板展开/收起控制
  const toggleSection = (section) => {
    setExpandedSections((prev) => ({
      ...prev,
      [section]: !prev[section],
    }));
  };

  // 新增事件发布
  useEffect(() => {
    window.chrome?.webview?.addEventListener("message", (event) => {
      const message = JSON.parse(event.data);
      if (message.type === "newFrame") {
        // 发布新帧事件,供主窗口处理
        window.dispatchEvent(
          new CustomEvent("detectorFrame", {
            detail: {
              data: message.imageData,
              width: message.width,
              height: message.height,
            },
          })
        );
      }
    });
  }, []);

  return (
    <div className="space-y-4">
      {/* Image Acquire Section */}
      <div className="bg-white border rounded-lg">
        <div
          className="flex items-center justify-between bg-gray-100 px-4 py-2 cursor-pointer"
          onClick={() => toggleSection("acquire")}
        >
          <h3 className="text-sm font-medium">Image acquire</h3>
          <ChevronDown
            className={`w-4 h-4 transform ${
              expandedSections.acquire ? "" : "rotate-180"
            }`}
          />
        </div>

        {expandedSections.acquire && (
          <div className="p-4 space-y-4">
            <div className="grid grid-cols-2 gap-4">
              {/* Live Control */}
              <button
                onClick={toggleLive}
                className={`flex items-center justify-center gap-2 px-4 py-2 rounded ${
                  acquisitionState.isLive
                    ? "bg-green-500 text-white"
                    : "bg-gray-200 hover:bg-gray-300"
                }`}
              >
                {acquisitionState.isLive ? (
                  <Square className="w-4 h-4" />
                ) : (
                  <Play className="w-4 h-4" />
                )}
                F2 Live
              </button>

              {/* Integration Control */}
              <select
                value={acquisitionState.integrationFrames}
                onChange={(e) =>
                  handleIntegrationChange(parseInt(e.target.value))
                }
                className="px-3 py-2 bg-gray-200 rounded"
              >
                <option value={1}>No Integration</option>
                <option value={2}>2 Frames</option>
                <option value={4}>4 Frames</option>
                <option value={8}>8 Frames</option>
              </select>
            </div>

            {/* Advanced Controls */}
            <div className="space-y-4">
              {/* FPS Control */}
              <div className="space-y-2">
                <div className="flex items-center justify-between">
                  <div className="flex items-center gap-2">
                    <Clock className="w-4 h-4" />
                    <span className="text-sm">FPS Control</span>
                  </div>
                  <span className="text-sm">{acquisitionState.fps} fps</span>
                </div>
                <input
                  type="range"
                  min="1"
                  max="60"
                  value={acquisitionState.fps}
                  onChange={(e) => handleFpsChange(parseInt(e.target.value))}
                  className="w-full"
                />
              </div>

              {/* Camera Gain */}
              <div className="space-y-2">
                <div className="flex items-center justify-between">
                  <div className="flex items-center gap-2">
                    <Sliders className="w-4 h-4" />
                    <span className="text-sm">Camera Gain</span>
                  </div>
                  <span className="text-sm">
                    {acquisitionState.gain.toFixed(1)}x
                  </span>
                </div>
                <input
                  type="range"
                  min="1"
                  max="10"
                  step="0.1"
                  value={acquisitionState.gain}
                  onChange={(e) => handleGainChange(parseFloat(e.target.value))}
                  className="w-full"
                />
              </div>
            </div>
          </div>
        )}
      </div>

      {/* Image Display Section */}
      <div className="bg-white border rounded-lg">
        <div
          className="flex items-center justify-between bg-gray-100 px-4 py-2 cursor-pointer"
          onClick={() => toggleSection("display")}
        >
          <h3 className="text-sm font-medium">Image display</h3>
          <ChevronDown
            className={`w-4 h-4 transform ${
              expandedSections.display ? "" : "rotate-180"
            }`}
          />
        </div>

        {expandedSections.display && (
          <div className="p-4 space-y-4">
            {/* Histogram Display */}
            <div className="h-32 bg-black rounded relative">
              <div className="absolute inset-0 flex items-center justify-center">
                <BarChart2
                  className={`w-8 h-8 ${
                    acquisitionState.histogram
                      ? "text-blue-500"
                      : "text-gray-500"
                  }`}
                  onClick={() =>
                    setAcquisitionState((prev) => ({
                      ...prev,
                      histogram: !prev.histogram,
                    }))
                  }
                />
              </div>
            </div>

            {/* Brightness/Contrast Controls */}
            <div className="space-y-2">
              <div className="flex items-center justify-between">
                <span className="text-sm">Brightness</span>
                <span className="text-sm">{acquisitionState.brightness}%</span>
              </div>
              <input
                type="range"
                min="0"
                max="200"
                value={acquisitionState.brightness}
                onChange={(e) =>
                  setAcquisitionState((prev) => ({
                    ...prev,
                    brightness: parseInt(e.target.value),
                  }))
                }
                className="w-full"
              />
            </div>

            <div className="space-y-2">
              <div className="flex items-center justify-between">
                <span className="text-sm">Contrast</span>
                <span className="text-sm">{acquisitionState.contrast}%</span>
              </div>
              <input
                type="range"
                min="0"
                max="200"
                value={acquisitionState.contrast}
                onChange={(e) =>
                  setAcquisitionState((prev) => ({
                    ...prev,
                    contrast: parseInt(e.target.value),
                  }))
                }
                className="w-full"
              />
            </div>
          </div>
        )}
      </div>

      {/* Image Processing Section */}
      <div className="bg-white border rounded-lg">
        <div
          className="flex items-center justify-between bg-gray-100 px-4 py-2 cursor-pointer"
          onClick={() => toggleSection("processing")}
        >
          <h3 className="text-sm font-medium">Image processing</h3>
          <ChevronDown
            className={`w-4 h-4 transform ${
              expandedSections.processing ? "" : "rotate-180"
            }`}
          />
        </div>

        {expandedSections.processing && (
          <div className="p-4 space-y-2">
            {/* Filter Controls */}
            {selectedFilters.map((filter, index) => (
              <div key={index} className="flex items-center gap-2">
                <div className="text-gray-500">▶</div>
                <div className="relative w-full">
                  <select
                    value={filter}
                    onChange={(e) => handleFilterChange(index, e.target.value)}
                    className="w-full bg-gray-200 py-1.5 px-2 rounded appearance-none text-sm"
                  >
                    <option value="none">&lt;none&gt;</option>
                    {IMAGE_FILTERS.map((filter) => (
                      <option key={filter} value={filter}>
                        {filter}
                      </option>
                    ))}
                  </select>
                </div>
              </div>
            ))}

            {/* Undo Button */}
            <button
              onClick={handleUndo}
              className="w-full flex items-center justify-center gap-2 bg-gray-200 py-1.5 rounded hover:bg-gray-300"
            >
              <RotateCcw className="h-4 w-4" />
              <span className="text-sm">Undo</span>
            </button>
          </div>
        )}
      </div>
    </div>
  );
};

export default DetectorControl;
