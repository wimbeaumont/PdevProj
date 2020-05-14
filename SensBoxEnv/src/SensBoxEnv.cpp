
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
 *  0.5   Apr 2020 9 works with RP  good version for debuging 
 *  0.6   Apr 2020 10 start to optimize for use, works with simpe socket communication
 *  0.7   forked fro sensboxenv simple , add scpi
 
 */ 

#define SENSBOXENVVER "0.7"


#include <cstdio>
#include <cstdlib>     // atof 
#include <unistd.h> 		// for sockets
#include <sys/socket.h> // for sockets
#include <netinet/in.h> // for sockets
#include <string.h> 
#include "envsensread.h"
#include "env_scpiparser.h"
#define PORT 9090 


// globals for char control 
bool  Get_Result = false ;
bool  Always_Result = false ;



int main(void) { 
   
   //const int nr_Tsens=2;



   //int waitms;
	// setup the socket  simple socket for 1 client
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1024] = {0};  // receive buffer 
    char message[1024];

	printf("Check Env  program version %s, compile date %s time %s\n\r",SENSBOXENVVER,__DATE__,__TIME__);

	// initialize the I2C devices   
    int status=init_i2c_dev();
    if ( status ){
    	printf("failed to init i2c devices returns %d \n\r",status);
    	return -1;
    }
    scpi_setup();// initialize the parser

    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
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



    int lc2=0;  
    bool  STAYLOOP =true;
    while(STAYLOOP ) {
    	// socket waiting
    	if (listen(server_fd, 3) < 0) {  perror("listen");   exit(EXIT_FAILURE);	 }
    	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
    		perror("accept");   	     exit(EXIT_FAILURE);
    	} // blocking socket
    	valread = read( new_socket , buffer, 1024);
    	buffer[valread]='\0';
  		//printf("This is from the client : %s length %d  expect %d \n\r",buffer,strlen(buffer),valread );
	  	if (valread > 0) {
	  		env_scpi_execute_command( buffer, valread);

	  		strcpy(message,	env_get_result());
	  		if( strcmp( message, "STOP done") == 0 ) STAYLOOP = false;
	  		if(strlen(message)== 0) { strcpy(message, "wrong SCPI cmd");}
	  	} else {
	  		strcpy(message, "message is zerro");
	  	}
	  	//printf( "will send %s length %d \n\r ", message ,strlen(message)  );
	  	send(new_socket , message , strlen(message) , 0 );
		//printf("Have sent message %s\n\r ", message);
    
	  	wait_for_ms(100);
  	  	
	  	lc2++;
  } //while stayloop


 

    
} // end main 
