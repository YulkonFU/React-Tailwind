#include <winsock2.h>
#include <windows.h>
#include "XrayHandler.h"
#include "Xray.h"
#include <sstream>

const char* GetStateString(CXray::XR_STATE state);


// XrayHandler.cpp 添加 DLL 加载相关实现

bool XrayHandler::LoadXrayDll() {
    // 尝试从当前目录加载
    m_hXrayDll = LoadLibrary(L"Xray2.dll");
    if (!m_hXrayDll) {
        // 如果失败，尝试从 bin 目录加载
        m_hXrayDll = LoadLibrary(L"bin\\Xray2.dll");
        if (!m_hXrayDll) {
            OutputDebugString(L"Failed to load Xray2.dll\n");
            return false;
        }
    }

    OutputDebugString(L"Successfully loaded Xray2.dll\n");
    return true;
}

void XrayHandler::UnloadXrayDll() {
    if (m_hXrayDll) {
        FreeLibrary(m_hXrayDll);
        m_hXrayDll = nullptr;
    }
}

// 修改构造函数
XrayHandler::XrayHandler() : m_xray(nullptr), m_hXrayDll(nullptr) {
    try {
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
    }
    catch (...) {
        UnloadXrayDll();
        throw std::runtime_error("Failed to create Xray instance");
    }
}

// 修改析构函数
XrayHandler::~XrayHandler() {
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
    UnloadXrayDll();
}

STDMETHODIMP XrayHandler::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
    WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
    EXCEPINFO* pExcepInfo, UINT* puArgErr)
{

    if (!m_xray)
        return E_FAIL;

    BOOL result = FALSE; // 将 result 的声明和初始化移到 switch 语句外部

    try
    {
        switch (dispIdMember)
        {
        case 1: // initialize
            return m_xray->Open() ? S_OK : E_FAIL;

        case 2: // startWarmup
            if (m_xray->IsCold())
            {
                return m_xray->StartWarmUp() ? S_OK : E_FAIL;
            }
            return S_OK;

        case 3: // setVoltage
            if (!pDispParams || pDispParams->cArgs != 1)
                return E_INVALIDARG;
            if (pDispParams->rgvarg[0].vt != VT_I4)
                return E_INVALIDARG;
            return m_xray->SetkVuA(pDispParams->rgvarg[0].intVal,
                m_xray->uASet())
                ? S_OK
                : E_FAIL;
            break;

        case 4: // setCurrent
            if (!pDispParams || pDispParams->cArgs != 1)
                return E_INVALIDARG;
            if (pDispParams->rgvarg[0].vt != VT_I4)
                return E_INVALIDARG;
            return m_xray->SetkVuA(m_xray->kVSet(),
                pDispParams->rgvarg[0].intVal)
                ? S_OK
                : E_FAIL;
            break;
        case 5: // turnOnWithParams
        {
            OutputDebugString(L"turnOnWithParams called\n");

            // 参数检查
            if (!pDispParams || pDispParams->cArgs != 2) {
                wchar_t buf[256];
                swprintf_s(buf, L"Argument count error: expected 2, got %d\n",
                    pDispParams ? pDispParams->cArgs : 0);
                OutputDebugString(buf);
                return E_INVALIDARG;
            }

            // 参数类型检查
            if (pDispParams->rgvarg[0].vt != VT_I4 || pDispParams->rgvarg[1].vt != VT_I4) {
                OutputDebugString(L"Invalid argument types\n");
                return E_INVALIDARG;
            }

            // 注意：参数顺序是反的！
            int current = pDispParams->rgvarg[0].intVal;
            int voltage = pDispParams->rgvarg[1].intVal;

            wchar_t buf[256];
            swprintf_s(buf, L"Parameters: voltage=%d, current=%d\n", voltage, current);
            OutputDebugString(buf);

            if (!m_xray->TurnOn(voltage, current)) {
                OutputDebugString(L"TurnOn failed\n");
                return E_FAIL;
            }

            OutputDebugString(L"TurnOn succeeded\n");
            return S_OK;
        }
        break;

        case 6: // turnOff
            result = m_xray->TurnOff();
            return result ? S_OK : E_FAIL;
            break;

        case 7: // setFocus
            if (!pDispParams || pDispParams->cArgs != 1)
                return E_INVALIDARG;
            if (pDispParams->rgvarg[0].vt != VT_I4)
                return E_INVALIDARG;
            result = m_xray->SetSpotsize(pDispParams->rgvarg[0].intVal);
            return result ? S_OK : E_FAIL;
            break;

        case 8: // getStatus
            if (!pVarResult)
                return E_INVALIDARG;
            try
            {
                std::ostringstream json;
                CXray::XR_STATE state = m_xray->State();
                json << "{"
                    << "\"state\":\"" << GetStateString(state) << "\","
                    << "\"isPowered\":" << (m_xray->IsBeamOn() ? "true" : "false") << ","
                    << "\"isWarmedUp\":" << (!m_xray->IsCold() ? "true" : "false") << ","
                    << "\"isBeaming\":" << (m_xray->IsBeaming() ? "true" : "false") << ","
                    << "\"voltage\":" << m_xray->kVSet() << ","
                    << "\"current\":" << m_xray->uASet()
                    << "}";

                std::wstring wstr = ConvertToWString(json.str());
                pVarResult->vt = VT_BSTR;
                pVarResult->bstrVal = SysAllocString(wstr.c_str());
                return S_OK;
            }
            catch (...)
            {
                return E_FAIL;
            }
             break;

        default:
            return DISP_E_MEMBERNOTFOUND;
        }
    }
    catch (...) {
        OutputDebugString(L"Exception in Invoke\n");
        return E_FAIL;
    }

    return S_OK;
}


// 辅助方法
std::wstring XrayHandler::ConvertToWString(const std::string& str)
{
    if (str.empty())
        return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size);
    return result;
}

STDMETHODIMP XrayHandler::GetTypeInfoCount(UINT* pctinfo)
{
    *pctinfo = 0;
    return S_OK;
}

STDMETHODIMP XrayHandler::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP XrayHandler::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
    LCID lcid, DISPID* rgDispId)
{
    static const struct
    {
        const wchar_t* name;
        DISPID id;
    } methods[] = {
        {L"initialize", 1},       // Initialize/Connect
        {L"startWarmup", 2},      // Start warmup if cold
        {L"setVoltage", 3},       // Set kV
        {L"setCurrent", 4},       // Set uA
        {L"turnOnWithParams", 5}, // Turn on with kV,uA
        {L"turnOff", 6},          // Turn off
        {L"setFocus", 7},         // Set focus mode
        {L"getStatus", 8}         // Get current status
    };

    if (!rgszNames || !rgDispId)
        return E_INVALIDARG;

    for (const auto& method : methods)
    {
        if (wcscmp(rgszNames[0], method.name) == 0)
        {
            rgDispId[0] = method.id;
            return S_OK;
        }
    }
    return DISP_E_UNKNOWNNAME;
}

// Helper function for GetStatus
const char* GetStateString(CXray::XR_STATE state)
{
    switch (state)
    {
    case CXray::XR_NOT_INIT_YET:
        return "XR_NOT_READY";
    case CXray::XR_IS_COLD:
        return "XR_IS_COLD";
    case CXray::XR_IS_OFF:
        return "XR_IS_OFF";
    case CXray::XR_IS_ON:
        return "XR_IS_ON";
    default:
        return "UNKNOWN";
    }
}