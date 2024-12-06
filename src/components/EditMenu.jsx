
const EditMenu = () => {
  return (
    <div className="absolute bg-white shadow-lg rounded p-2 mt-20 z-50">
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Undo</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Redo</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Cut</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Copy</button>
      <button className="block px-4 py-2 text-sm hover:bg-gray-100">Paste</button>
    </div>
  );
};

export default EditMenu;