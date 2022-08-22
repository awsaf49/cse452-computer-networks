#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>
#include <pthread.h>

#define ARP_REQUEST_OPTION 0
#define ARP_REPLY_OPTION 1

#pragma comment(lib, "ws2_32.lib") // Winsock Library

// thread related data structures
struct thread_data
{
	SOCKET s;
	char ip[5];
	char mac[7];
	// if you need add other feild here.
};

// do not change the following thread
struct thread_params
{
	void (*thread_func)(struct thread_data);
	struct thread_data td;
};

// ARP payload
struct arp_payload
{
	short hardware_type;		     // we will not use this field.
	short protocol_type;		     // we will not use this field.
	char hardware_address_length;    // we will not use this field.
	char protocol_address_length;    // we will not use this field.

	short operation;                 // put 0 for arp request and 1 for arp reply.
					                 // Please see the defined value in the top

	char sender_hardware_address[7]; // MAC address of sending machine
	char sender_protocol_address[5]; // ip of sender machine

	char target_hardware_address[7]; // MAC address of receiving machine
									 // (Ignore this field in ARP request)
	char target_protocol_address[5]; // ip of receiver machine
};

// Ethernet frame
struct ethernet_frame
{
	char preamble[7];			// we will not use this field.
	char start_frame_delimiter; // we will not use this field.

	char mac_destination[7];    // destination mac address
	char mac_source[7];		    // source      mac address

	char ethernet_type[2];      // we will not use this field.

	arp_payload payload;        // ARP         payload.
};

// Printing ip address in good format. eg- 192.168.0.10
void print_ip(char *ip)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		printf("%d", (unsigned char)ip[i]);
		if (i != 3)
			printf(".");
	}
}

// Printing mac address in good format. eg- ab:bc:cd:ef:10:12
void print_mac(char *mac)
{
	int i;
	for (i = 0; i < 6; i++)
	{
		printf("%x", (unsigned char)mac[i]);
		if (i != 5)
			printf(":");
	}
}

// Print both ip and mac. eg- IP: 192.168.0.10, and MAC: ab:bc:cd:ef:10:12
void print_ip_and_mac(char *ip, char *mac)
{
	printf("IP: ");
	print_ip(ip);

	printf(", and MAC: ");
	print_mac(mac);
}

void generate_mac(char *mac)
{
	srand(time(0));

	int i = 0;
	for (i = 0; i < 6; i++)
		mac[i] = rand() % 246 + 10;
	mac[i] = '\0';
}

void generate_broadcast_mac(char *mac)
{
	int i;
	for (i = 0; i < 6; i++)
		mac[i] = 255;
	mac[i] = '\0';
}

bool is_boadcast_mac(char *mac)
{
	char broadcast_mac[7];
	generate_broadcast_mac(broadcast_mac);
	return strcmp(mac, broadcast_mac) == 0;
}

void generate_ip(char *ip)
{
	srand(time(0));

	ip[0] = 192;
	ip[1] = 168;
	ip[2] = rand() % 9 + 1;
	ip[3] = rand() % 90 + 10;
	ip[4] = '\0';
}

// convert to ip[7] format
void extract_ip(char *ip_to, char *ip_from)
{
	char temp[5];
	strncpy(temp, ip_from, 3);
	temp[4] = '\0';
	ip_to[0] = atoi(temp);
	strncpy(temp, ip_from + 4, 3);
	temp[4] = '\0';
	ip_to[1] = atoi(temp);
	strncpy(temp, ip_from + 8, 1);
	temp[1] = '\0';
	ip_to[2] = atoi(temp);
	strncpy(temp, ip_from + 10, 2);
	temp[2] = '\0';
	ip_to[3] = atoi(temp);

	ip_to[4] = '\0';
}

// Networking utility
bool init_networking()
{
	WSADATA wsa;

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Network initialization failed. Error Code: %d\n\n", WSAGetLastError());
		return false;
	}

	printf("Network initialization successful.\n\n");
	return true;
}

SOCKET create_socket()
{
	SOCKET s = 0;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d\n\n", WSAGetLastError());
	}

	printf("Socket created.\n\n");
	return s;
}

bool connect_to_server(SOCKET client_socket, char *ip_address, int port)
{
	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr(ip_address);
	server.sin_port = htons(port);
	server.sin_family = AF_INET;

	// Connect to a server
	if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		puts("connect error\n\n");
		return false;
	}

	puts("Connected to server.\n\n");
	return true;
}

bool bind_socket(SOCKET server_socket, char *ip_address, int port)
{
	// Prepare the sockaddr_in structure
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	if (ip_address == 0)
		server_addr.sin_addr.s_addr = INADDR_ANY;
	else
		server_addr.sin_addr.s_addr = inet_addr(ip_address);
	server_addr.sin_port = htons(port);

	// Bind the socket
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		return false;
	}

	puts("Bind done");
	return true;
}

void listen_for_connections(SOCKET server_socket, int max_connections)
{
	// Listen for connections
	listen(server_socket, max_connections);
}

SOCKET accept_connection(SOCKET server_socket)
{
	sockaddr_in client_addr;
	int c = sizeof(struct sockaddr_in);
	SOCKET client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &c);
	if (client_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d", WSAGetLastError());
		return 0;
	}
	else
		return client_socket;
}

bool send_to_socket(SOCKET s, char *msg)
{
	if (send(s, msg, strlen(msg), 0) < 0)
	{
		puts("Send failed.\n\n");
		return false;
	}
	// printf("Message sent successfully.\n\n");
	return true;
}

int receive_from_socket(SOCKET s, char *msg, int max_len)
{
	int msg_size;
	if ((msg_size = recv(s, msg, max_len, 0)) == SOCKET_ERROR)
	{
		puts("Receive failed.\n\n");
		return -1;
	}
	msg[msg_size] = '\0'; // null terminate it for printing convenience
	// puts("Message received.\n\n");
	return msg_size;
}

// Seding frame
bool send_ethernet_frame(SOCKET socket, ethernet_frame *frame_pointer)
{
	//	const unsigned char *msg = (unsigned char *)frame_pointer;
	// int bytesSent = sendto(socket, (void *) frame_pointer, sizeof(ethernet_frame), 0, (const sockaddr*)&socket, sizeof(socket));

	char buffer[sizeof(ethernet_frame)];
	memcpy(buffer, frame_pointer, sizeof(ethernet_frame));

	if (send(socket, buffer, sizeof(ethernet_frame), 0) < 0)
	{
		puts("Send failed.\n\n");
		return false;
	}
	// printf("Message sent successfully.\n\n");
	return true;
}

// Receiving frame
int receive_ethernet_frame(SOCKET socket, ethernet_frame *frame_pointer)
{
	char *data = new char[sizeof(ethernet_frame)];

	int msg_size;

	if ((msg_size = recv(socket, data, sizeof(ethernet_frame), 0)) == SOCKET_ERROR)
	{
		puts("Receive failed.\n\n");
		return -1;
	}
	memcpy(frame_pointer, data, sizeof(ethernet_frame));
	// puts("Message received.\n\n");
	return msg_size;
}

// do not change the following thread
DWORD WINAPI myThread(LPVOID lpParameter)
{
	struct thread_params *tdp = (struct thread_params *)lpParameter;

	printf("Inside worker thread.\n");

	printf("Calling user function.\n");

	(*tdp->thread_func)(tdp->td);

	printf("User function completed.\n");

	printf("Finishing worker thread.\n");

	return 0;
}

void create_socket_thread(void (*tf)(struct thread_data), struct thread_data td)
{
	struct thread_params *tdp = (struct thread_params *)malloc(sizeof(struct thread_params));
	tdp->thread_func = tf;
	tdp->td = td;
	HANDLE myHandle = CreateThread(0, 0, myThread, tdp, 0, 0);
	CloseHandle(myHandle);
}

void sleep_for_ever()
{
	// prevent server from finishing. otherwise all threads will die! so stop it
	int i;
	while (true)
		Sleep(1000 * 5); // sleep for 5 second
}
