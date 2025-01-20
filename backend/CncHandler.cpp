#include "CncHandler.h"
#include "Cnc.h"
#include <sstream>
#include <vector>

const char* GetStateString(CNCZustand state);

CncHandler::CncHandler() : m_cnc(nullptr), m_hCncDll(nullptr)
{
    try
    {
        if (!LoadCncDll())
        {
            throw std::runtime_error("Failed to load Cnc DLL");
        }

        m_cnc = new CCnc();
        if (!m_cnc || !m_cnc->Open())
        {
            if (m_cnc)
            {
                delete m_cnc;
                m_cnc = nullptr;
            }
            throw std::runtime_error("Failed to initialize Cnc");
        }
    }
    catch (...)
    {
        UnloadCncDll();
        throw;
    }
}

CncHandler::~CncHandler()
{
    if (m_cnc)
    {
        try
        {
            m_cnc->Close();
            delete m_cnc;
        }
        catch (...) {}
    }
    UnloadCncDll();
}

STDMETHODIMP CncHandler::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
    WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
    EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
    if (!m_cnc)
        return E_FAIL;

    try
    {
        switch (dispIdMember)
        {
        case 1: // initialize
            return m_cnc->Open() ? S_OK : E_FAIL;

        case 2: // startReference
            if (!pDispParams || pDispParams->cArgs != 1)
                return E_INVALIDARG;
            return m_cnc->StartReference(pDispParams->rgvarg[0].intVal) ? S_OK : E_FAIL;

        case 3: // moveAxis 
            if (!pDispParams || pDispParams->cArgs != 2)
                return E_INVALIDARG;
            return m_cnc->StartTo(pDispParams->rgvarg[1].intVal,
                pDispParams->rgvarg[0].dblVal) ? S_OK : E_FAIL;

        case 4: // moveAllAxes
            if (!pDispParams || pDispParams->cArgs != 1)
                return E_INVALIDARG;
            {
                SAFEARRAY* psa = pDispParams->rgvarg[0].parray;
                double positions[CNC_MAX_AXIS] = { 0 };
                for (LONG i = 0; i < m_cnc->NrAxis(); i++)
                {
                    VARIANT value;
                    SafeArrayGetElement(psa, &i, &value);
                    positions[i] = value.dblVal;
                    VariantClear(&value);
                }
                return m_cnc->StartTo(positions) ? S_OK : E_FAIL;
            }

        case 5: // stop
            if (!pDispParams || pDispParams->cArgs != 1)
                return E_INVALIDARG;
            m_cnc->Stop(pDispParams->rgvarg[0].intVal);
            return S_OK;

        case 6: // enableJoy
            if (!pDispParams || pDispParams->cArgs != 2)
                return E_INVALIDARG;
            if (pDispParams->rgvarg[0].boolVal)
                m_cnc->EnableJoy(pDispParams->rgvarg[1].intVal);
            else
                m_cnc->DisableJoy(pDispParams->rgvarg[1].intVal);
            return S_OK;

        case 7: // getStatus 
            if (!pVarResult)
                return E_INVALIDARG;
            try {
                std::ostringstream json;
                CNCZustand state = (CNCZustand)m_cnc->Zustand();
                const char* stateStr = GetStateString(state);

                json << "{"
                    << "\"status\":\"" << stateStr << "\","
                    << "\"isOpen\":" << (m_cnc->IsOpen() ? "true" : "false") << ","
                    << "\"isRefDone\":" << (m_cnc->IsRefDone() ? "true" : "false") << ","
                    << "\"isMoving\":" << (m_cnc->IsMoving() ? "true" : "false") << ","
                    << "\"isJoyEnabled\":" << (!m_cnc->AllJoyDisabled() ? "true" : "false") << ","
                    << "\"isDoorOpen\":" << (!m_cnc->IsInterlockClosed() ? "true" : "false") << ","
                    << "\"isCollisionDetected\":" << (m_cnc->IsHWCollDetected() ? "true" : "false")
                    << "}";

                std::wstring wstr = ConvertToWString(json.str());
                pVarResult->vt = VT_BSTR;
                pVarResult->bstrVal = SysAllocString(wstr.c_str());
                return S_OK;
            }
            catch (...) {
                return E_FAIL;
            }

        case 8: // getAxesInfo
            if (!pVarResult)
                return E_INVALIDARG;
            try {
                std::ostringstream json;
                json << "{"
                    << "\"axisCount\":" << m_cnc->NrAxis() << ",";

                json << "\"axes\":[";
                for (UINT i = 0; i < m_cnc->NrAxis(); i++) {
                    if (i > 0) json << ",";
                    json << "{\"name\":\"" << m_cnc->AxisName(i) << "\"}";
                }
                json << "]}";

                std::wstring wstr = ConvertToWString(json.str());
                pVarResult->vt = VT_BSTR;
                pVarResult->bstrVal = SysAllocString(wstr.c_str());
                return S_OK;
            }
            catch (...) {
                return E_FAIL;
            }

        case 9: // getPositions
            if (!pVarResult)
                return E_INVALIDARG;
            try {
                double positions[CNC_MAX_AXIS] = { 0 };
                m_cnc->GetAllLastPositions(positions);

                std::ostringstream json;
                json << "[";
                for (UINT i = 0; i < m_cnc->NrAxis(); i++) {
                    if (i > 0) json << ",";
                    json << positions[i];
                }
                json << "]";

                std::wstring wstr = ConvertToWString(json.str());
                pVarResult->vt = VT_BSTR;
                pVarResult->bstrVal = SysAllocString(wstr.c_str());
                return S_OK;
            }
            catch (...) {
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
}

STDMETHODIMP CncHandler::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames,
    UINT cNames, LCID lcid, DISPID* rgDispId)
{
    if (!rgszNames || !rgDispId)
        return E_INVALIDARG;

    static const struct {
        const wchar_t* name;
        DISPID id;
    } methods[] = {
        {L"initialize", 1},
        {L"startReference", 2},
        {L"moveAxis", 3},
        {L"moveAllAxes", 4},
        {L"stop", 5},
        {L"enableJoy", 6},
        {L"getStatus", 7},
        {L"getAxesInfo", 8},
        {L"getPositions", 9}
    };

    for (const auto& method : methods) {
        if (wcscmp(rgszNames[0], method.name) == 0) {
            *rgDispId = method.id;
            return S_OK;
        }
    }

    return DISP_E_UNKNOWNNAME;
}

// Helper functions implementation
bool CncHandler::LoadCncDll()
{
    m_hCncDll = LoadLibrary(L"EcoCncx64.dll");
    if (!m_hCncDll)
    {
        m_hCncDll = LoadLibrary(L"bin\\EcoCncx64.dll");
        if (!m_hCncDll)
        {
            OutputDebugString(L"Failed to load EcoCncx64.dll\n");
            return false;
        }
    }
    return true;
}

void CncHandler::UnloadCncDll()
{
    if (m_hCncDll)
    {
        FreeLibrary(m_hCncDll);
        m_hCncDll = nullptr;
    }
}

std::wstring CncHandler::ConvertToWString(const std::string& str)
{
    if (str.empty())
        return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size);
    return result;
}

const char* GetStateString(CNCZustand state)
{
    switch (state)
    {
    case CNC_NOT_INIT_YET:
        return "CNC_NOT_READY";
    case CNC_DRIVING_REF:
        return "CNC_DRIVING_REF";
    case CNC_WAS_STARTED:
        return "CNC_MOVING";
    case CNC_STAND_STILL:
        return "CNC_STAND_STILL";
    case CNC_NOT_READY:
        return "CNC_NOT_READY";
    default:
        return "CNC_NOT_READY";
    }
}

// Implement basic IDispatch methods
STDMETHODIMP CncHandler::GetTypeInfoCount(UINT* pctinfo)
{
    *pctinfo = 0;
    return S_OK;
}

STDMETHODIMP CncHandler::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
{
    return E_NOTIMPL;
}