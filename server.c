#include "myhead.h"

void handler(int sig);													//回收僵尸进程
void rcv_cli_msg(struct cli *arg);										//处理客户端的请求
void database_init();													//数据库初始化
int process_user_or_admin_login_request(struct cli *cliInfo, MSG *msg); //普通员工和管理员登录
int process_admin_adduser_request(struct cli *cliInfo, MSG *msg);		//管理员添加新员工
int process_admin_query_request(struct cli *cliInfo, MSG *msg);			//管理员查询员工信息
int process_admin_deluser_request(struct cli *cliInfo, MSG *msg);		//管理员删除员工信息
int process_admin_modify_request(struct cli *cliInfo, MSG *msg);		//管理员修改员工信息
void process_user_query_request(struct cli *cliInfo, MSG *msg);			//普通员工查看自己的个人信息
void process_user_modify_request(struct cli *cliInfo, MSG *msg);		//普通员工修改自己的密码
 
int main(int argc, const char *argv[])
{
	if(argc != 2)
	{
		printf("Usage: %s <port>\n", argv[0]);
		return -1;
	}
	//初始化数据库
	database_init();
	sighandler_t s = signal(17, handler);
	if (SIG_ERR == s)
	{
		ERR_MSG("signal");
		return -1;
	}
	//打开数据库
	sqlite3 *db = NULL;
	if (sqlite3_open("./staff.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_open:%s\n", __LINE__, sqlite3_errmsg(db));
		return -1;
	}
	
	//创建套接字
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0)
	{
		ERR_MSG("socket");
		return -1;
	}
	//允许端口快速重用
	int reuse = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		ERR_MSG("setsockopt");
		return -1;
	}
	//填充地址信息结构体
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(atol(argv[1]));
	sin.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY表示本机的任意IP
 
	//绑定端口
	if (bind(sfd, (struct sockaddr *)&sin, sizeof(sin)))
	{
		ERR_MSG("bind");
		return -1;
	}
 
	//设置为被动监听状态
	if (listen(sfd, 10))
	{
		ERR_MSG("listen");
		return -1;
	}
	//客户端地址信息结构体
	struct sockaddr_in cin;
	socklen_t addrlen = sizeof(cin);
 
	int newfd = 0;
	struct cli cliInfo;
	cliInfo.db = db;
	pid_t pid;
	printf("服务器启动成功\n");
	while (1)
	{
		newfd = accept(sfd, (struct sockaddr *)&cin, &addrlen);
		if (newfd < 0)
		{
			ERR_MSG("accept");
			return -1;
		}
		printf("[%s : %d] newfd = %d 连接成功\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
 
		cliInfo.newfd = newfd;
		cliInfo.cin = cin;
		//创建线程客户端和服务器通讯
		pid = fork();
		if (pid > 0)
		{
			close(newfd);
		}
		else if (0 == pid)
		{
			close(sfd);
			rcv_cli_msg(&cliInfo);
			return 0;
		}
	}
	return 0;
}

void handler(int sig) //回收僵尸进程
{
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
}
 
void database_init()
{
	//判断数据库文件是否存在
	if (access("./staff.db", F_OK) == 0)
	{
		return;
	}
	//不存在创建数据库
	sqlite3 *db = NULL;
	if (sqlite3_open("./staff.db", &db) != SQLITE_OK)
	{
		printf("errmsg:%s\n", sqlite3_errmsg(db));
		fprintf(stderr, "__%d__ sqlite3_open failed\n", __LINE__);
		exit(0);
	}
	//创建信息表格
	char sql[256] = "CREATE TABLE `staff` ( `no` int, `usertype` int, `name` char, `passwd` char, `age` int, `phone` char, `addr` char, `work` char, `date` char, `level` int, `salary` double, PRIMARY KEY(`no`));";
	char *errmsg = NULL;
	if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		exit(0);
	}
	//添加管理员admin的账号信息
	strcpy(sql, "insert into staff values (1000,0,'admin','123456',0,'0','0','0','2024-3-12',0,0);");
	if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		exit(0);
	}
	//关闭数据库
	sqlite3_close(db);
	return;
}

void rcv_cli_msg(struct cli *arg)
{
	int newfd = arg->newfd;
	struct sockaddr_in cin = arg->cin;
	MSG msg;
	while (1)
	{
		memset(&msg, 0, sizeof(msg));
		//接收客户端的请求，判断指向的功能函数
		ssize_t res = recv(newfd, &msg, sizeof(msg), 0);
		if (res < 0)
		{
			ERR_MSG("recv");
			break;
		}
		else if (0 == res)
		{
			printf("[%s : %d] newfd = %d 断开连接\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
			break;
		}
		switch (msg.msgtype)
		{
		case USER_LOGIN:
		case ADMIN_LOGIN:
			process_user_or_admin_login_request(arg, &msg);
			break;
		case USER_MODIFY:
			process_user_modify_request(arg, &msg);
			break;
		case USER_QUERY:
			process_user_query_request(arg, &msg);
			break;
		case ADMIN_MODIFY:
			process_admin_modify_request(arg, &msg);
			break;
		case ADMIN_ADDUSER:
			process_admin_adduser_request(arg, &msg);
			break;
		case ADMIN_DELUSER:
			process_admin_deluser_request(arg, &msg);
			break;
		case ADMIN_QUERY:
			process_admin_query_request(arg, &msg);
			break;
		default:
			break;
		}
	}
	close(newfd);
}
 

int process_user_or_admin_login_request(struct cli *cliInfo, MSG *msg)
{
	//验证用户名密码
	char sql[128] = "select * from staff where name=";
	char **pres = NULL; //存储查询结果的首地址
	int row, column;	//查询结果的行列数
	char *errmsg = NULL;
	sprintf(sql, "%s'%s'", sql, msg->username);
	if (sqlite3_get_table(cliInfo->db, sql, &pres, &row, &column, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
		return -1;
	}
	msg->flags = 0;
	if (column == 0)
	{
		printf("[%s]账号不存在\n", msg->username);
		sprintf(msg->recvmsg, "[%s]账号不存在\n", msg->username);
	}
	//和数据库密码不相等，密码不正确
	else if (strcmp(pres[column + 3], msg->passwd))
	{
		printf("[%s]密码不正确\n", msg->username);
		sprintf(msg->recvmsg, "[%s]密码不正确\n", msg->username);
	}
	else
	{
		//登录成功
		if (*pres[column + 1] == msg->usertype + 48) //从数据库获取的是字符'1'，客户端的usertype和数据库相同
		{
			//填充个人的信息
			msg->info.no = atoi(pres[column]);
			msg->info.usertype = atoi(pres[column + 1]);
			strcpy(msg->info.name, pres[column + 2]);
			strcpy(msg->info.passwd, pres[column + 3]);
			msg->info.age = atoi(pres[column + 4]);
			strcpy(msg->info.phone, pres[column + 5]);
			strcpy(msg->info.addr, pres[column + 6]);
			strcpy(msg->info.work, pres[column + 7]);
			strcpy(msg->info.date, pres[column + 8]);
			msg->info.stock = atoi(pres[column + 9]);
			msg->info.salary = atoi(pres[column + 10]);
 
			printf("[%s]登录成功\n", msg->username);
			sprintf(msg->recvmsg, "[%s]登录成功", msg->username);
			msg->flags = 1;
		}
		else if (*pres[column + 1] == USER + 48)
		{
			printf("[%s]此账号不是管理员用户\n", msg->username);
			sprintf(msg->recvmsg, "[%s]此账号不是管理员用户\n", msg->username);
		}
		else if (*pres[column + 1] == ADMIN + 48)
		{
			printf("[%s]此账号不是普通用户\n", msg->username);
			sprintf(msg->recvmsg, "[%s]此账号不是普通用户\n", msg->username);
		}
	}
	if (send(cliInfo->newfd, msg, sizeof(MSG), 0) < 0)
	{
		ERR_MSG("send");
	}
	//释放获取到的空间
	sqlite3_free_table(pres);
	pres = NULL;
	return 0;
}
 
int process_admin_adduser_request(struct cli *cliInfo, MSG *msg)
{
	if (ADMIN_ADDUSER == msg->msgtype) //修改信息也会调用此函数，放在修改调用时打印提示
		printf("管理员[%s]请求添加员工信息\n", msg->username);
	char sql[512] = "";
	sprintf(sql, "%s(%d,%d,'%s','%s',%d,'%s','%s','%s','%s',%d,%lf)", "insert into staff values ", msg->info.no, msg->info.usertype, msg->info.name, msg->info.passwd, msg->info.age, msg->info.phone, msg->info.addr, msg->info.work, msg->info.date, msg->info.stock, msg->info.salary);
	sprintf(msg->recvmsg, "ID[%d]添加新员工成功", msg->info.no);
	msg->flags = 1;
	char *errmsg = NULL;
	if (sqlite3_exec(cliInfo->db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		if (sqlite3_errcode(cliInfo->db) == 19) //主键重复，ID重复
		{
			sprintf(msg->recvmsg, "[%d]ID编号重复\n", msg->info.no);
			msg->flags = 0;
		}
		else
		{
			sprintf(msg->recvmsg, "[%d]注册失败\n", msg->info.no);
			msg->flags = 0;
			fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		}
	}
	if (ADMIN_ADDUSER == msg->msgtype)
	{
		if (send(cliInfo->newfd, msg, sizeof(MSG), 0) < 0)
		{
			ERR_MSG("send");
		}
	}
	return 0;
}
 
int process_admin_query_request(struct cli *cliInfo, MSG *msg)
{
	char m[256] = "";
	char sql[256] = "";
	char sql_t[150] = "";
	char **pres = NULL; //存储查询结果的首地址
	int row, column;	//查询结果的行列数
	char *errmsg = NULL;
	if (msg->flags == 0) //管理员查询所有员工信息
	{
		sprintf(m, "管理员[%s]请求查询所有员工信息", msg->username);
	}
	else if (msg->flags == 1) //管理员按照id查询员工信息
	{
		sprintf(sql_t, "where no=%d", atoi(msg->recvmsg));
		sprintf(m, "管理员[%s]请求查询ID[%d]员工信息\n", msg->username, atoi(msg->recvmsg));
	}
	else if (msg->flags == 2) //管理员按照名字查询员工信息
	{
		sprintf(sql_t, "where name='%s'", msg->recvmsg);
		sprintf(m, "管理员[%s]请求查询ID[%s]员工信息\n", msg->username, msg->recvmsg);
	}
	else if (msg->flags == 3) //普通用户查询个人信息
	{
		sprintf(sql_t, "where no=%d", atoi(msg->recvmsg));
		sprintf(m, "普通员工[%s]请求查询员工个人信息", msg->username);
	}
	printf("%s\n", m);
 
	sprintf(sql, "select * from staff %s", sql_t);
	//查询信息
	if (sqlite3_get_table(cliInfo->db, sql, &pres, &row, &column, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
		return -1;
	}
	if (row == 0)
	{
		msg->flags = 0;
	}
	//发送有多少行
	sprintf(msg->recvmsg, "%d", row);
	if (send(cliInfo->newfd, msg, sizeof(MSG), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}
	int i, j;
	for (i = column, j = 0; j < row; j++) //循环发送数据给客户端
	{
		msg->info.no = atoi(pres[i++]);
		msg->info.usertype = atoi(pres[i++]);
		strcpy(msg->info.name, pres[i++]);
		strcpy(msg->info.passwd, pres[i++]);
		msg->info.age = atoi(pres[i++]);
		strcpy(msg->info.phone, pres[i++]);
		strcpy(msg->info.addr, pres[i++]);
		strcpy(msg->info.work, pres[i++]);
		strcpy(msg->info.date, pres[i++]);
		msg->info.stock = atoi(pres[i++]);
		msg->info.salary = atoi(pres[i++]);
		sprintf(msg->recvmsg, "%d", row);
		if (send(cliInfo->newfd, msg, sizeof(MSG), 0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}
	}
	sqlite3_free_table(pres);
	return 0;
}

int process_admin_deluser_request(struct cli *cliInfo, MSG *msg)
{
	char buf[128];
	char m[256];
	strcpy(buf, msg->recvmsg); //保存删除的用户信息，方便存储操作内容和打印提示消息
	if (ADMIN_DELUSER == msg->msgtype)
	{
		sprintf(m, "管理员[%s]请求删除ID:[%s]员工信息", msg->username, msg->recvmsg);
		printf("%s\n", m);
	}
	//判断是否存在此用户id
	char sql[128] = "select * from staff where no=";
	char **pres = NULL; //存储查询结果的首地址
	int row, column;	//查询结果的行列数
	char *errmsg = NULL;
	sprintf(sql, "%s %d", sql, atoi(msg->recvmsg));
	if (sqlite3_get_table(cliInfo->db, sql, &pres, &row, &column, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
		return -1;
	}
	//用户不存在
	if (column == 0)
	{
		printf("ID:[%s]删除失败,用户不存在\n", buf);
		sprintf(msg->recvmsg, "ID:[%s]删除失败,用户不存在", buf);
		msg->flags = 0;
	}
	else
	{
		sprintf(sql, "%s %d", "delete from staff where no= ", atoi(msg->recvmsg));
		if (sqlite3_exec(cliInfo->db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
		{
			msg->flags = 0;
			fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		}
		sprintf(msg->recvmsg, "ID:[%s]删除成功", buf);
		msg->flags = 1;
	}
	if (ADMIN_DELUSER == msg->msgtype)
	{
		if (send(cliInfo->newfd, msg, sizeof(MSG), 0) < 0)
		{
			ERR_MSG("send");
		}
	}
	return column ? 0 : -1;
}

int process_admin_modify_request(struct cli *cliInfo, MSG *msg)
{
	//修改直接调用的删除和插入
	char t[128] = "";
	char m[256] = "";
	strcpy(t, msg->recvmsg); //保存修改的ID
	sprintf(m, "管理员[%s]修改ID[%s]员工信息\n", msg->username, msg->recvmsg);
	printf("%s\n", m);
	if (process_admin_deluser_request(cliInfo, msg) == 0)
	{
		process_admin_adduser_request(cliInfo, msg);
		if (msg->flags == 0)
		{
			sprintf(msg->recvmsg, "ID[%s]信息修改失败\n", t);
		}
		else if (msg->flags == 1)
		{
			sprintf(msg->recvmsg, "ID[%s]信息修改成功\n", t);
		}
	}
	else
	{
		msg->flags = 0;
		sprintf(msg->recvmsg, "ID[%s]员工不存在，修改失败\n", t);
	}
	if (send(cliInfo->newfd, msg, sizeof(MSG), 0) < 0)
	{
		ERR_MSG("send");
	}
	return 0;
}
 
void process_user_query_request(struct cli *cliInfo, MSG *msg)
{
	process_admin_query_request(cliInfo, msg); //调用管理员查询函数，函数有判断
}

void process_user_modify_request(struct cli *cliInfo, MSG *msg)
{
	char *errmsg = NULL;
	char sql[256] = "";
	printf("普通员工[%s]请求修改密码\n", msg->username);
	sprintf(sql, "update staff set passwd='%s' where no=%d", msg->recvmsg, msg->info.no);
	if (sqlite3_exec(cliInfo->db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
	}
	sprintf(msg->recvmsg, "普通员工[%s]密码修改成功", msg->username);
	if (send(cliInfo->newfd, msg, sizeof(MSG), 0) < 0)
	{
		ERR_MSG("send");
		return;
	}
}