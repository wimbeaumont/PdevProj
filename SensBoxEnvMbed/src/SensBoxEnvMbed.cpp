
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
 
 */ 

#define SENSBOXENVMBEDVER "1.0"
#if defined  __MBED__ 
#define  OS_SELECT "MBED" 

#include "mbed.h"

// i2c devices are defined at a lower level ( envsensread.cpp)

#if  MBED_MAJOR_VERSION > 5 
BufferedSerial pc(USBTX, USBRX);
#else 
Serial pc(USBTX, USBRX);
#endif 
#endif //__MBED__ 

#include <cstdio>
#include <cstdlib>     // atof 
#include <string.h> 
#include "envsensread.h"
#include "env_scpiparser.h"



// globals for char control 
bool  Get_Result = false ;
bool  Always_Result = false ;



int main(void) { 

   char buffer[1024] = {0};  // receive buffer 
   char message[1024];
   int valread=0;
	//printf("Check Env  program version %s, compile date %s time %s\n\r",SENSBOXENVMBEDVER,__DATE__,__TIME__);

#if defined __DUMMY__ 
     printf("skip init i2c\n\r");
#else 
	// initialize the I2C devices   
    int status=init_i2c_dev();
    if ( status ){
    	//printf("failed to init i2c devices returns %d \n\r",status);
    	return -1;
    }
#endif
    scpi_setup();// initialize the parser

    int lc2=0;  
    bool  STAYLOOP =true;
    while(STAYLOOP ) {
    	scanf("%s",buffer);
       	// buffer[valread]='\0'; assume end with \0
  		//printf("This is from the client : %s length %d  expect %d \n\r",buffer,strlen(buffer),valread );
		valread=strlen(buffer);
	  	if (valread > 0) {
	  		env_scpi_execute_command( buffer, valread);
	  		strcpy(message,	env_get_result());
	  		if( strcmp( message, "STOP done") == 0 ) {STAYLOOP = false;}
	  		if(strlen(message)== 0) { strcpy(message, "wrong SCPI cmd");}
	  	} else {
	  		strcpy(message, "message is zerro");
	  	}
	  	//printf( "will send %s length %d \n\r ", message ,strlen(message)  );
	  	printf("%s", message );
		//printf("Have sent message %s\n\r ", message);
    
	  	wait_for_ms(100);
  	  	
	  	lc2++;
  } //while stayloop


 

    
} // end main 
