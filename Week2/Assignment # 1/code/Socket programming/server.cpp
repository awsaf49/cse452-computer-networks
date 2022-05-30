#include "helper.cpp"


SOCKET client_sockets[100];

int str2num(char msg_recv[])
{
    int i = 0;
    char num[3];
    while(msg_recv[i]!=' ')
    {
        num[i] = msg_recv[i];
        i++;
    }
    return atoi(num);
}

void thread_client2client(struct thread_data td)
{
    char msg_recv[1000];
    int dest;
    char msg[1000];
    strcpy(msg_recv,"");
    while(strcmp(msg_recv,"bye"))
    {
        int msg_len = receive_from_socket(td.s, msg_recv, 1000); // receive message from client
        if(msg_len==0) return;
        // extract id
        dest = str2num(msg_recv)- 100;
        // extract msg
        int l = strlen(msg_recv);
        int i = 0;
        while(msg_recv[l-i-1]!=' ')
        {
            msg[i] = msg_recv[l-i-1];
            i++;
        }
        msg[i] = '\0';
        bool success = send_to_socket(client_sockets[dest], strrev(msg)); // sent message to client
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

    int i = 0; // id

    while(1)
    {
        printf("Waiting for  connections...");

        client_sockets[i] = accept_connection(server_socket);

        printf("Connection %d accepted", i);
        char first_msg[100] = "Your ID is ";
        char id[5];
        itoa(i+100,id,10);
        strcat(first_msg, id);
        bool success = send_to_socket(client_sockets[i], first_msg); // sent message to client
        if(!success)
            printf("Send failed.\n\n");

        //create sending and receiving threads

        struct thread_data td;
        td.s = client_sockets[i]; // own socket
        td.id_ = i; // own id
        create_socket_thread(thread_client2client, td); // create new thread
        i++;
    }

    //finally process cleanup job
    //closesocket(server_socket);
    //closesocket(client_socket);
    WSACleanup();
    return 0;
}
