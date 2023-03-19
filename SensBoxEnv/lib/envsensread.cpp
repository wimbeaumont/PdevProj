/*
 * envsensread.cpp
 * 
 * implemenation of functions to read sensors from the I2C bus 
 * 
 *
 *  Created on: Apr 29, 2020
 *      Author: wimb
 *
 * 20201030 try to get it also working for MBED 
 * 20201103 added reseti2c bus function not tes
 * 20210414 added ADC readout 
 *  20230319 added reset amd status info 
 */



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
/* serial device is already defined in the main program 
#if  MBED_MAJOR_VERSION > 5 
BufferedSerial pc(USBTX, USBRX);
#else 
Serial pc(USBTX, USBRX);
#endif 
*/
#include "I2C.h"
#include "MBEDI2CInterface.h"  
#include "ADC_MBED.h"

MBEDI2CInterface mbedi2c( SDA, SCL); 
MBEDI2CInterface* mbedi2cp=  &mbedi2c ;
DigitalOut rst(PTB8,1);
//------------------ end MBED specific config
#elif defined __LINUX__
#define  OS_SELECT "linux_i2c" 

#include <cstdio>
#include <cstdlib>
#include "LinuxI2CInterface.h"
#include "ADCInterface.h"
char *filename = (char*)"/dev/i2c-1";  //hard coded for the moment 
LinuxI2CInterface  mbedi2c(filename);
LinuxI2CInterface* mbedi2cp= &mbedi2c;
int rst;
//------------------ end Linux I2C specific config
#else 
#define  OS_SELECT "linux_dummy" 

#include <cstdio>
#include <cstdlib>
#include "DummyI2CInterface.h"
#include "ADCInterface.h"

DummyI2CInterface  mbedi2c;
DummyI2CInterface* mbedi2cp= &mbedi2c;
int rst;
#endif  // __MBED__ 
//------------------ end Linux dummy specific config
#ifndef OS_SELECT 
#define OS_SELECT "linux_dummy" 
#endif
// --- end platform specific configs 

#include <stdio.h>
#include "AT30TSE75x_E.h"
#include "hts221.h"
#include "veml7700.h"  // differs from the MBED version
#include "env_scpiparser.h"

I2CInterface* i2cdev= mbedi2cp;

// all the devices are static ( in real world they really are )
VEML7700 luxm ( i2cdev);  // so static global  in this file
// init tempeature senesor with addr 1 en 2
const int NrTsens=2;
AT30TSE75x_E tid[NrTsens] ={ AT30TSE75x_E( i2cdev ,1), AT30TSE75x_E( i2cdev ,2)};
HTS221 shs ( i2cdev, true, true ); // initialize with one shot .

#if defined  __MBED__ 
ADC_MBED adc(A0 ); 
#else 
ADCInterface adc;
#endif 
 

static int status =0;
void wait_for_ms(int nr){ i2cdev->wait_for_ms(nr);};
int get_i2cstatus(void){return status;}

int init_i2c_dev(void) {
	//printf(" envsensread compiled for  %s \n\r", OS_SELECT);
 	 int status=shs.get_status( );
 	 if(status)  {
 		 //printf("get error %d after init humidity \n\r", status);
 		 //printf ( "HTS221  lib version :%s\n\r ",shs.getversioninfo());
 		 return status -200;
 	 }
 	 //printf ( "HTS221 lib version :%s\n\r ",shs.getversioninfo());
 	 int id=(int) shs.ReadID();
 	 //printf("Who Am I returns %02x \n\r", id);

	 
 	  status=luxm.get_status( );
 	 if( status) {  //  printf("get error %d after init \n\r", status);
 	 	 status=status-100;
 	 	 return status;
 	 }
 	 else {
 		 luxm.set_integrationtime(1000); // > max
 		 luxm.set_gain(3); // > max
		// printf ( "VEML770 lib version :%s\n\r ",luxm.getversioninfo());
 	 }
 	 for ( int lc=0 ; lc < NrTsens ; lc++) {
      	 //printf( "Taddr %x , Eaddr %x \n\r ", tid[lc].getTaddr(),tid[lc].getEaddr());
 		 status=tid[lc].err_status ;
 		 if(  status){
 			 //printf("reading config registers failed got status %d \n\r",status);
             if ( tid[lc].err_status== -20 ) {   tid[lc].err_status=0 ; status=0; }
             else {return status-200 - lc*100;} //  major failure
 		 }
 	 }
 	 return status;
}  // end init i2c

float read_temperature (int  ch){
	float Temp;
	if ( ch >= 0 && ch < NrTsens ){
        Temp=tid[ch].getTemperature( );
	} else if ( ch == 2 ){
        	status=shs.GetTemperature(&Temp);
        	//printf("after read humidityT %d  \n\r", status);
		} else { Temp=-274; }
	return Temp;
}

float read_voltage( int ch) {
	float volt;
	adc.getVoltage(volt);// channel is not supported for ADC_MBED interface 
	return volt;
	
}
float read_humidity (void){
	float hum;
	status=shs.GetHumidity(&hum);
			//printf("after read humidity  status %d \n\r", status );
	return hum;
}

float read_luminosity (void){
	float  lux=luxm.get_lux (false);
	status = luxm.get_status( );
	return lux;
}

int reset_i2cbus(void) {
	 rst=0; // full reset for MBED  with solder patch 
	 i2cdev-> stop  (); // for MBED there is not a real reset of the bus so try to force a stop condition
	 return 0;
}
