
#ifndef _COMMON_H_
#define _COMMON_H_
typedef void (*sighandler_t)(int);
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <pthread.h>

// 打印错误信息宏函数
#define ERR_MSG(msg)                                                               \
    do                                                                             \
    {                                                                              \
        fprintf(stderr, "行号%d 函数%s 文件名%s\n", __LINE__, __func__, __FILE__); \
        perror(msg);                                                               \
    } while (0)

struct cli // 客户端消息结构体，套接字，客户端ip，数据库指针
{
    int newfd;
    struct sockaddr_in cin;
    sqlite3 *db;
};

#define USER_LOGIN 0x00000000  // login	登陆    0x00000001
#define USER_MODIFY 0x00000001 // user-modification  修改
#define USER_QUERY 0x00000002  // user-query   查询

#define ADMIN_LOGIN 0x10000000   // login	登陆    0x00000001
#define ADMIN_MODIFY 0x10000001  // admin修改
#define ADMIN_ADDUSER 0x10000002 // admin添加
#define ADMIN_DELUSER 0x10000004 // admin删除
#define ADMIN_QUERY 0x10000008   // 查询

#define ADMIN 0 // 管理员
#define USER 1  // 用户

/*员工基本信息*/
typedef struct staff_info
{
    int no;         // 员工编号
    int usertype;   // ADMIN 1	USER 2
    char name[32];  // 姓名
    char passwd[8]; // 密码
    int age;        // 年龄
    char phone[32]; // 电话
    char addr[128]; // 地址
    char work[128]; // 职位
    char date[238]; // 入职年月
    int stock;      // 股权
    double salary;  // 工资
} staff_info_t;

/*定义双方通信的结构体信息*/
typedef struct
{
    int msgtype;       // 请求的消息类型
    int usertype;      // ADMIN 0	USER 1
    char username[32]; // 姓名
    char passwd[16];   // 登陆密码
    char recvmsg[128]; // 通信的消息
    int flags;         // 标志位 1成功0失败    0全部员工信息  1id查询   2姓名查询  3普通用户查询
    staff_info_t info; // 员工信息
} MSG;
#endif