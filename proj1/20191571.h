/*help/dir을 출력하는 함수*/
void p_help();
void p_dir();

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