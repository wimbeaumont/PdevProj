
/** program as interface between a client using sockets and a server with a serial port 
 *  (C) Wim Beaumont Universiteit Antwerpen 2019 2020
 *  * License see
 *  https://github.com/wimbeaumont/PeripheralDevices/blob/master/LICENSE

 *  Version history :
 *  0.1   inital version not compiled not tested only principle 
 *  1.2   added close ,  corrected message  '/0' pos
 
 */ 

#define SENSBOXENVSERVER "1.2"


#include <cstdio>
//#include <errno.h>  
#include <fcntl.h>  // File Control Definitions for serial         
#include <termios.h>// POSIX Terminal Control Definitions for serial 
#include <cstdlib>     // atof 
#include <unistd.h> 		// for sockets , serial 
#include <sys/socket.h> // for sockets
#include <netinet/in.h> // for sockets
#include <string.h> 
//#include "envsensread.h"  this program is just a sccket serial port translator 
// #include "env_scpiparser.h"
#define PORT 9090 

int serialportsetup() {
int fd=open("/dev/serial/by-id/usb-MBED_MBED_CMSIS-DAP_020002033E103E53C3ECC3AB-if01",O_RDWR | O_NOCTTY); //fd is locally defined but the open is static so also have to be closed
//int fd=open("/dev/serial/by-id/usb-MBED_MBED_CMSIS-DAP_020002033E5B3E58C3A7C3A0-if01",O_RDWR | O_NOCTTY); 
	if(fd ==-1)
     printf("\n  Error! in Opening ttyUSB0\n");
	else{ 
		printf("\n  ttyUSB0 Opened Successfully\n");
		
		struct termios SerialPortSettings;
		tcgetattr(fd, &SerialPortSettings);//get current settings 
		cfsetispeed(&SerialPortSettings,B9600);//read speed 
		cfsetospeed(&SerialPortSettings,B9600);//write speed 
		SerialPortSettings.c_cflag &= ~CSIZE ; // set char size mask to 0 
		SerialPortSettings.c_cflag |=  CS8 ; // set char size to 8 
		SerialPortSettings.c_cflag &= ~PARENB;   // No Parity
		SerialPortSettings.c_cflag &= ~CSTOPB; //Stop bits = 1 
		SerialPortSettings.c_cflag &= ~CRTSCTS; // no RTS .CTS 
		SerialPortSettings.c_lflag &= ~(ECHO | ECHOE | ECHONL) ; // Disable echo
		SerialPortSettings.c_lflag |=   ICANON;// read  full lines 
		SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY); // no flow control
		//SerialPortSettings.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any specia not expected
		tcsetattr(fd,TCSANOW,&SerialPortSettings); // write the settings to the hardware without waiting ( TCSANOW)
		//SerialPortSettings.c_cc[VMIN]  = 0; // Read 10 characters   we wait for the newline 
		SerialPortSettings.c_cc[VTIME] = 0;  // Wait indefinitely for the moment   
		
		
	}
    return fd ;
}


// globals for char control 
bool  Get_Result = false ;
bool  Always_Result = false ;



int main(void) { 
   

	// setup the socket  simple socket for 1 client
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1024] = {0};  // receive buffer 
    char message[1024];

	printf("Check Env  program version %s, compile date %s time %s\n\r",SENSBOXENVSERVER,__DATE__,__TIME__);
	
//serial port 
	int fd  = serialportsetup();
	if ( fd == -1 ) exit(-1); 

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
    	buffer[valread]='\0';//terminate string with \0   
    	printf("This is from the client : %s length %d  expect %d \n\r",buffer,(int)strlen(buffer),valread );
    	buffer[valread++]='\n';//send a new line and include it 
  	buffer[valread]='\0';//terminate string with \0   
  	tcflush(fd, TCIOFLUSH);
	  	if (valread > 0) {
	  		write(fd,buffer,valread);
			valread=read(fd, message, sizeof(message));//get full return message 
			//for ( int cc=0; cc < valread; cc++ ) { printf(" %02d",message[cc]) ;}
			//printf("\n\r");
			message[valread-2]='\0';	// messages is not terminated with \0 !!! overwrite newline
			if( strcmp( message, "STOP done") == 0 ) STAYLOOP = false;
			if( valread == 0) {
				strcpy(message, "resp message is zerro");
			}
	  	} else { 		strcpy(message, "cmd message is zerro");	}
		
		printf( "will send %s length %d reported %d \n\r", message ,(int)strlen(message)  ,valread-2);
	  	send(new_socket , message , strlen(message) , 0 );
		//printf("Have sent message %s\n\r ", message);
	  	lc2++;
    		close(new_socket); 	  	

  } //while stayloop

close (fd) ;
 

    
} // end main 
