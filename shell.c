#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include<fcntl.h>
#include<sys/stat.h>
#define BUFFER_SIZE 1<<16
#define ARR_SIZE 1<<16
char buffer[BUFFER_SIZE];    // Character array holding strings to parse
size_t checkhome(char check[],char source[],size_t sourcelen,size_t checklen)
{
    size_t len=0;
    if(sourcelen<checklen)
        return 0;
    while(len!=sourcelen)
    {
        if(len==checklen)
            return 1;
        if(check[len]!=source[len])
            return 0;
        len++;
    }
    return 1;
}
int main(int agrc,char *agrv[])
{
    pid_t pid;
    int counter=0;
    while(1)
    {
        char host[BUFFER_SIZE]; //contains system name
        char *username; // contain username currently logged in
        username=getlogin();
        char home[BUFFER_SIZE];
        if(counter==0)
        {
            getcwd(home,BUFFER_SIZE);
            counter++;
        }
        gethostname(host,BUFFER_SIZE); //writing the system host name to host variable
        char dir[BUFFER_SIZE];
        getcwd(dir,BUFFER_SIZE); //  writing the current working directory to dir variable
        char finaldir[BUFFER_SIZE];
        size_t checkfinaldir=checkhome(home,dir,strlen(dir),strlen(home));
        if(checkfinaldir==1)//checkfinaldir=1 if cwd containd /home/user in the beginning
        {
            size_t tempval=0;
            size_t checkfinaldirlen=0;
            finaldir[checkfinaldirlen]='~';
            checkfinaldirlen++;
            while(tempval<strlen(home))
            {
                tempval++;
            }
            while(tempval<strlen(dir))
            {
                finaldir[(int)checkfinaldirlen]=dir[(int)tempval];
                checkfinaldirlen++;
                tempval++;
            }
            finaldir[(int)checkfinaldirlen]='\0';
            printf("%s@%s:%s$ ",username,host,finaldir);//displaying the hostname,username and the current working directory
        } 
        else
            printf("%s@%s:%s$ ",username,host,dir);//displaying the hostname,username and the current working directory
        scanf(" %[^\n]",buffer); //scanning the input from command line
        char *end_buffer,*end_token;
        char tokenarr[1000][1000];
        char *token;
        int tokencount=0;
        token=strtok_r(buffer,";",&end_buffer);
        while(token!=NULL)
        {
            strcpy(tokenarr[tokencount],token);
            token=strtok_r(NULL,";",&end_buffer);
            tokencount++;
        }
        int cc=0;
        int temptokencount=0;
        while(temptokencount!=tokencount)
        {
            int IN=0;
            int pp;
            int numofpipes=0;
            for(pp=0;pp<strlen(tokenarr[temptokencount]);pp++)
                if(tokenarr[temptokencount][pp]=='|')
                    numofpipes++;       // count the number of pipes
            char *pipetoken;
            char *pipe_buffer;
            int pipetempcount=0;
            pipetoken=strtok_r(tokenarr[temptokencount],"|",&pipe_buffer);  //seperating pipes
            while(pipetoken!=NULL)
            {
                int fd[2];
                char *token2;
                token2=strtok_r(pipetoken," ",&end_token); //seperating arguments
                char **main_args;
                long long int count=0;
                main_args=(char **)malloc(sizeof(char *)*(10000000));
                while(token2!=NULL)
                {
                    main_args[count]=(char *)malloc((sizeof(char)*(strlen(token2)+1)));
                    main_args[count]=token2;
                    token2=strtok_r(NULL," ",&end_token);
                    count++;
                }
                pipe(fd);
                int in,out;
                in=0,out=0; //flags: in =1 for input file, out = 1 for write and out = 2 for append
                char mainin[1000],mainout[1000];
                int tempcount=0;
                for(tempcount=0;tempcount<count;tempcount++)
                {
                    if(strcmp(main_args[tempcount],">")==0)
                    {
                        main_args[tempcount]=NULL;
                        tempcount++;
                        if(main_args[tempcount]!=NULL)
                        {
                            out=1;
                            strcpy(mainout,main_args[tempcount]);
                        }
                    }
                    else if(strcmp(main_args[tempcount],">>")==0)
                    {
                        main_args[tempcount]=NULL;
                        tempcount++;
                        if(main_args[tempcount]!=NULL)
                        {
                            out=2;
                            strcpy(mainout,main_args[tempcount]);
                        }
                    }
                    else if(strcmp(main_args[tempcount],"<")==0)
                    {
                        main_args[tempcount]=NULL;
                        tempcount++;
                        if(main_args[tempcount]!=NULL)
                        {
                            in=1;
                            strcpy(mainin,main_args[tempcount]);
                        }
                    }
                }
                if(!strcmp(main_args[0],"exit")) //if exit command found, exit
                    _exit(0);
                else if(!strcmp(main_args[0],"cd")) //if cd command found, change the current working directory to the specific directory in the argument
                {
                    int ret;
                    if(!strcmp(main_args[1],"~"))
                    {
                        ret=chdir(home);
                    }
                    else
                        ret=chdir(main_args[1]);
                    if(ret==-1)
                    {
                        perror(main_args[1]); // output the error if directory does not exists or the command could not be successfull
                    }
                }
                /*else if(!strcmp(main_args[0],"echo"))
                {
                    int z=1;
                    while(z<count)
                    {
                        printf("%s ",main_args[z]);
                        z++;
                    }
                    printf("\n");
                }*/
                else
                {
                    pid=fork(); // create a child process
                    if(pid<0)
                    {
                        perror("Child process could not be created\n");
                        _exit(-1);
                    }
                    else if(pid==0)
                    {
                        if(in==1)
                        {
                            int infileno=open(mainin,O_RDONLY);  //open the file
                            dup2(infileno,STDIN_FILENO);
                            close(infileno);
                        }
                        else
                        {
                            if(dup2(IN,STDIN_FILENO)<0)
                            {
                                perror("dup2 failure");
                                _exit(EXIT_FAILURE);
                            }
                        }
                        if(out==1)
                        {
                            int outfileno=open(mainout, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR|S_IRGRP|S_IWGRP|S_IWUSR);
                            dup2(outfileno,STDOUT_FILENO);
                            close(outfileno);
                        }
                        if(out==2)
                        {
                            int outfileno=open(mainout,O_APPEND |  O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                            dup2(outfileno,STDOUT_FILENO);
                            close(outfileno);
                        }
                        else if(numofpipes!=0 && pipetempcount!=numofpipes)
                        {
                            if(dup2(fd[1],STDOUT_FILENO)<0)
                            {
                                perror("dup2 failure");
                                _exit(EXIT_FAILURE);
                            }
                            close(fd[0]);
                        }
                        if(!strcmp(main_args[count-1],"&")) // check for foreground process
                        {
                            setpgid(0,0);
                            main_args[count-1]=NULL;
                        }
                        int ret=execvp(main_args[0],main_args); //execute the command and the success value is stored in ret variable
                        if(ret!=0)
                        {
                            perror(main_args[0]);
                        } 
                        _exit(0);
                    }
                    if(strcmp(main_args[count-1],"&"))
                        wait();
                    close(fd[1]);
                    IN=fd[0];
                }
                pipetempcount++;
                pipetoken=strtok_r(NULL,"|",&pipe_buffer);
            }
            temptokencount++;
        }
    }
    return 0;
}
