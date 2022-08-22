#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

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
        mac[i] = rand() % 256;
    mac[i] = '\0';
}

void generate_broadcast_mac(char *mac)
{
    int i;
    for (i = 0; i < 6; i++)
        mac[i] = 255;
    mac[i] = '\0';
}

void generate_ip(char *ip)
{
    srand(time(0));

    ip[0] = 192;
    ip[1] = 168;
    ip[2] = rand() % 256;
    ip[3] = rand() % 256;
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

int main()
{
    char mac[7];
    char ip[5];
    generate_mac(mac);
    generate_ip(ip);

    printf("<<");
    print_mac(mac);
    printf(">>\n");

    printf("<<");
    print_ip(ip);
    printf(">>\n");

    generate_broadcast_mac(mac);
    generate_ip(ip);

    printf("<<");
    print_mac(mac);
    printf(">>\n");

    printf("<<");
    print_ip(ip);
    printf(">>\n");


    char find_mac_command[] = "find_mac 192.168.1.20";
    extract_ip(ip, find_mac_command + 9);

    printf("<<");
    print_ip(ip);
    printf(">>\n");

    return 0;
}