
/** program to check the environment status
 *  first usage  ,  Sensor IV test 
 *  Read temperature and humidty via RS232 software 
 *  if char g is send the program response with a string 
 *  Tx  temperature of sensor x 
 *  Hx  humidity of sensor x 
 *  This is a fork from the prog EnvChk on "my"  MBED repository 
 *  The original program was developed for the FRMD KL05Z
 *  This program targets the Raspberry Pi as this is the target machine 
 *  for the Sensor box control.  So moving this to the RP saves hardware 
 *  Due to limitation of the available software for the Raspberry Pi we have to go back to FRMD KL25Z 
 *  but want to keep the advatages of the SCPI  communicaton 
 *  (C) Wim Beaumont Universiteit Antwerpen 2019 2020
 *  * License see
 *  https://github.com/wimbeaumont/PeripheralDevices/blob/master/LICENSE

 *  Version history :
 *  0.1   inital version form from SensBoxEnv  for MBED ,  ( not other platform support ) 
 *  0.4x  checking with hardware 
 *  0.6x  initial production version 
 *  1.0   added hwversion ,  corrected memory leak in scpi parser 
 *  1.1   scpi lib  corrected for float format overflow 
 *  1.2   check without wait 
 *  1.3   added VOLT support (reading MBED ADC A0)
 */ 

#define SENSBOXENVMBEDVER "1.3"
#if defined  __MBED__ 
#define  OS_SELECT "MBED" 

#include "mbed.h"

// i2c devices are defined at a lower level ( envsensread.cpp)


DigitalOut rled(LED1);
DigitalOut gled(LED2);
DigitalOut bled(LED3);

#else //end __MBED__ 

int rled ;
int gled ;
int bled ;
#endif //

#include <stdio.h>
#include <stdlib.h>     // atof 
#include <string.h> 
#include "envsensread.h"
#include "env_scpiparser.h"



// globals for char control 
bool  Get_Result = false ;
bool  Always_Result = false ;



int main(void) { 

   rled=1;bled=1;gled=1;
   char buffer[512] = {0};  // receive buffer 
   char* message= buffer;
   int valread=0;

	// initialize the I2C devices   
    int status=init_i2c_dev();
    //printf("i2cinit done with status %d \n\r",status);
    char hwversion[]=SENSBOXENVMBEDVER;	
    scpi_setup( hwversion);// initialize the parser
    int lc2=0;  
    bool  STAYLOOP =true;
    while(STAYLOOP ) {
    	//int buflength=(int) sizeof(buffer);
    	scanf(" %512[^\n]s",buffer);// read unitl \n  the space before % is important  512 
   
       	// buffer[valread]='\0'; assume end with \0
  		//printf("This is from the client : %s length %d  expect %d \n\r",buffer,strlen(buffer),valread );
		valread=strlen(buffer);
		//printf("got message nr %d ,  %s with length %d\n\r",lc2,buffer, valread);
	  	if (valread > 0) {
	  		env_scpi_execute_command( buffer, valread);
	  		strcpy(message,	env_get_result());
	  		//if( strcmp( message, "STOP done") == 0 ) {STAYLOOP = false;}
	  		if(strlen(message)== 0) { strcpy(message, "wrong SCPI cmd");}
	  	} else {
	  		strcpy(message, "MB message is zerro");
	  	}
	  	//printf( "will send %s length %d \n\r ", message ,strlen(message)  );
	  	printf("%s\n\r", message );//need new line for the receiver , is waiting for that 
		//printf("Have sent message %s\n\r ", message);
    
	  	//wait_for_ms(1);
  	  	
	  	lc2++;
  } //while stayloop


 

    
} // end main 
