
/** program to check the environment status
 *  first usage  ,  Sensor IV test 
 *  Read temperature and humidty via RS232 software 
 *  if char g is send the program response with a string 
 *  Tx  temperature of sensor x 
 *  Hx  humidity of sensor x 
 *  This is a fork from the prog EnvChk on "my"  MBED repository 
 *  The original program was developed for the FRMD KL05Z 
 *  This program targets the Raspberry Pi as this is the target machine 
 *  for the Sensor box control.  So moving this to the RP saves hardware 
 *  (C) Wim Beaumont Universiteit Antwerpen 2019 2020
 *  * License see
 *  https://github.com/wimbeaumont/PeripheralDevices/blob/master/LICENSE

 *  Version history :
 *  0.1   inital version 
 *  0.2   correction , removed all the MBED LED commands 
 *  0.3   dummy compiles 
 *  0.4   remove all the MBED as this will target RP   reason to simplify the code 
 */ 

#define SENSBOXENVVER "0.2"


#include <cstdio>
#include  <time.h>
#include <cstdlib>     /* atof */
#include <unistd.h> 		// for sockets
#include <sys/socket.h> // for sockets
#include <netinet/in.h> // for sockets
#include <string.h> 
#define PORT 9090 

#if defined __LINUX__ 
#define  OS_SELECT "linux_i2c" 
#include "LinuxI2CInterface.h"

char *filename = (char*)"/dev/i2c-1";  //hard coded for the moment 
LinuxI2CInterface  mbedi2c(filename);
LinuxI2CInterface* mbedi2cp= &mbedi2c;

//------------------ end Linux I2C specific config
#else 
#define  OS_SELECT "linux_dummy" 
#include "DummyI2CInterface.h"   
DummyI2CInterface  mbedi2c;
DummyI2CInterface* mbedi2cp= &mbedi2c;
#endif
#ifndef OS_SELECT 
#define OS_SELECT "linux_dummy" 
#endif
// --- end platform specific configs 



#include "dev_interface_def.h"
#include "AT30TSE75x.h"
#include "hts221.h"
#include "veml7700.h"  // differs from the MBED version


I2CInterface* i2cdev= mbedi2cp;



// globals for char control 
bool  Get_Result = false ;
bool  Always_Result = false ;
bool  STAYLOOP =true;


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


int main(void) { 
   
   const int nr_Tsens=2;
   getVersion gv;
   time_t seconds;
   int waitms;
	// setup the socket  simple socket for 1 client
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1024] = {0};  // receive buffer 
    char message[1024];
    // Creating socket file descriptor 
/*    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))  { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
	// socket setup done 	


*/
	// initialize the I2C devices   
   printf("Check Env  program version %s, compile date %s time %s\n\r",SENSBOXENVVER,__DATE__,__TIME__);
   printf("getVersion :%s\n\r ",gv.getversioninfo());
   VEML7700 luxm ( i2cdev);
   printf ( "VEML770 lib version :%s\n\r ",luxm.getversioninfo());
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
                if ( tid[lc].err_status== -20 ) {   tid[lc].err_status=0 ; }
                else { } //  major failure 
        }
   } 
	// i2c devices setup dome 

   seconds=time(NULL);
   printf("started at  : %s \n\r",ctime(&seconds));
   waitms=1000;

   while(1){

    int lc2=0;     
    while(STAYLOOP ) {

	  // socket waiting 	
		
/*	  if (listen(server_fd, 3) < 0) {  perror("listen");   exit(EXIT_FAILURE);	 } 
     if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){ 
        perror("accept");   	     exit(EXIT_FAILURE); 
     } // blocking socket  
	  valread = read( new_socket , buffer, 1024); 

     printf("This is from the client : %s\n",buffer ); //buffer has to be interpreted 

*/
	  // read the I2C devices 
     float hum, Temp;
     seconds = time(NULL);
	  sprintf(message,"time = %s ",ctime(&seconds) 	);
		// remove the new line    
		char * p = strchr(message,'\n');	if ( p)  { *p = '\0' ;}


printf("before read temp\n");  
     for (int lc=0; lc<nr_Tsens ;lc++) {
 	         Temp=tid[lc].getTemperature( );
             sprintf(message, "%s T%d %.2f ",message, lc, Temp );
     }


printf("after read temperature %s \n\r",message);
        shs.GetHumidity(&hum);
        shs.GetTemperature(&Temp);

printf("after read humidity \n\r");
        sprintf(message,"%s T%d %.2f H1  %.2f  L %.3f\n\r",message,nr_Tsens, Temp, hum,luxm.get_lux (false));
		  	
		  printf("%s",message);
//		  send(new_socket , message , strlen(message) , 0 ); 		
        Get_Result=false;
    if( Always_Result) {i2cdev->wait_for_ms(waitms);}
    
    i2cdev->wait_for_ms(100);
  
    lc2++;
  } //while stayloop
 

  STAYLOOP =true;
  char dummybuf[512];
  printf("give dummy input  \n\r");  
  scanf("%s",dummybuf);
  printf("give current unix time  \n\r");  
  fflush(stdin); 
  scanf("%s",dummybuf);
  seconds= atol ( dummybuf);

  seconds = time(NULL);
  printf("started at  : %s \n\r",ctime(&seconds));
  printf("give ms \n\r");
  fflush(stdin); 
  scanf("%s",dummybuf);
  waitms= atoi ( dummybuf);
 
 }// while 1  
    
} // end main 
