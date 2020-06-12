#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <dirent.h>
#include <fcntl.h> 
#include <time.h>
#include <sys/wait.h>
#include <stdbool.h> 

using namespace std;
extern int errno;
typedef char BYTE;

#define SOCKET_TCP_CHECK(descriptor)      					if((descriptor = socket(AF_INET, SOCK_STREAM,0)) == -1){ perror("[CLIENT]: Socket error "); return errno;}
#define CONNECT_CHECK(descriptor,server)            if( connect(descriptor,(struct sockaddr *) &server,sizeof (struct sockaddr))==-1){perror("[CLIENT]: Connect error "); return errno; }
#define READ_CHECK(from,in,size)                   	if(read(from,in,size)<=0){perror("[CLIENT]: Read error "); return errno;}
#define WRITE_CHECK(from,in,size)                   if(write(from,in,size)<=0){perror("[CLIENT]: Write error "); return errno;}
#define READ_LINES_FROM_DATABASE_RES(errorMsg,col,type)\
                                                    READ_CHECK(serverDescriptor,&numberOfRows,sizeof(int));\
                                                    if(numberOfRows){\
                                                      if(type==0){\
                                                        printf("\n|              TITLE             |                 AUTHOR               |             PRINCIPAL GENRE           |");\
                                                      }\
                                                      else{\
                                                          printf("\n|              AUTHOR             |                 DESCRIPTION               |                GENRE              |              SUBGENRE            |");\
                                                      }\
                                                      printf("\n-----------------------------------------------------------------------------------------------------------------\n|");\
                                                    }\
                                                    for(int i=0;i<numberOfRows;i++){\
                                                        for(int j=0;j<col;j++){\
                                                            int size;\
                                                            READ_CHECK(serverDescriptor,&size,sizeof(int));\
                                                            char * message=(char*)malloc((size+1)*sizeof(char));\
                                                            READ_CHECK(serverDescriptor,message,size);\
                                                            message[size]='\0';\
                                                            printf("  %-30s|\t",message);\
                                                            free(message);\
                                                         }\
                                                        printf("\n-----------------------------------------------------------------------------------------------------------------\n");\
                                                        if(i+1!=numberOfRows)\
                                                          printf("|");\
                                                    }\
                                                    if(numberOfRows==0)\
                                                      printf("\n[CLIENT]: %s\n\n",errorMsg);\
                                                    else if(numberOfRows>0)\
                                                        printf("\n\n");

#define READ_LINES_FROM_DATABASE_RES_DESCRIPTION_TYPE(errorMsg)     READ_CHECK(serverDescriptor,&numberOfRows,sizeof(int));\
                                                    for(int i=0;i<numberOfRows;i++){\
                                                        for(int j=0;j<4;j++){\
                                                            int size;\
                                                            READ_CHECK(serverDescriptor,&size,sizeof(int));\
                                                            char * message=(char*)malloc((size+1)*sizeof(char));\
                                                            READ_CHECK(serverDescriptor,message,size);\
                                                            message[size]='\0';\
                                                            switch(j){\
                                                              case 0:   printf("author: ");\
                                                                        break;\
                                                              case 1:   printf("description: ");\
                                                                        break;\
                                                              case 2:   printf("Genres in his opera: ");\
                                                                        break;\
                                                              case 3:   printf("Subgenre in his opera:");\
                                                                        break;\
                                                            }\
                                                            printf("%s\n",message);\
                                                            free(message);\
                                                         }\
                                                    }\
                                                    if(numberOfRows==0)\
                                                      {printf("\n[CLIENT]: %s\n\n",errorMsg); errorOccured=1;}\
                                                    else if(numberOfRows>0)\
                                                        printf("\n\n");


int port;


static struct termios old, current;

void initTermios(int echo) {
  tcgetattr(0, &old);
  current = old;
  current.c_lflag &= ~ICANON; 
  if (echo) {
      current.c_lflag |= ECHO;
  } else {
      current.c_lflag &= ~ECHO;
  }
  tcsetattr(0, TCSANOW, &current); 
}

void resetTermios(void) {
  tcsetattr(0, TCSANOW, &old);
}
char getch_(int echo) {
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}
char getch(void) {
  return getch_(0);
}

int main (int argc, char *argv[])
{
  int serverDescriptor;
  struct sockaddr_in server;

  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }
  /* stabilim portul */
  port = atoi (argv[2]);

  SOCKET_TCP_CHECK(serverDescriptor);

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_port = htons (port);
  CONNECT_CHECK(serverDescriptor,server);

  int notLoggedIn=0;
  printf("╭━━━╮╱╱╱╱╱╱╱╭╮╱╱╭━━━╮╱╱╱╱╱╭━╮╭╮\n┃╭━╮┃╱╱╱╱╱╱╱┃┃╱╱┃╭━╮┃╱╱╱╱╱┃╭╯┃┃\n┃╰━╯┣━━┳━━┳━╯┣━━┫╰━╯┣━┳━━┳╯╰┳┫┃╭━━┳━╮\n┃╭╮╭┫┃━┫╭╮┃╭╮┃━━┫╭━━┫╭┫╭╮┣╮╭╋┫┃┃┃━┫╭╯\n┃┃┃╰┫┃━┫╭╮┃╰╯┣━━┃┃╱╱┃┃┃╰╯┃┃┃┃┃╰┫┃━┫┃\n╰╯╰━┻━━┻╯╰┻━━┻━━┻╯╱╱╰╯╰━━╯╰╯╰┻━┻━━┻╯\n");
  printf("\t\t\t\t\t╭╮\n\t\t\t\t\t┃┃\n\t\t\t\t\t┃╰━┳╮╱╭╮\n\t\t\t\t\t┃╭╮┃┃╱┃┃\n\t\t\t\t\t┃╰╯┃╰━╯┃\n\t\t\t\t\t╰━━┻━╮╭╯\n\t\t\t\t\t╱╱╱╭━╯┃\n\t\t\t\t\t╱╱╱╰━━╯\n");
  printf("\t\t\t\t\t\t\t\t╭╮╱╱╭┳╮╱╱╱╱╱╭╮\n\t\t\t\t\t\t\t\t┃╰╮╭╯┃┃╱╱╱╱╱┃┃╱╱╱╱╱╱╱╱╭╮\n\t\t\t\t\t\t\t\t╰╮┃┃╭┫┃╭━━┳━╯┃╭━━┳━━┳━╋╋╮╭┳━━╮\n\t\t\t\t\t\t\t\t╱┃╰╯┃┃┃┃╭╮┃╭╮┃┃╭━┫╭╮┃╭╋┫┃┃┃╭━╯\n\t\t\t\t\t\t\t\t╱╰╮╭╯┃╰┫╭╮┃╰╯┃┃╰━┫╰╯┃┃┃┃╰╯┃╰━╮\n\t\t\t\t\t\t\t\t╱╱╰╯╱╰━┻╯╰┻━━╯╰━━┻━━┻╯┃┣━━┻━━╯\n\t\t\t\t\t\t\t\t╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╭╯┃\n\t\t\t\t\t\t\t\t╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╰━╯\n\n");

  while(notLoggedIn==0){
    printf ("[CLIENT]: Welcome to our bookstore!\n");
    printf("\n\t\t\t\t\t█▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀█\n\t\t\t\t\t█░░╦─╦╔╗╦─╔╗╔╗╔╦╗╔╗░░█\n\t\t\t\t\t█░░║║║╠─║─║─║║║║║╠─░░█\n\t\t\t\t\t█░░╚╩╝╚╝╚╝╚╝╚╝╩─╩╚╝░░█\n\t\t\t\t\t█▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█\n\n");
    printf("[CLIENT]: Do I know you? (Y to login | N to register): ");
    fflush(stdout);

    char loginDecision[2];
    READ_CHECK(0,&loginDecision,2);
    BYTE logYes;
    if(loginDecision[0]=='Y'||loginDecision[0]=='y'){//LOGIN
      logYes=1;
      WRITE_CHECK(serverDescriptor,&logYes,sizeof(BYTE));
    }
    else{//REGISTER
      logYes=2;
      WRITE_CHECK(serverDescriptor,&logYes,sizeof(BYTE));
    }
    char username[100];
  /* citirea mesajului */
    bzero (username, 100);
    printf ("[CLIENT]: USERNAME= ");
    fflush (stdout);

    READ_CHECK(0,username,100);
    int usernameSize=strlen(username);
    username[usernameSize-1]='\0';
    WRITE_CHECK(serverDescriptor,&usernameSize,sizeof(int));
    WRITE_CHECK(serverDescriptor,username,usernameSize);


    char password[100];
    /* citirea mesajului */
    bzero (password, 100);
    printf ("[CLIENT]: PASSWORD= ");
    fflush (stdout);

    for(int i=0;i<100;i++){
      password[i]=getch();
      if(password[i]!='\r'&&password[i]!='\n')
      {
        printf("*");
      }
      if(password[i]==13||password[i]==27||password[i]=='\n')
        break;
    }
    printf("\n\n");
    int passSize=strlen(password);
    password[passSize-1]='\0';
    WRITE_CHECK(serverDescriptor,&passSize,sizeof(int));
    WRITE_CHECK(serverDescriptor,password,passSize);


    READ_CHECK(serverDescriptor,&notLoggedIn,sizeof(int)); 
    if(notLoggedIn==0){
      printf("[CLIENT]: I don't know if I know you, try again or register please.\n\n");
    }

  }
  printf("[CLIENT]: COMMANDS THAT YOU HAVE :\tdownld <TITLE>  : search a book by title and downloads the book specified if the user accept\n\t\t\t\t\tbrowse  : without tag will show all the books avaible\n\t\t\t\t\tbrowse -g <BOOK_GENRE> search books by genre\n\t\t\t\t\tbrowse -a <BOOK_AUTHOR> search books by author  \n\t\t\t\t\tsuggest  :server will suggest a book based on your and other users preferences\n\t\t\t\t\trating <STARS> <TITLE> : rate a the book woth title <TITLE> by <STARS> star( stars go from 1 to 10)\n\t\t\t\t\texit\n\n");

  while(1){
    printf ("[CLIENT]: what can i do for you?  ");
    fflush (stdout);
    char command[1000];    
    bzero (command, 1000);
    READ_CHECK(0,command,1000);

    int commandLen=strlen(command);
    command[commandLen-1]='\0';
    char shortChommand[10];

    strncpy(shortChommand,command,6);
    shortChommand[6]='\0';

    if(strcmp(command,"exit")==0){
      BYTE typeofCommand=0;
      WRITE_CHECK(serverDescriptor,&typeofCommand,sizeof(BYTE));
      printf("[CLIENT]: Byeee\n");

      printf("\t\t\t\t██████╗░██╗░░░██╗███████╗███████╗\n\t\t\t\t██╔══██╗╚██╗░██╔╝██╔════╝██╔════╝\n\t\t\t\t██████╦╝░╚████╔╝░█████╗░░█████╗░░\n\t\t\t\t██╔══██╗░░╚██╔╝░░██╔══╝░░██╔══╝░░\n\t\t\t\t██████╦╝░░░██║░░░███████╗███████╗\n\t\t\t\t╚═════╝░░░░╚═╝░░░╚══════╝╚══════╝\n");

      break;
    }

    else if(strcmp(shortChommand,"browse")==0){
      BYTE typeofCommand=2;
      WRITE_CHECK(serverDescriptor,&typeofCommand,sizeof(BYTE));

      if(strcmp(command,shortChommand)==0){
        const char * message="all";
        int size=3;
        WRITE_CHECK(serverDescriptor,&size,sizeof(int));
        WRITE_CHECK(serverDescriptor,message,size);

        int numberOfRows;
        READ_LINES_FROM_DATABASE_RES("No books in the bookstore",3,0);
        
      }
      else{
        int i=0;
        while(command[i]!='-'){
          i++;
        }
        i++;
        if(command[i]=='g'){
            i+=2;
            int init=i;
            char browseGenre[500];
            for(i;i<commandLen;i++){
              browseGenre[i-init]=command[i];
            } 
            browseGenre[i-init]='\0';

            const char * message="genre";
            int size=5;
            WRITE_CHECK(serverDescriptor,&size,sizeof(int));
            WRITE_CHECK(serverDescriptor,message,size);

            int genreSize=strlen(browseGenre);
            if(genreSize==0){
                genreSize=strlen("romantic");
                strcpy(browseGenre,"romantic");
                printf("[CLIENT]: No genre inserted, we selected for you romantic genre\n");
            } 

            WRITE_CHECK(serverDescriptor,&genreSize,sizeof(int));
            WRITE_CHECK(serverDescriptor,browseGenre,genreSize);

            int numberOfRows;
            READ_LINES_FROM_DATABASE_RES("No book of that genre in the bookstore!",3,0);

        }
        else if(command[i]=='a'){
            i+=2;
            int init=i;
            char browseAuthor[500];
            for(i;i<commandLen;i++){
              browseAuthor[i-init]=command[i];
            }
            browseAuthor[i-init]='\0'; 

            const char * message="author";
            int size=6;
            WRITE_CHECK(serverDescriptor,&size,sizeof(int));
            WRITE_CHECK(serverDescriptor,message,size);

            int authorSize=strlen(browseAuthor);
            WRITE_CHECK(serverDescriptor,&authorSize,sizeof(int));
            WRITE_CHECK(serverDescriptor,browseAuthor,authorSize);

            int numberOfRows;
            int errorOccured=0;

            READ_LINES_FROM_DATABASE_RES_DESCRIPTION_TYPE("That author is not in the bookstore!");
            if(errorOccured==0){
                if(authorSize==0){
                authorSize=strlen("Vlad Corjuc");
                strcpy(browseAuthor,"Vlad Corjuc");
                printf("[CLIENT]: No author inserted, we selected for you our best author\n");
                } 
            
              READ_LINES_FROM_DATABASE_RES("That author is not in the bookstore!",3,0);
            }
            
            
        }
        else{
           printf("[CLIENT]: unknow command\n");
        }
      }
    }
    else if(strcmp(shortChommand,"downld")==0){
        BYTE typeofCommand=1;
        WRITE_CHECK(serverDescriptor,&typeofCommand,sizeof(BYTE));

        int i=6;
        while(command[i]==' '){
          i++;
        }
        int init=i;
        char bookByTitle[500];
            for(i;i<commandLen;i++){
              bookByTitle[i-init]=command[i];
            } 
            bookByTitle[i-init]='\0';

        int titleSize=strlen(bookByTitle);
        WRITE_CHECK(serverDescriptor,&titleSize,sizeof(int));
        WRITE_CHECK(serverDescriptor,bookByTitle,titleSize);


        BYTE foundFlag;
        READ_CHECK(serverDescriptor,&foundFlag,sizeof(BYTE));

        if(foundFlag==0){
          printf("[CLIENT]: The bookstore dosen't have the title that you searched for and not even similar title\n");
        }
        else{
          printf("[CLIENT]: Found title or similar title for the one searched \n\n");

          int size;
          READ_CHECK(serverDescriptor,&size,sizeof(int));
          char * bookInfo=(char*)malloc(size*sizeof(char));
          READ_CHECK(serverDescriptor,bookInfo,size);
          bookInfo[size]='\0';
          printf("%s",bookInfo);
          free(bookInfo);


          printf("[CLIENT]: Do you want to dowload it? (Y|N): ");
          fflush (stdout);

          char decision[2];
          READ_CHECK(0,&decision,2);

          if(decision[0]=='Y' || decision[0]=='y'){
            
            BYTE dowloadFlag=1;
            WRITE_CHECK(serverDescriptor,&dowloadFlag,sizeof(BYTE));
            printf("[CLIENT]: File name :");
            fflush(stdout);
            char fileName[200];
            bzero(&fileName,200);
            READ_CHECK(0,&fileName,200);

            char filePath[1000];
            strcpy(filePath,"CLIENT/");
            strcat(filePath,fileName);

            FILE*  fileDescriptor=fopen(filePath,"w");


            printf("[CLIENT]: Downloading the selected book...");
            char ch=1;
            while(ch!='\0'){
              READ_CHECK(serverDescriptor,&ch,1);
              if(ch!='\0')
                fprintf(fileDescriptor,"%c",ch);
            }
            printf("DONE\n");
            fclose(fileDescriptor);
          }
          else{
            BYTE dowloadFlag=0;
            WRITE_CHECK(serverDescriptor,&dowloadFlag,sizeof(BYTE));
            printf("\n[CLIENT]: Sure! If you want to find books for you try our suggestion! \n\n");
          }



        }


    }
    else if(strcmp(shortChommand,"sugges")==0){
      BYTE typeofCommand=3;
      WRITE_CHECK(serverDescriptor,&typeofCommand,sizeof(BYTE));

      BYTE hasPreviousBook;
      READ_CHECK(serverDescriptor,&hasPreviousBook,sizeof(BYTE));

      int numberOfRows;

      printf("[CLIENT]: This is our week suggested lecture");
      READ_LINES_FROM_DATABASE_RES("Hmm...we dont't have anything this week",3,0);

      if(hasPreviousBook){
        printf("[CLIENT]: We have watched you, and we can say that this is one book that suits you ");
        READ_LINES_FROM_DATABASE_RES("Hmm...no result",3,0);
      }

      printf("[CLIENT]: This is our top book in this period\n");
      READ_LINES_FROM_DATABASE_RES("Hmm...we dont't have any rating since now, please rate our books",3,0);
      

    }
    else if(strcmp(shortChommand,"rating")==0){
        BYTE typeofCommand=4;
        WRITE_CHECK(serverDescriptor,&typeofCommand,sizeof(BYTE));

          int i=0;
          while(command[i]!=' '){
            i++;
          }

          i++;
          char number[100];
          int initNumb=i;
          while(command[i]!=' '){
            number[i-initNumb]=command[i];
            i++;
          }
          number[i-initNumb]='\0';
          i++;

          int init=i;
          char title[200];
          for(i;i<commandLen;i++){
            title[i-init]=command[i];
          } 
          title[i-init]='\0';
          
        int stars=atoi(number);
        if(stars>10||stars<=0){
          printf("[CLIENT]: Please use our 1-10 stars rating.\n");
          BYTE errorFlag=1;
          WRITE_CHECK(serverDescriptor,&errorFlag,sizeof(BYTE));
          continue;
        }
        else if(strlen(title)<=0){
           printf("[CLIENT]: Please specify title.\n");
           BYTE errorFlag=1;
           WRITE_CHECK(serverDescriptor,&errorFlag,sizeof(BYTE));
           continue;
        }
        BYTE errorFlag=0;
        WRITE_CHECK(serverDescriptor,&errorFlag,sizeof(BYTE));

        int titleSize=strlen(title);
        WRITE_CHECK(serverDescriptor,&titleSize,sizeof(int));
        WRITE_CHECK(serverDescriptor,title,titleSize);

        WRITE_CHECK(serverDescriptor,&stars,sizeof(int));

        printf("[CLIENT]: Thanks for your rating!\n");


    }
    else{
      printf("[CLIENT]: Unknow command!\n");
    }

  }
  close (serverDescriptor);
}