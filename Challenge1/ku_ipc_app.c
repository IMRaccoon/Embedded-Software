#include <stdio.h>
#include "ku_ipc_lib.c"

int main(void)
{
    int qid = ku_msgget(9, KU_IPC_CREATE);

    printf("%d\n", qid);
    struct ku_msg_lib receive_test;
    struct ku_msg_lib send_test =
        {
            .type = 2,
            .str = "test string"};

    int send_result = ku_msgsnd(qid, &send_test, sizeof(send_test.str), KU_IPC_NOWAIT);
    printf("Send Result : %d\n", send_result);

    int receive_result = ku_msgrcv(qid, &receive_test, sizeof(send_test.str), 2, KU_IPC_NOWAIT | KU_MSG_NOERROR);
    printf("Receive Result : %d\n", receive_result);

    ku_msgclose(qid);
}