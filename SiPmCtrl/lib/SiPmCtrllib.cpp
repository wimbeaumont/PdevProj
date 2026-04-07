/** program to controll the bias and trigger levels for a two channel 
 *  SiPm readout board 
 *  for more info see the README.MD of the repository 
 *  https://github.com/wimbeaumont/peripheral_dev_tst.git
 *  this version is mainly targeting testing on a Linux ( e.g. Raspberry Pi) platform 
 *  V 1.0  : copied from LTC2633_tst.cpp 
 *  V 1.24  : working with the hardware simple increasing
 *  V 1.40  : version copied from LTC2633_app  ( v 1.36 )
 *  V 1.31  :  corrected issue with addressing 
 *  V 1.52  :  version work with RP and  SiPm board
 *  V 1.53  :  change atoi to atof  for third argument 
 *  V 1.6   :  use the control as lib 
 *  V 1.7   :  corrections for setDiscriminatorLevel  added correct conversion level , tested 20260407
  * (C) Wim Beaumont Universiteit Antwerpen 2019
 *  License see
 *  https://github.com/wimbeaumont/PeripheralDevices/blob/master/LICENSE
 */ 
 

#define SIPMCTRLLIB "1.7"


#include "dev_interface_def.h"

// OS / platform  specific  configs 
#if defined  __MBED__ 
#include "mbed.h"

#if   defined (TARGET_KL25Z) || defined (TARGET_KL46Z)
  PinName const SDA = PTE0;
  PinName const SCL = PTE1;
#elif defined (TARGET_KL05Z)
  PinName const SDA = PTB4;
  PinName const SCL = PTB3;
#elif defined (TARGET_K20D50M)
  PinName const SDA = PTB1;
  PinName const SCL = PTB0;
#else
  #error TARGET NOT DEFINED
#endif

#include "I2C.h"
#include "MBEDI2CInterface.h"  
MBEDI2CInterface mbedi2c( SDA, SCL); 
MBEDI2CInterface* mbedi2cp=  &mbedi2c ;

//------------------ end MBED specific config
#elif defined __LINUX__

#include <cstdio>
#include <cstdlib>
#include "LinuxI2CInterface.h"

char *filename = (char*)"/dev/i2c-1";  //hard coded for the moment 
LinuxI2CInterface  mbedi2c(filename);
LinuxI2CInterface* mbedi2cp= &mbedi2c;

//------------------ end Linux I2C specific config
#else 

#include <cstdio>
#include <cstdlib>
#include "DummyI2CInterface.h"
DummyI2CInterface  mbedi2c;
DummyI2CInterface* mbedi2cp= &mbedi2c;

//DummyDigitalOut LDAC ;
//DummyDigitalOut CntPin ;


#endif  // 
//------------------ end Linux dummy specific config
// --- end platform specific configs 


#include "DACInterface.h" 

#include "dev_interface_def.h"

// #include "LTC2633setaddr.h"
#include "ltc2633.h"


I2CInterface* i2cdev= mbedi2cp;
// so something to do with the hardware so first initialize  		
int bias_ctrl_addr= 0x12;  //CAO at 5 Vdd
int discr_lvl_addr= 0x10;  //CAO at 0 Vdd
int Vreftype=1 , resolution=12;  // internal reference 
const float Vrefext=5.0;
LTC2633  biasctrl(i2cdev, bias_ctrl_addr,  Vrefext ,Vreftype , resolution  );
LTC2633  discr_lvl(i2cdev, discr_lvl_addr,  Vrefext ,Vreftype , resolution  );



extern  int setBiasVoltage(int ch , float volt ) {
    volt=volt/0.0342 ;
    int errcode;
    errcode= biasctrl.setDACvalue(int(volt) ,ch);  
    return errcode;
}

extern  int setDiscriminatorLevel( int ch, float volt) {
    volt=volt/0.000603;  // =2.5 /4096 
    int errcode;
    errcode= discr_lvl.setDACvalue(int(volt) ,ch);  
    return errcode;
}



