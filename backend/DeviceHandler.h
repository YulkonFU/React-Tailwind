#pragma once

#include <windows.h>
#include <WebView2.h>
#include <wrl.h>
#include <string>
#include "Xray.h"
#include "Cnc.h"
#include "DeviceHandler_i.h"  // 使用MIDL生成的接口定义

class DeviceHandler :
    public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IDeviceHandler,
    IDispatch>
{
private:
    // Device instances
    CXray* m_xray;
    CCnc* m_cnc;

    // DLL handles  
    HMODULE m_hXrayDll;
    HMODULE m_hCncDll;

    // Type library
    ITypeLib* m_typeLib;

    // Axis information
    double* positions;
    UINT axisCount;

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
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgDispId);
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
        EXCEPINFO* pExcepInfo, UINT* puArgErr);

    // IDeviceHandler Methods
    // Xray Control Methods
    STDMETHOD(InitializeXray)();
    STDMETHOD(StartWarmup)();
    STDMETHOD(SetVoltage)(INT kV);
    STDMETHOD(SetCurrent)(INT uA);
    STDMETHOD(SetFocus)(INT mode);
    STDMETHOD(TurnXrayOn)();
    STDMETHOD(TurnXrayOff)();
    STDMETHOD(GetXrayStatus)(VARIANT* pResult);

    // CNC Control Methods
    STDMETHOD(InitializeCnc)();
    STDMETHOD(StartReference)(INT axisIndex);
    STDMETHOD(MoveAxis)(INT axisIndex, DOUBLE position);
    STDMETHOD(MoveAllAxes)(SAFEARRAY* positions);
    STDMETHOD(Stop)(INT axisIndex);
    STDMETHOD(EnableJoy)(INT axisIndex, BOOL enable);
    STDMETHOD(GetCncStatus)(VARIANT* pResult);
    STDMETHOD(GetAxesInfo)(VARIANT* pResult);
    STDMETHOD(GetPositions)(VARIANT* pResult);
};