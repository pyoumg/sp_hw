/* $begin shellmain */
#include "myshell.h"
#include <errno.h>
#define MAXARGS 128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv, int *argc_num);
int builtin_command(char **argv);

int main()
{
    char cmdline[MAXLINE]; /* Command line */
    if (signal(SIGTERM, sigterm_handler) == SIG_ERR)
        unix_error("signal error");

    while (1)
    {
        /* Read */
        printf("CSE4100-SP-P4> ");
        Fgets(cmdline, MAXLINE, stdin);

        if (feof(stdin))
        {
            exit(0);
        }

        /* Evaluate */
        eval(cmdline);
    }
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline)
{

    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    int argc;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv, &argc);
    if (argv[0] == NULL)
        return; /* Ignore empty lines */

    pipeline(argv, 0, argc, 0, bg, cmdline);

    return;
}

//여기에서  fork 함
/*pipeline 관련 동작을 수행하는 함수*/
void pipeline(char **argv, int argv_idx, int argc, int input, int bg, char *cmdline)
{
    int fd[2];
    pid_t pid; /* Process id */
    int free_flag = 0;
    char *child_argv[MAXARGS];
    int flag;
    char pwd[200] = {'\0'};

    get_child_argv(argv, &argv_idx, argc, child_argv);
    if (!child_argv[0])
    {
        return;
    }

    if (pipe(fd) < 0)
    {
        Write(1, "pipe error", 10);
        exit(-1);
    }

    if (!builtin_command(child_argv))
    { //quit -> exit(0), & -> ignore, other -> run
        if ((pid = Fork()) == 0)
        { /* Child runs user job */

            if (argv_idx >= argc)
            { //output은 그냥 출력하면 되는 상태. 마지막

                Close(fd[1]); //use stdout
                Dup2(input, 0);
            }
            else
            { //input output 을 pipe를 사용하여 연결
                Close(fd[0]);
                Dup2(input, 0); //input
                Dup2(fd[1], 1); //output
            }

            flag = check_other_cmd(child_argv, &free_flag);

            int i = 0, j = 0;
            while (child_argv[i] != NULL)
            {
                if (child_argv[i][0] == '\'' || child_argv[i][0] == '"')
                {
                    for (j = 0; j < strlen(child_argv[i]) - 1; j++)
                    {
                        child_argv[i][j] = child_argv[i][j + 1];
                    }
                    child_argv[i][j - 1] = '\0'; //'가 2개씩 들어있기 때문에
                }

                //      printf("%s\n",child_argv[i]);
                i++;
            }

            if (!flag && execve(child_argv[0], child_argv, environ) < 0)
            {
                printf("%s: Command not found.\n", child_argv[0]);
                exit(0);
            }

            if (free_flag)
                Free(child_argv[0]);

            exit(0); //parent일때만 반복문
        }
        else
        { //parent

            Close(fd[1]);

            pipeline(argv, argv_idx, argc, fd[0], bg, cmdline); //recursive
        }

        /* Parent waits for foreground job to terminate */
        if (!bg)
        {
            int status;

            if (waitpid(pid, &status, 0) < 0)
                unix_error("waitfg: waitpid error");
        }
        else //when there is backgrount process!
            printf("%d %s", pid, cmdline);
    }
    return;
}

void get_child_argv(char **argv, int *argv_idx, int argc, char **child_argv)
{
    //malloc을 사용하지 않고 구현

    int i, j = 0;
    for (i = *argv_idx; i < argc; i++)
    {
        if (argv[i][0] == '|')
            break;
        child_argv[j] = argv[i];
        j++;
    }
    child_argv[j] = NULL;
    *argv_idx = i + 1; //next start index
}

void sigterm_handler(int sig)
{ /*sigterm handler*/
    exit(0);
}

int check_other_cmd(char **argv, int *flag)
{
    char *temp_argv;
    char fileread[MAXBUF] = {
        '\0',
    }; //초기화
    int sort_num, file_len;
    char *sort_char[200];
    //    char sort_char[200][200];
    char filewrite[MAXBUF]; //write file
    int j = 1, k = 0;       //index of sort_char[]
    int reverse = -1;

    if (!strcmp(argv[0], "exit"))
    {                     //parent process terminate
        kill(0, SIGTERM); //parent process
        exit(0);
    }
    else if (!strcmp(argv[0], "sort"))
    {
        //file input: 0
        //     printf("%d\n", fileread[0]);
        while (!fileread[0])
        {
            Read(0, fileread, MAXBUF);
        }
        file_len = strlen(fileread);
        fileread[file_len - 1] = '\0';
        sort_char[0] = fileread;
        for (int i = 1; i < file_len; i++)
        {
            if (fileread[i] == '\n')
            {
                fileread[i] = '\0';
                sort_char[j] = fileread + (i + 1);
                j++;
            }
        }

        if (argv[1])
        { //reverse
            reverse = 1;
        }

        Sort(sort_char, reverse, j);

        return 1; //따로 처리할 필요 없음
    }
    else if (!(strlen(argv[0]) > 4 && !strncmp(argv[0], "/bin", 4)))
    {
        *flag = 1;
        temp_argv = Malloc(sizeof(char) * MAXARGS);
        strcpy(temp_argv, "/bin/");
        strcat(temp_argv, argv[0]);

        argv[0] = temp_argv;

        return 0;
    }

    return 0;
}

//file write도 여기서

void Sort(char **arr, int flag, int num)
{
    // 별도의 malloc 없이

    char *temp;
    int len;
    for (int i = 0; i < num; i++)
    {
        for (int j = 0; j < num - 1; j++)
        {
            if (strcmp(arr[i], arr[j]) * flag > 0)
            {
                temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }

    for (int i = 0; i < num - 1; i++)
    {
        len = strlen(arr[i]);
        arr[i][len] = '\n';
        Write(1, arr[i], len + 1); //\n까지
    }
}

void Cd(char **argv)
{
    if (argv[1] == NULL)
    {
        if (chdir(getenv("HOME")))
            unix_error("cd path error\n");
    }
    else
    {                       //argc :2
        if (chdir(argv[1])) //error:-1
            unix_error("cd path error\n");
    }
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "quit")) /* quit command */
        exit(0);
    if (!strcmp(argv[0], "cd")) /* cd command */
    {
        Cd(argv);
        return 1;
    }
    if (!strcmp(argv[0], "&")) /* Ignore singleton & */
        return 1;
    return 0; /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv, int *argc_num)
{
    char *delim; /* Points to first space delimiter */
    int argc;    /* Number of args */
    int bg;      /* Background job? */

    buf[strlen(buf) - 1] = ' ';   /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' ')))
    {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;

    if (argc == 0) /* Ignore blank line */
        return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc - 1] == '&')) != 0)
        argv[--argc] = NULL;

    *argc_num = argc;

    return bg;
}
/* $end parseline */
