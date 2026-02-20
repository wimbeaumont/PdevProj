/** program to controll the bias and trigger levels for a two channel 
 *  SiPm readout board 
 *  for more info see the README.MD of the repository 
 *  https://github.com/wimbeaumont/peripheral_dev_tst.git
 *  this version is mainly targeting testing on a Linux ( e.g. Raspberry Pi) platform 
 *  using SiPmCtrllib forked from SiPmCrl v 1.53 
 *  (C) Wim Beaumont Universiteit Antwerpen 2019
 *  License see
 *  https://github.com/wimbeaumont/PeripheralDevices/blob/master/LICENSE
 */ 
 

#define SIPMCTRL "1.6"

#include <cstdio>
#include <cstdlib>

#include "SiPmCtrllib.h"

int main(int argc, char *argv[]) {
	char cmd='h'; // status
	 if ( argc > 1) {
		cmd=argv[1][0];
	}
	
   if (cmd == 'i') {		
	// get the version of getVersion 
	//getVersion gv;
	printf("SiPm ctrl version %s, compile date %s time %s for OS %s\n\r",SIPMCTRL,__DATE__,__TIME__,OS_SELECT);
	//printf("getVersion :%s\n\r ",gv.getversioninfo());
	//printf("I2C interface version  :%s\n\r ",i2cdev->getversioninfo());
	return (0) ;
   }
   if (cmd == 'h') { printf("give l for level, b for bias then channel then value \n\r"); return 0; }
   if ( argc < 4 ) { printf ("need 3 arguments \n\r" ) ; return -1; }
   int ch = atoi ( argv[2]);
   float volt ;
   volt =(float) atof ( argv[3]);
   int errcode;
   
   switch (cmd ) {
	   
		case 'b' : {volt=volt/0.0342 ;
					errcode= setBiasVoltage(ch , volt ) ;
					if (errcode ){
						printf("failed to set biasctrl value %d for channel %d errcode %d\n\r",int(volt) ,ch,errcode);
					}
					else printf(" set DAC bias ch %d to %d \n\r", ch, int(volt));
		}break;
		case 'l' : { if ( ch == 0) ch=1; else ch=0;// hw channels are inverted on the PCB 
					errcode= setDiscriminatorLevel(  ch,  volt);
					if (errcode ){
						printf("failed to set Lvl value %d for HW channel %d errcode %d\n\r",int(volt) ,ch,errcode);
					}
					else printf(" set DAC Lvl hw ch %d to digvalue %d \n\r", ch, int(volt));
		}break;
		default :	printf(" %c is not supported \n\r" , cmd);
   }
   
   return errcode;
}
