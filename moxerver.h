#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>

#define DATA_BUFLEN 128


struct server_t {
	int socket; 					/* server socket */
	struct sockaddr_in address; 	/* server address information */
	unsigned int port;				/* server port in host byte order, practical reference */
};

struct client_t {
	int socket; 						/* client socket */
	struct sockaddr_in address; 		/* client address information */
	char ip_string[INET_ADDRSTRLEN]; 	/* client IP address as a string */
	char data[DATA_BUFLEN]; 			/* buffer for data received from client */
};

struct tty_t {
	int fd; 					/* tty file descriptor */
	struct termios ttyset;		/* tty termios settings */
	char data[DATA_BUFLEN]; 	/* buffer for data received from tty */
};


/* Sets up the server on specific port, binds to a socket and listens for client connections. */
int server_setup(struct server_t *server, unsigned int port);
/* Closes the server. */
int server_close(struct server_t *server);
/* Accepts incoming client connection. */
int server_accept(struct server_t *server, struct client_t *accepted_client);
/* Rejects incoming client connection. */
int server_reject(struct server_t *server);


/* Closes client connection. */
int client_close(struct client_t *client);
/* Reads incoming data from client to client data buffer. */
int client_read(struct client_t *client);
/* Sends data from a buffer to client. */
int client_write(struct client_t *client, char *databuf, int datalen);


/* Opens the tty device and configures it. */
int tty_open(struct tty_t *tty_dev);
/* Closes the tty device. */
int tty_close(struct tty_t *tty_dev);
/* Reconfigures the tty device. */
int tty_reconfigure(struct tty_t *tty_dev, struct termios newttyset);
/* Reads incoming data from tty device to tty data buffer. */
int tty_read(struct tty_t *tty_dev);
/* Sends data from a buffer to tty device. */
int tty_write(struct tty_t *tty_dev, char *databuf, int datalen);