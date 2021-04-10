// 하나의 Queue에 최대 들어갈 수 있는 Message 개수
#define KUIPC_MAXMSG 10
// 하나의 Queue에 존재하는 Message들의 사이즈 총합 제한
#define KUIPC_MAXVOL 2560

#define KU_IPC_CREATE 0x01
#define KU_IPC_EXCL 0x02
#define KU_IPC_NOWAIT 0x04
#define KU_MSG_NOERROR 0x08