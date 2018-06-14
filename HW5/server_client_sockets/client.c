#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

#include <unistd.h> // for close

int main(int argc, char **argv) {
    int Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Socket < 0) {
        perror("Socket not created");
        exit(1);
    }

    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(12345);
    SockAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int Connect = connect(Socket, (struct sockaddr *) &SockAddr, sizeof(SockAddr));
    if (Connect < 0) {
        perror("failed to connect");
        exit(EXIT_FAILURE);
    }

    char Buffer[] = "PING";
    int Send = send(Socket, Buffer, sizeof(Buffer), MSG_NOSIGNAL);
    if (Send < 0) {
        perror("failed to send");
        exit(EXIT_FAILURE);
    }
    int Recv = recv(Socket, Buffer, sizeof(Buffer), MSG_NOSIGNAL);
    if (Send < 0) {
        perror("failed to recv");
        exit(EXIT_FAILURE);
    }

    shutdown(Socket, SHUT_RDWR);
    close(Socket);

    printf("Message from server: %s\n", Buffer);
    return 0;
}