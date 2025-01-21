#pragma once

#include <windows.h>
#include <WebView2.h>
#include <wrl.h>
#include <string>
#include "Xray.h"
#include "Cnc.h"

class DeviceHandler : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IDispatch> {

private:
    // Device instances
    CXray* m_xray;
    CCnc* m_cnc;

    // DLL handles  
    HMODULE m_hXrayDll;
    HMODULE m_hCncDll;

    // Helper methods
    bool LoadXrayDll();
    bool LoadCncDll();
    void UnloadXrayDll();
    void UnloadCncDll();
    std::wstring ConvertToWString(const std::string& str);

public:
    DeviceHandler();
    ~DeviceHandler();

    // IDispatch Methods
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override;
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgDispId) override;
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
        EXCEPINFO* pExcepInfo, UINT* puArgErr) override;

    // Xray Control Methods
    HRESULT InitializeXray();
    HRESULT SetVoltage(int kV);
    HRESULT SetCurrent(int uA);
    HRESULT SetFocus(int mode);
    HRESULT TurnXrayOn();
    HRESULT TurnXrayOff();
    HRESULT GetXrayStatus(VARIANT* pResult);

    // CNC Control Methods
    HRESULT InitializeCnc();
    HRESULT StartReference(int axisIndex = -1);
    HRESULT MoveAxis(int axisIndex, double position);
    HRESULT MoveAllAxes(double positions[]);
    HRESULT Stop(int axisIndex = -1);
    HRESULT EnableJoy(int axisIndex = -1, bool enable = true);
    HRESULT GetCncStatus(VARIANT* pResult);
    HRESULT GetAxesInfo(VARIANT* pResult);
    HRESULT GetPositions(VARIANT* pResult);
};