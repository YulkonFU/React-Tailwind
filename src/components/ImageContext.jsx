// ImageContext.jsx
import { createContext, useState } from "react";

export const ImageContext = createContext();

export const ImageProvider = ({ children }) => {
  const [selectedImageData, setSelectedImageData] = useState(null);

  return (
    <ImageContext.Provider value={{ selectedImageData, setSelectedImageData }}>
      {children}
    </ImageContext.Provider>
  );
};