#pragma once

#include <windows.h>
#include <WebView2.h>
#include <wrl.h>
#include <wil/com.h>
#include <string>
#include <mutex>    
#include <thread>   
#include "Xray.h"
#include "Cnc.h"
#include "DigGrabberDll.h"

class DeviceHandler : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IDispatch> {

private:
    // Device instances
    CXray* m_xray;
    CCnc* m_cnc;
    CDigGrabber* m_dig;

    // DLL handles  
    HMODULE m_hXrayDll;
    HMODULE m_hCncDll;
    HMODULE m_hDigDll;

    // Axis information
    double* positions;
    UINT axisCount;

    // Helper methods
    bool LoadXrayDll();
    bool LoadCncDll();
    bool LoadDigDll();
    void UnloadXrayDll();
    void UnloadCncDll();
    std::wstring ConvertToWString(const std::string& str);

    // 添加线程相关成员
    std::thread m_monitorThread;
    bool m_isMonitoring;
    std::mutex m_mutex;
    wil::com_ptr<ICoreWebView2> m_webView;

    // 监控线程函数
    void MonitorThreadFunc();

    // 图像缓冲区
    struct SharedImageBuffer {
        std::unique_ptr<WORD[]> buffer;
        SIZE_T width;
        SIZE_T height;
        HANDLE mappingHandle;
        LPVOID mappedAddress;
    };

    SharedImageBuffer m_imageBuffer;
    bool m_isLiveMode;
    static void CALLBACK EndFrameCallback(CDigGrabber& digGrabber);

    // 创建共享内存
    bool CreateSharedBuffer(SIZE_T width, SIZE_T height);
    void CleanupSharedBuffer();

public:
    DeviceHandler();
    ~DeviceHandler();

    // 线程控制方法
    void StartMonitoring(ICoreWebView2* webView);
    void StopMonitoring();

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

    // Detector Control Methods
    HRESULT InitializeDetector();
    HRESULT StartLive();
    HRESULT StopLive();
    HRESULT SetGain(UINT gainStep);
    HRESULT SetFPS(UINT timing);
    HRESULT GetDetectorStatus(VARIANT* pResult);

    // 添加共享内存读取方法
    HRESULT ReadSharedMemory(BSTR name, UINT size, VARIANT* pResult);
};