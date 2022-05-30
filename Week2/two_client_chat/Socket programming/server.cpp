#include "helper.cpp"


SOCKET client_sockets[3];
SOCKET client_socket1, client_socket2;
int ids[3];

/*
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
}*/

void thread_client_1_to_client_2(struct thread_data td)
{
    char msg_recv[1000];
    strcpy(msg_recv,"");
    while(strcmp(msg_recv,"bye"))
    {
        int msg_len = receive_from_socket(client_socket1, msg_recv, 1000);
        if(msg_len==0) return;

        bool success = send_to_socket(client_socket2, msg_recv);
        if(!success) printf("Send failed.\n\n");
    }
}



int main(int argc, char *argv[])
{
    bool success = init_networking();
    if(!success) return 0;

    SOCKET server_socket = create_socket();
    if(!server_socket) return 0;

    success = bind_socket(server_socket, 0, 8888);
    if(!success) return 0;

    listen_for_connections(server_socket, 1);
    int i = 0;
    while(1)
    {
        puts("Waiting for %d connections...", i);

        client_sockets[i] = accept_connection(server_socket);
        puts("Connection %d accepted", i);
        i++;


        //create sending and receiving threads

        struct thread_data td;
        create_socket_thread(thread_client_1_to_client_2, td);
        create_socket_thread(thread_client_2_to_client_1, td);
    }


    //go to sleep
    sleep_for_ever();

    //finally process cleanup job
    //closesocket(server_socket);
    //closesocket(client_socket);
    WSACleanup();
    return 0;
}



