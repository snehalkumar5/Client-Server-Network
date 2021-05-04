// Client side C/C++ program to demonstrate Socket programming
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include<libgen.h>
#define PORT 8080
#define SIZE 100024
#define CYAN "\e[1;36m"
#define GREEN "\e[1;32m"
#define YELLOW "\e[1;33m"
#define RED "\e[1;31m"

#define RESET "\e[0m"
#define MAX_NUM_OF_FILES (int)30
int argscount=0, sz=1e6;
void quit()
{
    exit(0);
}
int isfile(char *filename)
{
    struct stat buf;
    if(!stat(filename, &buf) && !S_ISDIR(buf.st_mode))
    {
        return 0;
    }
    return -1;
}

char** tokenise(char *input)
{
    char **args = (char**)malloc(MAX_NUM_OF_FILES*sizeof(char*));
    char *str = (char*) malloc(sizeof(char)*SIZE);
    char* buffer = (char*)malloc(sizeof(char)*SIZE);
    strcpy(buffer,input);
    str = strtok(buffer," \n");
    argscount=0;
    while(str)
    {
        args[argscount] = (char*)malloc(SIZE*sizeof(char));
        // str = strtok(NULL," \n\t\r");
        strcpy(args[argscount],str);
        // printf("%s\n",args[argscount]);
        argscount++;
        str = strtok(NULL," \n");
    }
    return args;
}

void prompt()
{
    printf(CYAN"\nclient>"RESET);
}

int main(int argc, char const *argv[])
{
    int sock = 0, valread;

    struct sockaddr_in address;
    struct sockaddr_in serv_addr;
    char *hello = "File names to be downloaded";

    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    // printf("sock fd:%d\n",sock);

    memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a 
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    printf("Connection established with server.\n");
    while(1)
    {
        prompt();
        char input[SIZE];
        fgets(input,SIZE,stdin);
        char** filename = (char**)malloc(sizeof(char*)*MAX_NUM_OF_FILES);
        filename[0]="afea";
        char **args = tokenise(input);  
        if(strcmp(args[0],"exit")==0)
        {
            printf("Closing connection...\n");
            close(sock);
            return 0;
        }
        else if(strcmp(args[0],"get")!=0)
        {
            printf("Incorrect command.. Try again!\n");
            continue;
        }
        // printf("%d",strlen)
        // printf("%s %ld\n",args[0],strlen(args[0]));
        // if(send(sock,args[0],strlen(args[0]),0)<0)  //send command
        // {
        //     perror("Error sending command request to server");
        // }
        
        for(int i=1;i<argscount;i++)
        {
            filename[i]=(char*)malloc(sizeof(char)*SIZE);
            // printf("%s\n",args[i]);
            strcpy(filename[i],args[i]);
            if(filename[i] == NULL)
            {
                perror("Couldn't get filename");
                continue;  
            }
            if(send(sock,args[i],strlen(args[i]),0)<0)  // send file request
            {
                perror("Couldn't send filename to server");
                continue;
            }
            printf("\nFile :%s request sent.\n",args[i]);
            
            
        int filesize=-69;
        char readbuf[SIZE];
        if(recv(sock,&filesize,sizeof(int),0)<0) // receive filesize
        {
            perror("Couldn't receive filesize");
            continue;
        } 
        int fullsize = filesize;
        // printf("filesize:%d\n",filesize);
        if(filesize==-69)
        {
            printf("Server has disconnected\n");
            exit(EXIT_FAILURE);
        }
        if(filesize<0)
        {
            printf(RED"File requested doesn't exist\n"RESET);
            continue;
        }
        printf("File %s of size %d bytes exists on server. Starting download..\n",args[i],filesize);
        // int fd = open(args[i],O_WRONLY|O_CREAT|O_TRUNC,0777);
        FILE *f = fopen(args[i],"w");
        if(f==NULL)
        {
            perror("Error opening file");
            continue;
        }
        char wrt[SIZE];
        int byte=0,n=0,flag=1,count=0;
        float ww=-1,written=0;
        while(byte<fullsize)
        {
            flag=0;
            // if((n=read(sock,readbuf,SIZE-1))<0)
            if((n=read(sock,readbuf,SIZE-1))<0)
            {
                perror("Error in Receive file");
                flag=1;
                break;
            }
            readbuf[n]='\0';
            // printf("%s\n",readbuf);
            // write(fd,readbuf,strlen(readbuf));
            if(fwrite(readbuf, sizeof(char),strlen(readbuf),f) != n)
            {
                perror(RED"Error in Write file"RESET);
                flag=1;
                break;
            }
            // if(ww==written)
            written = ((float)(fullsize-filesize)/fullsize)*100;
            if(ww==written)
            {
                // count++;
                // if(count>=50)
                {
                    printf("Server disconnected\n");
                    flag=1;
                    break;
                }
            }
            ww=written;
            sprintf(wrt, YELLOW"Downloading.... %.2f%%\r"RESET, written);
            printf("%s",wrt);
            fflush(stdout);
            filesize -= n;
            // readbuf[0]='\0';
            byte+=n;
        }
        fflush(stdout);
        fclose(f);
        if(!flag)
        {
            printf(GREEN"Downloaded complete: 100%%\n"RESET);
            printf(GREEN"File %s successfully downloaded from server.\n"RESET,args[i]);
        }
        else
        {
            printf(RED"File couldn't be downloaded due to error"RESET);
        }
        
        if(send(sock,"DONE from client",strlen("DONE from client"),0)<0)  // send DONE
        {
            perror("Couldn't send DONE to server");
            continue;
        }
        }
    }    
    return 0;
}
