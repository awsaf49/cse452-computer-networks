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
        else if (strcmp(command, "arp -a") == 0) // show mac table
        {
            printf("Printing ARP table\n");
            if (arp_table_size==0)
            {
                printf("There are no entry in ARP Table.\n");
            }
            else
            {
                int i;
                for (i = 0; i < arp_table_size; i++)
                {
                    printf("Entry: %d =>", i+1);
                    printf(" IP: %s, and", arp_table[i].ip);
                    printf(" MAC: %s\n", arp_table[i].mac);
                }
            }
        }

        else if (strcmp(command, "arp -d") == 0) // delete arp table
        {
            printf("Deleted All the entries\n");
            memset(arp_table, 0, sizeof(arp_table));
            arp_table_size = 0;
        }
        else // find mac
        {
            // extract ip from command
            char ip[5];
            extract_ip(ip, command + 9);

            // search it in arp table
            int index = find_index_using_ip(arp_table, arp_table_size, ip);
            if (index==-1)  // if ip not found in arp table then make arp request
            {
                strcpy(frame->mac_source, data.mac); // my mac

                char dest_mac[7];
                generate_broadcast_mac(dest_mac);
                strcpy(frame->mac_destination, dest_mac);

                // update arp payload for arp request
                frame->payload.operation = 0; // 0 for arp request
                strcpy(frame->payload.sender_hardware_address, data.mac); // my mac;
                strcpy(frame->payload.sender_protocol_address, data.ip); // my ip
                strcpy(frame->payload.target_protocol_address, ip); // ip we are searching

                // send frame to server(switch)
                send_ethernet_frame(data.s, frame);

            }
            else  // if ip found in arp table then access it
            {
                char mac[7];
                strcpy(mac, arp_table[index].mac);
                printf("ARP entry found.\n");
                printf("Entry: %d =>", index+1);
                printf(" IP: %s, and", ip);
                printf(" MAC: %s\n", mac);

            }
        }

        //

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
            int frame_op;
            char mac_sender[7];
            char ip_sender[5];
            char ip_search[5];

            // Update arp table
            strcpy(mac_sender, frame->payload.sender_hardware_address); // sender mac;
            strcpy(ip_sender, frame->payload.sender_protocol_address); // sender ip
            strcpy(ip_search, frame->payload.target_protocol_address); // ip we are searching

            // search sender ip in local arp if not expert update arp table
            int index = find_index_using_ip(arp_table, arp_table_size, ip_sender);
            if (index==-1) // ip not found in arp table
            {
                strcpy(arp_table[arp_table_size].ip, ip_sender);
                strcpy(arp_table[arp_table_size].mac, mac_sender);
                arp_table_size++;
            }

            // ARP request and reply
            if (frame->payload.operation==0) // ARP request
            {
                if (strcmp(data.ip, ip_search)==0) // match found
                {

                strcpy(frame->mac_destination, frame->mac_source); // convert sender mac to target mac
                strcpy(frame->mac_source, data.mac); // update sender mac with my mac

                // update arp payload for arp request
                frame->payload.operation = 1; // 1 for arp reply
                strcpy(frame->payload.target_hardware_address,frame->payload.sender_hardware_address); // convert sender mac to target mac
                strcpy(frame->payload.target_protocol_address,frame->payload.sender_protocol_address); // convert sender ip to target ip
                strcpy(frame->payload.sender_hardware_address, data.mac); // my mac;
                strcpy(frame->payload.sender_protocol_address, data.ip); // my ip

                // send frame to server(switch)
                send_ethernet_frame(data.s, frame);
                }
            }

            //

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
