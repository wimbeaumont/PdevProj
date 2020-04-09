/** program to controll the bias and trigger levels for a two channel 
 *  SiPm readout board 
 *  for more info see the README.MD of the repository 
 *  https://github.com/wimbeaumont/peripheral_dev_tst.git
 *  this version is mainly targeting testing on a Linux ( e.g. Raspberry Pi) platform 
 *  V 1.0  : copied from LTC2633_tst.cpp 
 *  V 1.24  : working with the hardware simple increasing
 *  V 1.40  : version copied from LTC2633_app  ( v 1.36 )
 *  V 1.51  :  version work with RP and  SiPm board
 * (C) Wim Beaumont Universiteit Antwerpen 2019
 *  License see
 *  https://github.com/wimbeaumont/PeripheralDevices/blob/master/LICENSE
 */ 
 

#define SIPMCTRL "1.51"


#include "dev_interface_def.h"

// OS / platform  specific  configs 
#if defined  __MBED__ 
#define  OS_SELECT "MBED" 

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

#define  OS_SELECT "linux_i2c" 

#include <cstdio>
#include <cstdlib>
#include "LinuxI2CInterface.h"

char *filename = (char*)"/dev/i2c-1";  //hard coded for the moment 
LinuxI2CInterface  mbedi2c(filename);
LinuxI2CInterface* mbedi2cp= &mbedi2c;

//------------------ end Linux I2C specific config
#else 
#define  OS_SELECT "linux_dummy" 

#include <cstdio>
#include <cstdlib>
#include "DummyI2CInterface.h"
DummyI2CInterface  mbedi2c;
DummyI2CInterface* mbedi2cp= &mbedi2c;

//DummyDigitalOut LDAC ;
//DummyDigitalOut CntPin ;


#endif  // __MBED__ 
//------------------ end Linux dummy specific config
#ifndef OS_SELECT 
#define OS_SELECT "linux_dummy" 
#endif
// --- end platform specific configs 


#include "DACInterface.h" 

#include "dev_interface_def.h"

// #include "LTC2633setaddr.h"
#include "ltc2633.h"


I2CInterface* i2cdev= mbedi2cp;
const float Vrefext=5.0;


int main(int argc, char *argv[]) {
	char cmd='h'; // status
	 if ( argc > 1) {
		cmd=argv[1][0];
	}
	
   if (cmd == 'i') {		
	// get the version of getVersion 
	getVersion gv;
	printf("SiPm ctrl version %s, compile date %s time %s for OS %s\n\r",SIPMCTRL,__DATE__,__TIME__,OS_SELECT);
	printf("getVersion :%s\n\r ",gv.getversioninfo());
	printf("I2C interface version  :%s\n\r ",i2cdev->getversioninfo());
	return (0) ;
   }
   if (cmd == 'h') { printf("no help \n\r"); return 0; }
   if ( argc < 4 ) { printf ("need 3 arguments \n\r" ) ; return -1; }
   int ch = atoi ( argv[2]);
   float volt = atoi ( argv[3]);
   // so something to do with the hardware so first initialize  		
   int bias_ctrl_addr= 0x12;  //CAO at 5 Vdd
   int discr_lvl_addr= 0x10;  //CAO at 0 Vdd
   int Vreftype=1 , resolution=12;
   LTC2633  biasctrl(i2cdev, bias_ctrl_addr,  Vrefext ,Vreftype , resolution  );
   LTC2633  discr_lvl(i2cdev, discr_lvl_addr,  Vrefext ,Vreftype , resolution  );
 
   i2cdev->wait_for_ms(1000);
   int errcode;
   
   switch (cmd ) {
	   
		case 'b' : {volt=volt/0.0342 ;
					errcode= biasctrl.setDACvalue(int(volt) ,ch);  
					if (errcode ){
						printf("failed to set biasctrl value %d for channel %d errcode %d\n\r",int(volt) ,ch,errcode);
					}
					else printf(" set DAC bias ch %d to %d \n\r", ch, int(volt));
		}break;
		case 'l' : { if ( ch == 0) ch=1; else ch=0;// hw channels are inverted 
					errcode= biasctrl.setDACvalue(int(volt) ,ch);  
					if (errcode ){
						printf("failed to set Lvl value %d for HW channel %d errcode %d\n\r",int(volt) ,ch,errcode);
					}
					else printf(" set DAC Lvl hw ch %d to digvalue %d \n\r", ch, int(volt));
		}break;
		default :	printf(" %c is not supported \n\r" , cmd);
   }
   
   return errcode;
}
