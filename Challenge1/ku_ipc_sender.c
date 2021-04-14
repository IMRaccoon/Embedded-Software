#include <stdio.h>
#include <string.h>
#include "ku_ipc_lib.c"

struct real_data
{
    short age;
    char name[32];
};

struct ku_msg_lib
{
    long type;
    struct real_data data;
};

int main(void)
{
    int qid;
    int ret;
    int type;

    int age;
    char *name = malloc(sizeof(char) * 128);

    printf("Choose queue ID : ");
    scanf("%d", &qid);

    printf("Choose type : ");
    scanf("%d", &type);

    printf("Input Age: ");
    scanf("%d", &age);

    printf("Input name: ");
    scanf("%s", name);

    ret = ku_msgget(qid, KU_IPC_CREATE);

    if (ret == -1)
    {
        printf("Not Open\n");
        return 0;
    }
    struct ku_msg_lib send_test = {
        .type = type,
        .data = {age}};

    strcpy(send_test.data.name, name);

    int send_result = ku_msgsnd(qid, &send_test, sizeof(send_test.data), 0);
    printf("Send Result : %d\n", send_result);

    ku_msgclose(qid);
}