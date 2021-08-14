#pragma once
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define MEMORY_SIZE 1048576//1MB
#define linesize 500

/*help/dir을 출력하는 함수*/
void p_help();
void p_dir();


//hash table
typedef struct _HASH {
	//data
	int op;
	char* mnemonic;
	char* num;
	struct _HASH* link;
}HASH;


//history linked list
typedef struct _HI_NODE {
	char* command;
	struct _HI_NODE* link;
}HI_NODE;


/*project2 struct */


typedef struct sort_symtab {
	char* name;
	int num;
}SORT_SYMTAB;


typedef struct listing {

	int loc;
	char* line;
	char* obj;
	int comment_flag;//주석인지 아닌지 
}LST;


//symtab (HASH TABLE)
typedef struct _SYMTAB {
	//data
	int error;
	char* name;
	int loc;
	struct _SYMTAB* link;
}SYMTAB;


typedef struct _tempfile {//pass1 에서 생성할 임시파일
	char* str1;
	char* str2;
	char* str3;
	char fullstr[101];
	int loc;
	int strnum;
}TEMPFILE;

typedef struct _ESTAB {

	int flag;// control section인지 symbol인지 구분: load map 에서 출력 형식이 다름!
	//0:control section, 1: symbol
	int address;
	char* name;
	int length;
	struct _ESTAB* link;
}ESTAB;


/*hashtable관련 함수*/
void i_hashtable(FILE* pFile);
int hash_function(char* str);
void f_hashtable();
int s_hashtable(char* mnemonic);
void p_hashtable();
/*history 관련 함수*/
void s_history(char* cmd);
void i_history();
void p_history();
void f_history();

/*각 명령어를 수행하는 함수*/
void dump(int start, int end);
void edit(int address, int value);
void fill(int start, int end, int value);
void reset();

//16진수 확인 함수
int check_hex(char* wrd, int len);
int check_hex2(char* command, int idx_a, int idx_b, int* start, int* end, int* value, int num);

/*command 관련 함수*/
char* clear_command(char* command, int* num, int len);
int check_command(char* command, char* ans1, char* ans2, int* start, int* end, int* value);
char* trim(char* command);
char* loader_trim(char* command);

/*assembly (project2) function*/
FILE* open_file(char* str);//파일 열기
int type(char* filename);//type command 
int pass1(FILE* pFile, HASH** hash_table, char* filename);
int pass2(HASH** hash_table, char* filename, int start_address);
void write_obj(char* str, int str_idx, int start_address, FILE* fObj);
void  pass2_close(char* lst, char* obj, int error, int idx);


void init_lst();
void free_lst();
void free_tempf();

int pc(int idx, HASH** hash_table);
int base(int idx, HASH** hash_table);
int format4(int idx, HASH** hash_table);
void sort_symtab();

void free_sort();
void p_symbol();
void init_symtab();

int get_opcode(int idx, HASH** hash_table, int* n, int* i, int* x, int* address, int* opcode, int pc_base, int* sym_flag);
int is_register(char* str, int* x);//register인지 확인
int get_address(int idx, int  num, char* a, char* b, char* c, char* buf, HASH** hash_table);
HASH* s_hashtable2(char* mnemonic, HASH** hash_table);
int is_num(char* num, int flag);

int search_symtab(char* str);

void free_symtab();
int insert_symtab(char* str, HASH** hash_table);
int hash_function2(char* str);//symtab을 위한 hashfunction
void get_line(char* buf, int* num, char* a, char* b, char* c);

/*linker (project3) function*/
int set_progaddr(char* address);
int file_len(char* str);

int loader(char* str) ;
int load_pass1(char* str) ;

void init_estab() ;
int insert_estab(int address, char* name, int flag, int length);
void print_estab() ;
int search_estab(char* str);
void free_estab();

int load_pass2(char* str);
int get_num(char* buf, int len);
void set_bp(char* str) ;
void clear_bp();
void print_bp();
int get_address2(int num2, int num3, int num4, int n, int i, int b, int p, int e);
void find_register(int* reg1, int* reg2);
void clear_register(int reg) ;
int update_num(int num, int n, int i);
int update_num2(int num, int n, int i) ;
void store_num(int num, int value, int n, int i);
int check_op(int num1, int num2, int num3, int num4, int* idx);
int check_bp();
int run(int run_bp);
