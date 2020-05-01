/*
 * envsensread.h
 *  this lib handles all the i2c readings
 *  Created on: Apr 29, 2020
 *      Author: wimb
 */

#ifndef SENSBOXENV_LIB_ENVSENSREAD_H_
#define SENSBOXENV_LIB_ENVSENSREAD_H_
int getstatus(void);
int init_i2c_dev(void) ;
float read_temperature (int  ch);
float read_humidity (void);
float read_luminosity (void);
void wait_for_ms(int nr);
#endif /* SENSBOXENV_LIB_ENVSENSREAD_H_ */
