
#pragma once

#include <winsock2.h>
#include <windows.h>
#include "XrayHandler.h"
#include "Xray.h"
#include <sstream>

XrayHandler::XrayHandler() : m_xray(nullptr), m_hXrayDll(nullptr) {
    Initialize();
}

XrayHandler::~XrayHandler() {
    if (m_xray) {
        if (m_xray->IsBeamOn()) {
            m_xray->TurnOff();
        }
        m_xray->Close();
        delete m_xray;
    }
    UnloadXrayDll();
}

bool XrayHandler::LoadXrayDll() {
    // 先尝试当前目录
    m_hXrayDll = LoadLibraryW(L"XRay2.dll");
    if (!m_hXrayDll) {
        // 尝试bin子目录
        m_hXrayDll = LoadLibraryW(L"bin\\XRay2.dll");
    }
    return m_hXrayDll != nullptr;
}

void XrayHandler::UnloadXrayDll() {
    if (m_hXrayDll) {
        FreeLibrary(m_hXrayDll);
        m_hXrayDll = nullptr;
    }
}

HRESULT XrayHandler::Initialize() {
    if (!LoadXrayDll()) {
        return E_FAIL;
    }

    m_xray = new CXray();
    if (!m_xray->Open()) {
        delete m_xray;
        m_xray = nullptr;
        return E_FAIL;
    }

    return S_OK;
}

HRESULT XrayHandler::SetVoltage(int kV) {
    if (!m_xray) return E_FAIL;
    m_xray->SetkVuA(kV, 0);
    return S_OK;
}

HRESULT XrayHandler::SetCurrent(int uA) {
    if (!m_xray) return E_FAIL;
    m_xray->SetkVuA(0, uA);
    return S_OK;
}

HRESULT XrayHandler::SetFocus(int mode) {
    if (!m_xray) return E_FAIL;
    m_xray->SetSpotsize(mode);
    return S_OK;
}

HRESULT XrayHandler::TurnOn() {
    if (!m_xray) return E_FAIL;

    if (m_xray->IsCold()) {
        m_xray->StartWarmUp();
        Sleep(100); // Give some time for warmup to start
    }

    m_xray->TurnOn();
    return S_OK;
}

HRESULT XrayHandler::TurnOff() {
    if (!m_xray) return E_FAIL;
    m_xray->TurnOff();
    return S_OK;
}

HRESULT XrayHandler::GetStatus(VARIANT* pResult) {
    if (!m_xray || !pResult) return E_FAIL;

    bool isPowered = m_xray->IsBeamOn();
    bool isWarmedUp = !m_xray->IsCold();
    std::string status;

    // Convert status to string
    switch (m_xray->State()) {
    case XR_NOT_INIT_YET: status = "XR_NOT_READY"; break;
    case XR_IS_COLD: status = "XR_IS_COLD"; break;
    case XR_IS_OFF: status = "XR_IS_OFF"; break;
    case XR_IS_ON: status = "XR_IS_ON"; break;
    default: status = "UNKNOWN";
    }

    // Create JSON string
    std::ostringstream json;
    json << "{\"isPowered\":" << (isPowered ? "true" : "false")
        << ",\"isWarmedUp\":" << (isWarmedUp ? "true" : "false")
        << ",\"status\":\"" << status << "\"}";

    // Convert to BSTR
    std::wstring wstr = ConvertToWString(json.str());
    pResult->vt = VT_BSTR;
    pResult->bstrVal = SysAllocString(wstr.c_str());

    return S_OK;
}

// IDispatch implementation
STDMETHODIMP XrayHandler::GetTypeInfoCount(UINT* pctinfo) {
    *pctinfo = 0;
    return S_OK;
}

STDMETHODIMP XrayHandler::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) {
    return E_NOTIMPL;
}

STDMETHODIMP XrayHandler::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
    LCID lcid, DISPID* rgDispId) {
    static const struct {
        const wchar_t* name;
        DISPID id;
    } methods[] = {
        {L"setVoltage", 1},
        {L"setCurrent", 2},
        {L"setFocus", 3},
        {L"turnOn", 4},
        {L"turnOff", 5},
        {L"getStatus", 6}
    };

    if (!rgszNames || !rgDispId) return E_INVALIDARG;

    if (cNames == 1) {
        for (const auto& method : methods) {
            if (wcscmp(rgszNames[0], method.name) == 0) {
                rgDispId[0] = method.id;
                return S_OK;
            }
        }
    }
    return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP XrayHandler::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
    WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
    EXCEPINFO* pExcepInfo, UINT* puArgErr) {

    if (!pDispParams) return E_INVALIDARG;

    switch (dispIdMember) {
    case 1: // setVoltage
        if (pDispParams->cArgs != 1) return E_INVALIDARG;
        return SetVoltage(pDispParams->rgvarg[0].intVal);

    case 2: // setCurrent
        if (pDispParams->cArgs != 1) return E_INVALIDARG;
        return SetCurrent(pDispParams->rgvarg[0].intVal);

    case 3: // setFocus
        if (pDispParams->cArgs != 1) return E_INVALIDARG;
        return SetFocus(pDispParams->rgvarg[0].intVal);

    case 4: // turnOn
        return TurnOn();

    case 5: // turnOff
        return TurnOff();

    case 6: // getStatus
        if (!pVarResult) return E_INVALIDARG;
        return GetStatus(pVarResult);

    default:
        return DISP_E_MEMBERNOTFOUND;
    }
}

std::wstring XrayHandler::ConvertToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size);
    return result;
}