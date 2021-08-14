/*help/dir�� ����ϴ� �Լ�*/
void p_help();
void p_dir();

/*hashtable���� �Լ�*/
void i_hashtable(FILE* pFile);
int hash_function(char* str);
void f_hashtable(); 
int s_hashtable(char* mnemonic);
void p_hashtable();
/*history ���� �Լ�*/
void s_history(char* cmd);
void i_history();
void p_history();
void f_history();

/*�� ��ɾ �����ϴ� �Լ�*/
void dump(int start, int end);
void edit(int address, int value);
void fill(int start, int end, int value);
void reset();

//16���� Ȯ�� �Լ�
int check_hex(char* wrd, int len);
int check_hex2(char* command, int idx_a, int idx_b, int* start, int* end, int* value, int num);

/*command ���� �Լ�*/
char* clear_command(char* command, int* num, int len);
int check_command(char* command, char* ans1, char* ans2, int* start, int* end, int* value);
char* trim(char* command);