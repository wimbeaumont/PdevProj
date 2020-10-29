# PdevProj
projecs using the PeripheralDevices

This programs that use Perhipheral devices for projects 

These can be used as examples how to use the libraries. 

Be aware that in most cases this only the development phase as the final code has to move to the repository of the project.

In respect to the  peripheral_dev_tst  these projects can target a specific platform or operating system. 

For the cmake go to the cmake/build directory and do cmake ../

The cmake relies that the following repositories are present : 

*   https://github.com/wimbeaumont/PeripheralDevices   The libs for the I2C devices
*   https://github.com/wimbeaumont/PdevUtils 

These reposotories should be in the same director as this reposotory 
   

The projects 

*   LTC2633_app    just some trails 
*   SiPmCtrl  this is for a PCB to which a SiPm can be connected, containing amplifiers discriminator and bias . The LTC2633 is used to to set levels
*   LinuxI2Ctest  to check some I2C functions  ( with a oscilloscope) 
*   SensBoxEnvSimple  readout a a number of sensors are response with one string containing all the measurements on any request communicaton with sockets to a cleint 
*   SensBoxEnv , readout the same sensors as SensBoxEnvSimple but wait for a scpi formated command to respond with the info requested by the command. 
*   SensBoxEnvMbed  fork of the SensBoxEnv for MBED  (KL025Z )  communication via serial interface
*   SensBoxEnvSer   fork of SensBox, translation between socket and serial 




