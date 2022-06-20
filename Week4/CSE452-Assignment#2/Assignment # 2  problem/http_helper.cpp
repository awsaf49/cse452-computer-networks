//the following functions copies the header part of the http request or respose in header

//**************string tokenizer********

//src = source string to search for token
//delim = delimeters for token
//buf = memory where token is stored
//returns the address of the rest string


int is_white_space(char ch)
{
    if(ch==' ' || ch=='\t' || ch=='\n' || ch=='\r') return 1;
    else return 0;
}


int is_delim(char ch, char * delim)
{
    int i = 0, j;
    while(delim[i]) i++; //find length of delim string
    for(j=0; j<i; j++)
        if(delim[j]==ch) return 1;
    return 0;
}

//string tokenizer
char * get_token(char * src, char * delim, char * buf)
{
    buf[0] = '\0';
    if(src==NULL) return NULL;

    int i = 0;
    while( is_white_space(src[i]) || is_delim(src[i], delim) ) i++; //skip initial delim chars or initial whitespaces
    int j = i;
    while( !(is_white_space(src[i]) || is_delim(src[i], delim)) && src[i] )
    {
        buf[i-j] = src[i];
        i++;
    }
    buf[i-j] = '\0';
    while( is_white_space(src[i]) ||  is_delim(src[i], delim) ) i++;

    if(!src[i]) return NULL;
    else return src+i;
}

//returns the length of the header
void get_http_header_body(char http_msg[], char header[], char body[])
{
    int i = 0, j = 0, k = 0;
    header[0] = '\0'; //empty header
    int state = 0;
    while(1)
    {
        switch(http_msg[i])
        {
        case ' ' :
        case '\t':
            if(state==0 || state==2 || state==3) { }
            else if(state==1) { header[j++] = http_msg[i]; }
            else if(state==4) { body[k++] = http_msg[i]; }
            break;
        case '\n':
            if(state==0 || state==3) { }
            else if(state==1) { header[j++] = http_msg[i]; state = 2; }
            else if(state==2) { header[j] ='\0'; state = 3; }
            else if(state==4) { body[k++] = http_msg[i]; }
            break;
        case '\0':
            if(state==0) { state = 5; }
            else if(state==1) { header[j] ='\0'; state = 5; }
            else if(state==2) { header[j] ='\0'; state = 5; }
            else if(state==3) { state = 5; }
            else if(state==4) { body[k] = '\0'; state = 5; }
            break;
        default:
            if(state==0) { header[j++] = http_msg[i]; state = 1; }
            else if(state==1) { header[j++] = http_msg[i]; state = 1; }
            else if(state==2) { header[j++] = http_msg[i]; state = 1; }
            else if(state==3) { body[k++] = http_msg[i]; state = 4; }
            else if(state==4) { body[k++] = http_msg[i]; state = 4; }
        }

        if(state==5) break;

        //printf("%c-", http_msg[i]);
        i++;
    }

    while(j>0 && is_white_space(header[--j])); header[++j] = '\0';
    while(k>0 && is_white_space(body[--k])); body[++k] = '\0';
}

void input_http_request(char http_request[])
{
    int i = 0;
    char ch = '\0';
    while(1)
    {
        scanf("%c",&ch);
        if(i==0 && is_white_space(ch)) continue; //skip initial whitespaces
        if(ch=='>') break;
        http_request[i++] = ch;
    }
    while(i>0 && is_white_space(http_request[--i]));
    http_request[++i] = '\0';
}

int read_html_file(char file_name[], char file_contents[])
{
    FILE * fp;
    fp = fopen(file_name, "rt");
    if(fp==NULL) return 0; //file not found
    int ch, i = 0;;
    while( (ch=fgetc(fp))!=EOF )
    {
        file_contents[i++] = ch;
    }
    file_contents[i] = '\0';
    return 1; //file read successful
}
