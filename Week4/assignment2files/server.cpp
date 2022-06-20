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
    char type[100], path[100], proto[100], name_part[100], email_part[100],  name[50], email[50], str_id[50], id_part[50], apid[50];

    request = get_token(request, " ", type);

    request = get_token(request, " ", path);

    request = get_token(request, " ", proto);

    // for </api/id>
    char * p = path;
    p = get_token(p, "/", apid);
    p = get_token(p, "/", apid);

    if (strcmp(proto,"HTTP/1.1")==0 && strlen(path)!=0)
    {
        if(strcmp(type,"GET")==0)
        {
            if(strcmp(path,"/")==0) // index.html
            {
                //prepare a response
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
            else if (strcmp(path,"/api")==0) // <api>
            {
                char * p = req_body;
                p = get_token(p, "=", str_id);
                p = get_token(p, "=", str_id);
                int id = atoi(str_id);
                int status = get_record(id,name,email);
                if (status)
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
            else if (strlen(apid)!=0) // </api/id> format
            {
                int id = atoi(apid);
                int status = get_record(id,name,email);
                if (status)
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
            else if (path[0]=='/') //root directory
            {
                // for /contact.html
                char *path2 = path+1; // remove first ch in string
                int status = read_html_file(path2, file_contents);
                if(status)
                {
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
                sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
            }
        }
        else if(strcmp(type, "POST")==0)
        {
            if(strcmp(path,"/api")==0)
            {
                int error_flag = 0;
                char * p = req_body;
                p = get_token(p, "&", name_part);
                p = get_token(p, "&", email_part);

                p = name_part;
                p = get_token(p, "=", name);
                if (strcmp(name, "name")!=0) // check if <name> is in correct form
                {
                    sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
                    error_flag = 1;
                }
                p = get_token(p, "=", name); //second token

                p = email_part;
                p = get_token(p, "=", email);
                if (strcmp(email, "email")!=0) // check if <name> is in correct form
                {
                    sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
                    error_flag = 1;
                }
                p = get_token(p, "=", email); //second token

                printf("Name = %s and email = %s\n", name, email);
                if (strlen(name)==0 || strlen(email)==0)
                {
                    sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
                    error_flag = 1;
                }
                if(!error_flag)
                {
                    int status = create_record(name, email);
                    if(status)
                    {
                        sprintf(response, "HTTP/1.1 201 CONTENT CREATED\nContent-length:0");
                    }
                }

            }
            else
            {
                sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
            }

        }
        else if(strcmp(type, "PUT")==0)
        {
            if(strcmp(path,"/api")==0)
            {
                char * p = req_body;
                p = get_token(p, "&", id_part);
                p = get_token(p, "&", name_part);
                p = get_token(p, "&", email_part);

                p = id_part;
                p = get_token(p, "=", str_id);
                int error_flag = 0;
                if (strcmp(str_id, "id")!=0) // check if <id> is in correct form
                {
                    sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
                    error_flag = 1;
                }
                p = get_token(p, "=", str_id); // second token
                int id = atoi(str_id);

                p = name_part;
                p = get_token(p, "=", name);
                if (strcmp(name, "name")!=0) // check if <name> is in correct form
                {
                    sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
                    error_flag = 1;
                }
                p = get_token(p, "=", name); //second token

                p = email_part;
                p = get_token(p, "=", email);
                if (strcmp(email, "email")!=0) // check if <email> is in correct form
                {
                    sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
                    error_flag = 1;
                }
                p = get_token(p, "=", email); //second token

                // check if malformed
                if (strlen(str_id)==0 || strlen(name)==0 || strlen(email)==0)
                {
//                printf(">> malformed: name or email");
                    sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
                    error_flag = 1;
                }
                printf("Id = %d | Name = %s | email = %s\n",id, name, email);
                if (!error_flag)
                {
                    int status = update_record(id, name, email);
                    if(status)
                    {
                        sprintf(response, "HTTP/1.1 204 NO CONTENT\nContent-length:0");
                    }
                    else
                    {
                        sprintf(response, "HTTP/1.1 404 NOT FOUND\nContent-length:0");
                    }
                }
            }
            else
            {
                sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
            }
        }

        else if(strcmp(type, "DELETE")==0)
        {
            if(strcmp(path,"/api")==0)
            {
                char * p = req_body;
                p = get_token(p, "=", str_id);
                p = get_token(p, "=", str_id);
                int id = atoi(str_id);
                int status = delete_record(id);
                if (status)
                {
                    sprintf(response, "HTTP/1.1 204 NO CONTENT\nContent-length:0");

                }
                else
                {
                    sprintf(response, "HTTP/1.1 404 NOT FOUND\nContent-length:0");
                }
            }
            else
            {
                sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
            }
        }
    }
    else
    {
        sprintf(response, "HTTP/1.1 400 BAD REQUEST\nContent-length:0");
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



