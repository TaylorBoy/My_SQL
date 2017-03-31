

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <mysql/mysql.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <mysql/mysql.h>

#include <termios.h>

struct termios oldterm;

/** 这个方法比较稳定 **/
void setstty2()//设置输入退格键，不回显
{
	//system("stty erase ^H");//执行shell命令，也可以 用来设置读取用户键盘输入的时候，退格键不回显
	
	struct termios term;
	if(tcgetattr(STDIN_FILENO, &term) == -1)//得到系统termion的设置
	{
		printf("tcgetattr error is %s\n", strerror(errno));
		return;
	}

	oldterm = term;//保留当前termios设置，以便程序退出的时候可以恢复termios //tty

	/*
	term.c_lflag &= ~ICANON;//取消ICANON选项（不规范输入）
	term.c_lflag |= ICANON;//设置ICANON选项(规范输入)
	term.c_cc字段为要设置的具体特殊输入字符，如c_cc[VERASE]代表退格键,
	term.c_cc[VERASE] = '\b';意思为把退格键修改为'\b'
	VERASE代表向前擦出一个字符,VINTR代表发送ctrl + C中断信号，ctrl + c的ASCII码为3
	例如：term.c_cc[VINTR] = '\t';意思为将tab键设置为终端信号
	tcsetattr中，第二个参数说明，TCSAFLUSH：发送了所有输出后更改才生效，在更改发生时，未读取的所有输入数据都被删除
	TCSANOW:更改立即生效
	TCSADRAIN:发送了所有输出后更改才发生，如果更改输出参数则应该使用该选项
	*/
	term.c_cc[VERASE] = '\b';//'\b'为退格键的ASCII码
	if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1)//设置系统termion
	{
		printf("tcsetattr error is %s\n", strerror(errno));
	}
	return;
}

/** 这个方法比较不稳定,不同系统可能不一样 **/
void setstty()//设置输入退格键，不回显
{
	system("stty erase ^H");//执行shell命令，也可以 用来设置读取用户键盘输入的时候，退格键不回显

}

void returnstty()//恢复系统的termios设置
{
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldterm) == -1)//设置系统termion
	{
		printf("tcsetattr error is %s\n", strerror(errno));
	}
	return;
}


int main(int arg, char *args[])
{
	int 			ret = 0, i=0;
	MYSQL 			mysql;
	MYSQL			*connect;
	MYSQL_RES 		*result;
	MYSQL_ROW		row;
	MYSQL_FIELD 	*fields;
	unsigned int 	num_fields;
	
	if (arg < 4)
	{
		printf("please enter: %s localhost user password dbname\n", args[0]);
		return -1;
	}
	
	setstty();//设置输入退格键，不回显
	
	mysql_init(&mysql);

	//连接到mysql server
	connect = mysql_real_connect(&mysql, args[1], args[2], args[3], args[4],0, 0, 0);
	if (connect == NULL)
	{
		printf("connect error, %s\n", mysql_error(&mysql));
		return -1;
	}

	ret = mysql_query(connect, "SET NAMES utf8");		//设置字符集为UTF8
	if (ret != 0)
	{
		printf("设置字符集错误, %s\n", mysql_error(&mysql));
		return ret;
	}

	char buf[4096];
    for( ; ; )  
    {  
        memset(buf, 0, sizeof(buf));
		//strcpy(buf, "mysql>");
		//write(STDOUT_FILENO, buf, strlen(buf));
		//memset(buf, 0, sizeof(buf));
		//read(STDIN_FILENO, buf, sizeof(buf));
		
        printf("\nmysql>");
        gets(buf);  
  
        /* EXIT(exit):退出 */  
        if ( 0 == strncmp(buf , "exit" , 4) || 0 == strncmp(buf , "EXIT" , 4) ||
        	 0 == strncmp(buf , "quit" , 4) || 0 == strncmp(buf , "QUIT" , 4) ) 
        {
        	break;
        }	 
        
        //printf("buf:%s \n", buf);
         ret = mysql_query(connect, buf); 
         if (ret != 0 )
		 {
			printf("func mysql_query() err: %s", mysql_error(&mysql) );
			continue;
		 }
		 else
		 {
		 	//printf("ok\n");
		 }
		 
		 if ( (strncmp(buf, "select", 6) == 0) || (strncmp(buf, "SELECT", 6) == 0) ||
			  (strncmp(buf, "show", 4) == 0) || (strncmp(buf, "SHOW", 4) == 0) ||
			  (strncmp(buf, "desc", 4) == 0) || (strncmp(buf, "DESC", 4) == 0) )
         {
			    //获取查询结果
				result = mysql_store_result(&mysql); 
				
				//获取列表头信息信息
				fields = mysql_fetch_fields(result);
				num_fields = mysql_num_fields(result);
								
				for (i=0; i<num_fields; i++)
				{
				   printf("%s\t", fields[i].name);
				}
				printf("\n");
							
				//按照行获取数据 检索结果集的下一行。
				while(row = mysql_fetch_row(result))
				{
					for (i=0; i<mysql_num_fields(result); i++)
					{
						printf("%s\t ",row[i]);
					}
					printf("\n");
				}
				
			 	mysql_free_result(result);     //free result after you get the result
         }
         else
         {
         	//printf("")mysql_affected_rows(connect); 
         }
	}	
		
	 mysql_close(connect);		//断开与SQL server的连接
	 //returnstty();//恢复系统的termios设置

	return 0;
}

