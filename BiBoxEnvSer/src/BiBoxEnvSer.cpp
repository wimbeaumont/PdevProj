
/** program to check the environment status
 *  first usage  ,  Sensor IV test 
 *  BI box is an fork of version 2.6 
 *  Read temperature and humidty via RS232 
 *  Using the SCPI protocol 
 *  This will be target to pico (v1)  
 *  (C) Wim Beaumont Universiteit Antwerpen 2019 2025
 *  * License see
 *  https://github.com/wimbeaumont/PeripheralDevices/blob/master/LICENSE
 
 *  Version history :
 *  0.1   inital version form fork from  SensBoxEnvSer only compilation check 
 *  0.2   this will  mainy target the pico platform  
 *  0.3   compiled and linked , added  Pico digital out 
 *  0.4   checking 
 */ 

#define BIBOXENVSERVER "0.46"

// OS / platform  specific  configs 
#ifdef __PICO__ 
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"
#include "PWM_PICO.h" 
//#elif defined  __MBED__ 
//#define  OS_SELECT "MBED" 
//#include "mbed.h"
#endif //
// i2c devices are defined at a lower level ( envsensread.cpp)
#if defined  __MBED__  // just for compatibility  but not maintained 

DigitalOut rled(LED1);
DigitalOut gled(LED2);
DigitalOut bled(LED3);

BufferedSerial pc(USBTX,USBRX);

#else //end __MBED__ 

int rled ;
int gled ;
int bled ;
#endif //


#include <stdlib.h>     // atof 
#include <string.h> 
// this contain the init devices ,setup the I2C devices 
#include "bi_sensread.h"
#include "bi_cpuread.h" // this contains the readout functions for hw resources
#include "env_scpiparser.h"
#include "hum2dewputils.h"
#include "Sens_hwstatus.h"

// globals for char control 
bool  Get_Result = false ;
bool  Always_Result = false ;
const unsigned int RDBUFSIZE=256;

int interface_status=0 ;// interface are not initialized

void kickWD(void) {
#ifdef __PICO__ 
watchdog_update();
#elif defined  __MBED__ 
	SIM->SRVCOP=0x55;
	SIM->SRVCOP=0xAA;
#endif
}

// serial buffer flush 
#ifdef __PICO__    
void  serial_bufferflush(void) {stdio_flush();}
#else 
void  serial_bufferflush(void) {pc.sync(); }//flush
#endif
// default values if not initialized 
float  Rntc_now;
float Hum=110; // humidity 
float Tp=-300; //temperature 
float Vsys=0;// 5V power , ref for temperature 
float dp=-300; //dewpoint  -300 if not iniitalized 
float Tint=-300; // internal temperatur


// temp functions 
//float read_dewpoint(int ch=0);
float read_dewpoint(int ch=0) { return dp; }
float read_temperature_ntc(int ch) {return Tp; } // read the temperature from the NTC sensor
float read_humidity_adc(int ch=0) { return Hum; }

#ifdef __PICO__ 
enum ADCINP {HUMIN=0, TEMPIN=1 , VSYSIN=2,NOTCON=3,TINT=4 }; // this are the ADC channels not the pins!! 


void core1_entry() {
	// contiously read of the NTC and humidyt sensor and ouptut this to the PWM outputs
	// to mimic a analogue dewpoint sensor 
	PWM_PICO pwmled( PICO_DEFAULT_LED_PIN,10000);  //GPIO 25 
	float delta_l=.05 ;
	float dc_set;
	int msleeptime=500;
	PWM_PICO  outT(10,10000); // have to check for  optimal frequency 
    PWM_PICO  outDP(11,10000);
	outT.init_PWMVout(-40,50,0,3.3);
    outDP.init_PWMVout(-60,20,0.3636,1.818); // for 20mA output equivalent
	update_hwstatus(HWstatus_core1);
	while(1) {
		// the alife LED 
		for (float dc =0; dc<100; dc+=delta_l) {
			if( delta_l < 2.5)delta_l=1.05*delta_l; // increase the delta each time
			dc_set=pwmled.set_dutycycle(dc);
		}
		kickWD();
		//if (interface_status 	) {
		if (true 	) {
			//Vsys=adc2Vsys(read_voltage(VSYSIN) );
			//Tp= adc2Tp(read_voltage(TEMPIN), Rntc_now);
			Hum=adc2Hum(read_voltage(HUMIN ),Tp);
			dp=dewpoint(Hum,Tp);
			outT.set_PWMVout(Tp);
			outDP.set_PWMVout(dp);
			//Vsys=adc2Vsys(read_voltage(VSYSIN) );
			Tint=adc2Tint(read_voltage(TINT) );
			Vsys=Tint; // for the moment to check
        }
		Tp=Tp+0.1; // just to have some change in the output
		sleep_ms(msleeptime);
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

		kickWD();// as long in the reading routine don't init a WD restart 
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
				//printf("recchar %c \r\n",recchar);
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
				
			}// end check if char is present
			if (cnt > 5000 ) {// stop reading 
				bufcnt=0; recchar='\n' ;
				serial_bufferflush();
			}
	} // end while check for \n 
			
	return bufcnt;
}	


int main(void) { 
#ifdef __PICO__    
     stdio_init_all();// pico init 

     multicore_launch_core1(core1_entry);

    if (watchdog_caused_reboot()) {
        printf("Rebooted by Watchdog ");
        update_hwstatus( HWstatus_wdrestart);
    }else update_hwstatus( HWstatus_wdrestart, false);
	// Enable the watchdog, requiring the watchdog to be updated every 5100ms or the chip will reboot
    // second arg is pause on debug which means the watchdog will pause when stepping through code
    //watchdog_enable(5100, 1);

#endif 



   char buffer[512] = {0};  // receive buffer 
   char* message= buffer;
   int valread=0;
   int rdbufcnt=0;	
	// initialize the I2C devices   
    int interface_status=init_i2c_dev(); 
	if (interface_status ) {
		update_hwstatus(HWstatus_init);
	}else {
		update_hwstatus(HWstatus_init,false);
	}
    //printf("i2cinit done with status %d \n\r",status);
    char hwversion[]=BIBOXENVSERVER;	
    scpi_setup( hwversion);// initialize the parser
    int lc2=0;  
    bool  STAYLOOP =true;
    while(STAYLOOP ) {
    	//printf("start reading input \r\n");
        rdbufcnt=read_noneblocking( buffer);
        if (rdbufcnt) {  // something is read else rdbucnt=0 
			valread=strlen(buffer);
			//printf("This is from the client : %s length %d  expect %d \n\r",buffer,strlen(buffer),valread );
			//printf("got message nr %d ,  %s with length %d\n\r",lc2,buffer, valread);
			//if ( strcmp( message, "HardReset") == 0 ) {STAYLOOP = false;	wait_for_ms(10);NIVC_SystemReset();	} 
			if (valread > 0) {
				env_scpi_execute_command( buffer, valread);
				kickWD();
				strcpy(message,	env_get_result());
				//if( strcmp( message, "STOP done") == 0 ) {STAYLOOP = false;}
				if(strlen(message)== 0) { strcpy(message, "wrong SCPI cmd");}
			} else {
				strcpy(message, "MB message is zerro");
			}
 	  	rled=1;	gled=0;bled=1;

		}else {  // time out none blocking read 
		 rled=1;gled=1;bled=0;	
		}
		
	  	//printf( "will send %s length %d \n\r ", message ,strlen(message)  );

	  	printf("%s\r\n", message );//need new line for the receiver , is waiting for that 
	  	//kickWD();
		//printf("Have sent message %s\n\r ", message);
		serial_bufferflush();
 	  	lc2++;
  } //while stayloop


 

    
} // end main 
