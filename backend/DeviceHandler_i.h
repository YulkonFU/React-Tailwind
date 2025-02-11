

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 11:14:07 2038
 */
/* Compiler settings for .\DeviceHandler.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __DeviceHandler_i_h__
#define __DeviceHandler_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __IDeviceHandler_FWD_DEFINED__
#define __IDeviceHandler_FWD_DEFINED__
typedef interface IDeviceHandler IDeviceHandler;

#endif 	/* __IDeviceHandler_FWD_DEFINED__ */


#ifndef __DeviceHandler_FWD_DEFINED__
#define __DeviceHandler_FWD_DEFINED__

#ifdef __cplusplus
typedef class DeviceHandler DeviceHandler;
#else
typedef struct DeviceHandler DeviceHandler;
#endif /* __cplusplus */

#endif 	/* __DeviceHandler_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __DeviceHandlerLib_LIBRARY_DEFINED__
#define __DeviceHandlerLib_LIBRARY_DEFINED__

/* library DeviceHandlerLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_DeviceHandlerLib;

#ifndef __IDeviceHandler_INTERFACE_DEFINED__
#define __IDeviceHandler_INTERFACE_DEFINED__

/* interface IDeviceHandler */
/* [local][object][uuid] */ 


EXTERN_C const IID IID_IDeviceHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C7AD56F-6D1D-41C6-AFC9-669688033B94")
    IDeviceHandler : public IUnknown
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE InitializeXray( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE StartWarmup( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetVoltage( 
            /* [in] */ INT kV) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetCurrent( 
            /* [in] */ INT uA) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE TurnXrayOn( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE TurnXrayOff( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetFocus( 
            /* [in] */ INT mode) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetXrayStatus( 
            /* [retval][out] */ VARIANT *pResult) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE InitializeCnc( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE StartReference( 
            /* [in] */ INT axisIndex) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE MoveAxis( 
            /* [in] */ INT axisIndex,
            /* [in] */ DOUBLE position) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE MoveAllAxes( 
            /* [in] */ SAFEARRAY * positions) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Stop( 
            /* [in] */ INT axisIndex) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE EnableJoy( 
            /* [in] */ INT axisIndex,
            /* [in] */ BOOL enable) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetCncStatus( 
            /* [retval][out] */ VARIANT *pResult) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetAxesInfo( 
            /* [retval][out] */ VARIANT *pResult) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetPositions( 
            /* [retval][out] */ VARIANT *pResult) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IDeviceHandlerVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDeviceHandler * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDeviceHandler * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDeviceHandler * This);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, InitializeXray)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *InitializeXray )( 
            IDeviceHandler * This);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, StartWarmup)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *StartWarmup )( 
            IDeviceHandler * This);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, SetVoltage)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetVoltage )( 
            IDeviceHandler * This,
            /* [in] */ INT kV);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, SetCurrent)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetCurrent )( 
            IDeviceHandler * This,
            /* [in] */ INT uA);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, TurnXrayOn)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *TurnXrayOn )( 
            IDeviceHandler * This);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, TurnXrayOff)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *TurnXrayOff )( 
            IDeviceHandler * This);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, SetFocus)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetFocus )( 
            IDeviceHandler * This,
            /* [in] */ INT mode);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, GetXrayStatus)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetXrayStatus )( 
            IDeviceHandler * This,
            /* [retval][out] */ VARIANT *pResult);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, InitializeCnc)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *InitializeCnc )( 
            IDeviceHandler * This);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, StartReference)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *StartReference )( 
            IDeviceHandler * This,
            /* [in] */ INT axisIndex);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, MoveAxis)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *MoveAxis )( 
            IDeviceHandler * This,
            /* [in] */ INT axisIndex,
            /* [in] */ DOUBLE position);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, MoveAllAxes)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *MoveAllAxes )( 
            IDeviceHandler * This,
            /* [in] */ SAFEARRAY * positions);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, Stop)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IDeviceHandler * This,
            /* [in] */ INT axisIndex);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, EnableJoy)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *EnableJoy )( 
            IDeviceHandler * This,
            /* [in] */ INT axisIndex,
            /* [in] */ BOOL enable);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, GetCncStatus)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetCncStatus )( 
            IDeviceHandler * This,
            /* [retval][out] */ VARIANT *pResult);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, GetAxesInfo)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetAxesInfo )( 
            IDeviceHandler * This,
            /* [retval][out] */ VARIANT *pResult);
        
        DECLSPEC_XFGVIRT(IDeviceHandler, GetPositions)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetPositions )( 
            IDeviceHandler * This,
            /* [retval][out] */ VARIANT *pResult);
        
        END_INTERFACE
    } IDeviceHandlerVtbl;

    interface IDeviceHandler
    {
        CONST_VTBL struct IDeviceHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDeviceHandler_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDeviceHandler_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDeviceHandler_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDeviceHandler_InitializeXray(This)	\
    ( (This)->lpVtbl -> InitializeXray(This) ) 

#define IDeviceHandler_StartWarmup(This)	\
    ( (This)->lpVtbl -> StartWarmup(This) ) 

#define IDeviceHandler_SetVoltage(This,kV)	\
    ( (This)->lpVtbl -> SetVoltage(This,kV) ) 

#define IDeviceHandler_SetCurrent(This,uA)	\
    ( (This)->lpVtbl -> SetCurrent(This,uA) ) 

#define IDeviceHandler_TurnXrayOn(This)	\
    ( (This)->lpVtbl -> TurnXrayOn(This) ) 

#define IDeviceHandler_TurnXrayOff(This)	\
    ( (This)->lpVtbl -> TurnXrayOff(This) ) 

#define IDeviceHandler_SetFocus(This,mode)	\
    ( (This)->lpVtbl -> SetFocus(This,mode) ) 

#define IDeviceHandler_GetXrayStatus(This,pResult)	\
    ( (This)->lpVtbl -> GetXrayStatus(This,pResult) ) 

#define IDeviceHandler_InitializeCnc(This)	\
    ( (This)->lpVtbl -> InitializeCnc(This) ) 

#define IDeviceHandler_StartReference(This,axisIndex)	\
    ( (This)->lpVtbl -> StartReference(This,axisIndex) ) 

#define IDeviceHandler_MoveAxis(This,axisIndex,position)	\
    ( (This)->lpVtbl -> MoveAxis(This,axisIndex,position) ) 

#define IDeviceHandler_MoveAllAxes(This,positions)	\
    ( (This)->lpVtbl -> MoveAllAxes(This,positions) ) 

#define IDeviceHandler_Stop(This,axisIndex)	\
    ( (This)->lpVtbl -> Stop(This,axisIndex) ) 

#define IDeviceHandler_EnableJoy(This,axisIndex,enable)	\
    ( (This)->lpVtbl -> EnableJoy(This,axisIndex,enable) ) 

#define IDeviceHandler_GetCncStatus(This,pResult)	\
    ( (This)->lpVtbl -> GetCncStatus(This,pResult) ) 

#define IDeviceHandler_GetAxesInfo(This,pResult)	\
    ( (This)->lpVtbl -> GetAxesInfo(This,pResult) ) 

#define IDeviceHandler_GetPositions(This,pResult)	\
    ( (This)->lpVtbl -> GetPositions(This,pResult) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDeviceHandler_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_DeviceHandler;

#ifdef __cplusplus

class DECLSPEC_UUID("7C48D2A7-83E1-4757-BC38-69F830504CA6")
DeviceHandler;
#endif
#endif /* __DeviceHandlerLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


