#include <fstream>  
#include <winsock2.h>
#include <windows.h>
#include "DeviceHandler.h"
#include <sstream>
#include <future>
#include <string>
#include <iomanip>


const char* GetXrayStateString(CXray::XR_STATE state);
const char* GetCncStateString(CNCZustand state);

DeviceHandler::DeviceHandler() : m_xray(nullptr), m_cnc(nullptr),
m_hXrayDll(nullptr), m_hCncDll(nullptr), positions(nullptr), axisCount(0), m_isMonitoring(false), m_isLiveMode(false)
{
	try {
		// Initialize Xray
		if (!LoadXrayDll()) {
			throw std::runtime_error("Failed to load Xray2.dll");
		}

		m_xray = new CXray();
		if (!m_xray || !m_xray->Open()) {
			if (m_xray) {
				delete m_xray;
				m_xray = nullptr;
			}
			throw std::runtime_error("Failed to initialize Xray");
		}

		// Initialize CNC
		if (!LoadCncDll()) {
			throw std::runtime_error("Failed to load CNC DLL");
		}

		m_cnc = new CCnc();
		if (!m_cnc || !m_cnc->Open()) {
			if (m_cnc) {
				delete m_cnc;
				m_cnc = nullptr;
			}
			throw std::runtime_error("Failed to initialize CNC");
		}

		// 初始化位置数组
		if (m_cnc) {
			axisCount = m_cnc->NrAxis();
			positions = new double[CNC_MAX_AXIS];
			std::wstring debug = L"Initialized with " + std::to_wstring(axisCount) + L" axes\n";
			OutputDebugString(debug.c_str());
		}

		// 初始化图像缓冲区相关
		m_imageBuffer = {};

		//初始化Dig
		m_dig = CDigGrabber::GetAndInitDigGrabber();
		if (!m_dig) {
			throw std::runtime_error("Failed to initialize detector");
		}

		// 添加测试代码
		try {
			OutputDebugString(L"\n=== Starting Image Acquisition Test ===\n");

			// 获取图像尺寸和位深
			CSize size = m_dig->GetActGrabbedImageSize();
			UINT bitsPerPixel = m_dig->GrabbedBits();
			std::wostringstream info;
			info << L"Image Size: " << size.cx << L"x" << size.cy
				<< L", Bits per pixel: " << bitsPerPixel << L"\n";
			OutputDebugString(info.str().c_str());

			// 分配缓冲区
			std::unique_ptr<WORD[]> buffer = std::make_unique<WORD[]>(size.cx * size.cy);
			if (!buffer) {
				OutputDebugString(L"Failed to allocate buffer\n");
				throw std::runtime_error("Buffer allocation failed");
			}

			// 设置目标缓冲区
			if (!m_dig->DefineDestBuffer(buffer.get(), 1, size.cy, size.cx)) {
				OutputDebugString(L"Failed to define destination buffer\n");
				throw std::runtime_error("Define buffer failed");
			}

			// 设置回调
			auto testCallback = [](CDigGrabber& digGrabber) {
				try {
					// 获取图像数据指针
					WORD* data = static_cast<WORD*>(digGrabber.GetAcqData());
					if (!data) return;

					SIZE_T size = digGrabber.GetActGrabbedImageSize().cx *
						digGrabber.GetActGrabbedImageSize().cy;

					// 计算统计信息
					WORD minVal = 65535, maxVal = 0;
					double sum = 0;
					for (SIZE_T i = 0; i < size; i++) {
						minVal = min(minVal, data[i]);
						maxVal = max(maxVal, data[i]);
						sum += data[i];
					}
					double mean = sum / size;

					// 输出统计信息
					std::wostringstream stats;
					stats << L"\nFrame Statistics:\n"
						<< L"Min value: " << minVal << L"\n"
						<< L"Max value: " << maxVal << L"\n"
						<< L"Mean value: " << mean << L"\n";
					OutputDebugString(stats.str().c_str());

					// 保存第一帧
					static bool firstFrame = true;
					if (firstFrame) {
						std::ofstream file("C:\\temp\\test_image.raw", std::ios::binary);
						if (file.is_open()) {
							file.write(reinterpret_cast<char*>(data), size * sizeof(WORD));
							file.close();
							OutputDebugString(L"First frame saved to C:\\temp\\test_image.raw\n");
						}
						firstFrame = false;
					}
				}
				catch (...) {
					OutputDebugString(L"Error in test callback\n");
				}
				};

			// 设置回调和采集数据
			if (!m_dig->SetCallbacksAndMessages(nullptr, 0, 0, testCallback, nullptr)) {
				OutputDebugString(L"Failed to set callbacks\n");
				throw std::runtime_error("Set callbacks failed");
			}
			m_dig->SetAcqData(buffer.get());

			// 开始采集测试
			OutputDebugString(L"Starting test acquisition...\n");
			if (!m_dig->AcquireImage(5, CDigGrabber::ACQ_ONE_BUFFER, nullptr, nullptr, nullptr)) {
				OutputDebugString(L"Failed to start acquisition\n");
				throw std::runtime_error("Start acquisition failed");
			}

			// 等待采集完成
			Sleep(1000);  // 等待一秒

			// 停止采集
			m_dig->AbortGrab();

			OutputDebugString(L"=== Image Acquisition Test Completed ===\n");

		}
		catch (const std::exception& e) {
			std::string error = "Test acquisition failed: " + std::string(e.what()) + "\n";
			OutputDebugStringA(error.c_str());
		}

		auto g_nImgSizeX = m_dig->GetActGrabbedImageSize().cx;
		auto g_nImgSizeY = m_dig->GetActGrabbedImageSize().cy;
		auto g_nBitsPerPx = m_dig->GrabbedBits();

		OutputDebugString(L"Dig test\n");
		OutputDebugString(std::to_wstring(g_nImgSizeX).c_str());
		OutputDebugString(std::to_wstring(g_nImgSizeY).c_str());
		OutputDebugString(std::to_wstring(g_nBitsPerPx).c_str());



	}
	catch (...) {
		UnloadXrayDll();
		UnloadCncDll();
		throw;
	}
}

DeviceHandler::~DeviceHandler()
{
	// 首先停止监控线程
	StopMonitoring();

	{
		std::lock_guard<std::mutex> lock(m_mutex);  // 添加互斥锁保护

		// 然后清理其他资源
		if (m_xray) {
			try {
				if (m_xray->IsBeamOn()) {
					m_xray->TurnOff();
				}
				m_xray->Close();
				delete m_xray;
				m_xray = nullptr;
			}
			catch (...) {}
		}

		if (m_cnc) {
			try {
				m_cnc->Close();
				delete m_cnc;
				m_cnc = nullptr;
			}
			catch (...) {}
		}

		// 释放 positions 的内存
		delete[] positions;
		positions = nullptr;

		UnloadXrayDll();
		UnloadCncDll();
	}
}

STDMETHODIMP DeviceHandler::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
	WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
	EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	try {
		// 处理属性 put 操作
		if (wFlags & DISPATCH_PROPERTYPUT) {
			switch (dispIdMember) {
			case 11: // voltage
				if (!m_xray || !pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
				return m_xray->SetkV(pDispParams->rgvarg[0].intVal) ? S_OK : E_FAIL;

			case 12: // current
				if (!m_xray || !pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
				return m_xray->SetuA(pDispParams->rgvarg[0].intVal) ? S_OK : E_FAIL;

			case 13: // focus
				if (!m_xray || !pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
				return m_xray->SetSpotsize(pDispParams->rgvarg[0].intVal) ? S_OK : E_FAIL;

			case 111: // targetPosition
				if (!m_cnc || !pDispParams || pDispParams->cArgs != 1) {
					OutputDebugString(L"Invalid parameters for targetPosition\n");
					return E_INVALIDARG;
				}

				try {
					// 检查参数类型
					OutputDebugString(L"Parameter type: ");
					OutputDebugString(std::to_wstring(pDispParams->rgvarg[0].vt).c_str());
					OutputDebugString(L"\n");

					// 获取并输出原始字符串
					std::wstring paramStr = pDispParams->rgvarg[0].bstrVal;
					OutputDebugString(L"Raw parameter string: ");
					OutputDebugString(paramStr.c_str());
					OutputDebugString(L"\n");

					// 使用简单的字符串分割
					size_t pos = paramStr.find(L",");
					if (pos == std::wstring::npos) {
						OutputDebugString(L"Comma delimiter not found in parameter string\n");
						return E_INVALIDARG;
					}

					// 分割并解析
					std::wstring axisStr = paramStr.substr(0, pos);
					std::wstring posStr = paramStr.substr(pos + 1);

					OutputDebugString(L"Axis string: ");
					OutputDebugString(axisStr.c_str());
					OutputDebugString(L"\n");

					OutputDebugString(L"Position string: ");
					OutputDebugString(posStr.c_str());
					OutputDebugString(L"\n");

					int axis = _wtoi(axisStr.c_str());
					double position = _wtof(posStr.c_str());

					// 输出解析后的值
					std::wstring debug = L"Parsed values - Axis: " + std::to_wstring(axis) +
						L", Position: " + std::to_wstring(position) + L"\n";
					OutputDebugString(debug.c_str());

					// 调用移动并输出结果
					BOOL result = m_cnc->StartTo(axis, position);
					OutputDebugString(result ? L"StartTo succeeded\n" : L"StartTo failed\n");

					return result ? S_OK : E_FAIL;
				}
				catch (...) {
					OutputDebugString(L"Exception occurred during targetPosition processing\n");
					return E_FAIL;
				}

			case 112: // targetPositions
				if (!m_cnc || !pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
				return m_cnc->StartTo((double*)pDispParams->rgvarg[0].parray->pvData) ?
					S_OK : E_FAIL;

			case 114: // stopAxis
				if (!m_cnc || !pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
				m_cnc->Stop(pDispParams->rgvarg[0].intVal);
				return S_OK;

			case 115: // joyEnabled
				if (!m_cnc || !pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
				{
					// 日志输出检查
					OutputDebugString(L"Setting joystick enabled state\n");

					// 获取并设置摇杆状态
					BOOL enable = pDispParams->rgvarg[0].boolVal;
					if (enable) {
						m_cnc->EnableJoy();
					}
					else {
						m_cnc->DisableJoy();
					}

					// 设置返回值
					if (pVarResult) {
						pVarResult->vt = VT_BOOL;
						pVarResult->boolVal = VARIANT_TRUE;
					}

					// 调试输出
					std::wstring debug = L"Joystick enabled state set to: " +
						std::to_wstring(enable) + L"\n";
					OutputDebugString(debug.c_str());

					return S_OK;
				}


			case 116: // positionReached
				if (!m_cnc || !pDispParams || pDispParams->cArgs != 0) return E_INVALIDARG;
				{
					// 日志输出检查
					OutputDebugString(L"Checking position reached for all axes\n");

					// 获取位置到达状态
					BOOL reached = m_cnc->PositionReached();

					// 设置返回值
					if (pVarResult) {
						pVarResult->vt = VT_BOOL;
						pVarResult->boolVal = reached ? VARIANT_TRUE : VARIANT_FALSE;
					}

					// 调试输出
					std::wstring debug = L"Position reached result for all axes: " +
						std::to_wstring(reached) + L"\n";
					OutputDebugString(debug.c_str());

					return S_OK;
				}

			case 204: // gain
				if (!m_dig || !pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
				return SetGain(pDispParams->rgvarg[0].intVal) ? S_OK : E_FAIL;

			case 205: // fps
				if (!m_dig || !pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
				return SetFPS(pDispParams->rgvarg[0].intVal) ? S_OK : E_FAIL;

			case 301: // ReadSharedMemory
				if (!pDispParams || pDispParams->cArgs != 2) {
					OutputDebugString(L"ReadSharedMemory: Invalid number of arguments\n");
					return E_INVALIDARG;
				}
				return ReadSharedMemory(
					pDispParams->rgvarg[1].bstrVal,  // name
					pDispParams->rgvarg[0].uintVal,  // size
					pVarResult                       // result
				);

			default:
				return DISP_E_MEMBERNOTFOUND;
			}
		}

		// 处理异步方法调用
		std::future<HRESULT> future;
		bool isAsyncOperation = false;

		switch (dispIdMember) {
			// X射线方法 (1-10)
		case 1: // initializeXray
			isAsyncOperation = true;
			future = std::async(std::launch::async, [this]() -> HRESULT {
				return m_xray->Open() ? S_OK : E_FAIL;
				});
			break;

		case 2: // startWarmup
			isAsyncOperation = true;
			future = std::async(std::launch::async, [this]() -> HRESULT {
				return m_xray->StartWarmUp() ? S_OK : E_FAIL;
				});
			break;

		case 5: // turnXrayOn
			isAsyncOperation = true;
			future = std::async(std::launch::async, [this]() -> HRESULT {
				return m_xray->TurnOn() ? S_OK : E_FAIL;
				});
			break;

		case 6: // turnXrayOff

			isAsyncOperation = true;
			future = std::async(std::launch::async, [this]() -> HRESULT {
				return m_xray->TurnOff() ? S_OK : E_FAIL;
				});
			break;

		case 8: // getXrayStatus
			return GetXrayStatus(pVarResult);


			// CNC方法 (101-110)
		case 101: // initializeCnc
			isAsyncOperation = true;
			future = std::async(std::launch::async, [this]() -> HRESULT {
				return m_cnc->Open() ? S_OK : E_FAIL;
				});
			break;

		case 102: // startReference
			if (pDispParams && pDispParams->cArgs > 0) {
				isAsyncOperation = true;
				future = std::async(std::launch::async, [this, axis = pDispParams->rgvarg[0].intVal]() -> HRESULT {
					return m_cnc->StartReference() ? S_OK : E_FAIL;
					});
			}
			break;

		case 107: // getCncStatus
			return GetCncStatus(pVarResult);

		case 108: // getAxesInfo
			return GetAxesInfo(pVarResult);

		case 109: // getPositions
			return GetPositions(pVarResult);


			// Detector方法 (201-210)
		case 201: // initializeDetector
		{
			OutputDebugString(L"Invoke: initializeDetector called\n");
			HRESULT hr = InitializeDetector();
			if (FAILED(hr)) {
				OutputDebugString(L"Invoke: initializeDetector failed\n");
			}
			else {
				OutputDebugString(L"Invoke: initializeDetector succeeded\n");
			}
			if (pVarResult) {
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE;
			}
			return S_OK;
		}

		case 202: // startLive
		{
			OutputDebugString(L"Invoke: startLive called\n");
			HRESULT hr = StartLive();
			if (FAILED(hr)) {
				OutputDebugString(L"Invoke: startLive failed\n");
			}
			else {
				OutputDebugString(L"Invoke: startLive succeeded\n");
			}
			if (pVarResult) {
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE;
			}
			return S_OK;
		}

		case 203: // stopLive
		{
			OutputDebugString(L"Invoke: stopLive called\n");
			HRESULT hr = StopLive();
			if (FAILED(hr)) {
				OutputDebugString(L"Invoke: stopLive failed\n");
			}
			else {
				OutputDebugString(L"Invoke: stopLive succeeded\n");
			}
			if (pVarResult) {
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE;
			}
			return S_OK;
		}


		default:
			return DISP_E_MEMBERNOTFOUND;
		}

		// 处理异步操作结果
		if (isAsyncOperation) {
			if (pVarResult) {
				// 等待异步操作完成并返回结果
				HRESULT hr = future.get();
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE;
				return hr;
			}
		}

		return S_OK;
	}
	catch (...) {
		return E_FAIL;
	}
}


STDMETHODIMP DeviceHandler::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames,
	UINT cNames, LCID lcid, DISPID* rgDispId)
{
	if (!rgszNames || !rgDispId) return E_INVALIDARG;

	static const struct {
		const wchar_t* name;
		DISPID id;
	} methods[] = {
		// 带参数的方法全部改为属性
		{L"voltage", 11},
		{L"current", 12},
		{L"focus", 13},
		{L"axisCurrent", 14},
		{L"targetPosition", 111},
		{L"targetPositions", 112},
		{L"stopAxis", 114},
		{L"joyEnabled", 115},
		{L"positionReached", 116},

		// 无参数方法保持不变
		{L"initializeXray", 1},
		{L"startWarmup", 2},
		{L"turnXrayOn", 5},
		{L"turnXrayOff", 6},
		{L"getXrayStatus", 8},
		{L"initializeCnc", 101},
		{L"startReference", 102},
		{L"getCncStatus", 107},
		{L"getAxesInfo", 108},
		{L"getPositions", 109},

		// 添加Detector相关方法
		{L"initializeDetector", 201},
		{L"startLive", 202},
		{L"stopLive", 203},
		{L"gain", 204},        // 属性
		{L"fps", 205},         // 属性

		{L"ReadSharedMemory", 301}
	};

	for (const auto& method : methods) {
		if (wcscmp(rgszNames[0], method.name) == 0) {
			*rgDispId = method.id;
			return S_OK;
		}
	}

	return DISP_E_UNKNOWNNAME;
}

// Implementation of DLL loading methods
bool DeviceHandler::LoadXrayDll()
{
	m_hXrayDll = LoadLibrary(L"Xray2.dll");
	if (!m_hXrayDll) {
		m_hXrayDll = LoadLibrary(L"bin\\Xray2.dll");
		if (!m_hXrayDll) return false;
	}
	return true;
}

bool DeviceHandler::LoadCncDll()
{
	m_hCncDll = LoadLibrary(L"EcoCncx64.dll");
	if (!m_hCncDll) {
		m_hCncDll = LoadLibrary(L"bin\\EcoCncx64.dll");
		if (!m_hCncDll) return false;
	}
	return true;
}

bool DeviceHandler::LoadDigDll()
{
	m_hDigDll = LoadLibrary(L"DigGrabberDllx64.dll");
	if (!m_hDigDll) {
		m_hDigDll = LoadLibrary(L"bin\\DigGrabberDllx64.dll");
		if (!m_hDigDll) return false;
	}
	return true;
}

void DeviceHandler::UnloadXrayDll()
{
	if (m_hXrayDll) {
		FreeLibrary(m_hXrayDll);
		m_hXrayDll = nullptr;
	}
}

void DeviceHandler::UnloadCncDll()
{
	if (m_hCncDll) {
		FreeLibrary(m_hCncDll);
		m_hCncDll = nullptr;
	}
}

// Helper methods implementation
std::wstring DeviceHandler::ConvertToWString(const std::string& str)
{
	if (str.empty()) return std::wstring();
	int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring result(size, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size);
	return result;
}

// Helper functions
const char* GetXrayStateString(CXray::XR_STATE state)
{
	switch (state) {
	case CXray::XR_NOT_INIT_YET: return "XR_NOT_READY";
	case CXray::XR_NOT_READY: return "XR_NOT_READY";
	case CXray::XR_IS_COLD: return "XR_IS_COLD";
	case CXray::XR_IS_OFF: return "XR_IS_OFF";
	case CXray::XR_IS_ON: return "XR_IS_ON";
	case CXray::XR_IS_RAMPINGUP: return "XR_IS_ON";
	default: return "XR_NOT_READY";
	}
}

const char* GetCncStateString(CNCZustand state)
{
	switch (state) {
	case CNC_NOT_INIT_YET: return "CNC_NOT_READY";
	case CNC_DRIVING_REF: return "CNC_DRIVING_REF";
	case CNC_WAS_STARTED: return "CNC_MOVING";
	case CNC_STAND_STILL: return "CNC_STAND_STILL";
	case CNC_NOT_READY: return "CNC_NOT_READY";
	default: return "CNC_NOT_READY";
	}
}

// Basic IDispatch Methods implementation
STDMETHODIMP DeviceHandler::GetTypeInfoCount(UINT* pctinfo)
{
	*pctinfo = 0;
	return S_OK;
}

STDMETHODIMP DeviceHandler::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
{
	return E_NOTIMPL;
}

HRESULT DeviceHandler::GetPositions(VARIANT* pResult)
{
	OutputDebugString(L"GetPositions called\n");
	if (!pResult)
		return E_INVALIDARG;
	try {
		// 获取最新位置
		if (!m_cnc) {
			throw std::runtime_error("CNC not initialized");
		}

		// 获取当前位置前先等待一小段时间
		Sleep(200);  // 等待200ms让位置更新

		// 获取当前位置
		m_cnc->GetAllSetPositions(positions);

		// Debug输出每个轴的位置
		for (UINT i = 0; i < axisCount; ++i) {
			std::wstring posStr = L"Axis " + std::to_wstring(i) + L" position: " + std::to_wstring(positions[i]) + L"\n";
			OutputDebugString(posStr.c_str());
		}

		// 构建JSON数组
		std::ostringstream json;
		json << "[";
		for (UINT i = 0; i < axisCount; ++i) {
			if (i > 0) json << ",";
			json << std::fixed << std::setprecision(3) << positions[i];
		}
		json << "]";

		std::wstring wstr = ConvertToWString(json.str());
		pResult->vt = VT_BSTR;
		pResult->bstrVal = SysAllocString(wstr.c_str());

		// Debug输出最终的JSON字符串
		OutputDebugString(L"GetPositions result: ");
		OutputDebugString(wstr.c_str());
		OutputDebugString(L"\n");

		return S_OK;
	}
	catch (const std::exception& e) {
		std::string error = "GetPositions failed: " + std::string(e.what());
		OutputDebugStringA(error.c_str());
		return E_FAIL;
	}
	catch (...) {
		OutputDebugString(L"GetPositions failed with unknown exception\n");
		return E_FAIL;
	}
}

HRESULT DeviceHandler::GetAxesInfo(VARIANT* pResult) {
	if (!pResult) return E_INVALIDARG;
	try {
		std::wstring temp = L"axisCount:" + std::to_wstring(axisCount);
		OutputDebugString(temp.c_str());

		std::ostringstream json;
		json << "{\"axisCount\":" << axisCount << ",\"axes\":[";

		m_cnc->GetAllLastPositions(positions);

		for (UINT i = 0; i < axisCount; ++i) {
			if (i > 0) json << ",";
			json << "{"
				<< "\"id\":" << i << ","
				<< "\"name\":\"" << m_cnc->AxisName(i) << "\","  // 使用实际轴名
				<< "\"minPos\":" << m_cnc->GetMinPos(i, positions) << ","
				<< "\"maxPos\":" << m_cnc->GetMaxPos(i, positions)
				<< "}";
		}
		json << "]}";

		std::wstring wstr = ConvertToWString(json.str());
		pResult->vt = VT_BSTR;
		pResult->bstrVal = SysAllocString(wstr.c_str());

		OutputDebugString(L"GetAxesInfo succeeded\n");
		OutputDebugString(wstr.c_str());

		return S_OK;
	}
	catch (const std::exception& e) {
		OutputDebugStringA(e.what());
		return E_FAIL;
	}
	catch (...) {
		OutputDebugString(L"GetAxesInfo failed with unknown exception\n");
		return E_FAIL;
	}
}



HRESULT DeviceHandler::GetCncStatus(VARIANT* pResult)
{
	OutputDebugString(L"GetCncStatus called\n");
	if (!pResult || !m_cnc)
		return E_INVALIDARG;
	try {
		std::ostringstream json;
		CNCZustand state = static_cast<CNCZustand>(m_cnc->Zustand());
		bool isMoving = m_cnc->IsMoving();
		bool positionReached = m_cnc->PositionReached();

		// 根据实际状态确定显示状态
		const char* stateStr;
		if (state == CNC_WAS_STARTED && !isMoving && positionReached) {
			stateStr = "CNC_STAND_STILL";
		}
		else if (state == CNC_WAS_STARTED && isMoving) {
			stateStr = "CNC_MOVING";
		}
		else {
			stateStr = GetCncStateString(state);
		}

		// 获取各种状态
		bool isDoorClosed = m_cnc->IsInterlockClosed();      // 门状态
		bool isJoyEnabled = !m_cnc->AllJoyDisabled();        // 摇杆状态
		bool isCollisionDetectionEnabled = !m_cnc->IsHWCollDetDisabled(); // 碰撞检测状态

		json << "{"
			<< "\"status\":\"" << stateStr << "\","
			<< "\"isMoving\":" << (isMoving ? "true" : "false") << ","
			<< "\"isDrivingRef\":" << (m_cnc->IsDrivingRef() ? "true" : "false") << ","
			<< "\"isPositionReached\":" << (positionReached ? "true" : "false") << ","
			<< "\"isJoyEnabled\":" << (isJoyEnabled ? "true" : "false") << ","
			<< "\"isDoorOpen\":" << (!isDoorClosed ? "true" : "false") << ","
			<< "\"isCollisionDetected\":" << (!isCollisionDetectionEnabled ? "true" : "false") << ","
			<< "\"temperature\":" << m_cnc->GetTemperature()
			<< "}";

		std::wstring wstr = ConvertToWString(json.str());
		pResult->vt = VT_BSTR;
		pResult->bstrVal = SysAllocString(wstr.c_str());
		OutputDebugString(L"GetCncStatus succeeded\n");
		OutputDebugString(wstr.c_str());
		return S_OK;
	}
	catch (...) {
		return E_FAIL;
	}
}

HRESULT DeviceHandler::GetXrayStatus(VARIANT* pResult)
{
	OutputDebugString(L"GetXrayStatus called\n");
	if (!pResult || !m_xray)  // 检查 m_xray 是否有效
		return E_INVALIDARG;

	try {
		std::lock_guard<std::mutex> lock(m_mutex);  // 添加互斥锁保护

		if (!m_xray) return E_POINTER;  // 双重检查

		std::ostringstream json;
		CXray::XR_STATE state = m_xray->State();
		const char* stateStr = GetXrayStateString(state);

		json << "{"
			<< "\"status\":\"" << stateStr << "\","
			<< "\"isPowered\":" << (m_xray->IsBeamOn() ? "true" : "false") << ","
			<< "\"isWarmedUp\":" << (!m_xray->IsCold() ? "true" : "false") << ","
			<< "\"isBeaming\":" << (m_xray->IsBeaming() ? "true" : "false") << ","
			<< "\"voltage\":" << m_xray->kVSet() << ","
			<< "\"current\":" << m_xray->uASet()
			<< "}";

		std::wstring wstr = ConvertToWString(json.str());
		pResult->vt = VT_BSTR;
		pResult->bstrVal = SysAllocString(wstr.c_str());

		OutputDebugString(L"GetXrayStatus succeeded\n");
		OutputDebugString(wstr.c_str());

		return S_OK;
	}
	catch (...) {
		return E_FAIL;
	}
}

void DeviceHandler::MonitorThreadFunc() {
	while (m_isMonitoring) {
		try {
			std::lock_guard<std::mutex> lock(m_mutex);  // 添加互斥锁保护

			if (!m_cnc || !m_xray || !m_webView) {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				continue;
			}

			// 获取CNC状态
			VARIANT cncStatus;
			VariantInit(&cncStatus);
			if (m_cnc) {  // 再次检查指针
				HRESULT hr = GetCncStatus(&cncStatus);
				if (SUCCEEDED(hr)) {
					m_webView->PostWebMessageAsJson(cncStatus.bstrVal);
					VariantClear(&cncStatus);
				}
			}

			// 获取Xray状态
			VARIANT xrayStatus;
			VariantInit(&xrayStatus);
			if (m_xray) {  // 再次检查指针
				HRESULT hr = GetXrayStatus(&xrayStatus);
				if (SUCCEEDED(hr)) {
					m_webView->PostWebMessageAsJson(xrayStatus.bstrVal);
					VariantClear(&xrayStatus);
				}
			}

		}
		catch (const std::exception& e) {
			OutputDebugStringA(("Monitor thread error: " + std::string(e.what())).c_str());
		}
		catch (...) {
			OutputDebugString(L"Unknown error in monitor thread\n");
		}

		// 释放锁后等待
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

void DeviceHandler::StartMonitoring(ICoreWebView2* webView) {
	if (m_isMonitoring) return;

	m_webView = webView;
	m_isMonitoring = true;
	m_monitorThread = std::thread(&DeviceHandler::MonitorThreadFunc, this);
}

void DeviceHandler::StopMonitoring() {
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_isMonitoring = false;
	}

	if (m_monitorThread.joinable()) {
		m_monitorThread.join();
	}

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_webView = nullptr;
	}
}

// Detector相关方法
bool DeviceHandler::CreateSharedBuffer(SIZE_T width, SIZE_T height) {
	OutputDebugString(L"CreateSharedBuffer: Starting...\n");
	try {
		SIZE_T bufferSize = width * height * sizeof(WORD);
		std::wostringstream debug;
		debug << L"CreateSharedBuffer: Creating buffer of size "
			<< bufferSize << L" bytes for image "
			<< width << L"x" << height << L"\n";
		OutputDebugString(debug.str().c_str());

		// 先清理旧的缓冲区
		CleanupSharedBuffer();

		// 创建共享内存映射
		m_imageBuffer.mappingHandle = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			static_cast<DWORD>(bufferSize),
			L"Local\\DetectorImageBuffer"
		);

		if (!m_imageBuffer.mappingHandle) {
			OutputDebugString(L"CreateSharedBuffer: Failed to create file mapping\n");
			return false;
		}

		// 映射视图
		m_imageBuffer.mappedAddress = MapViewOfFile(
			m_imageBuffer.mappingHandle,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			bufferSize
		);

		if (!m_imageBuffer.mappedAddress) {
			OutputDebugString(L"CreateSharedBuffer: Failed to map view of file\n");
			CloseHandle(m_imageBuffer.mappingHandle);
			m_imageBuffer.mappingHandle = nullptr;
			return false;
		}

		m_imageBuffer.width = width;
		m_imageBuffer.height = height;

		// 分配缓冲区
		m_imageBuffer.buffer = std::make_unique<WORD[]>(width * height);

		OutputDebugString(L"CreateSharedBuffer: Successfully created shared buffer\n");
		return true;
	}
	catch (const std::exception& e) {
		std::string error = "CreateSharedBuffer failed: " + std::string(e.what()) + "\n";
		OutputDebugStringA(error.c_str());
		return false;
	}
	catch (...) {
		OutputDebugString(L"CreateSharedBuffer: Unknown exception\n");
		return false;
	}
}

void DeviceHandler::CleanupSharedBuffer() {
	if (m_imageBuffer.mappedAddress) {
		UnmapViewOfFile(m_imageBuffer.mappedAddress);
		m_imageBuffer.mappedAddress = nullptr;
	}

	if (m_imageBuffer.mappingHandle) {
		CloseHandle(m_imageBuffer.mappingHandle);
		m_imageBuffer.mappingHandle = nullptr;
	}

	m_imageBuffer.buffer.reset();
}


//初始化Detector
HRESULT DeviceHandler::InitializeDetector() {
	OutputDebugString(L"InitializeDetector: Starting initialization...\n");
	try {
		std::lock_guard<std::mutex> lock(m_mutex);

		if (!m_dig) {
			OutputDebugString(L"InitializeDetector: Getting DigGrabber...\n");
			m_dig = CDigGrabber::GetAndInitDigGrabber();
			if (!m_dig) {
				OutputDebugString(L"InitializeDetector: Failed to get DigGrabber\n");
				return E_FAIL;
			}
			OutputDebugString(L"InitializeDetector: DigGrabber initialized successfully\n");
		}

		OutputDebugString(L"InitializeDetector: Getting image size...\n");
		CSize size = m_dig->GetActGrabbedImageSize();
		std::wstring sizeInfo = L"InitializeDetector: Image size - Width: " +
			std::to_wstring(size.cx) + L", Height: " +
			std::to_wstring(size.cy) + L"\n";
		OutputDebugString(sizeInfo.c_str());

		OutputDebugString(L"InitializeDetector: Creating shared buffer...\n");
		if (!CreateSharedBuffer(size.cx, size.cy)) {
			OutputDebugString(L"InitializeDetector: Failed to create shared buffer\n");
			return E_FAIL;
		}

		OutputDebugString(L"InitializeDetector: Defining destination buffer...\n");
		if (!m_dig->DefineDestBuffer(
			m_imageBuffer.buffer.get(),
			1,
			size.cy,
			size.cx
		)) {
			OutputDebugString(L"InitializeDetector: Failed to define destination buffer\n");
			return E_FAIL;
		}

		OutputDebugString(L"InitializeDetector: Setting callbacks...\n");
		if (!m_dig->SetCallbacksAndMessages(
			nullptr,
			0,
			0,
			EndFrameCallback,
			nullptr
		)) {
			OutputDebugString(L"InitializeDetector: Failed to set callbacks\n");
			return E_FAIL;
		}

		OutputDebugString(L"InitializeDetector: Initialization completed successfully\n");
		return S_OK;
	}
	catch (const std::exception& e) {
		std::string errorMsg = "InitializeDetector: Exception caught - " + std::string(e.what()) + "\n";
		OutputDebugStringA(errorMsg.c_str());
		return E_FAIL;
	}
	catch (...) {
		OutputDebugString(L"InitializeDetector: Unknown exception caught\n");
		return E_FAIL;
	}
}

HRESULT DeviceHandler::StartLive() {
	try {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_dig) {
			OutputDebugString(L"StartLive: DigGrabber is null\n");
			return E_FAIL;
		}

		OutputDebugString(L"StartLive: Setting acquisition data...\n");
		m_isLiveMode = true;
		// 将 this 指针设为采集数据的宿主
		m_dig->SetAcqData(this);

		// 去除“测试回调”，只保留真正的回调
		if (!m_dig->SetCallbacksAndMessages(
			nullptr,
			0,
			0,
			EndFrameCallback,
			nullptr
		)) {
			OutputDebugString(L"StartLive: Failed to set callbacks\n");
			return E_FAIL;
		}

		OutputDebugString(L"StartLive: Starting image acquisition...\n");
		if (!m_dig->AcquireImage(
			1,
			CDigGrabber::ACQ_CONTINUOUS,
			nullptr,
			nullptr,
			nullptr
		)) {
			OutputDebugString(L"StartLive: Failed to start image acquisition\n");
			return E_FAIL;
		}

		OutputDebugString(L"StartLive: Live mode started successfully\n");
		return S_OK;
	}
	catch (const std::exception& e) {
		std::string errorMsg = "StartLive: Exception caught - " + std::string(e.what()) + "\n";
		OutputDebugStringA(errorMsg.c_str());
		return E_FAIL;
	}
}

HRESULT DeviceHandler::StopLive() {
	OutputDebugString(L"StopLive: Stopping live mode...\n");
	try {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_dig) {
			OutputDebugString(L"StopLive: DigGrabber is null\n");
			return E_FAIL;
		}

		m_isLiveMode = false;

		OutputDebugString(L"StopLive: Aborting grab...\n");
		if (!m_dig->AbortGrab()) {
			OutputDebugString(L"StopLive: Failed to abort grab\n");
			return E_FAIL;
		}

		OutputDebugString(L"StopLive: Live mode stopped successfully\n");
		return S_OK;
	}
	catch (const std::exception& e) {
		std::string errorMsg = "StopLive: Exception caught - " + std::string(e.what()) + "\n";
		OutputDebugStringA(errorMsg.c_str());
		return E_FAIL;
	}
	catch (...) {
		OutputDebugString(L"StopLive: Unknown exception caught\n");
		return E_FAIL;
	}
}

void CALLBACK DeviceHandler::EndFrameCallback(CDigGrabber& digGrabber) {
	OutputDebugString(L"EndFrameCallback: Starting...\n");
	try {
		// 获取DeviceHandler实例
		DeviceHandler* handler = static_cast<DeviceHandler*>(digGrabber.GetAcqData());
		if (!handler) {
			OutputDebugString(L"EndFrameCallback: Handler is null\n");
			return;
		}

		OutputDebugString(L"EndFrameCallback: Handler retrieved successfully\n");

		//验证缓冲区
		if (!handler->m_imageBuffer.mappedAddress || !handler->m_imageBuffer.buffer) {
			OutputDebugString(L"EndFrameCallback: Invalid buffers\n");
			return;
		}

		// 获取图像大小
		SIZE_T bufferSize = handler->m_imageBuffer.width *
			handler->m_imageBuffer.height *
			sizeof(WORD);

		std::wostringstream debug;
		debug << L"EndFrameCallback: Processing frame of size "
			<< bufferSize << L" bytes\n";
		OutputDebugString(debug.str().c_str());

		// 复制数据
		memcpy(
			handler->m_imageBuffer.mappedAddress,
			handler->m_imageBuffer.buffer.get(),
			bufferSize
		);

		OutputDebugString(L"EndFrameCallback: Data copied to shared memory\n");

		// 发送消息
		if (handler->m_webView) {
			std::ostringstream json;
			json << "{\"type\":\"newFrame\",\"width\":"
				<< handler->m_imageBuffer.width
				<< ",\"height\":" << handler->m_imageBuffer.height
				<< ",\"size\":" << bufferSize << "}";

			handler->m_webView->PostWebMessageAsJson(
				handler->ConvertToWString(json.str()).c_str()
			);
			OutputDebugString(L"EndFrameCallback: Message posted to WebView\n");
		}
	}
	catch (...) {
		OutputDebugString(L"EndFrameCallback: Exception occurred\n");
	}
}


HRESULT DeviceHandler::SetGain(UINT gainStep) {
	try {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_dig) return E_FAIL;

		if (!m_dig->SetCameraGain(gainStep)) {
			return E_FAIL;
		}

		return S_OK;
	}
	catch (...) {
		return E_FAIL;
	}
}

HRESULT DeviceHandler::SetFPS(UINT timing) {
	try {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_dig) return E_FAIL;

		if (!m_dig->SetTiming(timing)) {
			return E_FAIL;
		}

		return S_OK;
	}
	catch (...) {
		return E_FAIL;
	}
}

HRESULT DeviceHandler::ReadSharedMemory(BSTR name, UINT size, VARIANT* pResult) {
	std::wostringstream debug;
	debug << L"ReadSharedMemory: Starting with size " << size << L"\n";
	OutputDebugString(debug.str().c_str());

	try {
		if (!pResult || size == 0) {
			OutputDebugString(L"ReadSharedMemory: Invalid arguments\n");
			return E_INVALIDARG;
		}

		if (!m_imageBuffer.mappedAddress) {
			OutputDebugString(L"ReadSharedMemory: Shared memory not initialized\n");
			return E_FAIL;
		}

		// 打印当前共享内存状态
		debug.str(L"");
		debug << L"ReadSharedMemory: Current buffer - Width: "
			<< m_imageBuffer.width << L", Height: "
			<< m_imageBuffer.height << L"\n";
		OutputDebugString(debug.str().c_str());

		// 创建 SafeArray
		SAFEARRAYBOUND bounds[1];
		bounds[0].lLbound = 0;
		bounds[0].cElements = size;

		SAFEARRAY* psa = SafeArrayCreate(VT_UI1, 1, bounds);
		if (!psa) {
			OutputDebugString(L"ReadSharedMemory: Failed to create SafeArray\n");
			return E_OUTOFMEMORY;
		}

		void* pData;
		HRESULT hr = SafeArrayAccessData(psa, &pData);
		if (FAILED(hr)) {
			SafeArrayDestroy(psa);
			OutputDebugString(L"ReadSharedMemory: Failed to access SafeArray data\n");
			return hr;
		}

		// 复制数据
		memcpy(pData, m_imageBuffer.mappedAddress, size);

		SafeArrayUnaccessData(psa);

		// 设置返回值
		VariantInit(pResult);
		pResult->vt = VT_ARRAY | VT_UI1;
		pResult->parray = psa;

		// 打印数据统计信息
		WORD* data = static_cast<WORD*>(m_imageBuffer.mappedAddress);
		WORD minVal = 65535, maxVal = 0;
		double sum = 0;
		for (SIZE_T i = 0; i < size / 2; i++) {
			minVal = min(minVal, data[i]);
			maxVal = max(maxVal, data[i]);
			sum += data[i];
		}
		double mean = sum / (size / 2);

		debug.str(L"");
		debug << L"ReadSharedMemory: Data stats - Min: " << minVal
			<< L", Max: " << maxVal
			<< L", Mean: " << mean << L"\n";
		OutputDebugString(debug.str().c_str());

		OutputDebugString(L"ReadSharedMemory: Completed successfully\n");
		return S_OK;
	}
	catch (...) {
		OutputDebugString(L"ReadSharedMemory: Exception occurred\n");
		return E_FAIL;
	}
}