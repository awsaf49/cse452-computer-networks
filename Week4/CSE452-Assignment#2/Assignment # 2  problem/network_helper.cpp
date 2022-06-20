#include<stdio.h>
#include<winsock2.h>
#include <windows.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

bool init_networking()
{
    WSADATA wsa;

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
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
    if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d\n\n" , WSAGetLastError());
	}

	printf("Socket created.\n\n");
	return s;
}

bool connect_to_server(SOCKET client_socket, char * ip_address, int port)
{
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip_address);
    server.sin_port = htons(port);
    server.sin_family = AF_INET;

    //Connect to a server
	if (connect(client_socket, (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error\n\n");
		return false;
	}

	puts("Connected to server.\n\n");
	return true;

}


bool bind_socket(SOCKET server_socket, char * ip_address, int port)
{
    //Prepare the sockaddr_in structure
    struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	if(ip_address==0)
        server_addr.sin_addr.s_addr = INADDR_ANY;
    else
        server_addr.sin_addr.s_addr = inet_addr(ip_address);
	server_addr.sin_port = htons(port);

	//Bind the socket
	if( bind(server_socket ,(struct sockaddr *)&server_addr , sizeof(server_addr)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d" , WSAGetLastError());
		return false;
	}

	puts("Bind done");
	return true;
}


void listen_for_connections(SOCKET server_socket, int max_connections)
{
    //Listen for connections
    listen(server_socket, max_connections);
}

SOCKET accept_connection(SOCKET server_socket)
{
    sockaddr_in client_addr;
    int c = sizeof(struct sockaddr_in);
	SOCKET client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &c);
	if (client_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d" , WSAGetLastError());
		return 0;
	}
	else return client_socket;
}

bool send_to_socket(SOCKET s, char * msg)
{
	if( send(s , msg , strlen(msg) , 0) < 0)
	{
		puts("Send failed.\n\n");
		return false;
	}
	//printf("Message sent successfully.\n\n");
	return true;
}


int receive_from_socket(SOCKET s, char * msg, int max_len)
{
    int msg_size;
	if((msg_size = recv(s, msg , max_len , 0)) == SOCKET_ERROR)
	{
		puts("Receive failed.\n\n");
		return -1;
	}
    msg[msg_size] = '\0'; //null terminate it for printing convenience
	//puts("Message received.\n\n");
	return msg_size;
}



//thread related helper functions and data structures

struct thread_data
{
    SOCKET s;
    int client_id;
};

//do not change the following thread
struct thread_params
{
    void (*thread_func)(struct thread_data);
    struct thread_data td;
};




DWORD WINAPI myThread(LPVOID lpParameter)
{
	struct thread_params * tdp = (struct thread_params *)lpParameter;

    printf("Inside worker thread.\n");

    printf("Calling user function.\n");

	(*tdp->thread_func)(tdp->td);

	printf("User function completed.\n");

	printf("Finishing worker thread.\n");

	return 0;
}

void create_socket_thread(void (*tf)(struct thread_data), struct thread_data td)
{
    struct thread_params * tdp = (struct thread_params *)malloc(sizeof(struct thread_params));
    tdp->thread_func = tf;
    tdp->td = td;
    HANDLE myHandle = CreateThread(0, 0, myThread, tdp, 0, 0);
    CloseHandle(myHandle);
}


void sleep_for_ever()
{
    //prevent server from finishing. otherwise all threads will die! so stop it
    int i;
    while(true) Sleep(1000 * 5); //sleep for 5 second
}


