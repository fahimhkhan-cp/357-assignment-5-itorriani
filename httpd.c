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



   while ((num = getline(&line, &size, network)) >= 0) //waits for information from the clien
   {

      write(nfd, line, num); //write back to the file descriptor

      //printf("%s", line); //remove print statement
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


}