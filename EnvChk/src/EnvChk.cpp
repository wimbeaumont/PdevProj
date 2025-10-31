
/** program to check the environment status
 *  first usage  ,  Sensor IV test 
 *  Read temperature and humidty via RS232 software 
 *  if char g is send the program response with a string 
 *  Tx  temperature of sensor x 
 *  Hx  humidity of sensor x 
 * (C) Wim Beaumont Universiteit Antwerpen 2019 
 * v0.81 
 * v0.82 init Always_Result with  true
 * v0.84 watchdog   
 * V0.90 moved to  MBED OS   ( getc  chaned to read ) 
 */ 

#define ENVCHKVER "0.90"

#ifdef __MBED__ 
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

UnbufferedSerial pc(USBTX, USBRX);

#include "I2C.h"
#include "I2CInterface.h"
#include "MBEDI2CInterface.h"  

#else 

#include <cstdio>
#include "DummyI2CInterface.h"  

#endif //__MBED__

#include  <time.h>
#include <stdlib.h>     /* atof */

 
#include "dev_interface_def.h"
#include "AT30TSE75x.h"
#include "hts221.h"
#include "veml7700.h"

MBEDI2CInterface mbedi2c( SDA, SCL); 
MBEDI2CInterface* mbedi2cp=  &mbedi2c ;
I2CInterface* i2cdev= mbedi2cp;

DigitalOut rled(LED1);
DigitalOut gled(LED2);
DigitalOut bled(LED3);

DigitalIn tal1(PTB7, PullUp);
DigitalIn tal2(PTA6, PullUp);

DigitalOut WDEn( PTB11,1);
DigitalOut WDI( PTB8,1);

// globals for char control 
bool  Get_Result = false ;
bool  Always_Result = true ;
bool  STAYLOOP =true;
void callbackpc() {
    // Note: you need to actually read from the serial to clear the RX interrupt
    char c;
    pc.read(&c,1);
    //printf("%c\n",c );
    if( c == 'g'){Get_Result = true;   } // go readout the sensors 
    if( c == 'c') {Always_Result= true ;} // set /unset countinous readout 
    if( c == 's') {Always_Result =false;} // set to single readout ( start readout with go 
    if( c == 'r') {  STAYLOOP=false;} // read settings ,restart measuring 
 }

void re_arm_wd( DigitalOut  pin) {
    pin=0;
    wait_us(2);
    pin=1;
    wait_us(2);
    pin=0;
    wait_us(2);
    pin=1;
       
}

void print_buf_hex( char *data, int length){
    int nr;
    
    char *ptr=data;
    for ( int lc=0; lc < length ; lc++){
        nr= (int) *(ptr++);
        printf( "%02x ",nr);
    }
    printf("\n\r");
}        

/* extends the AT30TSE75x class 
    * configure with the max resolution and other settings 
    * reads during initialization the temperature correction 
    * add new function , getTemperature, that returns the temperature with temperature correction 
*/ 
class  AT30TSE75x_E :public  AT30TSE75x {
    
public : 
    AT30TSE75x_E(I2CInterface* i2cinterface,  int device_address_bits, int eepromsize=2)
            :AT30TSE75x(i2cinterface,device_address_bits, eepromsize) , err_status(0) {
        int i2cerr;        
        // set configs 
        if( getInitStatus()) { err_status = -10 ; return; }
        // as I2C is working just assume these works , don't check each step 
        set_resolution(12 , i2cerr );
        set_FaultTollerantQueue('1', i2cerr );
        set_AlertPinPolarity(0,i2cerr);
        set_AlarmThermostateMode(0,i2cerr);
        set_THighLimitRegister(i2cerr, 30.40);
        set_TLowLimitRegister(i2cerr, 28.3);
        set_config(i2cerr,0);   
        if( i2cerr ) { err_status=i2cerr; }
        else { 
             char tc_str[16];
             i2cerr=read_eeprompage(tc_str, 16, 0, (uint8_t) 3); // read the register with the temperature correction
             if(i2cerr){
                    err_status=i2cerr;
                    printf("eeprom read error %d addr %d  \n\r",i2cerr, getTaddr());
                    temperature_cor=0.0;
                }
                else { // temperature correction string is valid 
                    // temperature  correction 
                    tc_str[7]='\0';
                    float tempcor=atof(tc_str);
                    if ( tempcor < -2.5 || tempcor > 2.5){  temperature_cor=0.0;err_status=-20;} 
                    else { temperature_cor=tempcor;}
                }
         } //end  set config and tempearature correction
    } // end constructor 

    float temperature_cor ; 
    int err_status;
    float getTemperature(void  )  {
         int i2cerr;
         float rT=-300.0; 
         if ( ! err_status ) {
            float Tmp=get_temperature(i2cerr);
            if ( i2cerr ) {  err_status =i2cerr; } 
            else rT=Tmp+temperature_cor; 
        }
        return rT;
    }// end getTemperature

}; // end class 
#define  WDACT 1  // if 0 the WD will be enabled if 1 stays always disabled 

int main(void) { 
   
   const int nr_Tsens=2;
   getVersion gv;
   time_t seconds;
   int waitms;
   gled=1; 
   rled=1;
   bled=0;
   printf("Check Env  program version %s, compile date %s time %s\n\r",ENVCHKVER,__DATE__,__TIME__);
   printf("getVersion :%s\n\r ",gv.getversioninfo());
   VEML7700 luxm ( i2cdev);
   printf ( "VEML7700 lib version :%s\n\r ",luxm.getversioninfo());
   int err=luxm.get_status( );
   if( err) {
        printf("get error %d after init \n\r", err);
        printf ( "VEML7700 lib version :%s\n\r ",luxm.getversioninfo());
    }
   else {
       luxm.set_integrationtime(1000); // > max  
       luxm.set_gain(3); // > max  
   }      
   HTS221 shs ( i2cdev);
   printf ( "HTS221 lib version :%s\n\r ",shs.getversioninfo());
   int id=(int) shs.ReadID();
   printf("Who Am I returns %02x \n\r", id);  
   AT30TSE75x_E tid[2] ={ AT30TSE75x_E( i2cdev ,1), AT30TSE75x_E( i2cdev ,2)};
   printf ( "AT30SE75x version :%s\n\r ",tid[0].getversioninfo());  // will be the same for both sensors 
   for ( int lc=0 ; lc < nr_Tsens ; lc++) {           
        printf( "Taddr %x , Eaddr %x \n\r ", tid[lc].getTaddr(),tid[lc].getEaddr());
        if(  tid[lc].err_status  ){ 
            printf("reading config registers failed \n\r");
                if ( tid[lc].err_status== -20 ) { bled=0;  tid[lc].err_status=0 ; }
                else { rled=0; gled=1;} //  major failure 
        }
   } 
   seconds=1568688881;
   set_time(seconds);  // Set RTC time to Wed, 5 september 2019 12:22
   seconds = time(NULL);
   printf("started at  : %s \n\r",ctime(&seconds));
   waitms=1000;
   gled=0;
   bled=1;
   pc.attach(&callbackpc);
   while(1){    
    re_arm_wd( WDI);
    int lc2=0;     
    while(STAYLOOP ) {
     seconds = time(NULL);
     if( Get_Result or Always_Result) {
        //printf("l %d t %d %s ",lc2,seconds,ctime(&seconds) );
        printf("l %06d t %d ",lc2,seconds);

        float hum, Temp;
        for (int lc=0; lc<nr_Tsens ;lc++) {
             WDEn=WDACT;
             Temp=tid[lc].getTemperature( );
             WDEn=1;
             printf( "T%d %.2f ", lc, Temp);
        }
        WDEn=WDACT;
        shs.GetHumidity(&hum);
        shs.GetTemperature(&Temp);
        re_arm_wd( WDI);
        printf("T%d %.2f H1  %.2f",nr_Tsens, Temp, hum);
        re_arm_wd( WDI);
        printf(" L %.3f",luxm.get_lux (false  ));// get the white ch in lux 
        WDEn=1;
        printf("\n\r");
        Get_Result=false;
    }
    if( Always_Result) {
        int totwait=waitms;
        while  (totwait > 0) {
        //wait_ms(waitms);
            re_arm_wd( WDI);
            i2cdev->wait_for_ms(3);
            totwait=totwait-3;
        }
    }
    gled=1;
    i2cdev->wait_for_ms(100);
    gled=0;
    lc2++;
  } //while stayloop
  gled=1;
  bled=0;
  pc.attach(NULL);
  STAYLOOP =true;
  char dummybuf[512];
  printf("give dummy input  \n\r");  
  scanf("%s",dummybuf);
  printf("give current unix time  \n\r");  
  fflush(stdin); 
  scanf("%s",dummybuf);
  seconds= atol ( dummybuf);
  set_time(seconds);  // Set RTC time to Wed, 5 september 2019 12:22
  seconds = time(NULL);
  printf("started at  : %s \n\r",ctime(&seconds));
  printf("give ms \n\r");
  fflush(stdin); 
  scanf("%s",dummybuf);
  waitms= atoi ( dummybuf);
  gled=0;
  bled=1;
 }// while 1  
    
}   
