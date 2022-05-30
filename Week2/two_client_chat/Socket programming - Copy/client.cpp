#include "helper.cpp"



void thread_to_send(struct thread_data td)
{
    SOCKET s = td.s;
    char msg_send[1000];
    while(strcmp(msg_send,"bye"))
    {
        gets(msg_send);
        bool success = send_to_socket(s, msg_send);
        if(!success)
        {
            return; //once send or receive fails, please return;
        }
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
        if( msg_len < 0)
        {
            return; //once send or receive fails, please return;
        }
        if(msg_len>0) printf("%50s\n", msg_recv);
    }
}


int main(int argc , char *argv[])
{
    bool success = init_networking();
    if(!success) return 0;

    SOCKET client_socket = create_socket();
    if(!client_socket) return 0;

    success = connect_to_server(client_socket, "127.0.0.1", 8888);
    if(!success) return 0;

    //create sending and receiving threads
    struct thread_data td;
    td.s = client_socket;

	create_socket_thread(thread_to_send, td);
	create_socket_thread(thread_to_recv, td);


	//go to sleep
    sleep_for_ever();


    //finally process cleanup job
    closesocket(client_socket);
    WSACleanup();
	return 0;
}








