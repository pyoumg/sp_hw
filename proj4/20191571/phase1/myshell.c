/* $begin shellmain */
#include "myshell.h"
#include <errno.h>
#define MAXARGS 128



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
            exit(0);

        /* Evaluate */
        eval(cmdline);
    }
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline)
{
    int free_flag = 0;
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    char temp[MAXARGS] = "/bin/";
    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL)
        return; /* Ignore empty lines */
    if (!builtin_command(argv))
    { //quit -> exit(0), & -> ignore, other -> run
        if ((pid = Fork()) == 0)
        { /* Child runs user job */

            int i = 0, j = 0;
            while (argv[i] != NULL)
            {
                if (argv[i][0] == '\'' || argv[i][0] == '"')
                {
                    for (j = 0; j < strlen(argv[i]) - 1; j++)
                    {
                        argv[i][j] = argv[i][j + 1];
                    }
                    argv[i][j - 1] = '\0'; //'가 2개씩 들어있기 때문에
                }

                //      printf("%s\n",argv[i]);
                i++;
            }

            if (check_other_cmd(argv, &free_flag))
            {}
            else if (execve(argv[0], argv, environ) < 0)
            {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
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

    if (free_flag)
        Free(argv[0]);
    return;
}

void sigterm_handler(int sig)
{ /*sigterm handler*/
    exit(0);
}

int check_other_cmd(char **argv, int *flag)
{
    char *temp_argv;

    if (!strcmp(argv[0], "exit"))
    {                     //parent process terminate
        kill(0, SIGTERM); //parent process
        exit(0);
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
/*
    return value 
    error:0 else: 1
*/
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
    if (!strcmp(argv[0], "cd"))
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
int parseline(char *buf, char **argv)
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

    return bg;
}
/* $end parseline */
