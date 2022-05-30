#include "helper.cpp"

void thread_to_send(struct thread_data td)
{
    SOCKET s = td.s;
    char msg_send[1000];
    while(strcmp(msg_send,"bye"))
    {
        gets(msg_send);
        bool success = send_to_socket(s, msg_send);
        if(!success) printf("Send failed.\n\n");
    }
}

void thread_to_recv(struct thread_data td)
{
    SOCKET s = td.s;
    char msg_recv[1000];
    strcpy(msg_recv,"");
    while(strcmp(msg_recv,"bye"))
    {
        int msg_len = receive_from_socket(s, msg_recv, 1000);
        if(msg_len>0) printf("%50s\n", msg_recv);
    }
}



int main(int argc , char *argv[])
{
    bool success = init_networking();
    if(!success) return 0;

    SOCKET server_socket = create_socket();
    if(!server_socket) return 0;

    success = bind_socket(server_socket, 0, 8888);
    if(!success) return 0;

    listen_for_connections(server_socket, 1);

    SOCKET client_socket;

    while(1)
    {
        //Accept and incoming connection
        puts("Waiting for incoming connections...");

        client_socket = accept_connection(server_socket);

        puts("Connection accepted");

        //create sending and receiving threads
        struct thread_data td;
        td.s = client_socket;

        create_socket_thread(thread_to_send, td);
        create_socket_thread(thread_to_recv, td);

    }

	//go to sleep
    sleep_for_ever();

    //finally process cleanup job
    closesocket(server_socket);
    closesocket(client_socket);
    WSACleanup();
	return 0;
}



