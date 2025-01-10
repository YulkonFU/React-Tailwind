#pragma once

#include <windows.h>
#include <WebView2.h>
#include <wrl.h>
#include <string>
#include "Xray.h"

// Debug 日志宏
#ifdef _DEBUG
#define LOG_DEBUG(msg, ...) { \
    wchar_t buf[1024]; \
    swprintf_s(buf, L"[DEBUG] " msg L"\n", __VA_ARGS__); \
    OutputDebugString(buf); \
}
#else
#define LOG_DEBUG(msg, ...) 
#endif

// 前向声明以避免包含 Xray.h
class CXray;

class XrayHandler : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IDispatch> {
private:
    CXray* m_xray;
    HMODULE m_hXrayDll;
    bool LoadXrayDll();
    void UnloadXrayDll();

    std::wstring ConvertToWString(const std::string& str);

public:
    XrayHandler();
    ~XrayHandler();

    // IDispatch Methods
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override;
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgDispId) override;
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
        EXCEPINFO* pExcepInfo, UINT* puArgErr) override;

    // Control Methods
    HRESULT Initialize();
    HRESULT SetVoltage(int kV);
    HRESULT SetCurrent(int uA);
    HRESULT SetFocus(int mode);
    HRESULT TurnOn();
    HRESULT TurnOff();
    HRESULT GetStatus(VARIANT* pResult);
};
