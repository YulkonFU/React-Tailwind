#include <winsock2.h>
#include <windows.h>
#include "XrayHandler.h"
#include "Xray.h"
#include <sstream>

const char* GetStateString(CXray::XR_STATE state);


// XrayHandler.cpp 添加 DLL 加载相关实现

bool XrayHandler::LoadXrayDll() {
    OutputDebugString(L"\n=== Starting LoadXrayDll ===\n");

    // 获取当前目录
    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDir);
    OutputDebugString(L"Current directory: ");
    OutputDebugString(currentDir);
    OutputDebugString(L"\n");

    // 尝试从当前目录加载
    OutputDebugString(L"Attempting to load Xray2.dll from current directory...\n");
    m_hXrayDll = LoadLibrary(L"Xray2.dll");
    if (!m_hXrayDll) {
        DWORD error = GetLastError();
        wchar_t buf[256];
        swprintf_s(buf, L"Failed to load from current directory. Error: %d\n", error);
        OutputDebugString(buf);

        // 尝试从 bin 目录加载
        OutputDebugString(L"Attempting to load from bin directory...\n");
        m_hXrayDll = LoadLibrary(L"bin\\Xray2.dll");
        if (!m_hXrayDll) {
            error = GetLastError();
            swprintf_s(buf, L"Failed to load from bin directory. Error: %d\n", error);
            OutputDebugString(buf);
            return false;
        }
    }

    OutputDebugString(L"Successfully loaded Xray2.dll\n");

    // 验证 DLL 导出函数
    FARPROC procAddress = GetProcAddress(m_hXrayDll, "?Open@CXray@@QAE_NPAVCWnd@@@Z");
    if (procAddress) {
        OutputDebugString(L"Found CXray::Open function\n");
    }
    else {
        OutputDebugString(L"Failed to find CXray::Open function\n");
    }

    OutputDebugString(L"=== LoadXrayDll completed ===\n\n");
    return true;
}

void XrayHandler::UnloadXrayDll() {
    if (m_hXrayDll) {
        FreeLibrary(m_hXrayDll);
        m_hXrayDll = nullptr;
    }
}


// XrayHandler.cpp
XrayHandler::XrayHandler() : m_xray(nullptr), m_hXrayDll(nullptr) {
    try {
        OutputDebugString(L"\n=== Starting XrayHandler initialization ===\n");

        if (!LoadXrayDll()) {
            OutputDebugString(L"Failed to load Xray2.dll\n");
            throw std::runtime_error("Failed to load Xray2.dll");
        }

        m_xray = new CXray();
        if (!m_xray) {
            OutputDebugString(L"Failed to create CXray instance\n");
            throw std::runtime_error("Failed to create CXray instance");
        }

        // 测试 CXray 的基本功能
        OutputDebugString(L"\n=== Testing basic CXray functions ===\n");
        if (!m_xray->Open()) {
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

        BOOL result = m_xray->TurnOn(testVoltage, testCurrent);
        swprintf_s(buf, L"TurnOn result: %d\n", result);
        OutputDebugString(buf);

        if (!result) {
            int error = m_xray->GetLastError();
            swprintf_s(buf, L"Last error code: %d\n", error);
            OutputDebugString(buf);
            OutputDebugString(L"Error description: ");
            OutputDebugStringA(m_xray->GetErrorText(error));
            OutputDebugString(L"\n");
        }

        OutputDebugString(L"\n=== XrayHandler initialization completed ===\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA("\nException in XrayHandler constructor: ");
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
        UnloadXrayDll();
        throw;
    }
    catch (...) {
        OutputDebugString(L"\nUnknown exception in XrayHandler constructor\n");
        UnloadXrayDll();
        throw;
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

STDMETHODIMP XrayHandler::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
    LCID lcid, DISPID* rgDispId)
{
    if (!rgszNames || !rgDispId)
        return E_INVALIDARG;

    if (wcscmp(rgszNames[0], L"turnOnWithParams") == 0)
        *rgDispId = 5;
    else
        return DISP_E_UNKNOWNNAME;

    return S_OK;
}

STDMETHODIMP XrayHandler::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
    WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
    EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
    if (!m_xray)
        return E_FAIL;

    if (dispIdMember == 5) // turnOnWithParams
    {
        OutputDebugString(L"turnOnWithParams called\n");

        if (!pDispParams || pDispParams->cArgs != 2) {
            OutputDebugString(L"Parameter count error\n");
            return E_INVALIDARG;
        }

        // 参数检查
        if (pDispParams->rgvarg[0].vt != VT_I4 ||
            pDispParams->rgvarg[1].vt != VT_I4) {
            OutputDebugString(L"Parameter type error\n");
            return E_INVALIDARG;
        }

        int voltage = pDispParams->rgvarg[1].intVal;
        int current = pDispParams->rgvarg[0].intVal;

        wchar_t buf[256];
        swprintf_s(buf, L"Attempting to turn on: V=%d, I=%d\n", voltage, current);
        OutputDebugString(buf);

        if (!m_xray->TurnOn(voltage, current)) {
            OutputDebugString(L"TurnOn failed\n");
            return E_FAIL;
        }

        OutputDebugString(L"TurnOn succeeded\n");
        return S_OK;
    }

    return DISP_E_MEMBERNOTFOUND;
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