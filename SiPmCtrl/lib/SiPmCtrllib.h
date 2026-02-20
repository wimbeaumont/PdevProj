#ifndef SIPMCTRLLIB_H
#define SIPMCTRLLIB_H

/** program to controll the bias and trigger levels for a two channel 

  * (C) Wim Beaumont Universiteit Antwerpen 2019
 *  License see
 *  https://github.com/wimbeaumont/PeripheralDevices/blob/master/LICENSE
 */ 
#if defined  __MBED__ 
#define  OS_SELECT "MBED" 
#elif defined __LINUX__
#define  OS_SELECT "linux_i2c" 
#else 
#define  OS_SELECT "linux_dummy" 
#endif

#define SIPMCTRLLIB "1.6"

int setBiasVoltage(int ch , float volt ) ;

int setDiscriminatorLevel( int ch, float volt);



#endif
