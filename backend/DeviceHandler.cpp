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
m_hXrayDll(nullptr), m_hCncDll(nullptr), positions(nullptr), axisCount(0)
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
	}
	catch (...) {
		UnloadXrayDll();
		UnloadCncDll();
		throw;
	}
}

DeviceHandler::~DeviceHandler()
{
	if (m_xray) {
		try {
			if (m_xray->IsBeamOn()) {
				m_xray->TurnOff();
			}
			m_xray->Close();
			delete m_xray;
		}
		catch (...) {}
	}

	if (m_cnc) {
		try {
			m_cnc->Close();
			delete m_cnc;
		}
		catch (...) {}
	}

	// 释放 positions 的内存
	delete[] positions;

	UnloadXrayDll();
	UnloadCncDll();
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
					return E_INVALIDARG;
				}

				try {
					// 解析JSON字符串
					std::wstring paramStr = pDispParams->rgvarg[0].bstrVal;
					// 使用简单的字符串分割
					size_t pos = paramStr.find(L",");
					if (pos == std::wstring::npos) {
						return E_INVALIDARG;
					}

					int axis = _wtoi(paramStr.substr(0, pos).c_str());
					double position = _wtof(paramStr.substr(pos + 1).c_str());

					return m_cnc->StartTo(axis, position) ? S_OK : E_FAIL;
				}
				catch (...) {
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
				if (!m_cnc || !pDispParams || pDispParams->cArgs != 2) return E_INVALIDARG;
				if (pDispParams->rgvarg[0].boolVal) {
					m_cnc->EnableJoy(pDispParams->rgvarg[1].intVal);
				}
				else {
					m_cnc->DisableJoy(pDispParams->rgvarg[1].intVal);
				}
				return S_OK;

			case 116: // positionReached
				if (!m_cnc || !pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
				if (pVarResult) {
					pVarResult->vt = VT_BOOL;
					pVarResult->boolVal = m_cnc->PositionReached(pDispParams->rgvarg[0].intVal) ?
						VARIANT_TRUE : VARIANT_FALSE;
				}
				return S_OK;

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

			// DeviceHandler.cpp中的Invoke方法修改

		case 107: // getCncStatus
			return GetCncStatus(pVarResult);

		case 108: // getAxesInfo
			return GetAxesInfo(pVarResult);

		case 109: // getPositions
			return GetPositions(pVarResult);

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
		{L"getPositions", 109}
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

		// 确保positions数组已初始化
		if (!positions) {
			positions = new double[CNC_MAX_AXIS];
		}

		// 获取当前位置
		m_cnc->GetAllLastPositions(positions);

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
	if (!pResult)
		return E_INVALIDARG;
	try {
		std::ostringstream json;
		CNCZustand state = static_cast<CNCZustand>(m_cnc->Zustand());
		const char* stateStr = GetCncStateString(state);

		json << "{"
			<< "\"status\":\"" << stateStr << "\","
			<< "\"isMoving\":" << (m_cnc->IsMoving() ? "true" : "false") << ","
			<< "\"isDrivingRef\":" << (m_cnc->IsDrivingRef() ? "true" : "false") << ","
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
	if (!pResult)
		return E_INVALIDARG;
	try {
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
