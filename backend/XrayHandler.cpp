#include <winsock2.h>
#include <windows.h>
#include "XrayHandler.h"
#include "Xray.h"
#include <sstream>

XrayHandler::XrayHandler() : m_xray(nullptr)
{
    try
    {
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
        throw std::runtime_error("Failed to create Xray instance");
    }
}

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
            // 析构函数中最好不要抛出异常
        }
    }
}

STDMETHODIMP XrayHandler::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
                                 WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult,
                                 EXCEPINFO *pExcepInfo, UINT *puArgErr)
{

    if (!m_xray)
        return E_FAIL;

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

        case 4: // setCurrent
            if (!pDispParams || pDispParams->cArgs != 1)
                return E_INVALIDARG;
            if (pDispParams->rgvarg[0].vt != VT_I4)
                return E_INVALIDARG;
            return m_xray->SetkVuA(m_xray->kVSet(),
                                   pDispParams->rgvarg[0].intVal)
                       ? S_OK
                       : E_FAIL;

        case 5: // turnOnWithParams
            if (!pDispParams || pDispParams->cArgs != 2)
            {
                LOG_DEBUG(L"turnOnWithParams: Invalid arguments count: %d", pDispParams ? pDispParams->cArgs : 0);
                return E_INVALIDARG;
            }

            // 记录参数值
            LOG_DEBUG(L"turnOnWithParams called with kV=%d, uA=%d",
                      pDispParams->rgvarg[1].intVal,
                      pDispParams->rgvarg[0].intVal);

            if (m_xray->IsCold())
            {
                LOG_DEBUG(L"X-ray is cold, starting warmup");
                if (!m_xray->StartWarmUp())
                {
                    LOG_DEBUG(L"Warmup failed");
                    return E_FAIL;
                }
                Sleep(100);
            }

            BOOL result = m_xray->TurnOn(
                pDispParams->rgvarg[1].intVal, // kV
                pDispParams->rgvarg[0].intVal  // uA
            );

            LOG_DEBUG(L"TurnOn result: %s", result ? L"SUCCESS" : L"FAILED");
            return result ? S_OK : E_FAIL;

        case 6: // turnOff
            return m_xray->TurnOff() ? S_OK : E_FAIL;

        case 7: // setFocus
            if (!pDispParams || pDispParams->cArgs != 1)
                return E_INVALIDARG;
            if (pDispParams->rgvarg[0].vt != VT_I4)
                return E_INVALIDARG;
            return m_xray->SetSpotsize(pDispParams->rgvarg[0].intVal) ? S_OK : E_FAIL;

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

        default:
            return DISP_E_MEMBERNOTFOUND;
        }
    }
    catch (...)
    {
        return E_FAIL;
    }
    return DISP_E_MEMBERNOTFOUND;
}

// 辅助方法
std::wstring XrayHandler::ConvertToWString(const std::string &str)
{
    if (str.empty())
        return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size);
    return result;
}

STDMETHODIMP XrayHandler::GetTypeInfoCount(UINT *pctinfo)
{
    *pctinfo = 0;
    return S_OK;
}

STDMETHODIMP XrayHandler::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP XrayHandler::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
                                        LCID lcid, DISPID *rgDispId)
{
    static const struct
    {
        const wchar_t *name;
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

    for (const auto &method : methods)
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
const char *GetStateString(CXray::XR_STATE state)
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