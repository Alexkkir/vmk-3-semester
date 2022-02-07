#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>

#define MAXLEN 1024
#define BLUE "\033[0;34m"
#define STANDARD_COLOR "\033[0m"

#include <linux/limits.h>

// чтение строки со стандартного ввода
int read_string(char* str);
// выделение из строки str команды с уловным выполнением <Команда Shellа > → < Команда с условным выполнением >{ [; | &] < Команда Shellа>}{ ; |&}
int make_cond_command(char* str);
// <Команда с условным выполнением > → <Команда> { [&& | || ] <Команда с условным выполнением>}
int make_command(char *str);
// конвеер
int pipeline(char* str, int flag);
// выделение из конвеера команды
int get_simplecmd(char* str, int fd1, int fd2);
// выполнение команды
int execute_command(char *dir, char** matrix, int fd1, int fd2);
// разбиение строки на массив
char** get_matrix(char* str);

//volatile char dir[MAXLEN];
char dir[MAXLEN];

int main()
{
    //char dir[MAXLEN];
    getcwd(dir, MAXLEN);

    for(;;)
    {
        printf("%s%s%s$ ", BLUE, dir, STANDARD_COLOR);
        
        char str[MAXLEN];
        if (!strcmp(str, "exit")) return 0;
        read_string(str);
        make_cond_command(str);
    }  
    return 0;
}

int read_string(char* str)
{
    char c;
    int i=0;
    int spacecount=0;
    while((c=getchar()) != '\n')
    {
    	if(isspace(c))
    	{
    		spacecount++;
    		//continue;
    	}
    	
        if(isspace(c) && (spacecount>=2))
        {
        	spacecount=0;
        	continue;
        }
        else if((spacecount!=0)&&(!isspace(c)))
        {
        	spacecount=0;
        }
        
        str[i++]=c;
    }
    
    //str[i]=' ';
    str[i]='\0';
    
    printf("\n\nstr: %s\n\n",str);
    return 0;
}
 
int make_cond_command(char* str)
{
    int k1 = 0;
    if (str==NULL) return 1;
    size_t start = 0;
    size_t end = 0;
    int status;
    int ex_code = 0;
    
    while (str[end] != '\0' && str[end] != '\n')
    {
        while(str[end] != ';' && str[end] != '&' && str[end] != '\0' && str[end] != '\n')
        {
            end++;
        } 

        if(str[end] == ';')
        {
            str[end] = '\0';
            end++;
            ex_code = make_command(str+start);
            int n = MAXLEN;
            while (n--) 
            {   
                if (str[end] == ' ') end++;
                else break;
            }
            start = end;
        }
        else if(str[end] == '&' && str[end+1] != '&') //условие на фоновый режим
        {
            pid_t p1, p2;
            p1 = fork();
            if (!p1) //сын запускает внука и умирает
            {
                p2 = fork();
                if(p2) // сын
                {
                    exit(0);
                }
                if(!p2) //внук работает в фоне
                {
                    signal(SIGINT, SIG_IGN);
                    int fd = open("/dev/null", O_RDWR);
                    //dup2(fd, 1);
                    dup2(fd, 0);
                    str[end] = '\0';
                    end++;                    
                    ex_code = make_command(str+start);
                    exit(ex_code);
                }
            }
            wait(NULL);
            //waitpid(p1, &status, 0); //ждем сына
            end++;
            int n = MAXLEN;
            while (n--) 
            {   
                if (str[end] == ' ') end++;
                else break;
            }
            start = end;
        }
        else if(str[end] == '&' && str[end+1] == '&')
            end+=2;
        else if(str[end] == '\0' || str[end] == '\n')
        {
            ex_code = make_command(str+start);
            int n = MAXLEN;
            while (n--) 
            {   
                if (str[end] == ' ') end++;
                else break;
            }
            start = end;
        }
    }
    //printf("make_cond_command\n");
    return ex_code;    
}

int make_command(char *str)
{
    int k1 = 0;
    if (str == NULL) return 1;
    size_t start = 0;
    size_t end = 0;
    int flag = 1;
    int flag2 = 1;
    int ex_code = 0;
    while (str[end] != '\0' && str[end] != '\n')
    {
        while(str[end] != '&' && str[end] != '|' && str[end] != '\0' && str[end] != '\n')
        {
            end++;
        }
        if ((str[end] == '&' && str[end+1] == '&') || (str[end] == '|' && str[end+1] == '|')) //условие на запуск конвеера
        {
            if (str[end] == '&') flag2 = 1; else flag2 = 0; 
            str[end] = '\0';
            end+=2;
            if (flag == 1 && ex_code == 0 || flag == 0 && ex_code != 0) 
                ex_code = pipeline(str+start, flag);
            flag = flag2;
            
            int n = MAXLEN;
            while (n--) 
            {   
                if (str[end] == ' ') end++;
                else break;
            }

            start = end;
        }
        else if (str[end] != '\0' && str[end]!='\n')
            end++;
    }
    if (str[end] == '\0' || str[end] == '\n')
    {
        if (flag == 1 && ex_code == 0 || flag == 0 && ex_code != 0) 
            ex_code = pipeline(str+start, flag);  
    }
    return ex_code;
}

int pipeline(char* str, int flag)
{
    //    printf("-------------------------- here1\n");

    int fd1 = 0;
    int fd2 = 1;
    if (str == NULL) return 1;
    int j;
    int is_append = 0;
    int is_begin_append = 0;
    int is_begin_in = 0;
    int is_begin_out = 0;
    int f_in = 0;
    int f_out = 0;
    size_t start = 0;
    size_t end = 0;
    int n = MAXLEN;
    int ex_code = 0;
    
    while (n--) 
    {   
        if (str[end] == ' ') end++;
        else break;
    }

    while (str[end] != '\0' && str[end] != '\n')
    {
        if (str[end] == '<') f_in = 1;
        if (str[end] == '>') f_out = 1;
        end++;
    }
    end = 0;
    
    
    n = MAXLEN;
    while (n--)   //если в начале были пробелы, то мы их сжали
    {   
        if (str[end] == ' ') end++;
        else break;
    }
    
    
    if (str[end] == '<') is_begin_in = 1;
    if (str[end] == '>') is_begin_out = 1;
    end = 0;
    if (f_in && !f_out && is_begin_in) //есть ввод, нет вывода, ввод в начале 
    {
        end = 0;
        while (str[end] != '<')
            end++;
        
        n = MAXLEN;
        end++;
        while (n--) 
        {   
            if (str[end] ==' ') end++;
            else break;
        }

        char f_in_name[MAXLEN];
        int j=0;
        while(str[end]!=' ' && str[end]!='&' &&  str[end]!='|' && str[end]!='\0' && str[end]!='\n')
            f_in_name[j++]=str[end++];
        
        f_in_name[j]='\0';
        fd1 = open(f_in_name, O_RDONLY);
        printf("Ввод был перенаправлен на ввод из файла %s\n", f_in_name);
        end++;
        start = end;
        ex_code = get_simplecmd(str+start, fd1, fd2);
        return ex_code;
    }
    if (f_in && !f_out && !is_begin_in) // есть ввод, нет вывода, ввод в конце
    {
        end = 0;
        while (str[end] != '<')
            end++;
        n = MAXLEN;
        int init = end;
        end++;
        
        while (n--) 
        {   
            if (str[end] ==' ') end++;
            else break;
        }
        
        char f_in_name[MAXLEN];
        j = 0;
        while(str[end]!=' ' && str[end]!='&' &&  str[end]!='|' && str[end]!='\0' && str[end]!='\n')
            f_in_name[j++]=str[end++];
        f_in_name[j]='\0';
        fd1 = open(f_in_name, O_RDONLY);
        printf("Ввод был перенаправлен на ввод из файла %s\n", f_in_name); 
        str[init]='\0';        
        ex_code = get_simplecmd(str+start, fd1, fd2);
        return ex_code;
    }
    if (f_out && !f_in && is_begin_out) //есть вывод, нет ввода, вывод в начале 
    {
        end = 0;
        while (str[end] != '>')
            end++;
        int append = (str[end+1] == '>');
        if (append) end++;
        n = MAXLEN;
        end++;
        while (n--) 
        {   
            if (str[end] ==' ') end++;
            else break;
        }
        char f_out_name[MAXLEN];
        int j = 0;
        while(str[end]!=' ' && str[end]!='&' &&  str[end]!='|' && str[end]!='\0' && str[end]!='\n')
            f_out_name[j++]=str[end++];
        f_out_name[j]='\0';
        if (!append)
        {
            //printf("kek\n");
            fd2 = open(f_out_name, O_WRONLY|O_CREAT|O_TRUNC, 0777);
        }
        else 
        {
            fd2 = open(f_out_name, O_WRONLY|O_CREAT|O_APPEND, 0777);
        }
        printf("Вывод был перенаправлен на вывод в файл %s\n", f_out_name);
        end++;
        start = end;
        ex_code = get_simplecmd(str+start, fd1, fd2);
        return ex_code;
    }
    if (f_out && !f_in && !is_begin_out) // есть вывод, нет ввода, вывод в конце
    {
        end = 0;
        while (str[end] != '>')
            end++;
        int append = (str[end+1] == '>');
        int init = end;
        if (append) end++;
        n = MAXLEN;
        end++;
        while (n--) 
        {   
            if (str[end] ==' ') end++;
            else break;
        }
        char f_out_name[MAXLEN];
        j = 0;
        while(str[end]!=' ' && str[end]!='&' &&  str[end]!='|' && str[end]!='\0' && str[end]!='\n')
            f_out_name[j++]=str[end++];
        f_out_name[j]='\0';
        if (!append)
        {
            //printf("kek\n");
            fd2 = open(f_out_name, O_WRONLY|O_CREAT|O_TRUNC, 0777);
        }
        else 
        {
            fd2 = open(f_out_name, O_WRONLY|O_CREAT|O_APPEND, 0777);
        }
        printf("Вывод был перенаправлен на вывод в файл %s\n", f_out_name); 
        str[init]='\0';        
        ex_code = get_simplecmd(str+start, fd1, fd2);
        return ex_code;
    }
    if (f_in && f_out && (is_begin_in || is_begin_out)) //есть и ввод и вывод, в начале
    {
        int first_in = 0;        
        int first_out = 0;   
        end = 0;
        while(str[end] != '>' && str[end] != '<')
            end++;
        if (str[end] == '>') 
            first_out = 1;
        else first_in = 1;
        if (first_out)     // сначала вывод
        {
            int append = (str[end+1] == '>');
            if (append) end++;
            n = MAXLEN;
            end++;
            while (n--) 
            {   
                if (str[end] == ' ') end++;
                else break;
            }
            char f_out_name[MAXLEN];
            int j = 0;
            while(str[end] != ' ' && str[end] != '&' &&  str[end] != '|' && str[end] != '\0' && str[end] != '\n')
                f_out_name[j++]=str[end++];
            f_out_name[j] = '\0';
            if (!append)
            {
               // printf("kek\n");
                fd2 = open(f_out_name, O_WRONLY|O_CREAT|O_TRUNC, 0777);
            }
            else 
            {
                fd2 = open(f_out_name, O_WRONLY|O_CREAT|O_APPEND, 0777);
            }
            printf("Вывод был перенаправлен на вывод в файл %s\n", f_out_name);
            n = MAXLEN;
            end++;
            while (n--) 
            {   
                if (str[end] == ' ') end++;
                else break;
            }
            end++;
            char f_in_name[MAXLEN];
            j = 0;
            while(str[end] != ' ' && str[end] != '&' &&  str[end] != '|' && str[end] != '\0' && str[end] != '\n')
                f_in_name[j++] = str[end++];
            f_in_name[j]='\0';
            fd1 = open(f_in_name, O_RDONLY);
            printf("Ввод был перенаправлен на ввод из файла %s\n", f_in_name);
            end++;
            start = end;
            ex_code = get_simplecmd(str+start, fd1, fd2);
            return ex_code;
        }
        if (first_in)     // сначала ввод
        {
            n = MAXLEN;
            end++;
            while (n--) 
            {   
                if (str[end] == ' ') end++;
                else break;
            }
            char f_in_name[MAXLEN];
            int j = 0;
            while(str[end]!=' ' && str[end]!='&' &&  str[end]!='|' && str[end]!='\0' && str[end]!='\n')
                f_in_name[j++]=str[end++];
            f_in_name[j]='\0';
            fd1 = open(f_in_name, O_RDONLY);
            printf("Ввод был перенаправлен на ввод из файла %s\n", f_in_name);
            n = MAXLEN;
            end++;
            while (n--) 
            {   
                if (str[end] == ' ') end++;
                else break;
            }
            char f_out_name[MAXLEN];
            j = 0;
            int append = (str[end+1] == '>');
            if (append) end++;
            end++;
            while(str[end]!=' ' && str[end]!='&' &&  str[end]!='|' && str[end]!='\0' && str[end]!='\n')
                f_out_name[j++]=str[end++];
            f_out_name[j]='\0';
            if (!append)
            {
                //printf("kek\n");
                fd2 = open(f_out_name, O_WRONLY|O_CREAT|O_TRUNC, 0777);
            }
            else 
            {
                fd2 = open(f_out_name, O_WRONLY|O_CREAT|O_APPEND, 0777);
            }
            printf("Вывод был перенаправлен на вывод в файл %s\n", f_out_name);
            end++;
            start = end;
            ex_code = get_simplecmd(str+start, fd1, fd2);
            return ex_code;
        }
    }
    if (f_in && f_out && (!is_begin_in && !is_begin_out)) //есть и ввод и вывод, в конце
    {
        int first_in = 0;        
        int first_out = 0;   
        end = 0;
        while(str[end] != '>' && str[end] != '<')
            end++;
        if (str[end] == '>') 
            first_out = 1;
        else first_in = 1;
        int init = end;
        if (first_out)     // сначала вывод
        {
            int append = (str[end+1] == '>');
            if (append) end++;
 
            n = MAXLEN;
            end++;
            while (n--) 
            {   
                if (str[end] ==' ') end++;
                else break;
            }
            char f_out_name[MAXLEN];
            int j = 0;
            while(str[end]!=' ' && str[end]!='&' &&  str[end]!='|' && str[end]!='\0' && str[end]!='\n')
                f_out_name[j++]=str[end++];
            f_out_name[j]='\0';
            if (!append)
            {
                fd2 = open(f_out_name, O_WRONLY|O_CREAT|O_TRUNC, 0777);
            }
            else 
            {
                fd2 = open(f_out_name, O_WRONLY|O_CREAT|O_APPEND, 0777);
            }
            printf("Вывод был перенаправлен на вывод в файл %s\n", f_out_name);
            n = MAXLEN;
            end++;
            end++;
            while (n--) 
            {   
                if (str[end] == ' ') end++;
                else break;
            }
            char f_in_name[MAXLEN];
            j = 0;
            while(str[end]!= ' ' && str[end] != '&' &&  str[end] != '|' && str[end] != '\0' && str[end] != '\n')
                f_in_name[j++]=  str[end++];
            f_in_name[j] = '\0';
            fd1 = open(f_in_name, O_RDONLY);
            printf("Ввод был перенаправлен на ввод из файла %s\n", f_in_name);
            end++;
            str[init] = '\0';
            ex_code = get_simplecmd(str+start, fd1, fd2);
            return ex_code;
        }
        if (first_in)     // сначала ввод
        {
            n = MAXLEN;
            end++;
            while (n--) 
            {   
                if (str[end] ==' ') end++;
                else break;
            }
            char f_in_name[MAXLEN];
            int j = 0;
            while(str[end]!=' ' && str[end]!='&' &&  str[end]!='|' && str[end]!='\0' && str[end]!='\n')
                f_in_name[j++]=str[end++];
            f_in_name[j]='\0';
            fd1 = open(f_in_name, O_RDONLY);
            printf("Ввод был перенаправлен на ввод из файла %s\n", f_in_name);
            n = MAXLEN;
            end++;
            while (n--) 
            {   
                if (str[end] == ' ') end++;
                else break;
            }
            int append = (str[end+1] == '>');
            if (append) end++;
            end++;
            char f_out_name[MAXLEN];
            j = 0;
            while(str[end] != ' ' && str[end] != '&' &&  str[end] != '|' && str[end] != '\0' && str[end] != '\n')
                f_out_name[j++]=str[end++];
            f_out_name[j]='\0';
            if (!append)
            {
                fd2 = open(f_out_name, O_WRONLY|O_CREAT|O_TRUNC, 0777);
            }
            else 
            {
                fd2 = open(f_out_name, O_WRONLY|O_CREAT|O_APPEND, 0777);
            }
            printf("Вывод был перенаправлен на вывод в файл %s\n", f_out_name);
            end++;
            str[init] = '\0';
            ex_code = get_simplecmd(str+start, fd1, fd2);
            return ex_code;
        }   
    }
   // printf("{3} %s\n", str);
    ex_code = get_simplecmd(str, fd1, fd2);  //перенаправления нет вообще
    return ex_code;
}

int get_simplecmd(char* str, int fd1, int fd2)
{

    int cnt = 1;
    int status = 0;
    int temp = 0;
    pid_t p;
    if (str == NULL)
        return 1;
    int marker = 0;
    char string[MAXLEN];
    strcpy(string, str);
    size_t start = 0;
    size_t end = 0;
    int fl = 0;
    while (string[end] != '\0' && string[end] != '\n')
    {
        if (string[end] == '|')
        cnt++;
        
        end++;
    }
    end = 0;
    //printf("%d\n", cnt);
    if(cnt == 1)
    {
        while(string[end] != '\0' && string[end] != '\n')
            end++;
        string[end] = '\0';
        char** matrix = get_matrix(string + start);
        p = fork();
        if (!p)
            execute_command(dir, matrix, fd1, fd2);
        wait(&status);
        return status;
    }
        
    int fdd0 = fd1; //ввод в текущую команду
    int fdd1; // вывод из текущей команды
    int fd[2];
    for (int i = 1; i < cnt;)
    {
        end++;
        if (string[end] == '|')
        {
            string[end] = '\0';
            char** matrix = get_matrix(string + start);
            end++;
            
            int n = MAXLEN;
            while (n--) 
            {   
                if (str[end] == ' ') end++;
                else break;
            }
            start = end;
            
            pipe(fd); // канал между текущей и следующей командой
            fdd1 = fd[1]; //вывод из текущей направляем в канал
            p = fork();
            if (!p) 
                execute_command(dir, matrix, fdd0, fdd1);
            if (fdd0 != 0)  //закрываем на чтение канал у "текущей команды" 
                close(fdd0);
            close(fdd1); 
            fdd0 = fd[0]; //ввод следующей перенаправляем на канал
            i++;
        }
    }
    while(string[end] != '\0' && string[end] != '\n')
        end++;
    string[end] = '\0';
    char** matrix = get_matrix(string + start);
    fdd1 = fd2; //вывод последней перенаправляем
    p = fork();
    if (!p)
        execute_command(dir, matrix, fdd0, fdd1);
    close(fdd0); 
    if (fdd1 != 1)
        close(fdd1);
    for (int i = 0; i<cnt; i++) // ждем всех сыновей
    {
        wait(&temp);
        if (status == 0)
            status = temp;
    }
    return status;
}

int execute_command(char *dir, char** matrix, int fd1, int fd2)
{
    char *home = getenv("HOME");
    char str1[MAXLEN];
    
    //printf("%s\n 0(%c) 1(%c) 2(%c)", matrix[0],matrix[0][0],matrix[0][1],matrix[0][2]);

    if (fd1 != 0) dup2(fd1, 0);
    if (fd2 != 1) dup2(fd2, 1);


		if (strcmp(matrix[0], "exit") == 0)
        { //exit
			return -1;
		}
		if (strcmp(matrix[0], "cd") == 0)
        {   //cd
        	printf("cd founded\n");
        	//printf("%s\n", matrix[0]);
			//if (matrix[1] == NULL)
				chdir(home);
			//else if (chdir(matrix[1]) == -1)
				//perror(matrix[1]);
			
			getcwd(dir, MAXLEN);
		}
		if(matrix[0][0]=='c' && matrix[0][1]=='d' && matrix[0][2] != '\0' && matrix[0][2] != '\n')
		{
			printf("IN IF CD_\n");
			//printf("%\n 0(%c) 1(%c) 2(%c)", str1[0],str1[1],str1[2]);
			
				int i=0;
				
				while(matrix[0][i+2] != '\n' && matrix[0][i+2] != '\0' && matrix[0][i+2] != '&' && matrix[0][i+2] != '|' && matrix[0][i+2] != ';')
				{
					str1[i]=matrix[0][i+2];
					printf("%c",str1[i]);
					i=i+1;
				}
				//str1[i+1]=matrix[0][i+2];
				//printf("%c",str1[i+1]);
				//str1[i+2]='\n';
				//printf("%c\n",str1[i+2]);
				
				printf("%c%c%c%c%c%c%c\n",str1[0],str1[1],str1[2],str1[3],str1[4],str1[5],str1[6]);
				
				
				//i=0;
				//while(str1[i]!='\0')
				//{
				//	printf("%c",str1[i]);
				//}
				
				
				//printf("%s\n",str1);
				
				printf("\nstr1: %s\n",str1);
			
			if(chdir(str1) == -1)
			perror(str1);
			getcwd(dir, MAXLEN);
			
			//free(*str1);
		}
		else
        {
        execvp(matrix[0], matrix);
        exit(1);
        }
}

char** get_matrix(char* str)
{
    int start = 0;
    int end = 0;
    int cnt = 1;
    while (str[end] != '\n' && str[end] != '\0')
    {
        if (str[end] == ' ' && str[end+1] != ' ' && str[end+1] != '\0')
            cnt++;
        end++;
    }
    end = 0;
    char **matrix = (char **)malloc(cnt*sizeof(char *));
    for(int i = 0; i < cnt; i++) 
    {
        matrix[i] = (char *)malloc(1024*sizeof(char));
    }
    int i = 0;
    while (str[end] != '\n' && str[end] != '\0')
    {
        while (str[end] != ' ' && str[end] != '\n' && str[end] != '\0')
            end++;
        if (str[end] == '\0')
        {
            strcpy(matrix[i++], str+start);
            break;
        }
        str[end] = '\0';
        end++; 
        strcpy(matrix[i++], str+start);
        start = end;
    }
   
    for (i = 0; i<cnt; i++)
        printf("-------------------------- %s\n", matrix[i]);
    
    matrix[i] = NULL;
    
    return matrix;
}
