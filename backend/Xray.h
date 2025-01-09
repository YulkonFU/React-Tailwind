/****************************************************************************
 *
 * xray.h
 *
 * Projekt              :  Roentgensteuerung
 *
 * Definition des Abbildes von X-Ray fuer die hardwareunabhaengige
 * Schnittstelle.
 * Beachte: dieses Headerfile muss fuer alle Steuerungen gelten. d.h. Wenn 
 * eine neue Funktion bei einer Steuerung dazukommt, muss sie auch in allen 
 * anderen dazukommen
 *
 * moegliche Steuerungen:
 *      - Hamamatsu direkt ueber RS232 (alte Inspector-Version)
 *      - Hamamatsu ueber xslib.ocx, pmec4nt und CTU
 *      - phoenix Roehren ueber xslib.ocx, pmec4nt und XTU
 *
 * 19.12.2002 Alois Russ
 *
 * Aenderungsdatum, Was wurde von wem geaendert
 * 18.6.2003 AR - Fokusgroessen/Spotsizes umdefiniert, damit mit
 *                XS Nanofokus vergleichbar
 *                DEF: 0 ist immer der kleinste Fokus
 *                     groessere Indices bedeuten groessere Foki
 *                Beachte: XS = 0 = groesster und absteigend, aber mit 
 *                         dieser Def. kann keine Initialiserung auf kleinsten 
 *                         Fokus in Konstruktoren stattfinden!!
 *                MAX_SPOTSIZES erhoeht auf 5
 *                Deshalb jetzt nur noch ACT, SMALLEST und MAX_SPOTSIZES
 *                als defines der Rest ist erst nach xray.IsOpen() bekannt.
 ******************************************************************************/  

#pragma once
#define _AFXDLL
#include <winsock2.h>
#include <windows.h>
#include <afxwin.h>
#include <cassert>

#ifdef XRay2_EXPORTS
#define DLLEXPORT    __declspec(dllexport)
#else
#define DLLEXPORT    __declspec(dllimport)
#endif

// Errorcodes 1..46 (49)
// Anlagenspezifische Errorcodes ab 50 aufwaerts
enum { XR_OK                    = 0, 
       XRCOM_ERROR_INIT         = 1,    // Schnittstelle koennte nicht geoeffnet werden
       XRCOM_ERROR_READTIMEOUT  = 2,    // Verbindug schlecht/unterbrochen
       XRCOM_ERROR_OVERFLOW     = 4,    // SendePuffer zu klein
       XR_ERROR_FEATURE_NA      = 30,   // Feature/Funktion gibts nicht
       XR_ERROR_INV_STATE       = 31,   // Funktion kann in diesem Zustan nicht ausgefuehrt werden
       XR_ERROR_NOTOPEN         = 40,   // Versuch nicht geoeffnete Xray anzusprechen
       XR_ERROR_RAMPTIMEOUT     = 41,   // Timeout Hochlauf
       XR_ERROR_WARMUPTIMEOUT   = 42,   // Timeout Warmup
       XR_ERROR_AGING           = 43,   // XON, waehrend Aging
       XR_ERROR_INTERLOCK       = 44,   // XON beiInterlock auf
       XR_ERROR_NOREMOTE        = 45,   // XON/OF, Set ohne Remote ON (nur HAM)
       XR_ERROR_CONTROL         = 46,   // Fehler in der Steuerung (z.Z. nur HAM)
       XR_ERROR_IOVER           = 47,   // Ueberstrom (nur HAM)
       XR_ERROR_VOVER           = 48,   // Ueberspannung (nur HAM)
       XR_ERROR_INV_VALUE       = 49,   // Nicht zulaessiger Wert uebergeben (SetSpotsize)
     };


class DLLEXPORT CXray   // ab V2.0 Export ueber .DEF
{
public:
    CXray();                            // Constructor
    ~CXray();                           // Destructor

    // Funktionen

    BOOL Open(CWnd *pParentWnd = NULL); // Xray init.
    BOOL IsOpen();                      // Alle Daten da
    void Close();                       // TurnOff and Close
    BOOL ShouldBeOpen() const;

    BOOL IsCold()       const;          // Cold and not warming up
    BOOL IsWarmingUp()  const;          // Warmup laeuft
    BOOL StartWarmUp();                 // Hamamatsu: Wirkung, nur wenn COLD
    BOOL TurnOn();                      // einschalten
    BOOL TurnOn(UINT kV, UINT uA);
    BOOL TurnOff();                     // ausschalten
    BOOL SetkVuA(UINT kV = 0, UINT uA = 0); // 0 = gesetzten Wert nehmen
    BOOL SetkV(UINT kV); 
    BOOL SetuA(UINT uA); 
	BOOL WarmUpFinished();

    enum
    { 
      ACT_SPOTSIZE      = -1,           // fuer Abfragefunktion GetMaxuA()
      SPOTSIZE_SMALLEST = 0,            // kleinster Foki hat immer Index 0
      MAX_SPOTSIZES     = 5,            // Max. 5 Fokusgroessen werden unterstuetzt
    };

    UINT HasHowManySpotsizes() const;   // wieviele Spotgroessen werden unterstuetzt 1...
                                        // 0 bedeutet auch nur eine also nicht umschaltbar
    BOOL SetSpotsize(UINT SpotSize = SPOTSIZE_SMALLEST);  // Spotgroesse setzen
    UINT GetSpotsize() const;           // Spotgroesse abfragen
    LPTSTR GetSpotsizeName(UINT Spotsize, LPTSTR Name, int Len) const;
    BOOL SetSpotsizeNonInverted(UINT SpotSize);  // Spotgroesse setzen
    UINT GetSpotsizeNonInverted() const;           // Spotgroesse abfragen

    BOOL IsBeaming();                   // strahlt XR stabil
    BOOL IsBeamOn();                    // wird Strahlung erzeugt
    BOOL CheckRemoteON();               // Ist Remote geschaltet, sonst Fehler
    BOOL IsInterlockClosed() const;     // Ist Interlock zu
    BOOL CheckInterlockClosed();        // Ist Interlock zu, wenn nein Fehler setzen

    UINT GetBeamTime();                 // Strahlzeit gesamt der Roehre [min]

    UINT GetMaxuA(UINT kV, UINT SpotSize = ACT_SPOTSIZE) const;  // 0=Small, 1=Large, 2=Middle, -1=Act
    UINT GetMinuA(UINT kV, UINT SpotSize = ACT_SPOTSIZE) const;  // 0=Small, 1=Large, 2=Middle, -1=Act

    int     GetLastError() const;
    void    ClearLastError();
    LPCSTR  GetErrorText(int ErrorNr) const;


    // lesender Zugriff auf Daten
    // Diese Zustaende kann X-ray annehmen
    typedef enum XR_STATE_T     // <=XR_IS_OFF = ISOFF
    {                           // aktueller Zustand der Achse
      XR_NOT_INIT_YET   = 0,    // noch nicht init
      XR_NOT_READY      = 1,    // Nicht bereit, warum ???, Tuer offen
      XR_IS_COLD        = 2,    // Ist noch kalt
      XR_IS_OFF         = 3,    // Ist aus und bereit zum Strahlen
      XR_IS_AGING       = 4,    // faehrt warm
      XR_IS_ON          = 5,    // Strahlt (ob stabil ueber IsBeaming())
      XR_IS_RAMPINGUP   = 6,    // Hochlauf
      XR_IS_SHADOW      = 7,    // Strahl ist im Schattentarget
      XR_IS_CENTERING   = 8,    // Zentrierung laeuft
      XR_DOES_TARGETCHECK=9,    // Targetcheck laeuft
      XR_IS_PREWARNING  =10,    // Prewarning aktiv
      XR_IS_AFTERHEATING=11,    // Nachheizen aktiv (Yxlon)

    } XR_STATE;                 // aktueller Zustand Xray

    XR_STATE State()        const;  // IstZustand
    UINT MaxkV()            const;  // maximal moegliche KV aus INI!!
    UINT MinkV()            const;  // minimal meogliche KV aus INI
    //UINT MaxuA()            const;  // absolut maximal moeglicher Strom aus INI
    //long MaxRampUpTime()    const;  // max. zul. Hochlaufzeit [ms] aus INI
    //long MaxWarmUpTime()    const;  // amx. zul. Warmlaufzeit [ms] aus INI
    UINT kVSet()            const;  // akt. Sollwerte fuer kV
    UINT uASet()            const;  // und uA

    BOOL  HasBeamShutter()  const;
    DWORD ShutterWait()     const;
    BOOL  BeamShutter(BOOL bON, BOOL bForce = FALSE);   // Ret=TRUE=Schattentarget geaendert
    BOOL  IsBeamShuttered() const;                      // Ist Strahl im Schattentarget?

    BOOL RodAnodeInstalled() const; // Ist Stabanode installiert

    double GetRunupTime(UINT FromkV, UINT FromuA, UINT TokV, UINT TouA);

    // Zentrieren, FilamentAdjust, TargetCheck, nur wenn XS nicht bei CTU
    enum    // Werte fuer Centering.WhichOne
    {
      FIRST_CENTERING   = 1,    // erste Zentrierung
      SECOND_CENTERING  = 2,    // zweite Zentrierung
    };
    BOOL CanDoCentering(UINT WhichOne = FIRST_CENTERING) const; // Kann die Anlage Zentrieren?
    BOOL StartFullAutoCentering(UINT WhichOne = FIRST_CENTERING);
    BOOL StartAutoCentering(UINT WhichOne = FIRST_CENTERING);   // an aktuellem kV Zentrieren
    BOOL CanDoFilamentAdjust() const;               // Kann die Anlage FilamentAdjust?
    BOOL StartFilamentAdjust();
    BOOL CanDoTargetCheck() const;                  // Kann die Anlage TargetCheck machen?
    BOOL StartTargetCheck();
    enum    // ReturnWerte von LastTargetCheckResult()
    {
      TARGET_UNKNOWN    = -1,   // Zustand des Targets ist unbekannt
      TARGET_FAIL       =  0,   // letztes Targetcheck war FAIL
      TARGET_OK         =  1,   // letztes Targetcheck war OK
    };

    int LastTargetCheckResult() const;

    // Spezielle Funktionen: fuer XS kann die XR_MSG 1:1 durchgereicht werden.
    // Damit koennen alle Fkt. von XS verwendet werden.
    // Fuer andere Roehren macht die Fkt. z.Z. Nichts und liefert immer FALSE.
    #pragma pack(1)                 // Byte-Packing
    #define MAX_XRMSG_DATALEN 32
    typedef struct
    {
      BYTE Dst, Src, Signal, DataLen;   // Ziel, Quelle, Signal, DatenLaenge
      BYTE Data[MAX_XRMSG_DATALEN];     // 32 Byte Meldungsdaten

    } CXrMsg;                           // entspricht PX_MSG in pmecdef.h von RF
    #pragma pack()                      // wieder default Packing

    BOOL SendXrMsg(const CXrMsg &XrMsg) const;

    // Funktionen fuer "normalized Current" (korrigierter Strom) [nuA]
    // Es werden dieselben Funktionen wie fuer uA verwendet. Man kann aber einstellen,
    // ob die uebergebenen uA Werte als normalized angesehen werden sollen oder nicht.
    // Ebenso die zurueckgelieferten Stromwerte.
    // Die Voreinstellung wird aus der xray.ini.[NormalizedCurrent] gelesen so wie die
    // Korrekturfaktoren.
    // Es werden 2 Keile (Al, Cu) mit je 2 Stufen vermessen und daraus die Korrekturen berechnet.

    enum NUA_T      // Werte fuer Set/GetNUA.nWhat  (NUA=NormalizedUA)
    {
      NUA_WEDGE1        = 1,    // Stufekeile werden 1-based angesprochen: 1=Al
      NUA_WEDGE2        = 2,    // 2=Cu

      // weitere Def.
      NUA_WEDGECOUNT    = 2,    // 2 verschiedene Keile
      NUA_STEPCOUNT     = 2,    // 2 Stufen pro Keil

      NUA_ISENABLED     = 1,    // wird NormStrom erwartet/geliefert
      NUA_KVFACTOR      = 2,    // Korrekturwerte: uA = (kVFactor*kV+kVOffset) * nuA + uAOffset
      NUA_KVOFFSET      = 3,    // bzw. nuA = (uA-uAOffset)/(kVFactor*kV+kVOffset)
      NUA_UAOFFSET      = 4,    // Das Model gibt diesen Wert zwar nicht her, aber sicher ist sicher.
      NUA_WEDGEFACTOR1  = 5,    // Keilvariation von Keil 1 (Al) rel. zum Ref-Keil
      NUA_WEDGEFACTOR2  = 6,    // Keilvariation von Keil 2 (Cu) rel. zum Ref-Keil

      NUA_REFGRAYLEVEL1 = 11,   // Referenz-Grauwert der Stufe 1 (Al-Keil Stufe 1); nur Lesen!
      NUA_REFGRAYLEVEL2 = 12,   // Referenz-Grauwert der Stufe 2 (Al-Keil Stufe 2)
      NUA_REFGRAYLEVEL3 = 13,   // Referenz-Grauwert der Stufe 3 (Cu-Keil Stufe 1)
      NUA_REFGRAYLEVEL4 = 14,   // Referenz-Grauwert der Stufe 4 (Cu-Keil Stufe 2)

      NUA_REFKV1        = 21,   // Spannung fuer Keil 1 = Al (50kV)
      NUA_REFKV2        = 22,   // Spannung fuer Keil 2 = Cu (80kV)
      NUA_REFUA1        = 23,   // Anlagenstrom fuer Keil 1 = Al (100uA)
      NUA_REFUA2        = 24,   // Anlagenstrom fuer Keil 2 = Cu (100uA)

      // Funktionscodes
      NUA_SAVETOINI     = 50,   // Daten in xray.ini schreiben
      NUA_READFROMINI   = 51,   // Daten aus xray.ini lesen
      NUA_CALCCORRECTION= 52,   // SetNUA(NUA_CALCCORRECTION, double [4])
      // wie NUA_CALCCORRECTION, aber die Korrekturfaktoren, double [4])
      NUA_CALCCORRECTIONTEST=53,
    };


    BOOL    SetNUA(enum NUA_T nWhat, double dValue);
    __inline BOOL NUA_Enable()                      { return SetNUA(NUA_ISENABLED, TRUE);};
    __inline BOOL NUA_Disable()                     { return SetNUA(NUA_ISENABLED, FALSE);};
    __inline BOOL NUA_SetkVFactor(double dkVFactor) { return SetNUA(NUA_KVFACTOR, dkVFactor);};
    __inline BOOL NUA_SetkVOffset(double dkVOffset) { return SetNUA(NUA_KVOFFSET, dkVOffset);};
    __inline BOOL NUA_SetuAOffset(double duAOffset) { return SetNUA(NUA_UAOFFSET, duAOffset);};
    __inline BOOL NUA_SetWedgeFactor(UINT uWedge, double dFactor)   { ASSERT((uWedge-1)<=(NUA_WEDGEFACTOR2-NUA_WEDGEFACTOR1));
                                                                      return SetNUA((enum NUA_T)(NUA_WEDGEFACTOR1-1+uWedge), dFactor);};
    // RefGrayLevel nur lesen
    //__inline BOOL NUA_SetRefGrayLevel(UINT uStep, double dGrayLevel){ ASSERT((uStep-1)<=(NUA_REFGRAYLEVEL4-NUA_REFGRAYLEVEL1));
    //                                                                  return SetNUA((enum NUA_T)(NUA_REFGRAYLEVEL1-1+uStep), dGrayLevel);}
    __inline BOOL NUA_SetWedgekV(UINT uWedge, UINT ukV) { ASSERT((uWedge-1)<=(NUA_REFKV2-NUA_REFKV1));
                                                          return SetNUA((enum NUA_T)(NUA_REFKV1-1+uWedge), double(ukV));};
    __inline BOOL NUA_SetWedgeuA(UINT uWedge, UINT uA)  { ASSERT((uWedge-1)<=(NUA_REFUA2-NUA_REFUA1));
                                                          return SetNUA((enum NUA_T)(NUA_REFUA1-1+uWedge), double(uA));};
    __inline void NUA_SaveToInifile()   { SetNUA(NUA_SAVETOINI,   0);};
    __inline void NUA_ReadFromInifile() { SetNUA(NUA_READFROMINI, 0);};
    
    // Neuberechnung der Korrekturen. Die neuen Werte koennen anschliessend via GetNUA() erfragt werden.
    // pdStepGraylevel zeigt auf DoubleArray mit mind. 4 Werten
    __inline BOOL NUA_CalcCorrection(const double dStepGraylevel[4]){ double dTemp; *((ULONG*)&dTemp) = (ULONG)dStepGraylevel;
                                                                      ASSERT((double*)*((ULONG*)&dTemp) == dStepGraylevel);
                                                                      return SetNUA(NUA_CALCCORRECTION, dTemp);};
    // Neuberechnung der Korrekturen aber die neuen Werte werden nicht verwendet, sondern landen in 
    // dStepGraylevel[0]=kVFac und dStepGraylevel[1]=kVOffset und dStepGraylevel[2]=uAOffset
    __inline BOOL NUA_CalcCorrectionTest(double dStepGraylevel[4])  { double dTemp; *((ULONG*)&dTemp) = (ULONG)dStepGraylevel;
                                                                      ASSERT((double*)*((ULONG*)&dTemp) == dStepGraylevel);
                                                                      return SetNUA(NUA_CALCCORRECTIONTEST, dTemp);};

    double  GetNUA(enum NUA_T nWhat) const;
    __inline BOOL   NUA_IsEnabled() const               { return (BOOL)GetNUA(NUA_ISENABLED);};
    __inline double NUA_kVFactor() const                { return GetNUA(NUA_KVFACTOR);};
    __inline double NUA_kVOffset() const                { return GetNUA(NUA_KVOFFSET);};
    __inline double NUA_uAOffset() const                { return GetNUA(NUA_UAOFFSET);};
    __inline double NUA_WedgeFactor(UINT uWedge) const  { ASSERT((uWedge-1)<=(NUA_WEDGEFACTOR2-NUA_WEDGEFACTOR1));
                                                          return GetNUA((enum NUA_T)(NUA_WEDGEFACTOR1-1+uWedge));}
    __inline UINT   NUA_GetWedgekV(UINT uWedge)         { ASSERT((uWedge-1)<=(NUA_REFKV2-NUA_REFKV1));
                                                          return (UINT)GetNUA((enum NUA_T)(NUA_REFKV1-1+uWedge));};
    __inline UINT   NUA_GetWedgeuA(UINT uWedge)         { ASSERT((uWedge-1)<=(NUA_REFUA2-NUA_REFUA1));
                                                          return (UINT)GetNUA((enum NUA_T)(NUA_REFUA1-1+uWedge));};
    __inline double NUA_RefGrayLevel(UINT uStep) const  { ASSERT((uStep-1)<=(NUA_REFGRAYLEVEL4-NUA_REFGRAYLEVEL1));
                                                          return GetNUA((enum NUA_T)(NUA_REFGRAYLEVEL1-1+uStep));}

    // Familien/TubeOptions
    BOOL SetFamily(UINT uFamilyID);
    UINT GetFamily(UINT uWhat) const;
    enum    // Werte von GetFamily(UINT uWhat);
    {
      FAM_COUNT     = 0,    // Wieviele Familie/TubeOption gibts?
      FAM_ACTID     = 1,    // ID der aktiven Familie/TubeOption
      FAM_ID1       = 2,    // ID der 1-ten Famile
      FAM_ID2       = 3,    // ID der 2-ten Famile
      FAM_ID3       = 4,    // ...
      FAM_ID4       = 5,
      FAM_ID5       = 6,
      FAM_ID6       = 7,
      FAM_ID7       = 8,
      FAM_ID8       = 9,
      FAM_ID9       = 10,   // ID der 9-ten Famile
    };

    __inline UINT FamilyCount()       const { return GetFamily(FAM_COUNT);};
    __inline UINT ActFamilyID()       const { return GetFamily(FAM_ACTID);};
    __inline UINT FamilyID(UINT uIdx) const { ASSERT(uIdx <= FAM_ID9-FAM_ID1); return GetFamily(FAM_ID1+uIdx);};

    // UseCases
    UINT GetUseCase(UINT uWhat) const;
    enum    // Werte von GetUseCase(UINT uWhat);
    {
      UC_ENABLED            = 0,    // Gibt es UseCases?
      UC_XOFF               = 1,    // Wird xray ausgeschaltet wegen UseCases?
      UC_REMAININGXONP      = 2,    // Soviel Prozent Xon-Zeit sind noch uebrig.
      UC_REMAININGXONS      = 3,    // soviele Sekunden entspricht das [max = 65535 s]
      UC_NEEDEDRECOVERYTIMES= 4,    // soviele Sekunden Xoff sind bis 100% noetig 
      UC_MAXXONTIMES        = 5,    // hochgerechnete maximal moegliche Xon-Zeit auf 60 s aufgerundet
    };

    __inline BOOL HasUseCases()             const   { return  GetUseCase(UC_ENABLED);};
    __inline BOOL TurnOfOnUseCase()         const   { return  GetUseCase(UC_XOFF);};
    __inline UINT RemainingXONPercentage()  const   { return  GetUseCase(UC_REMAININGXONP);};
    __inline UINT RemainingXONSecondds()    const   { return  GetUseCase(UC_REMAININGXONS);};
    __inline UINT NeededRecoveryTimeS()     const   { return  GetUseCase(UC_NEEDEDRECOVERYTIMES);};
    __inline UINT MaxXONTimeS()             const   { return  GetUseCase(UC_MAXXONTIMES);};
};

#undef DLLEXPORT
