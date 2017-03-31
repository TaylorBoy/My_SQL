

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

/** ��������Ƚ��ȶ� **/
void setstty2()//���������˸����������
{
	//system("stty erase ^H");//ִ��shell���Ҳ���� �������ö�ȡ�û����������ʱ���˸��������
	
	struct termios term;
	if(tcgetattr(STDIN_FILENO, &term) == -1)//�õ�ϵͳtermion������
	{
		printf("tcgetattr error is %s\n", strerror(errno));
		return;
	}

	oldterm = term;//������ǰtermios���ã��Ա�����˳���ʱ����Իָ�termios //tty

	/*
	term.c_lflag &= ~ICANON;//ȡ��ICANONѡ����淶���룩
	term.c_lflag |= ICANON;//����ICANONѡ��(�淶����)
	term.c_cc�ֶ�ΪҪ���õľ������������ַ�����c_cc[VERASE]�����˸��,
	term.c_cc[VERASE] = '\b';��˼Ϊ���˸���޸�Ϊ'\b'
	VERASE������ǰ����һ���ַ�,VINTR������ctrl + C�ж��źţ�ctrl + c��ASCII��Ϊ3
	���磺term.c_cc[VINTR] = '\t';��˼Ϊ��tab������Ϊ�ն��ź�
	tcsetattr�У��ڶ�������˵����TCSAFLUSH�������������������Ĳ���Ч���ڸ��ķ���ʱ��δ��ȡ�������������ݶ���ɾ��
	TCSANOW:����������Ч
	TCSADRAIN:�����������������Ĳŷ���������������������Ӧ��ʹ�ø�ѡ��
	*/
	term.c_cc[VERASE] = '\b';//'\b'Ϊ�˸����ASCII��
	if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1)//����ϵͳtermion
	{
		printf("tcsetattr error is %s\n", strerror(errno));
	}
	return;
}

/** ��������Ƚϲ��ȶ�,��ͬϵͳ���ܲ�һ�� **/
void setstty()//���������˸����������
{
	system("stty erase ^H");//ִ��shell���Ҳ���� �������ö�ȡ�û����������ʱ���˸��������

}

void returnstty()//�ָ�ϵͳ��termios����
{
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldterm) == -1)//����ϵͳtermion
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
	
	setstty();//���������˸����������
	
	mysql_init(&mysql);

	//���ӵ�mysql server
	connect = mysql_real_connect(&mysql, args[1], args[2], args[3], args[4],0, 0, 0);
	if (connect == NULL)
	{
		printf("connect error, %s\n", mysql_error(&mysql));
		return -1;
	}

	ret = mysql_query(connect, "SET NAMES utf8");		//�����ַ���ΪUTF8
	if (ret != 0)
	{
		printf("�����ַ�������, %s\n", mysql_error(&mysql));
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
  
        /* EXIT(exit):�˳� */  
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
			    //��ȡ��ѯ���
				result = mysql_store_result(&mysql); 
				
				//��ȡ�б�ͷ��Ϣ��Ϣ
				fields = mysql_fetch_fields(result);
				num_fields = mysql_num_fields(result);
								
				for (i=0; i<num_fields; i++)
				{
				   printf("%s\t", fields[i].name);
				}
				printf("\n");
							
				//�����л�ȡ���� �������������һ�С�
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
		
	 mysql_close(connect);		//�Ͽ���SQL server������
	 //returnstty();//�ָ�ϵͳ��termios����

	return 0;
}

