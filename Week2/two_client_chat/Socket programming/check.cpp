#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int str2num(char msg_recv[]){
    int i = 0;
    char num[3];
    while(msg_recv[i]!=' ')
    {
        num[i] = msg_recv[i];
        i++;
    }
    return atoi(num);
}

char* str2msg(char msg_recv[]){
    int i, l = 0;
    char msg[100];
    l = strlen(msg_recv);
    while(msg_recv[l-i-1]!=' ')
    {
        msg[i] = msg_recv[l-i-1];
        i++;
    }
    msg[i] = '\0';
    msg[0] = '0';
//    char * msg2 = strrev(msg);
    return msg;
}

int main(void){
    char str[100];
    int num;
    gets(str);
    printf("string: %s\n", str);
    char * msg3 = str2msg(str);
    printf("message: %s\n", msg3);
    num = str2num(str);
    printf("number: %d\n", num);
}
