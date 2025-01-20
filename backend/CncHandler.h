#pragma once

#include <WinSock2.h>
#include <windows.h>
#include <WebView2.h>
#include <wrl.h>
#include <string>
#include "Cnc.h"

class CncHandler : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IDispatch > {
private:
    CCnc* m_cnc;
    HMODULE m_hCncDll;
    bool LoadCncDll();
    void UnloadCncDll();
    std::wstring ConvertToWString(const std::string& str);

public:
    CncHandler();
    ~CncHandler();

    // IDispatch Methods
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override;
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgDispId) override;
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
        EXCEPINFO* pExcepInfo, UINT* puArgErr) override;

    // Control Methods 
    HRESULT Initialize();  // Open()
    HRESULT StartReference(int axisIndex = -1); // StartReference()
    HRESULT MoveAxis(int axisIndex, double position); // StartTo()
    HRESULT MoveAllAxes(double positions[]); // StartTo()
    HRESULT Stop(int axisIndex = -1); // Stop()
    HRESULT EnableJoy(int axisIndex = -1, bool enable = true); // EnableJoy()/DisableJoy()
    HRESULT GetStatus(VARIANT* pResult); // Zustand(), IsOpen(), etc.
    HRESULT GetAxesInfo(VARIANT* pResult); // NrAxis(), AxisName(), etc.
    HRESULT GetPositions(VARIANT* pResult); // GetAllLastPositions()
};