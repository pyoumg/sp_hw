/* 
 * echoservers.c - A concurrent echo server based on select
 */
/* $begin echoserversmain */
#include "csapp.h"

typedef struct { /* represents a pool of connected descriptors */ //line:conc:echoservers:beginpool
    int maxfd;        /* largest descriptor in read_set */   
    fd_set read_set;  /* set of all active descriptors */
    fd_set ready_set; /* subset of descriptors ready for reading  */
    int nready;       /* number of ready descriptors from select */   
    int maxi;         /* highwater index into client array */
    int clientfd[FD_SETSIZE];    /* set of active descriptors */
    rio_t clientrio[FD_SETSIZE]; /* set of active read buffers */
} pool; //line:conc:echoservers:endpool
/* $end echoserversmain */
void init_pool(int listenfd, pool *p);
void add_client(int connfd, pool *p);
void check_clients(pool *p);
/* $begin echoserversmain */
item *root = NULL, *iTemp;
char* printline;
int byte_cnt = 0; /* counts total bytes received by server */

int main(int argc, char **argv)
{
    int listenfd, connfd; 
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    static pool pool; 

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }


    listenfd = Open_listenfd(argv[1]);
    init_pool(listenfd, &pool); //line:conc:echoservers:initpool
	init_tree();
	printline=Malloc(sizeof(char)*MAXLINE);
    while (1) {
	/* Wait for listening/connected descriptor(s) to become ready */
	pool.ready_set = pool.read_set;
	pool.nready = Select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);

	/* If listening descriptor ready, add new client to pool */
	if (FD_ISSET(listenfd, &pool.ready_set)) { //line:conc:echoservers:listenfdready
		clientlen=sizeof(struct sockaddr_storage);
	    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:conc:echoservers:accept
	    add_client(connfd, &pool); //line:conc:echoservers:addclient
	}
	
	/* Echo a text line from each ready connected descriptor */ 
	check_clients(&pool); //line:conc:echoservers:checkclients
    }
	Free(printline);
}
/* $end echoserversmain */

/* $begin init_pool */
void init_pool(int listenfd, pool *p) 
{
    /* Initially, there are no connected descriptors */
    int i;
    p->maxi = -1;                   //line:conc:echoservers:beginempty
    for (i=0; i< FD_SETSIZE; i++)  
	p->clientfd[i] = -1;        //line:conc:echoservers:endempty

    /* Initially, listenfd is only member of select read set */
    p->maxfd = listenfd;            //line:conc:echoservers:begininit
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set); //line:conc:echoservers:endinit
}
/* $end init_pool */

/* $begin add_client */
void add_client(int connfd, pool *p) 
{
    int i;
    p->nready--;
    for (i = 0; i < FD_SETSIZE; i++)  /* Find an available slot */
	if (p->clientfd[i] < 0) { 
	    /* Add connected descriptor to the pool */
	    p->clientfd[i] = connfd;                 //line:conc:echoservers:beginaddclient
	    Rio_readinitb(&p->clientrio[i], connfd); //line:conc:echoservers:endaddclient

	    /* Add the descriptor to descriptor set */
	    FD_SET(connfd, &p->read_set); //line:conc:echoservers:addconnfd

	    /* Update max descriptor and pool highwater mark */
	    if (connfd > p->maxfd) //line:conc:echoservers:beginmaxfd
		p->maxfd = connfd; //line:conc:echoservers:endmaxfd
	    if (i > p->maxi)       //line:conc:echoservers:beginmaxi
		p->maxi = i;       //line:conc:echoservers:endmaxi
	    break;
	}
    if (i == FD_SETSIZE) /* Couldn't find an empty slot */
	app_error("add_client error: Too many clients");
}
/* $end add_client */

/* $begin check_clients */
void check_clients(pool *p) 
{
    int i, connfd, n;
    char buf[MAXLINE]; 
    rio_t rio;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++) {
	connfd = p->clientfd[i];
	rio = p->clientrio[i];

	/* If the descriptor is ready, echo a text line from it */
	if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) { 
	    p->nready--;
	    if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
		byte_cnt += n; //line:conc:echoservers:beginecho
		printline=memset(printline,0,MAXLINE);
        printf("server received %d bytes\n", n);
        
        if(!strncmp("show",buf,4)){
                show(root);
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
        update_file();
    }
    
	    /* EOF detected, remove descriptor from pool */
	    else { 
		Close(connfd); //line:conc:echoservers:closeconnfd
		FD_CLR(connfd, &p->read_set); //line:conc:echoservers:beginremove
		p->clientfd[i] = -1;          //line:conc:echoservers:endremove
	    }
	}
    }
}
/* $end check_clients */

void clear_printline(){
    for(int i=0;i<MAXLINE;i++)
        printline[i]='\0';
}
void show(item *node)
{
    //앞뒤로 blocking 해야함
    if (node == NULL)
        return;
    char temp_str[10]={'\0',};
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
   
    show(node->left);
    show(node->right);
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
    left_stock = item_stock->left_stock;

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

    item_stock->left_stock = left_stock - num; //산만큼 감소

    return 1;
}
int sell_stock(item *item_stock, int left_stock, int num)
{
    item_stock->left_stock = left_stock + num; //산만큼 감소

    return 1;
}

//fail: null
item *search_tree(int i, item *node)
{
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
        iTemp=Malloc(sizeof(item));
        iTemp->ID=id;
        iTemp->left_stock=left_stock;
        iTemp->price=price;
        iTemp->readcnt=0;
        iTemp->left=NULL;
        iTemp->right=NULL;
        root=iTemp;
        
        
        return root;
    }

    if(root->ID > id)
            root->left = insert_tree(id,left_stock,price,root->left);
    else
        root->right = insert_tree(id,left_stock,price,root->right);

    return root;
}

void update_file()
{
   
    int fd= Open("stock.txt",O_WRONLY,0);
    if(fd<0){
        printf("file doesn't exist\n");
        return;
    }
    printline=memset(printline,0,MAXLINE);
   show(root);
   printline[strlen(printline)]='\0';
//    printf("%s",printline);
    Write(fd,printline,strlen(printline));
    Close(fd);
   
}

 