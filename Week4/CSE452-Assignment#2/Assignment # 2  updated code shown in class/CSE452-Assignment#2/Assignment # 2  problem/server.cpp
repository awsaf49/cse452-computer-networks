#include "helper.cpp"

SOCKET client_sockets[100];

void thread_to_send(struct thread_data td)
{
    SOCKET s = td.s;
    char msg_send[1000];

    sprintf(msg_send, "Your id is %d\n", td.client_id);
    bool success = send_to_socket(s, msg_send);
    if(!success) printf("Send failed.\n\n");

}

void thread_to_recv(struct thread_data td)
{
    SOCKET s = td.s;
    char msg_recv[1000], msg_send[1000];
    strcpy(msg_recv,"");
    while(strcmp(msg_recv,"Bye"))
    {
        int msg_len = receive_from_socket(s, msg_recv, 1000);
        if(msg_len>0)
        {
            char id_str[]={msg_recv[0], msg_recv[1], msg_recv[2], '\0'};
            int id = atoi(id_str);
            sprintf(msg_send, "%d> %s", td.client_id, &msg_recv[4]);
            bool success = send_to_socket(client_sockets[id-100], msg_send);
            if(!success) printf("Send failed.\n\n");

        }
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

    int client_id=100;
    while(1)
    {

        //Accept and incoming connection
        puts("Waiting for incoming connections...");

        SOCKET s = accept_connection(server_socket);
        client_id++;
        client_sockets[client_id - 100] = s; //store the socket in global variable

        puts("Connection accepted");

        //create sending and receiving threads
        struct thread_data td;
        td.s = s;
        td.client_id = client_id;

        create_socket_thread(thread_to_send, td);
        create_socket_thread(thread_to_recv, td);
    }


	//go to sleep
    sleep_for_ever();

    //finally process cleanup job
    closesocket(server_socket);
   //closesocket(s);
    WSACleanup();
	return 0;
}



