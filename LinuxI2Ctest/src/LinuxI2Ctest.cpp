#include <cstdio>
#include <cstdlib>
#include "dev_interface_def.h"
#include "LinuxI2CInterface_a.h"

char *filename = (char*)"/dev/i2c-1";
LinuxI2CInterface  mbedi2c(filename);
LinuxI2CInterface* mbedi2cp= &mbedi2c;

int main () {

char readbuf[20];
int address = 144; 

mbedi2c.write ( address, readbuf, 2, false);


mbedi2c.read ( address, readbuf, 2, false);


mbedi2c.read ( address, readbuf, 2, false);


return 0;
}