#include <stdio.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<fcntl.h>
#include <unistd.h> // for close
using namespace std;


int main(int argc, char *argv[]) {

    struct stat obj;
    int choice;
    char Buffer[100], command[5], filename[20], *f;
    int k, size, status;
    int filehandle;

    int Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Socket < 0) {  perror("Socket not created"); exit(EXIT_FAILURE); }

    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(12345);
    SockAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int Connect = connect(Socket, (struct sockaddr *) &SockAddr, sizeof(SockAddr));
    if (Connect < 0) { perror("failed to connect"); exit(EXIT_FAILURE); }
    cout <<  "Client connected to server." << endl;

    int i = 1;
    while (1) {

        printf("Enter a choice:\n1- get\n2- put\n3- pwd\n4- ls\n5- cd\n6- quit\n");
        scanf("%d", &choice); //choice is int
        cout <<  "Your command: " <<  choice << endl;
        switch(choice) {
            case 1:
                printf("Enter filename to get: ");
                scanf("%s", filename);
                strcpy(Buffer, "get ");
                strcat(Buffer, filename);
                send(Socket, Buffer, 100, 0);
                recv(Socket, &size, sizeof(int), 0);
                if(!size)
                {
                    printf("No such file on the remote directory\n\n");
                    break;
                }
                f = (char*)malloc(size);
                recv(Socket, f, size, 0);
                while(1)
                {
                    filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
                    if(filehandle == -1)
                    {
                        sprintf(filename + strlen(filename), "%d", i);//needed only if same directory is used for both server and client
                    }
                    else break;
                }
                write(filehandle, f, size);
                close(filehandle);
                strcpy(Buffer, "cat ");
                strcat(Buffer, filename);
                system(Buffer);
                break;

            case 2:
                printf("Enter filename to put to server: ");
                scanf("%s", filename);
                filehandle = open(filename, O_RDONLY);
                if(filehandle == -1)
                {
                    printf("No such file on the local directory\n\n");
                    break;
                }
                strcpy(Buffer, "put ");
                strcat(Buffer, filename);
                send(Socket, Buffer, 100, 0);
                stat(filename, &obj);
                size = obj.st_size;
                send(Socket, &size, sizeof(int), 0);
                sendfile(Socket, filehandle, NULL, size);
                recv(Socket, &status, sizeof(int), 0);
                if(status)
                    printf("File stored successfully\n");
                else
                    printf("File failed to be stored to remote machine\n");
                break;

            case 3:
                strcpy(Buffer, "pwd");
                send(Socket, Buffer, 100, 0);
                recv(Socket, Buffer, 100, 0);
                printf("The path of the remote directory is: %s\n", Buffer);
                break;

            case 4:
                //ftp>ls (to list the files under the present directory of the server)
                strcpy(Buffer, "ls");
                send(Socket, Buffer, 100, 0);
                recv(Socket, &size, sizeof(int), 0);
                f = (char*) malloc(size);
                recv(Socket, f, size, 0);
                filehandle = creat("temp.txt", O_WRONLY);
                write(filehandle, f, size);
                close(filehandle);
                printf("The remote directory listing is as follows:\n");
                system("cat temp.txt");
                break;

            case 5:
                strcpy(Buffer, "cd ");
                printf("Enter the path to change the remote directory: ");
                scanf("%s", Buffer + 3);
                send(Socket, Buffer, 100, 0);
                recv(Socket, &status, sizeof(int), 0);
                if(status)
                    printf("Remote directory successfully changed\n");
                else
                    printf("Remote directory failed to change\n");
                break;
            case 6:
                strcpy(Buffer, "quit");
                send(Socket, Buffer, 100, 0);
                recv(Socket, &status, 100, 0);
                if(status)
                {
                    printf("Server closed\nQuitting..\n");
                    exit(0);
                }
                printf("Server failed to close connection\n");
            }
        }
/*
    int buffer_size = argc;
    for ( int i = 1; i < argc; ++i) {
        buffer_size += strlen(argv[i]);
    }
    char Buffer[buffer_size];
    char* tmp = Buffer;
    for(int i = 1; i < argc; i++) {
        char * tmp2 = argv[i];
        while (*tmp2) {
            *tmp++ = *tmp2++;
        }
        *tmp++ = ' ';
    }
    Buffer[buffer_size-1]=0;

    printf("Client request: %s\n", Buffer);

    int Send = send(Socket, Buffer, sizeof(Buffer), MSG_NOSIGNAL);
    if (Send < 0) {
        perror("failed to send");
        exit(EXIT_FAILURE);
    }

    int Recv = recv(Socket, Buffer, sizeof(Buffer), MSG_NOSIGNAL);
    if (Recv < 0) {
        perror("failed to recv");
        exit(EXIT_FAILURE);
    }
*/


    shutdown(Socket, SHUT_RDWR);
    close(Socket);

    printf("Server answer: %s\n", Buffer);
    return 0;
}