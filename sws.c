/*
* sws.c
* simple web server
* CSC 361
* Yaxi Yu 
* V00828218
* Instructor: Kui Wu
*/
//----------------------------------Include---------------------------------------
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h> /* for close() for socket */ 
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h> //directory
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h> 


//-----------------------global--varaiable--and-structure-------------------------
char *address;
int portNum;
char buffer[1024];
char recvbuff[1024];
struct sockaddr_in sa;
socklen_t fromlen;
ssize_t recsize;
char threeInput[3][100];//store each word into an array
                        //eg.  "GET"  "/index.html" "HTTP/1.0"
ssize_t fileSize;
char *fileNName;
//-----------------------------------methods--------------------------------------

//this method is on the connex lab5
void readFile(char *filename){
    size_t bytes_read;
    long int file_size;
    FILE *input = fopen (filename, "r");//r: "read: open file for input operations. the file must exist" 
    //determine the requested file size
    fseek (input, 0L, SEEK_END);
    file_size = ftell(input);
    fileSize=file_size;
    fseek (input, 0L, SEEK_SET);
    //reads the file into the buffer
    bytes_read = fread (buffer, sizeof(char), fileSize, input);
    fclose (input);
}


void cleanExit(){
    exit(0);
}

void getTime(){

    time_t t;
    time(&t);
    char *a= ctime(&t);
    char *b;
    b = strtok (a," ");
    a = strtok (NULL, " ");
    printf("%s ",a);  

    struct tm tm = *localtime(&t);
    printf ("%d %d:%d:%d", tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    time_t rawtime;

}


bool checkPath(char *location){//return true if file is found, false else.

    int result=access(location, F_OK);
    if(result==0){
    	return true; 
    }else{
  	return false;
    }
}



/*
1. GET /index.html HTTP/1.0     # HTTP/1.0 200 OK
2. GET /nofile HTTP/1.0         # HTTP/1.0 404 Not Found
3. GET /index.html HTTP/1.1     # HTTP/1.0 400 Bad Request
*/
void analyzeBash(char Input[]){
        int count=0;
	char * pch;
	pch = strtok (Input," \r\n");
	while (pch != NULL)
	{
		strcpy(threeInput[count],pch);
		pch = strtok (NULL, " \r\n");
		count++;
	}
	count=0;//clear count
}

int outputResponse(char *location){
	//concatenate two string
	char fileN[100];
        strcpy(fileN,location);
	strcat(fileN,threeInput[1]);

	if(strcmp(threeInput[0],"GET") != 0){
		printf("HTTP/1.0 400 Bad Request; ");
		printf("%s",threeInput[0]);
		return 400;
	}else if(strcmp(threeInput[2],"HTTP/1.0") !=0 ){
		printf("HTTP/1.0 400 Bad Request; ");
		return 400;
	}else if(checkPath(fileN)==true){
		printf("HTTP/1.0 200 OK; ");
		return 200;
	}else{
		printf("HTTP/1.0 404 Not Found; ");
		return 404;
	}
}

void getRequestLine(){
	char *request;
	printf ("%s ",threeInput[0]);
	printf ("%s; ",threeInput[2]);
	//printf ("hahahhahaha\n");
}



//-------------------------------------main--------------------------------------

int main(int argc, char *argv[])
{
	  //input sample:    8080 /tmp/www
	  if(argc=!3){
		printf("invalid input\n");
		printf("input example: ./sws 8080 /home/yuyaxi/csc361/Program1\n");
		//cleanExit();
		exit(EXIT_FAILURE);
	  }
	  //read the input
	  portNum=atoi(argv[1]);
	  address=argv[2];

	  DIR* dir = opendir(address);
	  if(dir){
		  int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);//open a socket
		  if(sock<0){
			perror("ERROR in opening a socket");
			cleanExit();
		  }

		  //set up server address data
		  memset(&sa, 0, sizeof sa);
		  sa.sin_family = AF_INET;
		  sa.sin_addr.s_addr = htonl(INADDR_ANY);
		  sa.sin_port = htons(portNum);
		  fromlen = sizeof(sa);
		  
		  //try bind a socket to the data we just set
		  if (-1 == bind(sock, (struct sockaddr *)&sa, sizeof sa)) {
		    	perror("error bind failed");
		    	close(sock);
		    	exit(EXIT_FAILURE);
		  }
		  
		  printf("sws is running on UDP port %d and serving %s\n",portNum,address);
		  printf("press 'q' to quit ...\n"); 
		  
		 
		  fd_set read_fds;
		  //an infinite loop (pseudo code on connex lab5)
		  for (;;) {		
			FD_ZERO(&read_fds); // Clears the file descriptor
			FD_SET(0, &read_fds); // Add stdin's fd into read_fds
			FD_SET(sock, &read_fds); // Add servsocket's fd into read_fds	  
			if (select(sock+1, &read_fds, 0, 0, 0) == -1){
				perror("Failed to select!");
				close(sock);  
			}		
			if (FD_ISSET(0, &read_fds)){ /*If the user press a key from the console*/
		
				fgets (recvbuff, sizeof(buffer), stdin); /*Check if the first char of the input is 'q'*/
				if (recvbuff[0] == 'q'){
						close (sock);
						exit(EXIT_SUCCESS);
				}		
				else{
						printf ("Unrecognized Command.\n");
				}				
			}				
			if (FD_ISSET(sock, &read_fds)){ /*If the socket got something*/
		
				time_t ticks = time(NULL);
				//output time, clientIP,client port number
				getTime();
				printf(" %s:%d ", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
				//listen for requests
				recsize = recvfrom(sock, (void*)buffer, sizeof buffer, 0, (struct sockaddr*)&sa, &fromlen);
				if (recsize < 0) {
						fprintf(stderr, "%s\n", strerror(errno));
						exit(EXIT_FAILURE);
				}
				
				analyzeBash(buffer); // store three strings into threeInput[3][100]
				getRequestLine();    //output requestline;
				int code=outputResponse(address);//output response
				//output full directory	
				char fullDirectory[1024];
				strcpy(fullDirectory,address);
				strcat(fullDirectory,threeInput[1]);
				printf("%s\n",fullDirectory);


				if(code==404){
					strcat(fullDirectory," HTTP/1.0 404 Bad Request;\n");
					//store full directory and response code into buffer
					sendto(sock, (void*)fullDirectory, (size_t)strlen(fullDirectory) ,0, (struct sockaddr *)&sa,fromlen);
					memset(fullDirectory, 0, sizeof(fullDirectory));
					memset(recvbuff, 0, sizeof(buffer)*sizeof(char));
				}else if(code==200){
					strcat(fullDirectory," HTTP/1.0 200 OK;\n");
					sendto(sock, (void*)fullDirectory, (size_t)strlen(fullDirectory) ,0, (struct sockaddr *)&sa,fromlen);
					memset(recvbuff, 0, sizeof(buffer)*sizeof(char)); /*Clears buffer for next connection*/

					fileNName = strtok (threeInput[1],"/");
					//read the file and store it into the buffer
					readFile(fileNName);
					//if filesize<=buffer size, send it 
					if(fileSize<=sizeof buffer){
						sendto(sock, (void*)buffer, (size_t)strlen(buffer) ,0, (struct sockaddr *)&sa,fromlen);
					}else{//if filesize>buffer size, sent it as parts
						int part=0;
						while (part < fileSize){
							sendto(sock, &buffer[part], (size_t)strlen(buffer), 0, (struct sockaddr*)&sa, fromlen);
							part = (size_t)strlen(buffer)+part;
						}
					}
					memset(fullDirectory, 0, sizeof(fullDirectory));
					memset(recvbuff, 0, sizeof(buffer)*sizeof(char)); /*Clears buffer for next connection*/

				}else if(code==400){
					strcat(fullDirectory," HTTP/1.0 400 Not Found;\n");
					//store full directory and response code into buffer
					sendto(sock, (void*)fullDirectory, (size_t)strlen(fullDirectory) ,0, (struct sockaddr *)&sa,fromlen);
					memset(fullDirectory, 0, sizeof(fullDirectory));
					memset(recvbuff, 0, sizeof(buffer)*sizeof(char));

				}
			}
			memset(recvbuff, 0, sizeof(buffer)*sizeof(char));
     
		  }
		  closedir(dir);
	}else if(ENOENT == errno){
	   printf("directory doesnt exist\n");
	   printf("test case input example: ./sws 8080 /home/yuyaxi/csc361/Program1\n");
           //cleanExit();
	}else{
	   printf("open-dir error\n");
	   //cleanExit();
	}	 	 
}

