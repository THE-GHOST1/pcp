#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include </home/prajwal/hiredis/hiredis.h>
#include </home/prajwal/hiredis/net.h>
#include </home/prajwal/hiredis/sds.h>
#include </home/prajwal/hiredis/async.h>

#define PORT_NO 44323

int guard(int n, char * err) { if (n == -1) { perror(err); exit(1); } return n; }

void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    int n;
    redisReply *reply = r;
    if (reply == NULL) return;
    printf("argv[%s]: %s\n", (char*)privdata, reply->str);
   // n = write(sock,reply->str,strlen(reply->str));
    redisAsyncDisconnect(c);
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
}

void SendRedisCommand (int clientsocket) {
    int n;
    char buffer[512];
    bzero(buffer,512);

    n = read(clientsocket,buffer,512);

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("Here is the message: %s\n",buffer);

    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);

    if (c->err) {
        printf("Error: %s\n", c->errstr);
        return;
    }

    redisAsyncSetConnectCallback(c,connectCallback);
    redisAsyncSetDisconnectCallback(c,disconnectCallback);
    redisAsyncCommand(c, getCallback, (char*)"end-1", buffer);

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

}

int main() {
    int listen_socket_fd;
    int flags;
    int clilen;
    int pid;
    int client_socket_fd;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;

    listen_socket_fd = guard(socket(AF_INET, SOCK_STREAM, 0), "error creating TCP socket");
    flags = guard(fcntl(listen_socket_fd, F_GETFL), "error setting flags in TCP socket");


    /*todo ask nathans */

   //set non-blocking connection but no idea when to wake up if client starts writing.
    guard(fcntl(listen_socket_fd, F_SETFL, flags | O_NONBLOCK), "Error setting TCP socket to non-blocking");

    //  May be better to do with iocntl() get SIGIO upon clients write


    bzero((char *)&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = (PORT_NO);
    serv_addr.sin_addr.s_addr = (INADDR_ANY);

    guard(bind(listen_socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)), "could not bind");
    guard(listen(listen_socket_fd, 100), "could not listen");
    clilen = sizeof(cli_addr);

    for (;;) {
        client_socket_fd = accept(listen_socket_fd,(struct sockaddr*) &cli_addr, &clilen);
        if (client_socket_fd == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
               // printf("No pending connections; sleeping for one second.\n");
                sleep(10);
            } else {
                perror("error when accepting connection");
                exit(1);
            }
        }
        pid = fork();
        if(pid < 0) {
            perror("Error in fork");
            exit(1);
        }
        else if(pid == 0)
        {
            close(listen_socket_fd);
                SendRedisCommand(client_socket_fd);
        }
        else
        {
            close(client_socket_fd);
        }
//
//        else {
//            char msg[] = "hello\n";
//            printf("Got a connection; writing 'hello' then closing.\n");
//            send(client_socket_fd, msg, sizeof(msg), 0);
//            close(client_socket_fd);
//        }
    }
    return EXIT_SUCCESS;
}
