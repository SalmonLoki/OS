#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <unistd.h> // for close

int main(int argc, char *argv[]) {

    if (argc != 2){
        perror("Error: required 1 argument address");
        exit(1);
    }

	int MasterSocket = socket(            //создание сокета
		AF_INET, /*для протокола IPv4 */
		SOCK_STREAM, /*для протокола TCP */
		IPPROTO_TCP // для протокола TCP
	);
    if (MasterSocket < 0) {
        perror("Socket not created");
        exit(1);
    }


	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;

	SockAddr.sin_port = htons(12345);

	//SockAddr.sin_addr.s_addr = htonl(INADDR_ANY); //0.0.0.0 - на какой адрес забиндимся
    SockAddr.sin_addr.s_addr = inet_addr(argv[1]);


	int Bind = bind(MasterSocket, (struct sockaddr *) &SockAddr, sizeof(SockAddr));
    if (Bind < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

	int Listen = listen(MasterSocket, SOMAXCONN);
    if (Listen < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

	while(1) {
		int SlaveSocket = accept(MasterSocket, 0, 0);
        if (SlaveSocket < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

		int Buffer[5] = {0, 0, 0, 0, 0};
		int Recv = recv(SlaveSocket, Buffer, sizeof(Buffer),  MSG_NOSIGNAL);//read to buffer
        if (Recv < 0) {
            perror("Failed to recv");
            exit(EXIT_FAILURE);
        }
		int Send = send(SlaveSocket, Buffer, sizeof(Buffer), MSG_NOSIGNAL);
        if (Send < 0) {
            perror("Failed to send");
            exit(EXIT_FAILURE);
        }

		shutdown(SlaveSocket, SHUT_RDWR);                 //разрыв соединения
		close(SlaveSocket);

		printf("Message from client: %s\n", Buffer);
	}

	return 0;
}
