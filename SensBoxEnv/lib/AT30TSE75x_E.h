#include "AT30TSE75x.h"
#include <stdlib.h>
#include <string.h>
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

