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

        // 测试 CXray 的基本功能
        OutputDebugString(L"\n=== Testing basic CXray functions ===\n");
        if (!m_xray->Open())
        {
            OutputDebugString(L"Failed to open CXray\n");
            throw std::runtime_error("Failed to open CXray");
        }

        // 输出 CXray 的初始状态
        wchar_t buf[256];
        swprintf_s(buf, L"CXray State: %d\n", m_xray->State());
        OutputDebugString(buf);
        swprintf_s(buf, L"IsCold: %d\n", m_xray->IsCold());
        OutputDebugString(buf);
        swprintf_s(buf, L"IsOpen: %d\n", m_xray->IsOpen());
        OutputDebugString(buf);

        // 测试调用 TurnOn 方法
        OutputDebugString(L"\n=== Testing TurnOn function ===\n");
        UINT testVoltage = 100;
        UINT testCurrent = 200;
        swprintf_s(buf, L"Attempting TurnOn with voltage=%d, current=%d\n",
            testVoltage, testCurrent);
        OutputDebugString(buf);

        // BOOL result = m_xray->TurnOn(testVoltage, testCurrent);
        BOOL result = m_xray->TurnOn();
        swprintf_s(buf, L"TurnOn result: %d\n", result);
        OutputDebugString(buf);

        // 查询电压电流
        swprintf_s(buf, L"Voltage: %d\n", m_xray->kVSet());
        OutputDebugString(buf);
        swprintf_s(buf, L"Current: %d\n", m_xray->uASet());
        OutputDebugString(buf);
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
            if (!pDispParams || pDispParams->cArgs != 1)
                return E_INVALIDARG;
            if (pDispParams->rgvarg[0].vt != VT_I4)
                return E_INVALIDARG;
            return m_xray->SetkVuA(pDispParams->rgvarg[0].intVal,
                m_xray->uASet())
                ? S_OK
                : E_FAIL;

        case 4: // setCurrent
            OutputDebugString(L"setCurrent called\n");
            if (!pDispParams || pDispParams->cArgs != 1)
                return E_INVALIDARG;
            if (pDispParams->rgvarg[0].vt != VT_I4)
                return E_INVALIDARG;
            return m_xray->SetkVuA(m_xray->kVSet(),
                pDispParams->rgvarg[0].intVal)
                ? S_OK
                : E_FAIL;

        case 5: // turnOn
            OutputDebugString(L"turnOn called\n");
            if (!m_xray->TurnOn())
            {
                OutputDebugString(L"TurnOn failed\n");
                return E_FAIL;
            }
            OutputDebugString(L"TurnOn succeeded\n");
            return S_OK;

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
                OutputDebugString(L"getStatus succeeded\n");
                return S_OK;
            }
            catch (...)
            {
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

STDMETHODIMP XrayHandler::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
    LCID lcid, DISPID* rgDispId)
{
    if (!rgszNames || !rgDispId)
        return E_INVALIDARG;

    // 添加 initialize 方法
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
        {L"getSpotsizeCount", 9},
        {L"getSpotsize", 10}
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