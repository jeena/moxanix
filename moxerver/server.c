#include <server.h>
#include <netinet/tcp.h> /* TCP_NODELAY */
#include <arpa/inet.h>

int server_setup(server_t *server, unsigned int port)
{
	int opt;
	char timestamp[TIMESTAMP_LEN];
	
	/* set up server address information */
	server->address.sin_family = AF_INET; 		  /* use IPv4 address family */
	server->address.sin_port = htons(port); 	  /* set up port number (htons for network byte order) */
	server->address.sin_addr.s_addr = INADDR_ANY; /* use local address */
	
	/* create stream socket using TCP */
	server->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server->socket == -1)
	{
		LOG("[@%d] error %d: %s", __LINE__, errno, strerror(errno));
		return -errno;
	}
	LOG("socket created");
	
	/* try to avoid "Address already in use" error */
	opt = 1; /* true value for setsockopt option */
	if (setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1)
	{
		LOG("[@%d] error %d: %s", __LINE__, errno, strerror(errno));
		return -errno;
	}
	/* turn off Nagle algorithm */
	opt = 1; /* true value for setsockopt option */
	if (setsockopt(server->socket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int)) == -1)
	{
		LOG("[@%d] error %d: %s", __LINE__, errno, strerror(errno));
		return -errno;
	}
	
	/* bind server address to a socket */
	if (bind(server->socket, (struct sockaddr *) &server->address, sizeof(server->address)) == -1)
	{
		LOG("[@%d] error %d: %s", __LINE__, errno, strerror(errno));
		return -errno;
	}
	LOG("bind successful");
	
	/* save server port number */
	server->port = port;
	LOG("assigned port %u", server->port); // ntohs(server->address.sin_port)
	
	/* listen for a client connection, allow (max-1) connections in queue */
	if (listen(server->socket, (SERVER_MAX_CONNECTIONS - 1)) == -1)
	{
		LOG("[@%d] error %d: %s", __LINE__, errno, strerror(errno));
		return -errno;
	}
	
	time2string(time(NULL), timestamp);
	LOG("server is up @ %s", timestamp);
	return 0;
}

void server_close(server_t *server)
{
	char timestamp[TIMESTAMP_LEN];

	/* force closing in case of error */
	if (close(server->socket) == -1)
	{
		close(server->socket);
	}

	time2string(time(NULL), timestamp);
	LOG("socket closed, server is down @ %s", timestamp);
}

int server_accept(server_t *server, client_t *accepted_client)
{
	int namelen;
	char timestamp[TIMESTAMP_LEN];
	
	/* accept connection request */
	namelen = sizeof(accepted_client->address);
	accepted_client->socket = accept(server->socket, (struct sockaddr *) &accepted_client->address, (socklen_t *) &namelen);
	if (accepted_client->socket == -1)
	{
		LOG("[@%d] error %d: %s", __LINE__, errno, strerror(errno));
		return -errno;
	}
	/* make the client socket non-blocking */
	if (fcntl(accepted_client->socket, F_SETFL, O_NONBLOCK) == -1)
	{
		LOG("[@%d] error %d: %s", __LINE__, errno, strerror(errno));
		return -errno;
	}
	
	/* get client IP address as a human readable string */
	inet_ntop(accepted_client->address.sin_family,
				&accepted_client->address.sin_addr.s_addr,
				accepted_client->ip_string, INET_ADDRSTRLEN);
	
	/* grab current time and store it as client's last activity*/
	accepted_client->last_active = time(NULL);
	
	/* print client information */
	time2string(accepted_client->last_active, timestamp);
	LOG("accepted client %s @ %s", accepted_client->ip_string, timestamp);
	return 0;
}
