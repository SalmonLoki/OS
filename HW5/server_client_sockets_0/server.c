#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h> // for close
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
using namespace std;

int main(int argc, char *argv[]) {


    //there should be 1 argument after "./server" in command line - address for bind (127.0.0.1 for ex)
    if (argc != 2){ perror("Error: required 1 argument - address"); exit(EXIT_FAILURE); }

    //create a socket
	int MasterSocket = socket(
		AF_INET, /*для протокола IPv4 */
		SOCK_STREAM, /*для протокола TCP */
		IPPROTO_TCP // для протокола TCP
	);
    if (MasterSocket < 0) { perror("Socket not created");exit(EXIT_FAILURE); }

    //preparation of the socket address
	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(12345); //port number
    SockAddr.sin_addr.s_addr = inet_addr(argv[1]); //= inet_addr(127.0.0.1), = htonl(INADDR_ANY) - address for bind

    //bind the socket
	int Bind = bind(MasterSocket, (struct sockaddr *) &SockAddr, sizeof(SockAddr));
    if (Bind < 0) { perror("bind"); exit(EXIT_FAILURE); }

    //listen to the socket, then wait for clients
	int Listen = listen(MasterSocket, SOMAXCONN);
    if (Listen < 0) { perror("listen"); exit(EXIT_FAILURE); }
    cout << "Server running...waiting for connections." << endl;

	while(1) {

        int SlaveSocket = accept(MasterSocket, 0, 0);
        if (SlaveSocket < 0) { perror("accept"); exit(EXIT_FAILURE); }

        char host[NI_MAXHOST];
        char service[NI_MAXHOST];

        if (getnameinfo((sockaddr *) &SockAddr, sizeof(SockAddr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
            //cout << host << " connected on port " <<  service <<endl;
            cout <<  host << "client connected." << endl;
        } else {
            inet_ntop(AF_INET, &SockAddr.sin_addr, host, NI_MAXHOST);
            cout << host << " connected on port " <<
                 htons(SockAddr.sin_port) << endl;
        }


        char Buffer[100];
        char command[5];
        char filename[20];
        int i, size, filehandle, c;
        struct stat obj;
        char *f;

        while (1) {
            int Recv = recv(SlaveSocket, Buffer, sizeof(Buffer), MSG_NOSIGNAL);

            //разрыв соединения
            if (Recv < 0) {
                perror("recv failed");
                shutdown(SlaveSocket, SHUT_RDWR);
                close(SlaveSocket);
            }

                //соединение закрыто клиентом
            else if (Recv == 0) { perror("client disconnected..."); }

                //приняты данные от клиента
            else if (Recv > 0) {
                cout << "Command received." << endl;

                sscanf(Buffer, "%s", command);

                //ftp>ls (to list the files under the present directory of the server)
                if (!strcmp(command, "ls") ) {
                    //int system( const char* command );
                    //Вызов командного интерпретатора среды выполнения (/bin/sh, cmd.exe, command.com)
                    // с параметром command
                    system("ls >temps.txt");
                    i = 0;
                    //int stat(char *filename, struct stat *statbuf)
                    //Функция stat() вносит в структуру, на которую указывает statbuf,
                    // информацию, содержащуюся в файле, связанном с указателем filename.
                    stat("temps.txt", &obj);
                    size = obj.st_size;
                    //int Send = send(SlaveSocket, Buffer, sizeof(Buffer), MSG_NOSIGNAL);
                    int Send = send(SlaveSocket, &size, sizeof(int), MSG_NOSIGNAL);
                    if (Send < 0) { perror("Failed to send"); exit(EXIT_FAILURE); }
                    int filehandle = open("temps.txt", STA_RONLY);
                    sendfile(SlaveSocket, filehandle, NULL, size); //send file
                }

                //ftp>get <filename> 		(to download a file named filename from the server)
                else if (!strcmp(command, "get"))
                {
                    sscanf(Buffer, "%s%s", filename, filename);
                    stat(filename, &obj);
                    filehandle = open(filename, STA_RONLY);
                    size = obj.st_size;
                    if(filehandle == -1)
                        size = 0;
                    int Send = send(SlaveSocket, &size, sizeof(int), MSG_NOSIGNAL);
                    if (Send < 0) { perror("Failed to send"); exit(EXIT_FAILURE); }
                    if (size)
                        sendfile(SlaveSocket, filehandle, NULL, size); //send file
                }

                //ftp>put <filename>  (to upload a file named filename to the server)
                else if (!strcmp(command, "put")) {
                    int c = 0, len;
                    char *f;
                    sscanf (Buffer + strlen(command), "%s", filename);
                    recv(SlaveSocket, &size, sizeof(int), MSG_NOSIGNAL);
                    i = 1;
                    while(1) {

                        filehandle = open (filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
                        if (filehandle == -1) sprintf(filename + strlen(filename), "%d", i);
                        else break;
                    }
                    //Выделяет size байт неинициализированной памяти
                    f = (char*) malloc(size);
                    recv(SlaveSocket, f, size, 0);
                    c = write(filehandle, f, size);
                    close(filehandle);
                    send(SlaveSocket, &c, sizeof(int), MSG_NOSIGNAL);
                }

                //ftp>pwd (to display the present working directory of the server)
                else if (!strcmp(command, "pwd")) {

                    system ("pwd>temp.txt");
                    i = 0;
                    FILE*f = fopen("temp.txt","r");
                    while(!feof(f))
                        Buffer[i++] = fgetc(f);
                    Buffer[i-1] = '\0';
                    fclose(f);
                    send(SlaveSocket, Buffer, 100, 0);
                }

                //ftp>cd <directory> (to change the present working directory of the server)
                else if (!strcmp(command, "cd")) {
                    if (chdir(Buffer + 3) == 0)
                        c = 1;
                    else
                        c = 0;
                    send(SlaveSocket, &c, sizeof(int), MSG_NOSIGNAL);
                }

                //ftp>quit 	(to quit from ftp session at the client and return to Unix prompt)
                else if(!strcmp(command, "bye") || !strcmp(command, "quit")) {
                    printf("FTP server quitting..\n");
                    i = 1;
                    send(SlaveSocket, &i, sizeof(int), MSG_NOSIGNAL);
                    exit(0);
                }

                //int Send = send(SlaveSocket, Buffer, sizeof(Buffer), MSG_NOSIGNAL);
                //if (Send < 0) { perror("Failed to send"); exit(EXIT_FAILURE); }

                //printf("Client request: %s\n", Buffer);
            }
        }
        shutdown(SlaveSocket, SHUT_RDWR);
        close(SlaveSocket);
    }

	return 0;
}
