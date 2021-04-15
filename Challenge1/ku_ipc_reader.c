#include <stdio.h>
#include <string.h>
#include "ku_ipc_lib.c"

struct real_data
{
    short age;
    char name[16];
};

struct ku_msg_lib
{
    long type;
    char a[128];
};

int main(void)
{
    int qid;
    int ret;
    int type;
    int size;

    printf("Choose queue ID : ");
    scanf("%d", &qid);

    printf("Choose type : ");
    scanf("%d", &type);

    printf("Input Size : ");
    scanf("%d", &size);

    ret = ku_msgget(qid, KU_IPC_CREATE);

    if (ret == -1)
    {
        printf("Not Open\n");
        return 0;
    }
    struct ku_msg_lib receive_test;

    int receive_result = ku_msgrcv(qid, &receive_test, size, type, 0);
    printf("Receive Result : %d\n", receive_result);
    if (receive_result != -1)
    {
        printf("type: %ld, string:%s\n", receive_test.type, receive_test.a);
    }

    ku_msgclose(qid);
}