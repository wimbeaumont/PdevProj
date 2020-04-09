/** program to controll the bias and trigger levels for a two channel 
 *  SiPm readout board 
 *  It compiles with Linux ( Raspberry Pi) and most likely it will work typing in the correct 
 *  command , but it is mainly targeting the MBED serial communication. (as intital test program to debug the board) 
 *  for the Raspberry I started with the file SiPmCtrl 
 *  for more info see the README.MD of the repository 
 *  https://github.com/wimbeaumont/peripheral_dev_tst.git
 *
 *  V 1.0  : copied from LTC2633_tst.cpp 
 *  V 1.24  : working with the hardware simple increasing
 *  V 1.63  : with commands via the serial interface,   MBED targeting . 
 *  V 1.64  	Apr 2020 put  "ltc2633.h" as this defines types ( linux) still targeting MBED  
 * (C) Wim Beaumont Universiteit Antwerpen 2019
 *  License see
 *  https://github.com/wimbeaumont/PeripheralDevices/blob/master/LICENSE
 */ 
 

#define LTC2633_SIPM_CTRL "1.64"


// #include "LTC2633setaddr.h"
#include "ltc2633.h"

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
#include <string.h>
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


//#include <stdlib.h>



#define MAX_ARG 5
#define BUFFSIZE 14 

class decodecmd {


public :
enum DEVSEL {BIAS=0 , LVL=1 ,INVALIDDEV=99 } ;
enum ACTIONTYPE { NOACT=0, SETDAC_V=1 , SETDAC_D=2 , INVALIDACT=99 };

int ch;
DEVSEL device;
float value;
char response[30];
ACTIONTYPE action;
char* splitstr[MAX_ARG];
char splstr0[BUFFSIZE],splstr1[BUFFSIZE],splstr2[BUFFSIZE],splstr3[BUFFSIZE],splstr4[BUFFSIZE];

decodecmd(){
	splitstr[0]=splstr0;
	splitstr[1]=splstr1;
	splitstr[2]=splstr2;
	splitstr[3]=splstr3;
	splitstr[4]=splstr4;
};


int   splitcmdstr(const char* s, const char& c){
	
    char buff[BUFFSIZE];
	int bufcnt=0;
	int bufcharcnt=0;
	unsigned int k=strlen(s);
    for (unsigned int i = 0; i < k /*strlen(s) */&& bufcnt < MAX_ARG; i++) {
        char n=s[i];
        if(n != c && bufcharcnt < BUFFSIZE-1 ) buff[bufcharcnt++]=n; else
        	if(strlen(buff) != 0) {buff[bufcharcnt]='\0'; bufcharcnt=0; strcpy( splitstr[bufcnt++],buff); }
    }
	return bufcnt;
}




void request_action(char* reqtype ,char* dev_str , char* ch_str, char* value_str  ){
    if ( strcmp(reqtype, "VER") == 0 ) {action=NOACT;  value= 1.0; ;strcpy(response,reqtype);  return; }
    if ( strcmp(reqtype, "RES") == 0 ) {action=NOACT; strcpy(response,reqtype); return ; }
    if ( strcmp(reqtype, "TST") == 0 ) { action=NOACT; value=1.23 ; strcpy(response,reqtype); ; return;}
    strcpy(response,"reqerr"); action=INVALIDACT;
}



void ch_assign(char* dev , char* chstr  ){
		strcpy(response,"success"); // assume success
        device=INVALIDDEV;
        if (  strcmp(dev, "BIAS") == 0 ) { device = BIAS;}
        else if ( strcmp(dev, "LVL") == 0  ) {  device =LVL; }
        if (device== INVALIDDEV ) { strcpy(response,"devselerr");}
        else if (strcmp(chstr,"0")==0 ) { ch=0; }
             else if (strcmp(chstr,"1")==0 ) { ch=1; }
                 else { ch =-1; strcpy(response,"chselerr"); }
}

void set_action( char* settype ,char* dev_str , char*  ch_str, char* value_str  ){
    strcpy(response,"seterr");
    action = INVALIDACT;
    if ( strcmp(settype,"V")==0  || strcmp(settype,"D")==0 ) {
    	if ( strcmp(settype,"V")==0)  action =SETDAC_V; else action =SETDAC_D;
        ch_assign( dev_str, ch_str) ;
        value = atof( value_str); // no check if it is a correct float
        strcpy(response,"success");
    }

}


void  handle_cmd (const char*  cmdstr  ) {
    int rcmd =0; // invalid cmd
    strcpy(response,"unknown");
    if ( splitcmdstr( cmdstr, ':') == 5) {
        if ( strlen(splitstr[0]) == 1){  // if not cmd is invalid
          if ( splitstr[0][0] == '?' ) rcmd=1; // request
          if ( splitstr[0][0] == '!' ) rcmd=2; // set
          switch (rcmd ) {
              case 0 :  {strcpy(response , "invalid") ;}  break;
              case 1 :  { request_action(splitstr[1],splitstr[2],splitstr[3],splitstr[4]); } break;
              case 2 :  {     set_action(splitstr[1],splitstr[2],splitstr[3],splitstr[4]); } break;
              default : {strcpy(response , "unkown") ;}  break;
           }
        }
    } else {strcpy(response ,"cmdformaterr") ; action=INVALIDACT; device=INVALIDDEV ; ch =-1;     }
}

} ;// end class


I2CInterface* i2cdev= mbedi2cp;
const float Vrefext=5.0;

using namespace std;


int main(void) {
  
   int bias_ctrl_addr= 0x12;  //CAO at 5 Vdd
   int discr_lvl_addr= 0x10;  //CAO at 0 Vdd
   printf("SiPm ctrl version %s, compile date %s time %s for OS %s\n\r",LTC2633_SIPM_CTRL,__DATE__,__TIME__,OS_SELECT);
   printf("I2C interface version  :%s\n\r ",i2cdev->getversioninfo());
   int Vreftype=1 , resolution=12;
   LTC2633  biasctrl(i2cdev, bias_ctrl_addr,  Vrefext ,Vreftype , resolution  );
   int errcode=biasctrl.setDACvalue(0,0);biasctrl.setDACvalue(0,1);
   LTC2633  discr_lvl(i2cdev, discr_lvl_addr,  Vrefext ,Vreftype , resolution  );
   printf("\n\raddr %d err %d , LTC2633 :%s\n\r", bias_ctrl_addr,errcode,biasctrl.getversioninfo());
   i2cdev->wait_for_ms(1000);
   
	  // now the same with the DAC interface 
  DACInterface* di[2] = {  &biasctrl,&discr_lvl};
  
  decodecmd cmd; // no init
   char cmdinchar[50] ;
 
   while ( 1) {
		scanf("%s",cmdinchar);
		cmdinchar[49]='\0';
		//printf( "%s", cmdinchar);
		cmd.handle_cmd ( cmdinchar) ;
		switch ( cmd.action) {
			case decodecmd::SETDAC_D : {
					 if ( di[(int)cmd.device]->setDACvalue((int)cmd.value, cmd.ch) ) {
						 strcpy(cmdinchar,"i2cerr");
					 }else {
						 strcpy(cmdinchar,cmd.response);
						 strcat(cmdinchar,"_I2C");
					 }
			} break;
			case decodecmd::SETDAC_V : {
					 int  voltset;
					 if( cmd.device ==  decodecmd::BIAS ) { cmd.value =  cmd.value/0.0342 ;}
					 if ( di[(int)cmd.device]->setDACvalue((int)cmd.value, cmd.ch) ) {
						 strcpy(cmdinchar,"i2cerr");
					 }else {
						 strcpy(cmdinchar,cmd.response);
						 strcat(cmdinchar,"_I2C");
					 }
			} break; 				
			default :	strcpy(cmdinchar,cmd.response);
		};
		cmdinchar[49]='\0';
		printf( "%d:%f:%s",(int)cmd.action,cmd.value, cmdinchar);
   }
  
    // never reach this   
    return 1;
}   
