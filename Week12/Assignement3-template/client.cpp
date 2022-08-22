#include "helper.cpp"

char my_ip[5];   // eg- 192.178.255.255   (one extra for storing null char at last)
				 // (only used in client part)
char my_mac[7];  // eg- ab:bc:cd:ef:10:12 (one extra for storing null char at last)

struct arp_table_entry
{
    char ip[5];  // eg- 192.168.255.255
    char mac[7]; // eg- ab:bc:cd:ef:10:12
};

arp_table_entry arp_table[255]; // take a big table
int arp_table_size = 0;

// find the index of the arp table entry for the specified ip
int find_index_using_ip(arp_table_entry *arp_table, int table_size, char *ip)
{
    int i = 0;
    for (i = 0; i < table_size; i++)
    {
        if (strcmp(arp_table[i].ip, ip) == 0)
            return i; // find a match so return this ip
    }
    return -1; // for not finding any entry for the specified ip
}

void thread_to_send(thread_data data)
{
    Sleep(100);

    puts("\n\n\n");

    // take a ethernet frame pointer and allocate memory. Allocation memory is important here.
    ethernet_frame *frame = (ethernet_frame *)malloc(sizeof(ethernet_frame));

    // Add your code here

    char receiver_ip[5];
    char command[100];

    while (true)
    {
        gets(command);

        if (strcmp(command, "exit") == 0) // exiting
            exit(0);
        // Add your code here

        puts("\n\n");
    }

    free(frame); // free up the allocated memory before exiting from this function.
}

void thread_to_recv(thread_data data)
{
    // take a ethernet frame pointer and allocate memory. Allocation memory is important here.
    ethernet_frame *frame = (ethernet_frame *)malloc(sizeof(ethernet_frame));

    while (true)
    {
        int msg_len = receive_ethernet_frame(data.s, frame);
        if (msg_len < 0)
            break; // once send or receive fails, please return;
        if (msg_len > 0)
        {
            // Add your code here

            puts("\n\n");
        }
    }

    free(frame); // free up the allocated memoru before exiting from this function.
}

int main(int argc, char *argv[])
{
    bool success = init_networking();
    if (!success)
        return 0;

    SOCKET client_socket = create_socket();
    if (!client_socket)
        return 0;

    char server_ip[16];
    strcpy(server_ip, "127.0.0.1");
    success = connect_to_server(client_socket, server_ip, 8888);
    if (!success)
        return 0;

    // create sending and receiving threads
    thread_data data;

    // Add your code here
    data.s = client_socket;

    printf("Generated IP: ");
    generate_ip(data.ip);
    print_ip(data.ip);
    printf("\n");

    printf("Generated MAC: ");
    generate_mac(data.mac);
    print_mac(data.mac);
    printf("\n\n");
    //
    create_socket_thread(thread_to_send, data);
    create_socket_thread(thread_to_recv, data);

    // go to sleep
    sleep_for_ever();

    // finally process cleanup job
    closesocket(client_socket);
    WSACleanup();
    return 0;
}
