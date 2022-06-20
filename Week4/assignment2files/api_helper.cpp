#include<string.h>

struct api_user
{
    int id;
    char name[20];
    char email[50];
};

struct api_user api_users[100]; //maximum is 100 api_users
int n_api_users;

void read_from_file()
{
    FILE * fp = fopen("api.db", "rt");
    int userid;
    n_api_users = 0;
    while( fscanf(fp,"%d", &userid)==1 )
    {
        api_users[n_api_users].id = userid;
        fscanf(fp,"%s",api_users[n_api_users].name);
        fscanf(fp,"%s",api_users[n_api_users].email);
        n_api_users++;
    }
    fclose(fp);
}
void write_to_file()
{
    FILE * fp = fopen("api.db", "wt");
    int i;
    for(i=0; i<n_api_users; i++)
        fprintf(fp,"%d %s %s\n", api_users[i].id, api_users[i].name, api_users[i].email);
    fclose(fp);
}

int create_record(char name[], char email[])
{
    //response to a POST request
    read_from_file();

    int max_id = 0, i;
    for(i=0; i<n_api_users; i++)
        if(api_users[i].id>max_id) max_id = api_users[i].id;

    api_users[n_api_users].id = max_id + 1;
    strcpy(api_users[n_api_users].name, name);
    strcpy(api_users[n_api_users].email, email);
    n_api_users++;

    write_to_file();
    return 1;
}

int get_record(int id, char name[], char email[])
{
    read_from_file();
    int i;
    for(i=0; i<n_api_users; i++)
    {
        if(api_users[i].id==id)
        {
            strcpy(name, api_users[i].name);
            strcpy(email, api_users[i].email);
            return 1;
        }
    }
    return 0;
}

int update_record(int id, char name[], char email[])
{
    //response to a PUT request
    read_from_file();
    int i, found = 0;
    for(i=0; i<n_api_users; i++)
    {
        if(api_users[i].id==id)
        {
            strcpy(api_users[i].name, name);
            strcpy(api_users[i].email, email);
            found = 1;
        }
    }

    if(found)
    {
        write_to_file();
        return 1;
    }

    return 0;
}

int delete_record(int id)
{
    //response to a PUT request
    read_from_file();
    int i, found = 0;
    for(i=0; i<n_api_users; i++)
    {
        if(api_users[i].id==id)
        {
            found = 1;
            break;
        }
    }

    if(found)
    {
        for(i=i+1; i<n_api_users; i++)
            api_users[i-1] = api_users[i];
        n_api_users--;
        write_to_file();
        return 1;
    }

    return 0;
}

