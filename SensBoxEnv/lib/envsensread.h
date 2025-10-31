/*
 * envsensread.h
 *  this lib handles all the i2c readings
 *  Created on: Apr 29, 2020
 * 20201104  : added  reset_i2cbus not tested on mbed 
 *      Author: wimb
 * V 1.0   used  up to April 2021
 * V 1.1   added voltage ( adc read ) support 
 * V 1.2   added compatibilty with BI project 
 */

#ifndef SENSBOXENV_LIB_ENVSENSREAD_H_
#define SENSBOXENV_LIB_ENVSENSREAD_H_
int getstatus(void);
int init_i2c_dev(void) ;
float read_temperature (int  ch);
float read_voltage (int  ch);
float read_humidity (int ch=0);
float read_luminosity (void);
void wait_for_ms(int nr);
int reset_i2cbus(void);
// these are dummys to be stay compatible with burnin box
// next will be called with chan =3 
float read_temperature_ntc(int  ch) { return read_temperature (0);}
float read_dewpoint (int ch=0) { return -300.2;}
float read_humidity_adc(int ch=0){ return read_humidity ( ch) ;}
int   read_io(int chan) {return 0;}
int   write_io(int chan,int setvalue) {return setvalue;}
 

#endif /* SENSBOXENV_LIB_ENVSENSREAD_H_ */
