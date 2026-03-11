#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>   
#include <sys/wait.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

/*
GET /test.txt HTTP/1.1
HEAD /test.txt HTTP/1.1

GET /cgi-like/test HTTP/1.1
GET /cgi-like/test2?hello HTTP/1.1
*/
#define PORT 2828

void sig_handler(int signum)
{
   while (waitpid(-1, NULL, WNOHANG) > 0) { //do nothing, just check for desired signal in loop }
      }
}

void cgiLike(char* name, int nfd, char *line, FILE *network)
{

      
      char *argArray[100]; //initiailize argument array

      int argPopulator = 0; 

      argArray[argPopulator++] = strtok(name, "?");

      char *arg = strtok(NULL, "&"); //begin splitting arguments

      while (arg != NULL) { argArray[argPopulator++] = arg; arg = strtok(NULL, "&"); } // continue to split and append args

      argArray[argPopulator] = NULL; //null terminate


      char tmp[300]; //create a buffer for the output to track size

      snprintf(tmp, sizeof(tmp),"/tmp/%d.txt", getpid()); // assign pid

      pid_t f1 = fork(); //create fork

      if (f1 == 0)
      {
         int tmpfd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 777); //open file

         dup2(tmpfd, STDOUT_FILENO); //redirect output

         close(tmpfd); //close it

         execv(argArray[0], argArray); //execute

         perror(argArray[0]);  // error checking


         _exit(1); //if fails, reach this point and indicate such

      } else if (f1 < 0) {

         char *err = "HTTP/1.0 500 Internal Error\r\nContent-Type: text/html\r\nContent-Length: 18\r\n\r\n500 Internal Error"; //error msg

         write(nfd, err, strlen(err)); // write error message

         free(line); fclose(network); return; //chores


      }

      waitpid(f1, NULL, 0); //wait for child to finish

      //Functionality for ptining headers
      struct stat st;

      stat(tmp, &st);

      long fileSize = st.st_size;

      char fileSizeBuff[100];

      snprintf(fileSizeBuff, sizeof(fileSizeBuff), "%ld", fileSize);

      write(nfd, "HTTP/1.0 200 OK\n", 16);

      write(nfd, "Content-Type: text/html\r\n", 25);

      write(nfd, "Content-Length: ", 16);

      write(nfd, fileSizeBuff, strlen(fileSizeBuff));

      write(nfd, "\r\n\r\n", 4);

      FILE *tmpfp = fopen(tmp, "r");

      char buff[4096];

      size_t n;

      while ((n = fread(buff, 1, sizeof(buff), tmpfp)) > 0)

        write(nfd, buff, n); //write out output that was stored

      fclose(tmpfp); // memory / vagrlind related

      remove(tmp); // memory/valgrind related

      free(line); fclose(network); return;
}

/*
GET /index.html HTTP/1.1

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
   size_t size = 0;
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

   char *FILENAME = strtok(NULL, " \r\n");

   if (TYPE == NULL || FILENAME == NULL)
   {
      char *ermsg = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length: 15\r\n\r\n400 Bad Request";

      write(nfd, ermsg, strlen(ermsg));

      free(line); 
      
      fclose(network); 
      
      return; 
   }

   char *HTTPVERSION = strtok(NULL, " \n"); 

   FILENAME++; //increment filename to ignore the leading slash

   

   if (strcmp(TYPE, "GET") == 0)
   {    
      //Cgi related functionality
      if (strncmp(FILENAME, "cgi-like/", 9) == 0)
      {
         cgiLike(FILENAME, nfd, line, network);

         return;
      }    
      
      FILE *file = fopen(FILENAME, "r");
      
      if (file == NULL) {
         perror("fopen");
         const char *notfound = "HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\n\r\n";
         write(nfd, notfound, strlen(notfound));
         free(line);
         fclose(network);
         return;
}
      if (access(FILENAME, R_OK) != 0) 
      {
         /*
         Check for access errors
         */
         const char *forbidden = "HTTP/1.0 403 Permission Denied\r\nContent-Type: text/html\r\nContent-Length: 21\r\n\r\n403 Permission Denied"; 

         write(nfd, forbidden, strlen(forbidden));
         
         free(line); fclose(network); return;
      
      }

 
        struct stat st; //intiialize stat structure

        lstat(FILENAME, &st); // populate stat struct

        long fileSize = st.st_size; //create variable for file size
        
        char fileSizeBuff[100]; //create buffer for filesize

        snprintf(fileSizeBuff, sizeof(fileSizeBuff), "%ld", fileSize); //write onto fileSize buffer

        char *headerInfo = "HTTP/1.0 200 OK\n"; //hardcore header info

        char *contentTypeInfo = "Content-Type: text/html\r\n"; //content type 

        char *contentLengthLabel = "Content-Length: "; //content length

        //write all info to client
        write(nfd, headerInfo, strlen(headerInfo));

        write(nfd, contentTypeInfo, strlen(contentTypeInfo));

        write(nfd, contentLengthLabel, strlen(contentLengthLabel));

        write(nfd, fileSizeBuff, strlen(fileSizeBuff));
        
        write(nfd, "\r\n\r\n", 4);





      char buff[4096];
      size_t n;


      //AI Generated fix to print the contents of a file
      while ((n = fread(buff, 1, sizeof(buff), file)) > 0) {
         size_t off = 0;
         while (off < n) {
            ssize_t w = write(nfd, buff + off, n - off);
            if (w <= 0) { /* error */ break; }
            off += (size_t)w;
         }
      }
      //AI Generated fix to print the contents of a file
      
      write(nfd, "\n\n", 2); // new lines for readability

      fclose(file); //close file



   } else if (strcmp(TYPE, "HEAD") == 0)
   {

      FILE *file = fopen(FILENAME, "r");
      
      if (file == NULL) {
         perror("fopen");
         const char *notfound = "HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\n\r\n";
         write(nfd, notfound, strlen(notfound));
         free(line);
         fclose(network);
         return;
}
      if (access(FILENAME, R_OK) != 0) 
      {
         /*
         Check for access errors
         */
         const char *forbidden = "HTTP/1.0 403 Permission Denied"; 

         write(nfd, forbidden, strlen(forbidden));
         
         free(line); fclose(network); return;
      
      }
        struct stat st; //intiialize stat structure

        lstat(FILENAME, &st); // populate stat struct

        long fileSize = st.st_size; //create variable for file size
        
        char fileSizeBuff[100]; //create buffer for filesize

        snprintf(fileSizeBuff, sizeof(fileSizeBuff), "%ld", fileSize); //write onto fileSize buffer

        char *headerInfo = "HTTP/1.0 200 OK\n"; //hardcore header info

        char *contentTypeInfo = "Content-Type: text/html\n"; //content type 

        char *contentLengthLabel = "Content-Length: "; //content length

        //write all info to client
        write(nfd, headerInfo, strlen(headerInfo));

        write(nfd, contentTypeInfo, strlen(contentTypeInfo));

        write(nfd, contentLengthLabel, strlen(contentLengthLabel));

        write(nfd, fileSizeBuff, strlen(fileSizeBuff));
        
        write(nfd, "\n\n", 2);

        fclose(file);
      
   }
   else 
   {
      char* ermsgOther = "HTTP/1.0 501 Not Implemented\r\nContent-Type: text/html\r\nContent-Length: 19\r\n\r\n501 Not Implemented";

      write(nfd, ermsgOther, strlen(ermsgOther));

      free(line); 
      
      fclose(network); 
      
      return; 
   }


   char drain[256];
   while (fgets(drain, sizeof(drain), network) != NULL &&
          strcmp(drain, "\r\n") != 0 &&
          strcmp(drain, "\n") != 0);

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
         
         
         
      }
   }
}


int main()
{
    /*
    argv[1]: port number
    */

    // short portNumber = atoi(argv[1]); // define port number as a short

    int fd = create_service(PORT); // create socket and bind to given port

    if (fd == -1) { printf("%s", "Create service failed"); return 1; } //socket fail check

    printf("Listening on port: %hd", PORT); // print message indicating listening status

    run_service(fd); //

    return 0;


}