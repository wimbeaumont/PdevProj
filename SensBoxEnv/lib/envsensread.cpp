/*
 * envsensread.cpp
 *
 *  Created on: Apr 29, 2020
 *      Author: wimb
 */





#if defined __LINUX__
#define  OS_SELECT "linux_i2c"
#include "LinuxI2CInterface.h"

char *filename = (char*)"/dev/i2c-1";  //hard coded for the moment
LinuxI2CInterface  mbedi2c(filename);
LinuxI2CInterface* mbedi2cp= &mbedi2c;

//------------------ end Linux I2C specific config
#else
#define  OS_SELECT "linux_dummy"
#include "DummyI2CInterface.h"
DummyI2CInterface  mbedi2c;
DummyI2CInterface* mbedi2cp= &mbedi2c;
#endif
#ifndef OS_SELECT
#define OS_SELECT "linux_dummy"
#endif
// --- end platform specific configs

#include <stdio.h>

#include "dev_interface_def.h"
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

int status =0;
void wait_for_ms(int nr){ i2cdev->wait_for_ms(nr);};
int get_status(void){return status;}

int init_i2c_dev(void) {
	printf(" envsensread compiled for  %s \n\r", OS_SELECT);
 	 int status=shs.get_status( );
 	 if(status)  {
 		 printf("get error %d after init humidity \n\r", status);
 		 printf ( "HTS221  lib version :%s\n\r ",shs.getversioninfo());
 		 return status -200;
 	 }
 	 //printf ( "HTS221 lib version :%s\n\r ",shs.getversioninfo());
 	 //int id=(int) shs.ReadID();
 	 //printf("Who Am I returns %02x \n\r", id);

	 
 	  status=luxm.get_status( );
 	 if( status) {    printf("get error %d after init \n\r", status);
 	 	 status=status-100;
 	 	 return status;
 	 }
 	 else {
 		 luxm.set_integrationtime(1000); // > max
 		 luxm.set_gain(3); // > max
		  //printf ( "VEML770 lib version :%s\n\r ",luxm.getversioninfo());
 	 }
 	 for ( int lc=0 ; lc < NrTsens ; lc++) {
      	  //printf( "Taddr %x , Eaddr %x \n\r ", tid[lc].getTaddr(),tid[lc].getEaddr());
 		 status=tid[lc].err_status ;
 		 if(  status){
 			 printf("reading config registers failed got status  \n\r",status);
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
	} else if ( ch == 3 ){
        	status=shs.GetTemperature(&Temp);
        	//printf("after read humidityT %d  \n\r", status);
		} else { Temp=-274; }
	return Temp;
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
