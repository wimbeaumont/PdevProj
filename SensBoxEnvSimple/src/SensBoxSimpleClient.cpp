
// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#define PORT 9090 
   
int main(int argc, char const *argv[]) 
{ 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    //char *hello = "Hello from client"; 
    char hello[1024];
    char buffer[1024] = {0}; 
/*    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
*/   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
    int lc; 
	 while(lc++ < 100) {    
		 if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)  	 { 
   	     printf("\n Socket creation error \n"); 
   	     return -1; 
   	 } 
   	 if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
   	 { 
   	     printf("\nConnection Failed \n"); 
   	     return -1; 
   	 }

	    
 		 sprintf(hello,"message from client nr %d\n ",lc);
		 send(sock , hello , strlen(hello) , 0 );
		 //write( sock , hello , strlen(hello)) ; 	
   	 printf("Hello message sent %d \n",lc); 
   	 valread = read( sock , buffer, 1024); 
   	 printf("this is the response from the server : %s\n",buffer ); 
		 close(sock);  	
		 sleep(1); 
		 
    }

  return 0; 
} 

