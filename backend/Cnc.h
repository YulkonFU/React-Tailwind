/******************************************************************
 *
 * cnc.h
 *
 * Projekt              :  EcoStep CNC
 *
 * Definition des Abbildes der CNC fuer die hardwareunabhaengige
 * Schnittstelle.
 * Hier fuer die EcoStep von JAT ueber Serielle Schnittstelle
 *
 * Alle Positionsangaben erfolgen in [mm] bzw. [Grad] als double
 *
 * 22.7.99 Alois Russ
 *
 * Aenderungsdatum, Was wurde von wem geaendert
 * 9.12.04 Reinhard Friedemann:
 *         Funktion int Zusynchron(int code,...):
 *                  (Name alphabetisch hinter 'Zustand', damit DLL laufzeitkompatibel)
 *         Parameter code siehe CNC_SYNFKT...
 *         alle weitere Params und der Returnwert abhaengig von code
 *
 ********************************************************************/  

#ifndef _CNC_H_
#define _CNC_H_

/*
 * Indizes der festen Achsen (Normal) fuer PCBA/Package-Analyser
 */

#if (!defined _DUMMY) && (defined _DEBUG)   // nicht Dummy aber Debug -> Schreibtisch
#pragma message (__FILE__"("__STRLINE__") : warning **** Using Debug Axis Indices")

#define X_AXIS  0                           // Ich habe nur 2 Achsen da
#define Y_AXIS  1
#define Z_AXIS  2

#else  // Normal oder _DUMMY mind. 3 Achsen X/Y/Z

#define X_AXIS  0           // Probe LinksRechts
#define Y_AXIS  1           // Probe HochRunter
#define Z_AXIS  2           // Probe VorZurueck

#endif

// Beachte: 6 muss bleiben, da Programme vorheriger Versionen mit 6 Achsen
// gespeichert wurden.

#define CNC_MAX_AXIS    6   // Maximale Anzahl von steuerbaren Achsen                            

#define AXIS_VALID(ax)  (unsigned(ax) < CNC_MAX_AXIS)

#define ALL_AXIS        UINT(-1)

// statt diesen Werten die maximal moeglichen setzen

#define AXIS_VMAX       (-1)        // SetVeloc
#define AXIS_VSET       (-2)        // Sollgeschw. bei GetMovingTimeEx()
#define AXIS_ACCMAX     (-1)        // SetAccDecc
#define AXIS_DECCMAX    (-1)        // SetAccDecc

/*
 * Werte fuer MoveBy.what
 */

#define MOVE_POS        0x00
#define MOVE_NEG        0x01
#define MOVE_SMALLSTEP  0x00
#define MOVE_LARGESTEP  0x02

// Fuer RotAchsen, Positionsangabe erfolgt in Grad statt in mrad
#ifndef PI
  #define PI 3.1415926535897932384626433832795
#endif

/*
 * Klasse CAxis: nur Zugriff auf die public Daten einer Achse
 * Daten aus INI sind public, sonst nur Zugriff ueber CNC
 */
#pragma once

#include <windows.h>
#include <afxwin.h>
#include <cassert>
class CAxis
{
public: 
    virtual void   SetSlavePos(double SlaveAxisPos) = 0;        // Axis-Koordinaten
    virtual void   SlaveDiscouple(BOOL bDiscouple=TRUE) = 0;

    // alte Funktion, bei der INI immer geschrieben wurde. s. ChangeRefOffsetByEx()
    virtual void   ChangeRefOffsetBy(double dOffset) = 0;
    virtual BOOL   ChangeLimitsBy(double dMinOffset, double dMaxOffset) = 0; // SW+- neu setzen [mm]
    virtual void   SetAbsoluteMinMaxPos(double dAbsolutMinPos, double dAbsolutMaxPos) = 0; // Axis-Koordinaten

    virtual       int    GetAxisID()    const = 0;
    virtual const char*  Name()         const = 0;

    // Zugriff auf Daten aus cnc.ini: [Axis].Type(), MasterAxis()
    enum
    {
      AXIS_UNUSED               = 0x000,// Achse wird nicht gebraucht
      AXIS_EXIST                = 0x001,// Soll die Achse existieren
      AXIS_IS_ROT               = 0x002,// Rotations- oder Kippachse Einheit [mrad] statt [mm]
      AXIS_IGNORE_END           = 0x004,// ignore SW-End, nur Zeahlerueberlauf
      AXIS_ONLY_TWO_POS         = 0x008,// nur Min und Max-Pos anfahren
      AXIS_NOTOPTIONAL          = 0x010,// Achse ist nicht optional (bei Achse 1..3 immer 1)
                                        // -> existiert auch dann, wenn beide Endschalter da
      AXIS_DISABLED_ON_REACHED  = 0x020,// Achse Disablen, wenn TargetReached
      AXIS_STARTTO_POSAFTERREF  = 0x040,// Achse auf PosAfterRef starten, bei Interlock zu

      AXIS_CHECK_POSHWSWITCH    = 0x100,// Fehler setzen bei positivem Endschalter
      AXIS_CHECK_NEGHWSWITCH    = 0x200,// Fehler setzen bei negativem Endschalter

      HAS_NO_MASTER       = -1    // Achse ist keine Slave-Achse, sonst AxIdx 0...
    };

    virtual const DWORD  &Type()        const = 0;
    virtual const int    &MasterAxis()  const = 0;

    __inline BOOL ShouldExist() const   { return Type() & AXIS_EXIST;};
    __inline BOOL IsRot()       const   { return Type() & AXIS_IS_ROT;};
    __inline BOOL IgnoreEnd()   const   { return Type() & AXIS_IGNORE_END;};
    __inline BOOL IsTwoPosAxis()const   { return Type() & AXIS_ONLY_TWO_POS;};
    __inline BOOL IsOptional()  const   { return !(Type() & AXIS_NOTOPTIONAL);};
    __inline BOOL IsDisabledOnTargetReached()   const   
                                        { return Type() & AXIS_DISABLED_ON_REACHED;};
    __inline BOOL DoPosLimitCheck()const{ return Type() & AXIS_CHECK_POSHWSWITCH;};
    __inline BOOL DoNegLimitCheck()const{ return Type() & AXIS_CHECK_NEGHWSWITCH;};

    __inline BOOL IsSlaveAxis() const   { return MasterAxis() > HAS_NO_MASTER;};

    virtual const double &MinPos()      const = 0;      // Axis-Koordinaten (ohne 2D-Comp)
    virtual const double &MaxPos()      const = 0;      // Axis-Koordinaten (ohne 2D-Comp)
    virtual const double &AccMax()      const = 0;
    virtual const double &DeccMax()     const = 0;
    virtual const double &VMax()        const = 0;
    virtual const double &IncrProMM()   const = 0;
    virtual const double &PosAfterRef() const = 0;      // Axis-Koordinaten (ohne 2D-Comp)
    virtual const double &DeltaPos()    const = 0;
    
    virtual const double &AbsolutMinPos() const = 0;    // Axis-Koordinaten (ohne 2D-Comp)
    virtual const double &AbsolutMaxPos() const = 0;    // Axis-Koordinaten (ohne 2D-Comp)

    virtual       BOOL   Din(int Nr)    const = 0;
    enum // Werte fuer Nr in Din(Nr)
    {
      DIN1            =  1,
      DIN2            =  2,
      DIN3            =  3,
      DIN4            =  4,
      INTERLOCKCLOSED =  5,
      POSHWSWITCH     =  6,  
      NEGHWSWITCH     =  7,  
      REFSWITCH       =  8,  
      RESET           =  9,  
      ENABLE          = 10,  
    };

    /*
     * notwendige Geschwindigkeit, um in T sec S mm weit zu fahren mit der aktuellen Beschl. und
     * Verzoegerung, begrenzt auf MIN_VELOC < V < VMax [mm/s]
     */

    virtual double GetVelocity(double S, double T) const = 0;

    // neue Funktion, bei der INI optional geschrieben wird
    enum    // Werte von ChangeRefOffsetByEx.dwFlags
    {
      CRO_FLAG_NONE         = 0x00,
      CRO_FLAG_PERMANENT    = 0x01, // Save in CNC.INI
      CRO_FLAG_CHANGE_MINMAX= 0x02, // Verfahrweg an geaenderten RefOffset anpassen

      CRO_FLAGS_OLD         = -1,   // Flags fuer altes ChangeRefOffsetBy z.Z. =ALL
    };
    virtual void   ChangeRefOffsetByEx(double dOffset, DWORD dwFlags) = 0;

    // Added: 8.1.2009
    virtual const double &AccSet()      const = 0;
    virtual const double &DeccSet()     const = 0;
    virtual const double &VSet()        const = 0;

    // Externes Messsystem:
    //   IncrProMM() liefert Wert fuer Umrechnung der Position          : externes Messsystem 
    //   IncrProMMVel() fuer Umrechnung von geschwindigkeit und Acc/Decc: Motorencoder
    //   Ohne externes Messsystem sind beide Werte immer identisch.
    virtual __inline const double &IncrProMMVel()   const = 0;
    __inline const double &IncrProMMPos()           const { return IncrProMM();};   // neuer Name

}; /* end class CAxis */

typedef enum
{                               // aktueller Zustand der CNC
  CNC_NOT_INIT_YET   = 0,       // noch nicht init
  CNC_DRIVING_REF    = 1,       // CNC faehrt Referenz
  CNC_WAS_STARTED    = 2,       // faehrt wegen Fahrbefehl
  CNC_STAND_STILL    = 3,       // faehrt nicht wegen Fahrbefehl
  CNC_NOT_READY      = 4,       // nicht bereit, Fehler auf Achse

} CNCZustand;                   // Zustand der CNC

class CCnc                      
{
public:
    CCnc();
    ~CCnc();
    
    BOOL Open();                                        // FALSE=Com konnte nicht geoeffnet werden
    BOOL IsOpen();                                      // Alle Daten aller erwarteter Achsen da
    void Close();
    void Reset(UINT AxisIdx = ALL_AXIS);                // Fehlerreset, wie TuerAufZu

    BOOL IsInterlockClosed();   // sind alle Tuern zu

    BOOL   StartReference(UINT AxisIdx = ALL_AXIS);     // Referenzfahrt starten
    void   ResetReference(UINT AxisIdx = ALL_AXIS);     // Ref zurucksetzen

    BOOL   StartTo(UINT AxisIdx, double Pos);           // losfahren eine Achse
    BOOL   StartTo(double Pos[CNC_MAX_AXIS]);           // losfahren alle Achsen

    BOOL   PositionReached(UINT AxisIdx = ALL_AXIS);    // Position erreicht ?
    double GetLastPosition(UINT AxisIdx);               // letzte gelesene Position
    double GetSetPosition(UINT AxisIdx);                // letzte gesetze Position
    double GetCheckedLastPosition(UINT AxisIdx);        // letzte gelesene Position auf SW+- begrenzt
    void   GetAllLastPositions(double Pos[CNC_MAX_AXIS]) const;
    void   GetAllSetPositions(double Pos[CNC_MAX_AXIS]) const;
    BOOL   CheckPosition(UINT AxisIdx, double &Pos);    // auf SW+- begrenzen
    BOOL   CheckAllPositions(double Pos[CNC_MAX_AXIS]);

    // steht die CNC auf der angegebenen Position auf +-Delta genau
    BOOL IsPositionReached(double Pos[CNC_MAX_AXIS]) const;
    // Abweichung < DeltaPos?
    BOOL IsSamePosition(double Pos1[CNC_MAX_AXIS], double Pos2[CNC_MAX_AXIS]) const;
    BOOL IsSamePosition(UINT AxisIdx, double Pos1, double Pos2) const;

    double GetMinPos(UINT AxisIdx, const double Pos[CNC_MAX_AXIS]) const;
    double GetMaxPos(UINT AxisIdx, const double Pos[CNC_MAX_AXIS]) const;

    void Stop(UINT AxisIdx = ALL_AXIS);         // Stop mit Rampe, einzeln mgl.
    BOOL Continue(UINT AxisIdx = ALL_AXIS);     // Weiterfahren auf SollPos, Nachtriggern
    void Exit(UINT AxisIdx = ALL_AXIS);         // Anhalten und Joyenable

    void DisableJoy(UINT AxisIdx = ALL_AXIS);   // Joysticks disable
    void EnableJoy(UINT AxisIdx = ALL_AXIS);    // Joysticks enable
    BOOL AllJoyDisabled() const;                // Sind alle Joysticks disabled?

    // Jede Achse hat max. 2 dig. Ausgaenge, die aber nicht alle vom PC aus gesetzt werden koennen.
    // wenn sie z.B. für Zeilentrigger genutzt werden. Diese im Regler gesetzten Ausgaenge werden durch 
    // SetDigOut/GetDigOut nicht abgedeckt. SetDigOut() liefert CNC_ERROR_INVARG und GetDigOut() liefert irgendwas.
    // Bis V5.2 war Dout2 immer IsMoving. Ab V5.3 kann Dout2 auch setzbar sein.
    // Ob dieser vom PC aus setzbar ist, muss ueber IsDigOutChangable() geprueft werden.
    BOOL SetDigOut(UINT AxisIdx, UINT uNr, BOOL bSet);  // Dig-Ausgang nr auf Achse n setzen
    BOOL GetDigOut(UINT AxisIdx, UINT uNr);             // setzbaren Dig-Ausgang nr auf Achse n lesen

    enum {QUERY_STATE     = 0x00000000,
          QUERY_CHANGABLE = 0x00010000,};
    __inline BOOL IsDigOutSet      (UINT AxisIdx, UINT uNr) { return GetDigOut(AxisIdx | QUERY_STATE,     uNr);};
    __inline BOOL IsDigOutChangable(UINT AxisIdx, UINT uNr) { return GetDigOut(AxisIdx | QUERY_CHANGABLE, uNr);};
    
    void MovingSignalOn(BOOL bON = TRUE);       // Movingsignal setzen (auf Achse[0])

    void MoveBy(UINT AxisIdx, DWORD dwWhat);    // kleinen/grossen Schritt machen

    BOOL __cdecl SetVeloc(UINT count, UINT AxisIdx, double veloc, ...);
    BOOL __cdecl SetAccDecc(UINT count, UINT AxisIdx, double acc, double decc, ...);

    int     GetLastError();
    LPCSTR  GetErrorText(int ErrorNr);
    void    ClearLastError();

    // Errorcodes: CncCOM 1..7 (9)
    enum { COM_OK                    =  0, 
           COM_INITERROR             =  1,  // Schnittstelle nicht vorhanden oder schon belegt
           COM_ERROR_READTIMEOUT     =  2,  // timeout read from comm
           COM_ERROR_WRITETIMEOUT    =  3,  // timeout write to comm
           COM_ERROR_OVERFLOW        =  4,  // Sendepuffer zu klein bei SendCommand
           COM_ERROR_NOTOPEN         =  5,  // Versuch nicht geoeffnete COM anzusprechen
           COM_ERROR_RETRY           =  6,  // Eine SendeWiederholung ist erfordelich
           COM_ERROR_CHKS            =  7,  // Checksumme falsch (schlechte Verbindung)
    };
    
    // Errorcodes: Axis 10..20 (29)
    enum { AXIS_OK                   = 0, 
           AXIS_INITERROR            = 10, 
           AXIS_ERROR_NOTOPEN        = 11,  // Versuch nicht geoeffnete Achse anzusprechen
           AXIS_ERROR_NOREF          = 12,  // es wurde nicht Referenzgefahren
           AXIS_ERROR_INVPOS         = 13,  // ungueltige Position Step > 1^23
           AXIS_ERROR_POSLIMIT       = 14,  // pos SW-Endschalter
           AXIS_ERROR_NEGLIMIT       = 15,  // neg Sw-Endschalter
           AXIS_ERROR_CONTROLLER     = 16,  // Fehler im Achs-Controller
           AXIS_ERROR_TEMP           = 17,  // Temperatur ausserhalb zul. Bereich
           AXIS_ERROR_POSTIMEOUT     = 18,  // Timeout Move Einzelachse
           AXIS_ERROR_POSHWSWITCH    = 19,  // pos. HW-Endschalter betaetigt
           AXIS_ERROR_NEGHWSWITCH    = 20,  // neg. HW-Endschalter betaetigt
         };
    
    // Errorcodes: Cnc 30..36 (49)
    enum { CNC_OK                   = 0, 
           CNC_ERROR_NOTOPEN        = 30,   // Versuch nicht geoeffnete Cnc anzusprechen
           CNC_ERROR_NOREF          = 31,   // es wurde nicht Referenzgefahren
           CNC_ERROR_INVARG         = 32,   // ungueltiges Argument 
           CNC_ERROR_POSTIMEOUT     = 33,   // Timeout Move
           CNC_ERROR_REFTIMEOUT     = 34,   // Timeout Ref
           CNC_ERROR_INTERLOCK      = 35,   // Tuer ist offen
           CNC_ERROR_OPTAXIS        = 36,   // Optionale Achse ist nicht die letzte
           CNC_ERROR_SOFTSLAVE      = 37,   // Es gibt softcoupled Slave aber kein CAN/MMTimer
         };
    
    const UINT &NrAxis()            const;
    const UINT &ExpectedAxis()      const;
    const UINT  Zustand()           const;
    const char* AxisName(UINT Idx)  const;
    BOOL IsMoving()                 const;
    BOOL IsDrivingRef()             const;
    BOOL IsRefDone();

    // Beachte: mit diesen Operatoren hat jeder vollen Zugriff auf die Achse, wie CNC
    CAxis &operator[](int nIdx);                    // Zugriff ueber Achsindex, falls Fehler Achse0
    const CAxis &operator[](int nIdx) const { return (*this)[nIdx]; }   // fuer Const-Funktionen
    CAxis &operator[](const char *AxisName);        // Zugriff ueber Achsnamen, falls Fehler Achse0
    const CAxis &operator[](const char* AxisName) const { return (*this)[AxisName]; }
    UINT  GetAxisIndex(const char *AxisName) const; // 0.. CNC_MAX_AXIS-1, CNC_MAX_AXIS, falls Fehler
    BOOL  AxisExist(UINT AxIndex)           const;  // {return (AxIndex < m_NrAxis);}
    BOOL  AxisExist(const char *AxisName)   const;  // {return AxisExist(GetAxisIndex(AxisName));}

    void SyncAllSlaves();   // Alle SlaveAchsen mit ihren Mastern syncronisieren

    // Zugriff auf Kollisionsraum-Daten
    const double &DetectorWidth()                   const;
    const double &SideSupportDistanceX()            const;
    const double &FokusSideSupportDistanceZ()       const;
    const double &FokusSideSupportDistanceY()       const;
    const double &FokusDetectorDistanceZ()          const;
    const double &SideSupportTableBottomDistanceZ() const;
    const double &FokusTableBackDistanceY()         const;
    const double &FokusIIHolderDistanceR()          const;

    const UINT &CycleTime()         const;
    const long &WaitingTime()       const;
    const BOOL &IsRotTiltPlugged()  const;
    const BOOL &IsCTAxisPlugged()   const;

    // Verfahrzeit in sec bei max. Geschwindigkeit bei aktueller Acc/Decc.
    double GetMovingTime(double FromPos[CNC_MAX_AXIS], double ToPos[CNC_MAX_AXIS]) const;
    // Verfahrzeit in sec bei max. oder Soll-Geschwindigkeit bei aktueller Acc/Decc.
    double GetMovingTimeEx(double FromPos[CNC_MAX_AXIS], double ToPos[CNC_MAX_AXIS], int nWhichVeloc = AXIS_VSET) const;

	BOOL   SetTemperature(double Celsius, UINT AxisIdx=ALL_AXIS);
    UINT   GetSensorCount() const;                  // #TemperaturSensoren
    double GetTemperature(UINT uIdx =-1) const;     // -1=Mittelwert aller Sensoren
                                                    // Ret=-276.0 bei Fehler
    UINT    NrCmdsInSendBuf() const;

    // Erweiterung 9.12.04 RF, DLL bleibt laufzeitkompatibel auch mit alten Exe'n
    int __cdecl Zusynchron(int code, ...);

    BOOL IsNDTETO() const;                          // Ist die Anlage eine NDT-ETO Anlage Xargos
    __inline BOOL IsXargos() const { return IsNDTETO();};   // ETO heisst jetzt x|argos.

    // Funktionen fuer HWCollisionDetection: Lichtschranke
    BOOL    IsHWCollDetInstalled();                 // Gibt es eine HWCollDet?
    void    DisableHWCollDet(BOOL bDisable = TRUE); // HWCollDet an/aus
    BOOL    IsHWCollDetDisabled();                  // Ist HWCollDet aus?
    BOOL    IsHWCollDetected();                     // Wurde HWColl erkannt?

    // Auf Anlagen mit Autoload Ausgang Out1 oder Out2 auf X, Y, Z setzen, wenn geg. Pos erreicht
    // Return FALSE, wenn nicht offen, oder nicht gewuenscht
    // Wenn Pos==NULL, dann nur Abfrage, ob SetLoadPosition() unterstuetzt wird.
    BOOL SetLoadPosition(const double Pos[CNC_MAX_AXIS]);

    // Funktionen fuer 1D-Kompensation
    // Return = Anzahl Achsen, die 1D-Comp enabled haben
    long Enable1DCompensation(BOOL bEnable = TRUE, UINT uAxis = ALL_AXIS);     // Ein/Aus
    __inline void Disable1DCompensation(UINT uAxis = ALL_AXIS)
        { (void)Enable1DCompensation(FALSE, uAxis);};                   // return ist immer 0
    // Return = Anzahl Achsen, die auf die Anfrage TRUE liefern.
    long Query1DCompensation(UINT uWhat, UINT uAxis = ALL_AXIS) const;  // Abfrage
    enum // Werte fuer Query1DCompensation.uWhat
    {
      Q1D_CAN_BE_ENABLED    = 0,                                        // Kann kompensiert werden
      Q1D_IS_ENABLED        = 1,                                        // Wird z.Z. kompensiert

      Q1D_READ_COMP         = 5,                                        // Comp-Daten (neu) einlesen
    };
    __inline BOOL Can1DCompensationBeEnabled(UINT uAxis = ALL_AXIS) const
                                        { return Query1DCompensation(Q1D_CAN_BE_ENABLED, uAxis);};
    __inline BOOL Is1DCompensationEnabled(UINT uAxis = ALL_AXIS)    const
                                        { return Query1DCompensation(Q1D_IS_ENABLED, uAxis);};
    __inline BOOL Read1DComp(UINT uAxis = ALL_AXIS) 
                                        { return Query1DCompensation(Q1D_READ_COMP, uAxis);};

    // Funktionen fuer 2D-Kompensation (z.Z. nur X/Y)
    BOOL Enable2DCompensation(BOOL bEnable, UINT uAxis1, UINT uAxis2);      // Ein/Aus, TRUE=Enabled
    __inline void Disable2DCompensation(UINT uAxis1, UINT uAxis2)
        { (void)Enable2DCompensation(FALSE, uAxis1, uAxis2);};              // return ist immer 0
    BOOL Query2DCompensation(UINT uWhat, UINT uAxis1, UINT uAxis2) const;   // Abfrage
    enum // Werte fuer Query2DCompensation.uWhat
    {
      Q2D_CAN_BE_ENABLED    = 0,                                // Kann kompensiert werden
      Q2D_IS_ENABLED        = 1,                                // Wird z.Z. kompensiert

      Q2D_READ_COMP         = 5,                                // Comp-Daten (neu) einlesen
    };
    __inline BOOL Can2DCompensationBeEnabled(UINT uAxis1, UINT uAxis2) const
                                        { return Query2DCompensation(Q2D_CAN_BE_ENABLED, uAxis1, uAxis2);};
    __inline BOOL Is2DCompensationEnabled(UINT uAxis1, UINT uAxis2) const
                                        { return Query2DCompensation(Q2D_IS_ENABLED, uAxis1, uAxis2);};
    __inline BOOL Read2DComp(UINT uAxis1=ALL_AXIS, UINT uAxis2=ALL_AXIS) 
                                        { return Query2DCompensation(Q2D_READ_COMP, uAxis1, uAxis2);};

    // Funktionen fuer X(Oh)-Kompensation
    BOOL EnableXofOCompensation(BOOL bEnable = TRUE);               // Ein/Aus: Return TRUE = Enabled
    __inline void DisableXofOCompensation()                         // Aus
        { (void)EnableXofOCompensation(FALSE);};                    // return ist immer 0
    BOOL QueryXofOCompensation(UINT uWhat) const;                   // Abfrage: Return TRUE = OK/Enabled
    // Werte fuer QueryXofOCompensation.uWhat wie Query1D
    __inline BOOL CanXofOCompensationBeEnabled() const { return QueryXofOCompensation(Q1D_CAN_BE_ENABLED);};
    __inline BOOL IsXofOCompensationEnabled()    const { return QueryXofOCompensation(Q1D_IS_ENABLED);};
    __inline BOOL ReadXofOComp()                       { return QueryXofOCompensation(Q1D_READ_COMP);};

private: // Daten
    HANDLE  m_hCnc;                 // Handle auf die CNC, muss erstes Datum sein !!

    UINT    m_uHWCollDetOffAxis;    // Index der Achse auf der mit DOut1 die HWCollDet ausgeschaltet wird 
};  

// Als Parameter fuer die Funktionen, falls der MaximalWert gewuenscht ist
#define CNC_VMAX    AXIS_VMAX       // SetVeloc
#define CNC_ACCMAX  AXIS_ACCMAX     // SetAccDecc
#define CNC_DECCMAX AXIS_DECCMAX    // SetAccDecc

/*
 * Definitionen zu Funktionscode von Zusynchron(code,...)
 */
                                // code   Parameterbedeutung                            / Returwert
#define CNC_SYNFKT_GET_VERSION    0     //CString*, gibt Softwareversionsstring       / 0, wenn OK
#define CNC_SYNFKT_SET_OBJHEIGHT  1     //double, setzt die aktuelle Objekthoehe      / 0, wenn OK
#define CNC_SYNFKT_SET_SYNCSTATE  2     //int, bit 0=Synchronfahren EIN/AUS           / alter Status
#define CNC_SYNFKT_GET_SYNCSTATE  3     //Ret:int, bit0=0x01=Synchronfahren EIN/AUS        
                                        //         bit1=0x02=Joystick auf Viewkoordinaten
                                        //         bit2=0x04=AutoSync darf aktiv sein (INI.active=1)
                                        //         bit3=0x08=AutoSyncBusy ab 3.4.0.0
#define CNC_SYNFKT_OPEN_DIALOG    4     //BOOL anzeigen/verstecken Hilfsdialog        / 0 wenn OK
#define CNC_SYNFKT_SET_JOY4VIEW   5     //int bit0=1:setzt Joy auf Viewkoordinaten,      //0 wenn OK
                                        //       0=0:setzt Joy auf Maschinenkoordianten
#define CNC_SYNFKT_SET_ROTCENTER  6     // double, double, Drehzentrum des Drehtisches setzen (Ana/Nano)

#define CNC_SYNFKT_SET_JOYMOVE    7     //int AchsID, double JoyGeschwindigkeit, double Zielpos
                                        //Ergebnis: 0: Befehl wurde angenommen, Achse wird fahren
                                        //dabei wird statusbit 3 von syncstate gesetzt, solange eine Achse faehrt
                                        //sonst: 1: irgendein Joystick ist aktiv
                                        //       2: nicht im Zustand Synchronfahren
                                        //       3: falsche Parameter
                                        //       4: Außerhalb Verfahrbereich
#define CNC_SYNFKT_SET_FTA        8     //double, setzt aktuellen FokusTischAbstand     / 0, wenn OK
#define CNC_SYNFKT_SET_FDA        9     //double, setzt aktuellen FokusDetektorAbstand  / 0, wenn OK
    
// Hilfsfunktionen (defines und inline) fuer AutoSync
// geht nur mit Uebergabe der CCnc-Instanz
// ab 26.10.2005 __inline statt Defines
__inline int CncSync_EnableAutoSync (CCnc &cnc) { return cnc.Zusynchron(CNC_SYNFKT_SET_SYNCSTATE, 1);};
__inline int CncSync_DisableAutoSync(CCnc &cnc) { return cnc.Zusynchron(CNC_SYNFKT_SET_SYNCSTATE, 0);};

__inline BOOL CncSync_IsAutoSyncEnabled(CCnc &cnc)  { return (cnc.Zusynchron(CNC_SYNFKT_GET_SYNCSTATE) & 0x01);};
__inline BOOL CncSync_IsJoyInViewCoord (CCnc &cnc)  { return (cnc.Zusynchron(CNC_SYNFKT_GET_SYNCSTATE) & 0x02);};
__inline BOOL CncSync_IsAutoSyncAllowed(CCnc &cnc)  { return (cnc.Zusynchron(CNC_SYNFKT_GET_SYNCSTATE) & 0x04);};
__inline BOOL CncSync_IsAutoSyncBusy(CCnc &cnc)     { return (cnc.Zusynchron(CNC_SYNFKT_GET_SYNCSTATE) & 0x08);};

__inline int CncSync_SetObjectThick(CCnc &cnc, double dMM)      // Objectdicke setzen
    { return cnc.Zusynchron(CNC_SYNFKT_SET_OBJHEIGHT, dMM);};
__inline int CncSync_SetFTA(CCnc &cnc, double dFTA)             // FTA setzen
    { return cnc.Zusynchron(CNC_SYNFKT_SET_FTA, dFTA);};
__inline int CncSync_SetFDA(CCnc &cnc, double dFDA)             // FDA setzen
    { return cnc.Zusynchron(CNC_SYNFKT_SET_FDA, dFDA);};

__inline int CncSync_SetRotCenter(CCnc &cnc, double dCx, double dCy)
    { return cnc.Zusynchron(CNC_SYNFKT_SET_ROTCENTER, dCx, dCy);};

// Joystick Override ON/OFF
__inline int CncSync_SetJoyMove(CCnc &cnc, UINT uAxisIdx, double dSpeed, double dZielPos)
    { return cnc.Zusynchron(CNC_SYNFKT_SET_JOYMOVE, uAxisIdx, dSpeed, dZielPos);}
__inline int CncSync_JoyMoveOff(CCnc &cnc)
    { return cnc.Zusynchron(CNC_SYNFKT_SET_JOYMOVE, 0, 0, 0);}

/*
 * OLD Defines, DO NOT USE in future Versions, use inline-functions instead!!!
 */
#ifndef DONT_USE_OLDCNCDEFINES
    #define ENABLEAUTOSYNC(cnc)                         CncSync_EnableAutoSync(cnc)
    #define DISABLEAUTOSYNC(cnc)                        CncSync_DisableAutoSync(cnc)
    #define ISAUTOSYNCENABLED(cnc)                      CncSync_IsAutoSyncEnabled(cnc)
    #define ISJOYINVIEWCOORD (cnc)                      CncSync_IsJoyInViewCoord (cnc)
    #define ISAUTOSYNCALLOWED(cnc)                      CncSync_IsAutoSyncAllowed(cnc)
    #define SETOBJECTTHICK(cnc, dHeightOverTable)       CncSync_SetObjectThick(cnc, dHeightOverTable)
    #define SETROTCENTER(cnc, dXm, dYm)                 CncSync_SetRotCenter(cnc, dXm, dYm)
    #define ISAUTOSYNCALLOWED(cnc)                      CncSync_IsAutoSyncAllowed(cnc)
    #define SETJOYMOVE(cnc, uAxisIdx, dSpeed, dZielPos) CncSync_SetJoyMove(cnc, uAxisIdx, dSpeed, dZielPos)
#endif // DONT_USE_OLDCNCDEFINES

#endif _CNC_H_
