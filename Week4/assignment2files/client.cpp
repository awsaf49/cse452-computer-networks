#include "network_helper.cpp"
#include "http_helper.cpp"

void thread_to_send(struct thread_data td)
{

    SOCKET s = td.s;

    char msg_send[1000], msg_recv[1000];
    input_http_request(msg_send);

    bool success = send_to_socket(s, msg_send); // to server

    int msg_len = receive_from_socket(s, msg_recv, 1000); // from server

    if(msg_len>0)
    {
        printf("<--- \n\n %s \n\n -->\n", msg_recv);
    }

}


int main(int argc, char *argv[])
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
    //go to sleep
    sleep_for_ever();


    //finally process cleanup job
    closesocket(client_socket);
    WSACleanup();
    return 0;
}








