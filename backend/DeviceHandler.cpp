#include <winsock2.h>
#include <windows.h>
#include "DeviceHandler.h"
#include <sstream>

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
        // Handle property put operations
        if (wFlags & DISPATCH_PROPERTYPUT) {
            switch (dispIdMember) {
            case 11: // Voltage property
                if (!pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
                return SetVoltage(pDispParams->rgvarg[0].intVal);
            case 12: // Current property  
                if (!pDispParams || pDispParams->cArgs != 1) return E_INVALIDARG;
                return SetCurrent(pDispParams->rgvarg[0].intVal);
            default:
                return DISP_E_MEMBERNOTFOUND;
            }
        }

        // Handle method calls
        switch (dispIdMember) {
            // Xray Methods (1-10)
        case 1: return InitializeXray();
        case 2: return m_xray->StartWarmUp() ? S_OK : E_FAIL;
        case 3: return SetVoltage(pDispParams->rgvarg[0].intVal);
        case 4: return SetCurrent(pDispParams->rgvarg[0].intVal);
        case 5: return TurnXrayOn();
        case 6: return TurnXrayOff();
        case 7: return SetFocus(pDispParams->rgvarg[0].intVal);
        case 8: return GetXrayStatus(pVarResult);

            // CNC Methods (101-110) 
        case 101: return InitializeCnc();
        case 102: return StartReference(pDispParams->rgvarg[0].intVal);
        case 103: // moveAxis
            return MoveAxis(pDispParams->rgvarg[1].intVal, pDispParams->rgvarg[0].dblVal);
        case 104: return MoveAllAxes((double*)pDispParams->rgvarg[0].parray->pvData);
        case 105: return Stop(pDispParams->rgvarg[0].intVal);
        case 106: // enableJoy
            return EnableJoy(pDispParams->rgvarg[1].intVal, pDispParams->rgvarg[0].boolVal);
        case 107: return GetCncStatus(pVarResult);
        case 108: return GetAxesInfo(pVarResult);
        case 109: return GetPositions(pVarResult);

        default:
            return DISP_E_MEMBERNOTFOUND;
        }
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