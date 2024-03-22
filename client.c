#include "myhead.h"

#include "myhead.h"

void print_menu(int t);                           //打印的选项菜单
void admin_or_user_login(int sockfd, MSG *msg);   //普通员工和管理员登录
void admin_menu(int sockfd, MSG *msg);            //管理员成功登录系统
void user_menu(int sockfd, MSG *msg);             //普通员工成功登录系统
void do_admin_adduser(int sockfd, MSG *msg);      //管理员添加新员工
void do_admin_query(int sockfd, MSG *msg);        //管理员查询员工信息
void do_admin_deluser(int sockfd, MSG *msg);      //管理员删除员工信息
void do_admin_modification(int sockfd, MSG *msg); //管理员修改员工信息

void do_user_query(int sockfd, MSG *msg);         //普通员工查看自己的个人信息
void do_user_modification(int sockfd, MSG *msg);  //普通员工修改自己的密码

void clear_input_buffer();

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return -1;
    }

    //创建流式套接字
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        perror("socket");
        return -1;
    }

    //填充要连接的服务器的地址信息结构体
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(atoi(argv[2]));        // 端口号
    sin.sin_addr.s_addr = inet_addr(argv[1]);   // IP地址

    //连接服务器 connect
    if (connect(sfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("connect");
        return -1;
    }
    printf("连接服务器成功...\n");

    int choice;

    while (1)
    {
        print_menu(1);
        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice)
        {
        case 1:
            {
                MSG msg;
                msg.msgtype = ADMIN_LOGIN;
                msg.usertype = ADMIN;
                admin_or_user_login(sfd, &msg);
                break;
            }
        case 2:
            {
                MSG msg;
                msg.msgtype = USER_LOGIN;
                msg.usertype = USER;
                admin_or_user_login(sfd, &msg);
                break;
            }
        case 0:
            exit(0);
        default:
            printf("输入错误，请重新输入\n");
            break;
        }
    }

    return 0;
}

void print_menu(int t)
{
    if (t == 1)
    {
        printf("\n*****************************************\n");
        printf("\t\t员工管理系统\n");
        printf("*****************************************\n");
        printf("\t1. root 登录(admin 123456)\n");
        printf("\t2. user 登录\n");
        printf("\t0. 退出\n");
        printf("*****************************************\n");
        printf("请选择: ");
    }
    else if (t == 2)
    {
        printf("\n*****************************************\n");
        printf("\t\t管理员菜单\n");
        printf("*****************************************\n");
        printf("\t1. 添加员工\n");
        printf("\t2. 修改员工信息\n");
        printf("\t3. 删除员工\n");
        printf("\t4. 查询员工信息\n");
        printf("\t0. 返回上一页\n");
        printf("*****************************************\n");
        printf("请选择: ");
    }
    else if (t == 3)
    {
        printf("\n*****************************************\n");
        printf("\t\t普通员工菜单\n");
        printf("*****************************************\n");
        printf("\t1. 查询个人信息\n");
        printf("\t2. 修改密码\n");
        printf("\t0. 返回上一页\n");
        printf("*****************************************\n");
        printf("请选择: ");
    }
}

void admin_or_user_login(int sockfd, MSG *msg)
{
    do
    {
        printf("请输入用户名: ");
        fgets(msg->username, sizeof(msg->username), stdin);
        msg->username[strcspn(msg->username, "\n")] = '\0';

        printf("请输入密码: ");
        fgets(msg->passwd, sizeof(msg->passwd), stdin);
        msg->passwd[strcspn(msg->passwd, "\n")] = '\0';

        if (send(sockfd, msg, sizeof(MSG), 0) < 0)
        {
            perror("send");
        }

        int res = recv(sockfd, msg, sizeof(MSG), 0);
        if (res < 0)
        {
            perror("recv");
            return;
        }
        printf("%s\n", msg->recvmsg);
    } while (msg->flags != 1);

    if (msg->usertype == 0)
    {
        admin_menu(sockfd, msg);     //管理员登录
    }
    else if (msg->usertype == 1)
    {
        user_menu(sockfd, msg);     //普通员工登录
    }
    else
        printf("服务器错误...\n");
}

void admin_menu(int sockfd, MSG *msg)
{
    int choice;
    while (1)
    {
        print_menu(2);
        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice)
        {
        case 1:
            do_admin_adduser(sockfd, msg);
            break;
        case 2:
            do_admin_modification(sockfd, msg);
            break;
        case 3:
            do_admin_deluser(sockfd, msg);
            break;
        case 4:
            do_admin_query(sockfd, msg);
            break;
        case 0:
            return;
        default:
            printf("输入错误，请重新输入...\n");
            break;
        }
    }
}

void user_menu(int sockfd, MSG *msg)
{
    int choice;
    while (1)
    {
        print_menu(3);
        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice)
        {
        case 1:
            do_user_query(sockfd, msg);
            break;
        case 2:
            do_user_modification(sockfd, msg);
            break;
        case 0:
            return;
        default:
            printf("输入错误，请重新输入...\n");
            break;
        }
    }
}

void do_admin_adduser(int sockfd, MSG *msg)
{
    staff_info_t info_t = msg->info; //保存自己的个人信息
    msg->msgtype = ADMIN_ADDUSER;
    printf("请输入员工ID: ");
    while (scanf("%d", &msg->info.no) == 0)
    {
        printf("请输入正确的数字...\n");
        printf("请重新输入员工ID: ");
        while (getchar() != 10)
            ;
    }
    while (getchar() != 10)
        ;
    printf("请输入用户类型(0:root,1:user): ");
    while (scanf("%d", &msg->info.usertype) == 0)
    {
        printf("请输入正确的数字...\n");
        printf("请重新输入用户类型(0:root,1:user): ");
        while (getchar() != 10)
            ;
    }
    while (getchar() != 10)
        ;
    printf("请输入姓名: ");
    fgets(msg->info.name, sizeof(msg->info.name), stdin);
    msg->info.name[strlen(msg->info.name) - 1] = 0;
    printf("请输入密码: ");
    fgets(msg->info.passwd, sizeof(msg->info.passwd), stdin);
    msg->info.passwd[strlen(msg->info.passwd) - 1] = 0;
    printf("请输入年龄: ");
    while (scanf("%d", &msg->info.age) == 0)
    {
        printf("请输入正确的数字\n");
        printf("请重新输入年龄: ");
    }
    while (getchar() != 10)
        ;
    printf("请输入电话: ");
    fgets(msg->info.phone, sizeof(msg->info.phone), stdin);
    msg->info.phone[strlen(msg->info.phone) - 1] = 0;
    printf("请输入地址: ");
    fgets(msg->info.addr, sizeof(msg->info.addr), stdin);
    msg->info.addr[strlen(msg->info.addr) - 1] = 0;
    printf("请输入职位: ");
    fgets(msg->info.work, sizeof(msg->info.work), stdin);
    msg->info.work[strlen(msg->info.work) - 1] = 0;
    printf("请输入入职时间: ");
    fgets(msg->info.date, sizeof(msg->info.date), stdin);
    msg->info.date[strlen(msg->info.date) - 1] = 0;
    printf("请输入股权: ");
    while (scanf("%d", &msg->info.stock) == 0)
    {
        printf("请输入正确的数字...n");
        printf("请重新输入股权: ");
        while (getchar() != 10)
            ;
    }
    while (getchar() != 10)
        ;
    printf("请输入工资: ");
    while (scanf("%lf", &msg->info.salary) == 0)
    {
        printf("请输入正确的数字...\n");
        printf("请重新输入工资: ");
        while (getchar() != 10)
            ;
    }
    while (getchar() != 10)
        ;
 
    if (send(sockfd, msg, sizeof(MSG), 0) < 0)
    {
        ERR_MSG("send");
    }
    int res = recv(sockfd, msg, sizeof(MSG), 0);
    if (res < 0)
    {
        ERR_MSG("recv");
        return;
    }
    memset(&msg->info, 0, sizeof(msg->info)); // 添加这一行
    msg->info = info_t; //恢复自己的个人信息
    printf("%s\n", msg->recvmsg);
}
void do_admin_query(int sockfd, MSG *msg)
{
    int num;
    msg->msgtype = ADMIN_QUERY;
    int which;
    printf("1.查询所有员工信息 2.按照ID查询 3.按照名字查询 0.返回上一页\n");
    printf("请选择: ");
    scanf("%d", &which);
    while (getchar() != 10)
        ;
    switch (which)
    {
    case 1:
        msg->flags = 0;
        break;
    case 2:
        msg->flags = 1;
        printf("请输入ID: ");
        fgets(msg->recvmsg, sizeof(msg->recvmsg), stdin);
        msg->recvmsg[strlen(msg->recvmsg) - 1] = 0;
        break;
    case 3:
        msg->flags = 2;
        printf("请输入名字: ");
        fgets(msg->recvmsg, sizeof(msg->recvmsg), stdin);
        msg->recvmsg[strlen(msg->recvmsg) - 1] = 0;
        break;
    case 0:
        return;
    default:
        printf("输入错误...\n");
        break;
    }
    if (send(sockfd, msg, sizeof(MSG), 0) < 0)
    {
        ERR_MSG("send");
        return;
    }
    int res = recv(sockfd, msg, sizeof(MSG), 0);
    if (res < 0)
    {
        ERR_MSG("recv");
        return;
    }
    num = atoi(msg->recvmsg);
    if (num == 0)
    {
        printf("没有查询到此员工...\n");
        return;
    }
    printf("员工编号\t权限\t姓名\t密码\t年龄\t电话\t地址\t职位\t入职年月\t股权\t工资\n");
    while (num--)
    {
        res = recv(sockfd, msg, sizeof(MSG), 0);
        if (res < 0)
        {
            ERR_MSG("recv");
            return;
        }
        printf("%d\t%d\t%s\t%s\t%d\t%s\t%s\t%s\t%s\t%d\t%.2lf\n", msg->info.no, msg->info.usertype, msg->info.name, msg->info.passwd, msg->info.age, msg->info.phone, msg->info.addr, msg->info.work, msg->info.date, msg->info.stock, msg->info.salary);
    }
}
 


void do_admin_deluser(int sockfd, MSG *msg)
{
    char buf[16];
    msg->msgtype = ADMIN_DELUSER;
    printf("请输入要删除的用户id: ");
    fgets(msg->recvmsg, sizeof(msg->recvmsg), stdin);
    msg->recvmsg[strlen(msg->recvmsg) - 1] = 0;
    if (atoi(msg->recvmsg) == msg->info.no)
    {
        printf("不可以删除自己...\n");
        return;
    }
    if (send(sockfd, msg, sizeof(MSG), 0) < 0)
    {
        ERR_MSG("send");
        return;
    }
    strcpy(buf, msg->recvmsg);
    int res = recv(sockfd, msg, sizeof(MSG), 0);
    if (res < 0)
    {
        ERR_MSG("recv");
        return;
    }
    printf("%s\n", msg->recvmsg);
    return;
}

void do_admin_modification(int sockfd, MSG *msg)
{
    staff_info_t info_t = msg->info; //保存自己的个人信息
    msg->msgtype = ADMIN_MODIFY;
    printf("请输入要修改的id: ");
    fgets(msg->recvmsg, sizeof(msg->recvmsg), stdin);
    msg->recvmsg[strlen(msg->recvmsg) - 1] = 0;
    msg->info.no = atoi(msg->recvmsg);
    printf("请输入用户类型(0:root,1:user): ");
    while (scanf("%d", &msg->info.usertype) == 0)
    {
        printf("请输入正确的数字\n");
        printf("请重新输入用户类型(0:root,1:user): ");
        while (getchar() != 10)
            ;
    }
    while (getchar() != 10)
        ;
    printf("请输入姓名: ");
    fgets(msg->info.name, sizeof(msg->info.name), stdin);
    msg->info.name[strlen(msg->info.name) - 1] = 0;
    printf("请输入密码: ");
    fgets(msg->info.passwd, sizeof(msg->info.passwd), stdin);
    msg->info.passwd[strlen(msg->info.passwd) - 1] = 0;
    printf("请输入年龄: ");
    while (scanf("%d", &msg->info.age) == 0)
    {
        printf("请输入正确的数字...\n");
        printf("请重新输入年龄: ");
        getchar();
    }
    while (getchar() != 10)
        ;
    printf("请输入电话: ");
    fgets(msg->info.phone, sizeof(msg->info.phone), stdin);
    msg->info.phone[strlen(msg->info.phone) - 1] = 0;
    printf("请输入地址: ");
    fgets(msg->info.addr, sizeof(msg->info.addr), stdin);
    msg->info.addr[strlen(msg->info.addr) - 1] = 0;
    printf("请输入职位: ");
    fgets(msg->info.work, sizeof(msg->info.work), stdin);
    msg->info.work[strlen(msg->info.work) - 1] = 0;
    printf("请输入入职时间: ");
    fgets(msg->info.date, sizeof(msg->info.date), stdin);
    msg->info.date[strlen(msg->info.date) - 1] = 0;
    printf("请输入股权: ");
    while (scanf("%d", &msg->info.stock) == 0)
    {
        printf("请输入正确的数字...\n");
        printf("请重新输入股权: ");
        while (getchar() != 10)
            ;
    }
    while (getchar() != 10)
        ;
    printf("请输入工资>>>");
    while (scanf("%lf", &msg->info.salary) == 0)
    {
        printf("请输入正确的数字...\n");
        printf("请重新输入工资: ");
        while (getchar() != 10)
            ;
    }
    while (getchar() != 10)
        ;
 
    if (send(sockfd, msg, sizeof(MSG), 0) < 0)
    {
        ERR_MSG("send");
    }
    int res = recv(sockfd, msg, sizeof(MSG), 0);
    if (res < 0)
    {
        ERR_MSG("recv");
        return;
    }
    msg->info = info_t; //恢复自己的个人信息
    printf("%s\n", msg->recvmsg);
}

void do_user_query(int sockfd, MSG *msg)
{
    msg->msgtype = USER_QUERY;
    msg->flags = 3;
    sprintf(msg->recvmsg, "%d", msg->info.no);
    if (send(sockfd, msg, sizeof(MSG), 0) < 0)
    {
        ERR_MSG("send");
        return;
    }
    int res = recv(sockfd, msg, sizeof(MSG), 0);
    if (res < 0)
    {
        ERR_MSG("recv");
        return;
    }
    int num = atoi(msg->recvmsg);
    if (num == 0)
    {
        printf("没有查询到此员工...\n");
        return;
    }
    printf("员工编号\t权限\t姓名\t密码\t年龄\t电话\t地址\t职位\t入职年月\t股权\t工资\n");
    while (num--)
    {
        res = recv(sockfd, msg, sizeof(MSG), 0);
        if (res < 0)
        {
            ERR_MSG("recv");
            return;
        }
        printf("%d\t%d\t%s\t%s\t%d\t%s\t%s\t%s\t%s\t%d\t%.2lf\n", msg->info.no, msg->info.usertype, msg->info.name, msg->info.passwd, msg->info.age, msg->info.phone, msg->info.addr, msg->info.work, msg->info.date, msg->info.stock, msg->info.salary);
    }
}

void do_user_modification(int sockfd, MSG *msg)
{
    msg->msgtype = USER_MODIFY;
    printf("请输入新的密码: ");
    fgets(msg->recvmsg, sizeof(msg->recvmsg), stdin);
    msg->recvmsg[strlen(msg->recvmsg) - 1] = 0;
    if (send(sockfd, msg, sizeof(MSG), 0) < 0)
    {
        ERR_MSG("send");    //发送失败
        return;
    }
    int res = recv(sockfd, msg, sizeof(MSG), 0);
    if (res < 0)
    {
        ERR_MSG("recv");    //接收失败
        return;
    }
    printf("%s\n", msg->recvmsg);
}

void clear_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}