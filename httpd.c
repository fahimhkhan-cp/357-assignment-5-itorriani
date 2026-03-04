#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>   
#include <sys/wait.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

void sig_handler(int signum)
{
   while (waitpid(-1, NULL, WNOHANG) > 0) { //do nothing, just check for desired signal in loop }
      }
}

/*
Idea: Sets up listening socket on port 2828, then passes it to run service. Run service loops accept connection 
which waits until a client connects and returns with a new file descriptor.

*/
void handle_request(int nfd)
{
   /*
   Reads client information line by line and prints it to terminal. 
   Frees socket nd port when done

   Change it so that it sends data back to client. 
   */
   FILE *network = fdopen(nfd, "r"); 

   char *line = NULL;
   size_t size;
   ssize_t num;

   if (network == NULL)//error checking
   {
      perror("fdopen");
      close(nfd);
      return;
   }



   num = getline(&line, &size, network);

   if (num == -1){ free(line); fclose(network); printf("error reading client"); return;  } //errors

   //tokenize TYPE, FILENAME, HTTPVERSION
   char *TYPE = strtok(line, " ");

   char *FILENAME = strtok(NULL, " \n");

   char *HTTPVERSION = strtok(NULL, " \n"); 

   if (strcmp(TYPE, "GET") == 0)
   {    
        FILE *file = fopen(FILENAME, "r"); //open file for reading

        char buff[500]; //create a massive buff for file contents

        while (fgets(buff, sizeof(buff), file) != NULL)
        {
            //read contents into buff

            write(nfd, buff, strlen(buff)); //write contents of buff back into desired fd
        }



   } else if (strcmp(TYPE, "HEAD") == 0)
   {
        struct stat st;

        int fd1 = open(FILENAME, O_RDONLY); //open file descripto 

        if (fd1 == -1) { printf("Error opening file"); return; } //error checking

        lstat(FILENAME, &st); //populate stat struct with information 

        long fileSize = st.st_size; // create variable for size

        char fileSizeBuff[100]; //create string representation of fileSize

        snprintf(fileSizeBuff, sizeof(fileSizeBuff), "%ld", fileSize); // write in fileSize into fileSizeBuff

        char *headerInfo = "HTTP/1.0 200 OK\r\n"; //hardcode header info

        char *contentTypeInfo = "Content-Type: text/html\r\n";

        write(nfd, headerInfo, strlen(headerInfo)); //write headerInfo

        write(nfd, fileSizeBuff, strlen(fileSizeBuff)); //write fileSizeBuff

        write(nfd, contentTypeInfo, strlen(contentTypeInfo)); //write contentSizeInfo
        
    
   }





   free(line); // valgrind related
   fclose(network); //valgrind related
}

void run_service(int fd)
{
   /*
   Blocks and waits until client connects. Returns 
   the file descriptor from that client and passes it to handle request. 
   After handle request is finished, the process is over and connections are closed
   */

   pid_t f1;

   struct sigaction sa; //initialize sigaction struct

   sa.sa_handler = sig_handler; //set the sig handler

   sa.sa_flags = 0; //no flags needed

   sigemptyset(&sa.sa_mask); //handle situations when handler is running

   sigaction(SIGCHLD, &sa, NULL); // set up signal

   while (1)
   {
      int nfd = accept_connection(fd); //waits for client to connect, then returns file descriptor of that client

   
      if (nfd != -1)
      { 

         printf("Connection established\n");


         f1 = fork();

         if ( f1 == 0)
         {
            handle_request(nfd);

            _exit(0);

         } else 
         {
            close(nfd);
         }
         
         
         printf("Connection closed\n");
      }
   }
}


int main(int argc, char *argv[])
{
    /*
    argv[1]: port number
    */

    short portNumber = atoi(argv[1]); // define port number as a short

    int fd = create_service(portNumber); // create socket and bind to given port

    if (fd == -1) { printf("%s", "Create service failed"); return 1; } //socket fail check

    printf("Listening on port: %hd", portNumber); // print message indicating listening status

    run_service(fd); //


}