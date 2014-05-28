/* Copyright 2012 Geehalel (geehalel AT gmail DOT com) */
/* This file is part of the Skywatcher Protocol INDI driver.

    The Skywatcher Protocol INDI driver is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The Skywatcher Protocol INDI driver is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the Skywatcher Protocol INDI driver.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SKYWATCHER_H
#define SKYWATCHER_H

#include <sys/time.h>
#include <time.h>
#include <inditelescope.h>
#include <lilxml.h>

#include "eqmoderror.h"
//#include "eqmod.h"
class EQMod; // TODO

#ifdef WITH_SIMULATOR
#include "simulator/simulator.h"
#endif

#define SKYWATCHER_MAX_CMD        16
#define SKYWATCHER_MAX_TRIES      3
#define SKYWATCHER_ERROR_BUFFER   1024

#define SKYWATCHER_SIDEREAL_DAY 86164.09053083288
#define SKYWATCHER_SIDEREAL_SPEED 15.04106864
#define SKYWATCHER_STELLAR_DAY 86164.098903691
#define SKYWATCHER_STELLAR_SPEED 15.041067179

#define SKYWATCHER_LOWSPEED_RATE 128
#define SKYWATCHER_MAXREFRESH 0.5

#define HEX(c) (((c) < 'A')?((c)-'0'):((c) - 'A') + 10)

class Skywatcher 
{

public:
    Skywatcher(EQMod *t);
    ~Skywatcher();
   
    bool Connect(char *port) throw (EQModError);
    bool Disconnect() throw (EQModError);
    void setDebug (bool enable);
    const char *getDeviceName ();


    unsigned long GetRAEncoder()  throw (EQModError);
    unsigned long GetDEEncoder()  throw (EQModError);
    unsigned long GetRAEncoderZero();
    unsigned long GetRAEncoderTotal();
    unsigned long GetRAEncoderHome();
    unsigned long GetDEEncoderZero();
    unsigned long GetDEEncoderTotal();
    unsigned long GetDEEncoderHome();
    unsigned long GetRAPeriod()  throw (EQModError);
    unsigned long GetDEPeriod()  throw (EQModError);
    void GetRAMotorStatus(ILightVectorProperty *motorLP) throw (EQModError);
    void GetDEMotorStatus(ILightVectorProperty *motorLP) throw (EQModError);
    void InquireBoardVersion(ITextVectorProperty *boardTP) throw (EQModError); 
    void InquireRAEncoderInfo(INumberVectorProperty *encoderNP) throw (EQModError); 
    void InquireDEEncoderInfo(INumberVectorProperty *encoderNP) throw (EQModError); 
    void Init(ISwitchVectorProperty *parkSP) throw (EQModError);
    void SlewRA(double rate) throw (EQModError);
    void SlewDE(double rate) throw (EQModError);
    void StopRA() throw (EQModError);
    void StopDE() throw (EQModError);
    void SetRARate(double rate)  throw (EQModError);
    void SetDERate(double rate)  throw (EQModError);
    void SlewTo(long deltaraencoder, long deltadeencoder);
    void StartRATracking(double trackspeed) throw (EQModError);
    void StartDETracking(double trackspeed) throw (EQModError);
    bool IsRARunning() throw (EQModError);
    bool IsDERunning() throw (EQModError);
#ifdef WITH_SIMULATOR
    void setSimulation(bool);
    bool isSimulation();
    bool simulation;
#endif
    // Park 
    unsigned long GetRAEncoderPark();
    unsigned long GetRAEncoderParkDefault();
    unsigned long GetDEEncoderPark();
    unsigned long GetDEEncoderParkDefault();
    unsigned long SetRAEncoderPark(unsigned long steps);
    unsigned long SetRAEncoderParkDefault(unsigned long steps);
    unsigned long SetDEEncoderPark(unsigned long steps);
    unsigned long SetDEEncoderParkDefault(unsigned long steps);
    void SetParked(bool parked);
    bool isParked();
    bool WriteParkData();

 private: 

    // Official Skywatcher Protocol
    // See http://code.google.com/p/skywatcher/wiki/SkyWatcherProtocol
    // Constants
    static const char SkywatcherLeadingChar = ':';
    static const char SkywatcherTrailingChar= 0x0d;
    static const double MIN_RATE=0.05;
    static const double MAX_RATE=800.0;
    unsigned long minperiods[2];

    // Types
    enum SkywatcherCommand {      
      Initialize ='F',
      InquireMotorBoardVersion='e',
      InquireGridPerRevolution='a',
      InquireTimerInterruptFreq='b',
      InquireHighSpeedRatio='g',
      InquirePECPeriod='s',
      InstantAxisStop='L',
      NotInstantAxisStop='K',
      SetAxisPosition='E',
      GetAxisPosition='j',
      GetAxisStatus='f',
      SetSwitch='O',
      SetMotionMode='G',
      SetGotoTargetIncrement='H',
      SetBreakPointIncrement='M',
      SetBreakSteps='U',
      SetStepPeriod='I',
      StartMotion='J',
      GetStepPeriod='D', // See Merlin protocol http://www.papywizard.org/wiki/DevelopGuide
      ActivateMotor='B', // See eq6direct implementation http://pierre.nerzic.free.fr/INDI/
      SetGuideRate='P',  // See EQASCOM driver
      Deactivate='d',
      NUMBER_OF_SkywatcherCommand
    };
    
    
    enum SkywatcherAxis { 
      Axis1='1',       // RA/AZ
      Axis2='2',       // DE/ALT
      NUMBER_OF_SKYWATCHERAXIS
    };
  
    enum SkywatcherDirection {BACKWARD=0, FORWARD=1};
    enum SkywatcherSlewMode { SLEW=0, GOTO=1  };
    enum SkywatcherSpeedMode { LOWSPEED=0, HIGHSPEED=1  };
    typedef struct SkywatcherAxisStatus {SkywatcherDirection direction; SkywatcherSlewMode slewmode; SkywatcherSpeedMode speedmode; } SkywatcherAxisStatus;
    enum SkywatcherError { NO_ERROR, ER_1, ER_2, ER_3 };
  
    struct timeval lastreadmotorstatus[NUMBER_OF_SKYWATCHERAXIS];
    struct timeval lastreadmotorposition[NUMBER_OF_SKYWATCHERAXIS];

    // Functions
    void CheckMotorStatus(SkywatcherAxis axis)  throw (EQModError);
    void ReadMotorStatus(SkywatcherAxis axis) throw (EQModError);
    void SetMotion(SkywatcherAxis axis, SkywatcherAxisStatus newstatus) throw (EQModError);
    void SetSpeed(SkywatcherAxis axis, unsigned long period) throw (EQModError);
    void SetTarget(SkywatcherAxis axis, unsigned long increment) throw (EQModError);
    void SetTargetBreaks(SkywatcherAxis axis, unsigned long increment) throw (EQModError);
    void StartMotor(SkywatcherAxis axis) throw (EQModError);
    void StopMotor(SkywatcherAxis axis)  throw (EQModError);
    void InstantStopMotor(SkywatcherAxis axis)  throw (EQModError);
    void StopWaitMotor(SkywatcherAxis axis) throw (EQModError);

    bool read_eqmod()  throw (EQModError);
    bool dispatch_command(SkywatcherCommand cmd, SkywatcherAxis axis, char *arg)  throw (EQModError);

    unsigned long Revu24str2long(char *);
    unsigned long Highstr2long(char *);
    void long2Revu24str(unsigned long ,char *);

    double get_min_rate();
    double get_max_rate();
    bool isDebug();

    // Variables
    //string default_port;
    // See Skywatcher protocol
    unsigned long MCVersion; // Motor Controller Version
    unsigned long MountCode; // 

    unsigned long RASteps360;
    unsigned long DESteps360;
    unsigned long RAStepsWorm;
    unsigned long DEStepsWorm;
    unsigned long RAHighspeedRatio; // Motor controller multiplies speed values by this ratio when in low speed mode
    unsigned long DEHighspeedRatio; // This is a reflect of either using a timer interrupt with an interrupt count greater than 1 for low speed
                                    // or of using microstepping only for low speeds and half/full stepping for high speeds
    unsigned long RAStep;  // Current RA encoder position in step
    unsigned long DEStep;  // Current DE encoder position in step
    unsigned long RAStepInit;  // Initial RA position in step
    unsigned long DEStepInit;  // Initial DE position in step
    unsigned long RAStepHome;  // Home RA position in step
    unsigned long DEStepHome;  // Home DE position in step
    unsigned long RAPeriod;  // Current RA worm period
    unsigned long DEPeriod;  // Current DE worm period

    bool RAInitialized, DEInitialized, RARunning, DERunning;
    bool wasinitialized;
    SkywatcherAxisStatus RAStatus, DEStatus;

    int fd;
    char command[SKYWATCHER_MAX_CMD];
    char response[SKYWATCHER_MAX_CMD];

    bool debug;
    const char *deviceName;
    bool debugnextread;
    EQMod *telescope;

    //Park
    void initPark();
    char *LoadParkData(const char *filename);
    char *WriteParkData(const char *filename);
    unsigned long RAParkPosition;
    unsigned long RADefaultParkPosition;
    unsigned long DEParkPosition;
    unsigned long DEDefaultParkPosition;
    bool parked;
    const char *ParkDeviceName;
    const char * Parkdatafile;
    XMLEle *ParkdataXmlRoot, *ParkdeviceXml, *ParkstatusXml, *ParkpositionXml, *ParkpositionRAXml, *ParkpositionDEXml;
};

#endif
