#include <winsock2.h>
#include <windows.h>
#include "XrayHandler.h"
#include "Xray.h"
#include <sstream>

const char* GetStateString(CXray::XR_STATE state);

// XrayHandler.cpp 添加 DLL 加载相关实现

bool XrayHandler::LoadXrayDll()
{
    // 尝试从当前目录加载
    m_hXrayDll = LoadLibrary(L"Xray2.dll");
    if (!m_hXrayDll)
    {
        // 如果失败，尝试从 bin 目录加载
        m_hXrayDll = LoadLibrary(L"bin\\Xray2.dll");
        if (!m_hXrayDll)
        {
            OutputDebugString(L"Failed to load Xray2.dll\n");
            return false;
        }
    }

    OutputDebugString(L"Successfully loaded Xray2.dll\n");
    return true;
}

void XrayHandler::UnloadXrayDll()
{
    if (m_hXrayDll)
    {
        FreeLibrary(m_hXrayDll);
        m_hXrayDll = nullptr;
    }
}

// 修改构造函数
XrayHandler::XrayHandler() : m_xray(nullptr), m_hXrayDll(nullptr)
{
    try
    {
        if (!LoadXrayDll())
        {
            throw std::runtime_error("Failed to load Xray2.dll");
        }

        m_xray = new CXray();
        if (!m_xray || !m_xray->Open())
        {
            if (m_xray)
            {
                delete m_xray;
                m_xray = nullptr;
            }
            throw std::runtime_error("Failed to initialize Xray");
        }
    }
    catch (...)
    {
        UnloadXrayDll();
        throw std::runtime_error("Failed to create Xray instance");
    }
}

// 修改析构函数
XrayHandler::~XrayHandler()
{
    if (m_xray)
    {
        try
        {
            if (m_xray->IsBeamOn())
            {
                m_xray->TurnOff();
            }
            m_xray->Close();
            delete m_xray;
        }
        catch (...)
        {
        }
    }
    UnloadXrayDll();
}

STDMETHODIMP XrayHandler::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
    WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
    EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
    if (!m_xray)
        return E_FAIL;

    OutputDebugString(L"Invoke called\n");

    BOOL result = FALSE;

    try
    {
        // 对 PROPERTYPUT（赋值操作）进行处理
        if (wFlags & DISPATCH_PROPERTYPUT)
        {
            switch (dispIdMember)
            {
            case 11: // Voltage property
                OutputDebugString(L"Voltage property PUT called\n");
                if (!pDispParams || pDispParams->cArgs != 1) {
                    OutputDebugString(L"Invalid argument count for Voltage property\n");
                    return E_INVALIDARG;
                }
                if (pDispParams->rgvarg[0].vt != VT_I4) {
                    OutputDebugString(L"Invalid argument type for Voltage property\n");
                    return E_INVALIDARG;
                }
                try {
                    UINT kV = static_cast<UINT>(pDispParams->rgvarg[0].intVal);
                    if (kV > 180) {
                        OutputDebugString(L"Voltage out of range\n");
                        return E_INVALIDARG;
                    }
                    return m_xray->SetkV(kV) ? S_OK : E_FAIL;
                }
                catch (...) {
                    OutputDebugString(L"Exception in Voltage property PUT\n");
                    return E_FAIL;
                }

            case 12: // Current property
                OutputDebugString(L"Current property PUT called\n");
                if (!pDispParams || pDispParams->cArgs != 1) {
                    OutputDebugString(L"Invalid argument count for Current property\n");
                    return E_INVALIDARG;
                }
                if (pDispParams->rgvarg[0].vt != VT_I4) {
                    OutputDebugString(L"Invalid argument type for Current property\n");
                    return E_INVALIDARG;
                }
                try {
                    UINT uA = static_cast<UINT>(pDispParams->rgvarg[0].intVal);
                    if (uA > m_xray->GetMaxuA(m_xray->kVSet())) {
                        OutputDebugString(L"Current out of range\n");
                        return E_INVALIDARG;
                    }
                    return m_xray->SetuA(uA) ? S_OK : E_FAIL;
                }
                catch (...) {
                    OutputDebugString(L"Exception in Current property PUT\n");
                    return E_FAIL;
                }

            default:
                return DISP_E_MEMBERNOTFOUND;
            }
        }

        switch (dispIdMember)
        {
        case 1: // initialize
            OutputDebugString(L"initialize called\n");
            return m_xray->Open() ? S_OK : E_FAIL;

        case 2: // startWarmup
            OutputDebugString(L"startWarmup called\n");
            if (m_xray->IsCold())
            {
                return m_xray->StartWarmUp() ? S_OK : E_FAIL;
            }
            return S_OK;

        case 3: // setVoltage
            OutputDebugString(L"setVoltage called\n");

            if (!pDispParams || pDispParams->cArgs != 1) {
                OutputDebugString(L"Args count: ");
                wchar_t buffer[10];
                swprintf(buffer, 10, L"%d", pDispParams->cArgs);
                OutputDebugString(buffer);
                OutputDebugString(L" Invalid argument count\n");

                return E_INVALIDARG;
            }
            if (pDispParams->rgvarg[0].vt != VT_I4) {
                OutputDebugString(L"Invalid argument type\n");
                return E_INVALIDARG;
            }
            try {
                UINT kV = static_cast<UINT>(pDispParams->rgvarg[0].intVal);
                if (kV > 180) { // 添加范围检查
                    OutputDebugString(L"Voltage value out of range\n");
                    return E_INVALIDARG;
                }
                return m_xray->SetkV(kV) ? S_OK : E_FAIL;
            }
            catch (...) {
                OutputDebugString(L"Exception in setVoltage\n");
                return E_FAIL;
            }

        case 4: // setCurrent
            OutputDebugString(L"setCurrent called\n");
            if (!pDispParams || pDispParams->cArgs != 1) {
                OutputDebugString(L"Invalid argument count\n");
                return E_INVALIDARG;
            }
            if (pDispParams->rgvarg[0].vt != VT_I4) {
                OutputDebugString(L"Invalid argument type\n");
                return E_INVALIDARG;
            }
            try {
                UINT uA = static_cast<UINT>(pDispParams->rgvarg[0].intVal);
                if (uA > m_xray->GetMaxuA(m_xray->kVSet())) { // 添加范围检查
                    OutputDebugString(L"Current value out of range\n");
                    return E_INVALIDARG;
                }
                return m_xray->SetuA(uA) ? S_OK : E_FAIL;
            }
            catch (...) {
                OutputDebugString(L"Exception in setCurrent\n");
                return E_FAIL;
            }


        case 5: // turnOn
            OutputDebugString(L"turnOn called\n");
            result = m_xray->TurnOn();
            return result ? S_OK : E_FAIL;


        case 6: // turnOff
            OutputDebugString(L"turnOff called\n");
            result = m_xray->TurnOff();
            return result ? S_OK : E_FAIL;

        case 7: // setFocus
            OutputDebugString(L"setFocus called\n");
            if (!pDispParams || pDispParams->cArgs != 1)
                return E_INVALIDARG;
            if (pDispParams->rgvarg[0].vt != VT_I4)
                return E_INVALIDARG;
            result = m_xray->SetSpotsize(pDispParams->rgvarg[0].intVal);
            return result ? S_OK : E_FAIL;

        case 8: // getStatus
            OutputDebugString(L"getStatus called\n");
            if (!pVarResult)
                return E_INVALIDARG;
            try {
                std::ostringstream json;
                CXray::XR_STATE state = m_xray->State();
                const char* stateStr = GetStateString(state);

                json << "{"
                    << "\"status\":\"" << stateStr << "\","  // 修改这里，确保状态字符串被包含
                    << "\"isPowered\":" << (m_xray->IsBeamOn() ? "true" : "false") << ","
                    << "\"isWarmedUp\":" << (!m_xray->IsCold() ? "true" : "false") << ","
                    << "\"isBeaming\":" << (m_xray->IsBeaming() ? "true" : "false") << ","
                    << "\"voltage\":" << m_xray->kVSet() << ","
                    << "\"current\":" << m_xray->uASet()
                    << "}";

                std::wstring wstr = ConvertToWString(json.str());
                pVarResult->vt = VT_BSTR;
                pVarResult->bstrVal = SysAllocString(wstr.c_str());
                OutputDebugString(L"getStatus succeeded\n");
                OutputDebugString(wstr.c_str());  // 添加调试输出
                return S_OK;
            }
            catch (...) {
                OutputDebugString(L"getStatus failed\n");
                return E_FAIL;
            }

        default:
            OutputDebugString(L"Unknown DISPID\n");
            return DISP_E_MEMBERNOTFOUND;
        }
    }
    catch (...)
    {
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

STDMETHODIMP XrayHandler::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames,
    UINT cNames, LCID lcid, DISPID* rgDispId)
{
    if (!rgszNames || !rgDispId)
        return E_INVALIDARG;

    static const struct {
        const wchar_t* name;
        DISPID id;
    } methods[] = {
        {L"initialize", 1},
        {L"startWarmup", 2},
        {L"setVoltage", 3},
        {L"setCurrent", 4},
        {L"turnOn", 5},
        {L"turnOff", 6},
        {L"setFocus", 7},
        {L"getStatus", 8},
        // 新增属性 "Voltage"
        {L"Voltage", 11},
        {L"Current", 12}
    };

    OutputDebugString(L"GetIDsOfNames called for: ");
    OutputDebugString(rgszNames[0]);
    OutputDebugString(L"\n");

    for (const auto& method : methods) {
        if (wcscmp(rgszNames[0], method.name) == 0) {
            *rgDispId = method.id;
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
    case CXray::XR_NOT_READY:
        return "XR_NOT_READY";
    case CXray::XR_IS_COLD:
        return "XR_IS_COLD";
    case CXray::XR_IS_OFF:
        return "XR_IS_OFF";
    case CXray::XR_IS_ON:
        return "XR_IS_ON";
    case CXray::XR_IS_RAMPINGUP:
        return "XR_IS_ON";
    default:
        return "XR_NOT_READY";
    }
}