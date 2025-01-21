#include <winsock2.h>
#include <windows.h>
#include "DeviceHandler.h"
#include <sstream>
#include <future>

const char* GetXrayStateString(CXray::XR_STATE state);
const char* GetCncStateString(CNCZustand state);

DeviceHandler::DeviceHandler() : m_xray(nullptr), m_cnc(nullptr),
m_hXrayDll(nullptr), m_hCncDll(nullptr)
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
			case 11: // Voltage property
				if (!pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
				return m_xray->SetkV(pDispParams->rgvarg[0].intVal) ? S_OK : E_FAIL;
			case 12: // Current property  
				if (!pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
				return m_xray->SetuA(pDispParams->rgvarg[0].intVal) ? S_OK : E_FAIL;
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

		case 3: // setVoltage
			if (!pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
			isAsyncOperation = true;
			future = std::async(std::launch::async, [this, voltage = pDispParams->rgvarg[0].intVal]() -> HRESULT {
				return m_xray->SetkV(voltage) ? S_OK : E_FAIL;
				});
			break;

		case 4: // setCurrent
			if (!pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
			isAsyncOperation = true;
			future = std::async(std::launch::async, [this, current = pDispParams->rgvarg[0].intVal]() -> HRESULT {
				return m_xray->SetuA(current) ? S_OK : E_FAIL;
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

		case 7: // setFocus
			if (!pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
			isAsyncOperation = true;
			future = std::async(std::launch::async, [this, focus = pDispParams->rgvarg[0].intVal]() -> HRESULT {
				return m_xray->SetSpotsize(focus) ? S_OK : E_FAIL;
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

		case 103: // moveAxis
			if (!pDispParams || pDispParams->cArgs != 2) return E_INVALIDARG;
			isAsyncOperation = true;
			future = std::async(std::launch::async, [this, axis = pDispParams->rgvarg[1].intVal,
				position = pDispParams->rgvarg[0].dblVal]() -> HRESULT {
					return m_cnc->StartTo(axis, position) ? S_OK : E_FAIL;
				});
			break;

		case 104: // moveAllAxes
			if (!pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
			isAsyncOperation = true;
			future = std::async(std::launch::async, [this, positions = (double*)pDispParams->rgvarg[0].parray->pvData]() -> HRESULT {
				return m_cnc->StartTo(positions) ? S_OK : E_FAIL;
				});
			break;

		case 105: // stop
			if (pDispParams && pDispParams->cArgs > 0) {
				isAsyncOperation = true;
				future = std::async(std::launch::async, [this, axis = pDispParams->rgvarg[0].intVal]() -> HRESULT {
					m_cnc->Stop(axis);
					return S_OK;
					});
			}
			break;

		case 106: // enableJoy
			if (!pDispParams || pDispParams->cArgs != 2) return E_INVALIDARG;
			isAsyncOperation = true;
			future = std::async(std::launch::async, [this, axis = pDispParams->rgvarg[1].intVal,
				enable = pDispParams->rgvarg[0].boolVal]() -> HRESULT {
					if (enable) {
						m_cnc->EnableJoy(axis);
					}
					else {
						m_cnc->DisableJoy(axis);
					}
					return S_OK;
				});
			break;

		case 107: // getCncStatus
			return GetCncStatus(pVarResult);

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
		// Xray methods
		{L"initializeXray", 1},
		{L"startWarmup", 2},
		{L"setVoltage", 3},
		{L"setCurrent", 4},
		{L"turnXrayOn", 5},
		{L"turnXrayOff", 6},
		{L"setFocus", 7},
		{L"getXrayStatus", 8},
		{L"Voltage", 11},
		{L"Current", 12},

		// CNC methods - using 100+ IDs
		{L"initializeCnc", 101},
		{L"startReference", 102},
		{L"moveAxis", 103},
		{L"moveAllAxes", 104},
		{L"stop", 105},
		{L"enableJoy", 106},
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

// Add the missing method definitions
HRESULT DeviceHandler::GetPositions(VARIANT* pResult)
{
	OutputDebugString(L"GetPositions called\n");
	if (!pResult)
		return E_INVALIDARG;
	try {
		std::ostringstream json;
		double positions[CNC_MAX_AXIS];
		m_cnc->GetAllLastPositions(positions);

		json << "[";
		for (int i = 0; i < CNC_MAX_AXIS; ++i) {
			if (i > 0) json << ",";
			json << positions[i];
		}
		json << "]";

		std::wstring wstr = ConvertToWString(json.str());
		pResult->vt = VT_BSTR;
		pResult->bstrVal = SysAllocString(wstr.c_str());
		OutputDebugString(L"GetPositions succeeded\n");
		OutputDebugString(wstr.c_str());
		return S_OK;
	}
	catch (...) {
		return E_FAIL;
	}
}

HRESULT DeviceHandler::GetAxesInfo(VARIANT* pResult) {
    if (!pResult) return E_INVALIDARG;
    try {
        // 获取实际轴数
        const UINT axisCount = m_cnc->NrAxis();
        
        std::ostringstream json;
        json << "{\"axisCount\":" << axisCount << ",\"axes\":[";
        
        // 只遍历实际存在的轴
        for (UINT i = 0; i < axisCount; ++i) {
            if (i > 0) json << ",";
            json << "{"
                << "\"id\":" << i << ","
                << "\"name\":\"" << m_cnc->AxisName(i) << "\","  // 使用实际轴名
                << "\"minPos\":" << m_cnc->GetMinPos(i, nullptr) << ","
                << "\"maxPos\":" << m_cnc->GetMaxPos(i, nullptr)
                << "}";
        }
        json << "]}";

        std::wstring wstr = ConvertToWString(json.str());
        pResult->vt = VT_BSTR;
        pResult->bstrVal = SysAllocString(wstr.c_str());
        
        // 添加调试输出
        OutputDebugString(L"GetAxesInfo succeeded\n");
        OutputDebugString(wstr.c_str());
        
        return S_OK;
    }
    catch (...) {
        OutputDebugString(L"GetAxesInfo failed\n");
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

