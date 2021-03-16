This program is for a MBED processor. 

It uses the serial port to exhange  environment sensor data, using a SCPI protocol 
 
Tested with mbed-cli  on linux and the KL25z  platform (easiest way to install is with `python3 -m pip install mbed-cli`) 

mbed-cli  is using a repository system, git by default.  This feature should be able to disable when starting a new project but seems not always to work.

So I make a working directory 

~/mbedtst/common$

( I put there the mbed-os libs) 
make  a project dir:    mbed new SensBoxEnvMbed

In this dir make a soft link to SensBoxEnvMbed.cpp and to mbed_app.json  (this is important as it instruct to use the bare metal profile. with the standard mbed os6 the KL25z runs out of heap. 

In the project dir  after mbed new  there is also SensBoxEnvMbed  

```
To compile I use the command mbed compile -t GCC_ARM -m KL25z --source SensBoxEnvMbed/ --source /home/wb/github/PdevProj/SensBoxEnv/lib/ --source /home/wb/github/PeripheralDevices/ --source mbed-os --build ./BUILD/KL25Z/GCC_ARM
``` 

The GCC ARM cross compiler is in the PATH variable. 

The program is used together with SensBoxEnvSer that is in principle a socket to serial interface translator. 








