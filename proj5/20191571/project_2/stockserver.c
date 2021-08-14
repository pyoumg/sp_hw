/* 
 * echoservert.c - A concurrent echo server using threads
 */
/* $begin echoservertmain */
#include "csapp.h"

void echo2(int connfd) ;
void echo(int connfd);
void *thread(void *vargp);
item *root = NULL;
sem_t file_mutex;
int main(int argc, char **argv) 
{
    
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid; 


    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }
    listenfd = Open_listenfd(argv[1]);
   init_tree();

    while (1) {
        clientlen=sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int)); //line:conc:echoservert:beginmalloc
        *connfdp = Accept(listenfd, (SA *) &clientaddr, &clientlen); //line:conc:echoservert:endmalloc
        Pthread_create(&tid, NULL, thread, connfdp);
    }
}

/* Thread routine */
void *thread(void *vargp) 
{  
   
    int connfd = *((int *)vargp);
    Pthread_detach(pthread_self()); //line:conc:echoservert:detach
    Free(vargp);                    //line:conc:echoservert:free
    echo2(connfd);
    Close(connfd);
    return NULL;
}
/* $end echoservertmain */
void echo2(int connfd) 
{
    int n; 
    char buf[MAXLINE]; 
    rio_t rio;
    char* printline;
    Rio_readinitb(&rio, connfd);
    printline=Malloc(sizeof(char)*MAXLINE);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printline=memset(printline,0,MAXLINE);
        printf("server received %d bytes\n", n);
        
        if(!strncmp("show",buf,4)){
                show(root,printline);
            }
        else if(!strncmp("buy ",buf,4)){
           
            if(update_stock(buf,1)<0){
                strcpy(printline,"Not enough left stocks\n");
            }
            else{
                 strcpy(printline,"[buy] success\n");
            }
           
        }
        else if(!strncmp("sell ",buf,5)){
            update_stock(buf,2);
            strcpy(printline,"[sell] success\n");
           
        }
        else{
            strcpy(printline,"incorrect command\n");
        }

        Rio_writen(connfd, printline, MAXLINE);
        
    }
    update_file(printline);
    Free(printline);
}
void echo(int connfd)
{
    char* printline;
    printline=Malloc(MAXLINE);
    update_file(printline);
    Free(printline);
}
void show(item *node,char* printline)
{
   
    if (node == NULL)
        return;
    char temp_str[10]={'\0',};

    P(&(node->mutex));

    int pr = node->price;
    
    if(node==root){
        itoa(node->ID,printline,10);
    }
    else{
        itoa(node->ID,temp_str,10);
        strcat(printline, temp_str);
    }
    printline[strlen(printline)] = ' ';
    itoa(node->left_stock, temp_str, 10);
    strcat(printline, temp_str);
    printline[strlen(printline)] = ' ';
    itoa(pr, temp_str, 10); //10진수
    strcat(printline, temp_str);
    printline[strlen(printline)] = '\n';
    V(&(node->mutex));
    show(node->left,printline);
    show(node->right,printline);
}
//flag:1(buy) 2:sell
int update_stock(char *cmd, int flag)
{
    item *item_stock;
    int id = 0;
    int left_stock;
    int len = strlen(cmd);
    int num,blank=0;
    for (int i = len - 1; i >= 0; i--)
    {
        if (cmd[i] == ' ')
        {
            blank = i;
            break;
        }
    }
    if(flag==1)
        id = atoi(cmd+4);
    else
        id=atoi(cmd+5);
    num = atoi(cmd + blank);
 
    item_stock = search_tree(id, root);
    if (!item_stock)
        return -1; //id fail
    P(&(item_stock->mutex));
    left_stock = item_stock->left_stock;
    V(&(item_stock->mutex));
    if (flag == 1)
    {
        return buy_stock(item_stock,left_stock,num);
    }
    else
        return sell_stock(item_stock,left_stock,num);
}
int buy_stock(item *item_stock, int left_stock, int num)
{
   
    if (left_stock < num)
        return -2; //stock fail
    P(&(item_stock->mutex));
    item_stock->left_stock = left_stock - num; //산만큼 감소
    V(&(item_stock->mutex));
    return 1;
}
int sell_stock(item *item_stock, int left_stock, int num)
{
    P(&(item_stock->mutex));
    item_stock->left_stock = left_stock + num; //산만큼 감소
    V(&(item_stock->mutex));
    return 1;
}

//fail: null
item *search_tree(int i, item *node)
{
    //값 변동x : 세마포어 필요 없음,트리의 left right는 생성시 결정
    if (node == NULL)
        return NULL;
  // printf("search %d %d\n",node->ID,i);

    if (node->ID < i)
        return search_tree(i, node->right);
    else if (node->ID > i)
        return search_tree(i, node->left);
    else
    {
        return node;
    }
}

void free_tree(item *node)
{
    if (node == NULL)
        return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}
/*tree*/
void init_tree()
{
    int fd= Open("stock.txt",O_RDONLY,0);
    rio_t rio;
    char buf[MAXLINE];
    char *temp_atoi;
    int id, left_stock = -1, price = -1;
    Sem_init(&file_mutex,0,1);
    Rio_readinitb(&rio, fd);
    while (Rio_readlineb(&rio, buf, MAXLINE) != 0)
    {
       temp_atoi=strtok(buf," ");
       id=atoi(temp_atoi);
       temp_atoi=strtok(NULL," ");
        left_stock=atoi(temp_atoi);
        temp_atoi=strtok(NULL," ");
        price=atoi(temp_atoi);
   //     printf("%d %d %d\n",id,left_stock,price);
        root=insert_tree(id,left_stock,price,root);

    }
    Close(fd);
}
item* insert_tree(int id,int left_stock,int price,item* root)
{
    if(!root){
        item* iTemp=Malloc(sizeof(item));
        iTemp->ID=id;
        iTemp->left_stock=left_stock;
        iTemp->price=price;
        iTemp->readcnt=0;
        iTemp->left=NULL;
        iTemp->right=NULL;
        Sem_init(&(iTemp->mutex),0,1);
        root=iTemp;
        
        
        return root;
    }

    if(root->ID > id)
            root->left = insert_tree(id,left_stock,price,root->left);
    else
        root->right = insert_tree(id,left_stock,price,root->right);

    return root;
}


void update_file(char* printline)
{
    P(&file_mutex);
    int fd= Open("stock.txt",O_WRONLY,0);
    if(fd<0){
        printf("file doesn't exist\n");
        return;
    }
    printline=memset(printline,0,MAXLINE);
   show(root,printline);
   printline[strlen(printline)]='\0';
  
//    printf("%s",printline);
    Write(fd,printline,strlen(printline));
    Close(fd);
    V(&file_mutex);
//    printf("update end\n");
}

 