#include "network_helper.cpp"
#include "http_helper.cpp"
#include "api_helper.cpp"
#include <stdlib.h>
#include <string.h>

SOCKET client_sockets[100];

/*
void thread_to_send(struct thread_data td)
{
    SOCKET s = td.s;
    char msg_send[1000];

    sprintf(msg_send, "Your id is %d\n", td.client_id);
    bool success = send_to_socket(s, msg_send);
    if(!success) printf("Send failed.\n\n");
}*/

void thread_to_recv(struct thread_data td)
{
    SOCKET s = td.s; //client socket
    char msg_recv[1000], msg_send[1000];

    int msg_len = receive_from_socket(s, msg_recv, 1000); // receive from client

    //if(msg_len) printf("%s", msg_recv);

    char req_header[1000], req_body[1000];
    char response[1000], file_contents[1000];

    get_http_header_body(msg_recv, req_header, req_body);

    char * request = req_header;
    char type[100], path[100], proto[100], name_part[100], email_part[100],  name[50], email[50], id_part[10];

    request = get_token(request, " ", type);

    request = get_token(request, " ", path);

    request = get_token(request, " ", proto);

    if(strcmp(type,"GET")==0)
    {
        if(strcmp(path,"/")==0 && strlen(proto)!=0)
        {
            //prepare a response
//            printf(">> check fail: %s\n",path);
            int found = read_html_file("index.html", file_contents);
            if(found)
            {
                int con_len = strlen(file_contents);
                sprintf(response, "HTTP/1.1 200 OK\nContent-length:%d\n\n%s",con_len,file_contents);
            }
            else
            {
                sprintf(response, "HTTP/1.1 404 NOT FOUND\nContent-length:0");
            }

        }
        else if (strcmp(path,"/api")==0 && strlen(proto)!=0)
        {
            char * p = req_body;
            p = get_token(p, "=", id_part);
            p = get_token(p, "=", id_part);
            int id = atoi(id_part);
            int record_found = get_record(id,name,email);
            if (record_found)
            {
                sprintf(file_contents, "id=%d&name=%s&email=%s",id,name,email);
                int con_len = strlen(file_contents);
                sprintf(response, "HTTP/1.1 200 OK\nContent-length:%d\n\n%s",con_len,file_contents);

            }
            else
            {
                sprintf(response, "HTTP/1.1 404 NOT FOUND\nContent-length:0");
            }
        }
        else
        {
            //prepare a response
//            printf(">> check: path=%s | proto=%s\n",path,proto);
            if(strlen(path)==0 || strlen(proto)==0) //GET
            {
                sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
            }
            else
            {
                char *path2 = path+1;
//                printf(">> Path: %s\n",path2);
                int found = read_html_file(path2, file_contents);
                if(found)
                {
                    int con_len = strlen(file_contents);
                    sprintf(response, "HTTP/1.1 200 OK\nContent-length:%d\n\n%s",con_len,file_contents);
                }
                else
                {
                    sprintf(response, "HTTP/1.1 404 NOT FOUND\nContent-length:0");
                }
            }
        }
    }
    else if(strcmp(type, "POST")==0)
    {
        if(strcmp(path,"/api")==0)
        {
            char * p = req_body;
            p = get_token(p, "&", name_part);
            p = get_token(p, "&", email_part);

            p = name_part;
            p = get_token(p, "=", name);
            p = get_token(p, "=", name); //second token

            p = email_part;
            p = get_token(p, "=", email);
            p = get_token(p, "=", email); //second token

            printf("Name = %s and email = %s\n", name, email);
            if (strlen(name)==0 || strlen(email)==0)
            {
//                printf(">> malformed: name or email");
                sprintf(response, "HTTP/1.1 404 NOT FOUND\nContent-length:0");
            }
            else
            {
                int status = create_record(name, email);
                if(status)
                {
                    sprintf(response, "HTTP/1.1 204 NO CONTENT\nContent-length:0");
                }
            }

        }

    }

    bool success = send_to_socket(s, response);
    if(!success) printf("Send failed.\n\n");

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

        //create_socket_thread(thread_to_send, td);
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



