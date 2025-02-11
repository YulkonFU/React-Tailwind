#pragma once

#include <windows.h>
#include <WebView2.h>
#include <wrl.h>
#include <string>
#include "Xray.h"
#include "Cnc.h"
#include "DeviceHandler_i.h"  // 生成的头文件将会在这里

// GUID可以使用guidgen.exe生成
// {026BB2C1-CBDC-4497-9772-39A2F75032D7}
interface __declspec(uuid("026BB2C1-CBDC-4497-9772-39A2F75032D7")) IDeviceHandler : IUnknown
{
    // Xray Control Methods
    [id(1)] HRESULT InitializeXray();
    [id(2)] HRESULT StartWarmup();
    [id(3)] HRESULT SetVoltage([in] INT kV);
    [id(4)] HRESULT SetCurrent([in] INT uA);
    [id(5)] HRESULT TurnXrayOn();
    [id(6)] HRESULT TurnXrayOff();
    [id(7)] HRESULT SetFocus([in] INT mode);
    [id(8)] HRESULT GetXrayStatus([out, retval] VARIANT* pResult);

    // CNC Control Methods
    [id(101)] HRESULT InitializeCnc();
    [id(102)] HRESULT StartReference([in] INT axisIndex);
    [id(103)] HRESULT MoveAxis([in] INT axisIndex,[in] DOUBLE position);
    [id(104)] HRESULT MoveAllAxes([in] SAFEARRAY* positions);
    [id(105)] HRESULT Stop([in] INT axisIndex);
    [id(106)] HRESULT EnableJoy([in] INT axisIndex,[in] BOOL enable);
    [id(107)] HRESULT GetCncStatus([out, retval] VARIANT* pResult);
    [id(108)] HRESULT GetAxesInfo([out, retval] VARIANT* pResult);
    [id(109)] HRESULT GetPositions([out, retval] VARIANT* pResult);
};

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
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override;
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgDispId) override;
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
        EXCEPINFO* pExcepInfo, UINT* puArgErr) override;

    // IDeviceHandler Methods
    // Xray Control Methods
    STDMETHOD(InitializeXray)() override;
    STDMETHOD(StartWarmup)() override;
    STDMETHOD(SetVoltage)(INT kV) override;
    STDMETHOD(SetCurrent)(INT uA) override;
    STDMETHOD(SetFocus)(INT mode) override;
    STDMETHOD(TurnXrayOn)() override;
    STDMETHOD(TurnXrayOff)() override;
    STDMETHOD(GetXrayStatus)(VARIANT* pResult) override;

    // CNC Control Methods
    STDMETHOD(InitializeCnc)() override;
    STDMETHOD(StartReference)(INT axisIndex) override;
    STDMETHOD(MoveAxis)(INT axisIndex, DOUBLE position) override;
    STDMETHOD(MoveAllAxes)(SAFEARRAY* positions) override;
    STDMETHOD(Stop)(INT axisIndex) override;
    STDMETHOD(EnableJoy)(INT axisIndex, BOOL enable) override;
    STDMETHOD(GetCncStatus)(VARIANT* pResult) override;
    STDMETHOD(GetAxesInfo)(VARIANT* pResult) override;
    STDMETHOD(GetPositions)(VARIANT* pResult) override;
};
