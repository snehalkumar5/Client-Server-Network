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
#include <signal.h>
#define PORT 8080
#define MAX_NUM_OF_FILES (int)20
#define SIZE 100024
const long bufsize = 1e6;
int var=0;
char buffer[1024];

void signal_handler(int sig)
{
    printf("SIGPIPE caught\n");
    var=1;
    // exit(EXIT_FAILURE);
}
int main(int argc, char const *argv[])
{
    char **args = (char**)malloc(MAX_NUM_OF_FILES*sizeof(char*));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;  
    int opt = 1;
    int addrlen = sizeof(address);
    char *hello = (char*)malloc(sizeof(char)*256);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))<0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc. 
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
    while(1)
    {
    puts("Waiting for connection ..."); 
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("\nConnection established\n");
    // printf("sock fd:%d\n",new_socket);
    
    while(1)
    {
        signal(SIGPIPE,signal_handler);
        if(var==1)
        {
            var=0;
            break;
        }
        // buffer[0]='\0';
        int redn=-1;
        // if(recv(new_socket,buffer,SIZE,0)<0)
        // if((redn = read(new_socket,buffer,SIZE-1))<0)
        // {
        //     perror("Couldn't receive command");
        // }
        // buffer[redn]='\0';
        printf("cmd:%s %ld\n",buffer,strlen(buffer));

        // if(strcmp(buffer,"exit")==0)
        // {
        //     // printf("Closing connection of\n");
        //     // close(new_socket);
        //     // exit(0);
        //     continue;
        // }
        // buffer[0]='\0';
        // if(recv(new_socket,buffer,SIZE,0)<0)
        redn = read(new_socket,buffer,SIZE);
        if(redn<0)
        {
            perror("Couldn't receive filename");
            continue;
        }
        buffer[redn]='\0';
        printf("File request from client received: %s\n",buffer);
        // buffer[0]='\0';
        // FILE *f = fopen(buffer,"r");
        // if(f==NULL)
        // {
        //     perror("Error opening file");
        // }
        int filesize = -1;
        struct stat st;
        stat(buffer,&st);
        filesize = st.st_size;
        int fd=open(buffer,O_RDONLY);
        if(fd<0)
        {
            perror("Error in opening file");
            filesize=-1;
        }
        // fseek(f,(off_t)0,SEEK_SET);
        // char filesz[SIZE];
        // sprintf(filesz,"%ld",filesize);
        // printf("%s\n",filesz);
        // send(new_socket,filesz,strlen(filesz),0);
        send(new_socket,&filesize,sizeof(int),0);
        // filesz[0]='\0';

        int n=0;  
        char sendline[SIZE]; 
        // while ((n = fread(sendline, sizeof(char), SIZE, f)) > 0) 
        // while((n=read(fd,sendfile,SIZE))>0)
        while(n<filesize)
        {
            int rb = read(fd,sendline,SIZE-1);
            // int rb = fread(sendline,sizeof(char),SIZE-1,f);
            if(rb<0)
            {
                perror("Error reading from file");
                close(new_socket);
                break;
            }
            sendline[rb]='\0';
            // printf("%s\n",sendline);
            if(send(new_socket, &sendline, rb,0)<0)
            {
                perror("Can't send file");
                close(new_socket);
                break;
            }
            sendline[0]='\0';
            n+=rb;
        }
        close(fd);
        if(n=(recv(new_socket,buffer,SIZE-1,0))<0)
        {
            perror("Couldn't receive DONE");
            break;
        }
        buffer[n]='\0';
        printf("Received DONE\n");
    }    
    printf("\nClosed connection from client\n");
    // close(new_socket);
    }
    // exit(EXIT_SUCCESS);
    // return 0;
}
