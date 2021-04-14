#include <stdio.h>
#include "ku_ipc_lib.c"

struct ku_msg_lib
{
    long type;
    char str[128];
};

int main(void)
{
    int qid = ku_msgget(9, KU_IPC_CREATE);

    printf("%d\n", qid);
    struct ku_msg_lib receive_test;
    struct ku_msg_lib send_test =
        {
            .type = 2,
            .str = "test string"};

    int send_result = ku_msgsnd(qid, &send_test, sizeof("test string"), KU_IPC_NOWAIT);
    printf("Send Result : %d\n", send_result);

    int receive_result = ku_msgrcv(qid, &receive_test, sizeof(send_test.str), 2, KU_IPC_NOWAIT);
    printf("Receive Result : %d\n", receive_result);
    if (receive_result != -1)
    {
        printf("type: %ld, str: %s\n", receive_test.type, receive_test.str);
    }
    ku_msgclose(qid);
}