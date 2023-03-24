
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
 *  KL25Z specific :
 *  to activate the Watchdog function set #define DISABLE_WDOG    0  ( defaul 1) in the file  * 
 *  mbed-os/targets/TARGET_Freescale/TARGET_KLXX/TARGET_KL25Z/device/system_MKL25Z4.c closed.
 *  Program works also without activating the WD.  

 *  Version history :
 *  0.1   inital version form from SensBoxEnv  for MBED ,  ( not other platform support ) 
 *  0.4x  checking with hardware 
 *  0.6x  initial production version 
 *  1.0   added hwversion ,  corrected memory leak in scpi parser 
 *  1.1   scpi lib  corrected for float format overflow 
 *  1.2   check without wait 
 *  1.3   added VOLT support (reading MBED ADC A0)
 *  1.4   WD implementaton check doc , system_MKL25Z4.c has to be changed (mbed_os) 
 *  2.0   WD  and none blocking read 
 */ 

#define SENSBOXENVMBEDVER "2.0"

// OS / platform  specific  configs 
#ifdef __PICO__ 
#include <stdio.h>
#include "pico/stdlib.h"
#elif defined  __MBED__ 
#define  OS_SELECT "MBED" 

#include "mbed.h"
#endif //
// i2c devices are defined at a lower level ( envsensread.cpp)
#if defined  __MBED__ 

DigitalOut rled(LED1);
DigitalOut gled(LED2);
DigitalOut bled(LED3);

BufferedSerial pc(USBTX,USBRX);

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



#define RDBUFSIZE 512

void kickWD(void) {
#ifdef __PICO__ 
#elif defined  __MBED__ 
	SIM->SRVCOP=0x55;
	SIM->SRVCOP=0xAA;
#endif
}

int  read_noneblocking(  char* readbuf) {
	char recchar='$'; // just dummy 
	int bufcnt=0,cnt=0;
	while (recchar != '\n') { // continue reading
#ifdef __PICO__ 
				sleep_ms (10); // has to be checked
#elif defined  __MBED__ 
				thread_sleep_for(1); // min wait to slow down the while loop 
#endif
				if(cnt < 100000 ) {kickWD();} // wait 
				cnt++;
#ifdef __PICO__ 
				while(1)  {
#elif defined  __MBED__ 
				while(pc.readable())  {
#endif
					cnt=0; kickWD();
#ifdef __PICO__ 
					int reccharpico=getchar_timeout_us(0);
					if ( reccharpico  == PICO_ERROR_TIMEOUT) continue;
					else recchar=(char) reccharpico;
#else					
					pc.read(&recchar , 1);
#endif
					//printf("recchar %c \n\r",recchar);
					if ( recchar == '\r' ) continue; // ignore line feed 
					if ( recchar == '\n' ) {						
						readbuf[bufcnt]='\0';
						//printf("read done %s \n\r", readbuf);
					}		
					else {
						if ( bufcnt == RDBUFSIZE-1) { readbuf[0]='\0';recchar='\n' ;}
						else { readbuf[bufcnt]=recchar;
							//printf("%c , bufcnt %d  %c\r\n" , recchar, bufcnt,readbuf[bufcnt]);
							bufcnt++;
						}
					}
				}
			}
	return bufcnt;
}	





int main(void) { 
#ifdef __PICO__    
       stdio_init_all();// pico init 
#endif 

   rled=0;bled=1;gled=1;
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
    	//scanf(" %512[^\n]s",buffer);// read unitl \n  the space before % is important  512 
        read_noneblocking( buffer);
       	// buffer[valread]='\0'; assume end with \0
       	valread=strlen(buffer);
  		//printf("This is from the client : %s length %d  expect %d \n\r",buffer,strlen(buffer),valread );		
		//printf("got message nr %d ,  %s with length %d\n\r",lc2,buffer, valread);
	  	if (valread > 0) {
	  		env_scpi_execute_command( buffer, valread);
	  		//kickWD();
	  		strcpy(message,	env_get_result());
	  		//if( strcmp( message, "STOP done") == 0 ) {STAYLOOP = false;}
	  		if(strlen(message)== 0) { strcpy(message, "wrong SCPI cmd");}
	  	} else {
	  		strcpy(message, "MB message is zerro");
	  	}
	  	//printf( "will send %s length %d \n\r ", message ,strlen(message)  );
#ifdef __PICO__    
		stdio_flush();
#else 
	  	pc.sync(); //flush
#endif
	  	printf("%s\r\n", message );//need new line for the receiver , is waiting for that 
	  	//kickWD();
		//printf("Have sent message %s\n\r ", message);
    
	  	//wait_for_ms(1);
  	  	rled=1;
  	  	gled=0;
	  	lc2++;
  } //while stayloop


 

    
} // end main 
