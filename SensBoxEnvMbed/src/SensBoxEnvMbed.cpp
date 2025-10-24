
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
 *  1.4   hardreset , for I2C stuck  problem 
 *  1.4b   WD implementaton check doc , system_MKL25Z4.c has to be changed (mbed_os) 
 *  2.0   WD  and none blocking read 
 *  2.2   after merge with 1.4 
 *  2.3   after merging changes for pico 
 *  2.4   adjusting none blocking read 
 */ 


#define SENSBOXENVMBEDVER "2.45"

// OS / platform  specific  configs 
#ifdef __PICO__ 
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"
#include "PWM_PICO.h" 
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
watchdog_update();
#elif defined  __MBED__ 
	SIM->SRVCOP=0x55;
	SIM->SRVCOP=0xAA;
#endif
}

#ifdef __PICO__ 
void core1_entry() {
	PWM_PICO pwmled( PICO_DEFAULT_LED_PIN,10000);  //GPIO 25 
	float delta_l=.05 ;
	float dc_set;
	int msleeptime=500;
	if(watchdog_enable_caused_reboot()) msleeptime=10;
	while(1) {
		for (float dc =0; dc<100; dc+=delta_l) {
			if( delta_l < 2.5)delta_l=1.05*delta_l; // increase the delta each time
			dc_set=pwmled.set_dutycycle(dc);
			sleep_ms(msleeptime);
		}
		
	}
}
#endif



int  read_noneblocking(  char* readbuf) {
	//for the moment still blocking , to be done  time out check 
	char recchar='$'; // just dummy 
	int bufcnt=0,cnt=0;
	while (recchar != '\n') { // continue reading
		#ifdef __PICO__ 
			sleep_us (1000); // has to be checked
			//sleep_ms (1); // has to be checked
		#elif defined  __MBED__ 
			thread_sleep_for(1); // min wait to slow down the while loop 
		#endif

		if(cnt < 100 ) {kickWD();} // wait 
		cnt++;
		#ifdef __PICO__ 
			int reccharpico=getchar_timeout_us(0);
			if ( reccharpico  != PICO_ERROR_TIMEOUT){
				recchar=(char) reccharpico;
		#elif defined  __MBED__ 
		 	if (pc.readable())  { 
				pc.read(&recchar , 1);
		#endif
				// char is read 
				cnt=0; kickWD(); 
				//printf("recchar %c \n\r",recchar);
				if ( recchar == '\r' ) continue; // ignore line feed 
				if ( recchar == '\n' ) {						
					readbuf[bufcnt]='\0';
					//printf("\n detected read done %s bufcnt = %d \n\r", readbuf,bufcnt);
				}		
				else {
					if ( bufcnt == RDBUFSIZE-1) { readbuf[0]='\0';recchar='\n' ;}
					else { readbuf[bufcnt]=recchar;
							//printf("%c , bufcnt %d  %c\r\n" , recchar, bufcnt,readbuf[bufcnt]);
							bufcnt++;
					}
				}
			}
	} // end while check for \n 
			
	return bufcnt;
}	





int main(void) { 
#ifdef __PICO__    
       stdio_init_all();// pico init 
       //watchdog_enable(40000, 1); //40 s 
       multicore_launch_core1(core1_entry);
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
    	//printf("start reading input \r\n");
        read_noneblocking( buffer);
       	// buffer[valread]='\0'; assume end with \0
       	valread=strlen(buffer);
  		//printf("This is from the client : %s length %d  expect %d \n\r",buffer,strlen(buffer),valread );
		//printf("got message nr %d ,  %s with length %d\n\r",lc2,buffer, valread);
		/* if ( strcmp( message, "HardReset") == 0 ) {
			STAYLOOP = false;
			printf("perform a hard reset\r\n");
			wait_for_ms(10);
			NIVC_SystemReset();
		} */
	  	if (valread > 0) {
	  		env_scpi_execute_command( buffer, valread);
	  		kickWD();
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
