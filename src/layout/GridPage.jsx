// GridPage.jsx
import { useRef, useEffect } from "react";
import { useNavigate, useLocation } from "react-router-dom";
import ImageViewer from "../components/ImageViewer";

const GridPage = () => {
  const navigate = useNavigate();
  const location = useLocation();
  const { selectedImageData } = location.state || {};
  const gridViewerRefs = useRef([]);

  // GridPage.jsx
useEffect(() => {
  if (selectedImageData && gridViewerRefs.current) {
    console.log("Loading image to grid views");
    gridViewerRefs.current.forEach(async (ref) => {
      if (ref?.loadImage) {
        if (selectedImageData instanceof File) {
          // 为每个网格创建新的 File 对象
          const newFile = new File([selectedImageData], selectedImageData.name, {
            type: selectedImageData.type
          });
          await ref.loadImage(newFile);
        } else {
          await ref.loadImage(selectedImageData);
        }
      }
    });
  }
}, [selectedImageData]);

  const handleCanvasClick = (index) => {
    // 返回到主页面，并传递图片数据
    navigate("/", {
      state: { selectedImageData },
    });
  };

  return (
    <div className="relative w-screen h-screen bg-black">
      {/* 固定的白色十字分割线容器 */}
      <div className="fixed inset-0 pointer-events-none z-10">
        <div className="absolute top-0 left-1/2 w-2 h-full bg-white transform -translate-x-1/2" />
        <div className="absolute top-1/2 left-0 w-full h-2 bg-white transform -translate-y-1/2" />
      </div>

      {/* 2x2 网格 */}
      <div className="grid grid-cols-2 grid-rows-2 w-full h-full">
        {Array.from({ length: 4 }).map((_, index) => (
          <div
            key={index}
            className="relative w-full h-full cursor-pointer"
            onClick={() => handleCanvasClick(index)}
          >
            <ImageViewer
              ref={(el) => (gridViewerRefs.current[index] = el)}
              onImageLoad={() => console.log(`Grid ${index + 1} image loaded`)}
            />
          </div>
        ))}
      </div>
    </div>
  );
};

export default GridPage;
