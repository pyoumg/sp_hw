#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "20191571.h" //user-defined header file
#define MEMORY_SIZE 1048576//1MB

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




int memory[MEMORY_SIZE] = { 0. };//�޸� ����,0���� �ʱ�ȭ

int last_address = -1;//���������� dump�� ����� address


/*0:default 1:short 2:long */
int is_l_s = 0;//command�� �� �ܾ����� ª�� �ܾ����� ���� (command ������ �� ���)

HI_NODE* pNew, * pPre = NULL, * head = NULL;
HASH* hash_table[20], * hNew, * hPre[20]; //size20�� hash table


int main() {
	char command[100];// �Է¹޴� ��ɾ�
	int start = -1, end = -1, value = -1;//dump,edit,fill���� parameter�� ���
	char* command_trim;//trim()������ command 
	int flag;//�ٸ��Լ��� ȣ���ؼ� return���� ����
	int start_end_value[3] = { 0, };//start,end,value�� �����ؼ� parameter�� ����� �迭

	FILE* pFile = fopen("opcode.txt", "r");// opcode ����
	if (pFile == NULL) {//���Ͽ��� ����
		printf("file open error\n");
		return -1;
	}
	i_history();//history node init
	i_hashtable(pFile);//hash table init
	fclose(pFile);//���ϴݱ�
	while (1) {
		printf("sicsim> ");
		fgets(command, 100, stdin);//command �Է¹ޱ� (����)

		command[strlen(command) - 1] = '\0'; //���๮�� ����

		command_trim = trim(command);//���� ���� ����
		

		if (strcmp(command_trim, "h") == 0 || strcmp(command_trim, "help") == 0) { //h[elp]
			s_history(command_trim);
			p_help();// ��ɾ� ����Ʈ ���

		}
		else if (strcmp(command_trim, "d") == 0 || strcmp(command_trim, "dir") == 0) {//d[ir]
			s_history(command_trim);
			p_dir();//���͸� ���

		}
		else if (strcmp(command_trim, "q") == 0 || strcmp(command_trim, "quit") == 0) {//q[uit]

			s_history(command_trim);
			break;//����
		}

		else if (strcmp(command_trim, "hi") == 0 || strcmp(command_trim, "history") == 0) {//hi[story]

			s_history(command_trim);
			p_history();
		}
		else if (strncmp(command_trim, "du", 2) == 0) {//'du'�� �����ϴ� ���
			start = -1, end = -1, value = -1;//�ʱ�ȭ
			is_l_s = 0;//�ʱ�ȭ
			flag = check_command(command_trim, "du", "dump", &start, &end, &value);
			if (flag == -1) {
				printf("incorrect command\n");
				continue;//history store ���� ���� 
			}
			else {
				dump(start, end);

				start_end_value[0] = start;
				start_end_value[1] = end;
				//dump�� value�� ������ �ʴ´�.
				if (is_l_s == 1)//ª�� command
					command_trim = clear_command("du", start_end_value, flag);
				else//2
					command_trim = clear_command("dump", start_end_value, flag);
			}

			s_history(command_trim);

		}
		else if (strncmp(command_trim, "e", 1) == 0) {//'e'�� �����ϴ� ���
			start = -1, end = -1, value = -1;//�ʱ�ȭ
			is_l_s = 0;//�ʱ�ȭ
			flag = check_command(command_trim, "e", "edit", &start, &end, &value);
			if (flag == -1) {
				printf("incorrect command\n");
				continue;
			}
			else {
				edit(start, end);

				start_end_value[0] = start;
				start_end_value[1] = end;
				//edit�� value�� ������ �ʴ´�.(end�� value��� ���)
				if (is_l_s == 1)
					command_trim = clear_command("e", start_end_value, flag);
				else//2
					command_trim = clear_command("edit", start_end_value, flag);
			}

			s_history(command_trim);

		}
		else if (strncmp(command_trim, "f", 1) == 0) {//'f'�� �����ϴ� ���
			start = -1, end = -1, value = -1;//�ʱ�ȭ
			is_l_s = 0;//�ʱ�ȭ
			flag = check_command(command_trim, "f", "fill", &start, &end, &value);
			if (flag == -1) {
				printf("incorrect command\n");
				continue;
			}
			else {
				fill(start, end, value);

				start_end_value[0] = start;
				start_end_value[1] = end;
				start_end_value[2] = value;

				if (is_l_s == 1)
					command_trim = clear_command("f", start_end_value, flag);
				else//2
					command_trim = clear_command("fill", start_end_value, flag);
			}

			s_history(command_trim);

		}
		else if (strcmp(command_trim, "reset") == 0) {//reset
			reset();
			s_history(command_trim);
		}
		else if (strncmp(command_trim, "opcode ", 7) == 0) {//opcode mnemonic
			char* mnemonic = trim(command_trim + 7);
			if (s_hashtable(mnemonic) == 1) {//�����ϴ� mnemonic�ΰ��
				strcpy(command_trim + 7, mnemonic);//trim()���� \0�� �־��ֱ⶧���� strcpy()�� ����ص� ������
				s_history(command_trim);
				//��ã���� ��� �����޼����� s_hashtable()���� ���
			}
		}
		else if (strcmp(command_trim, "opcodelist") == 0) {//opcodelist
			p_hashtable();
			s_history(command_trim);
		}
		else {
			printf("incorrect command\n");
		}

		free(command_trim);
	}


	f_hashtable();
	f_history();
	return 0;
}


/* help ����ϴ� �Լ�*/

void p_help() {

	printf("h[elp]\n");
	printf("d[ir]\n");
	printf("q[uit]\n");
	printf("hi[story]\n");
	printf("du[mp][start, end]\n");
	printf("e[dit] address, value\n");
	printf("f[ill] start, end, value\n");
	printf("reset\n");
	printf("opcode mnemonic\n");
	printf("opcodelist\n");


	return;
}


/* ���丮 ����ϴ� �Լ�*/

void p_dir() {
	int idx = 0;//���� 4���� ���๮�� ����ϱ� ���� ����

	DIR* dir_ptr = opendir("."); //���� ��� ����
	struct stat st;
	struct dirent* file = NULL;

	if (dir_ptr == NULL) { //���丮 ���� ����
		printf("directory open error\n");
		return;
	}


	while ((file = readdir(dir_ptr)) != NULL) {

		if (lstat(file->d_name, &st) == -1) {
			printf("stat error");
			return;
		}

		if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
			continue; //.�� ..�� ��� ����
		}

		idx++;



		if ((st.st_mode & S_IFMT) == S_IFDIR) { //���丮
			printf("\t%s/", file->d_name);

		}
		else if (st.st_mode & S_IXGRP || st.st_mode & S_IXUSR || st.st_mode & S_IXOTH) {// ���� ����
			printf("\t%s*", file->d_name);
		}
		else {
			printf("\t%s", file->d_name);
		}

		if (idx % 4 == 0)
			printf("\n");
	}

	if (idx % 4 != 0)
		printf("\n");

	closedir(dir_ptr);//���丮 �ݱ�




	return;
}



/*hash_table*/

//opcode.txt �� �־����⶧���� �������� �Է��� ���´ٰ� �����Ѵ�.
/*
	hashtable�� �����ϴ� �Լ�.
	op, mnemonic,num:opcode
	idx: hash table �� ����� index
*/
void i_hashtable(FILE* pFile) {
	int op, idx;
	char* mnemonic, * num;

	mnemonic = malloc(sizeof(char) * 6);
	num = malloc(sizeof(char) * 4);

	for (int i = 0; i < 20; i++) {
		hash_table[i] = malloc(sizeof(HASH));
		hash_table[i]->link = NULL;
		hPre[i] = hash_table[i];
	}//�ʱ�ȭ

	while (EOF!= fscanf(pFile, "%X\t%s\t\t%s", &op, mnemonic, num))//���� ������ �д´�.
	{
		
		hNew = malloc(sizeof(HASH));
		hNew->mnemonic = malloc(sizeof(char) * 6);
		hNew->num = malloc(sizeof(char) * 4);
		strcpy(hNew->mnemonic,mnemonic);
		strcpy(hNew->num, num);

		hNew->op = op;
		hNew->link = NULL;

		/*hash function*/
		idx = hash_function(mnemonic);
		hPre[idx]->link = hNew;
		hPre[idx] = hPre[idx]->link;


	}
	free(mnemonic);
	free(num);
}

/*�ؽ��Լ�*/
int hash_function(char* str) {
	int idx = strlen(str);
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] < 'A' || str[i]>'Z')
			return -1;
		idx += (str[i] - 'A') * pow(3, i);
	}
	idx = idx % 20;
	return idx;
}


void f_hashtable() {
	for (int i = 0; i < 20; i++) {
		HASH* temp = hash_table[i]->link, * temp2;
		while (temp != NULL) {
			temp2 = temp->link;

			free(temp->num);
			free(temp->mnemonic);
			free(temp);
			temp = temp2;
		}
	}
}

/*search hashtable*/
int s_hashtable(char* mnemonic) {
	int flag = hash_function(mnemonic);
	if (flag == -1) {
		printf("opcode search error\n");
		return -1;
	}
	HASH* temp = hash_table[flag]->link;
	while (temp != NULL) {
		if (strcmp(mnemonic, temp->mnemonic) == 0) {
			printf("opcode is %X\n", temp->op);
			return 1;
		}
		temp = temp->link;
	}
	printf("opcode search error\n");
	return -1;
}

/*print hashtable*/
void p_hashtable() {
	for (int i = 0; i < 20; i++) {
		HASH* temp = hash_table[i]->link;
		printf("%d : ", i);
		while (temp != NULL) {
			printf("[%s,%02X]", temp->mnemonic, temp->op);
			if (temp->link != NULL)
				printf(" -> ");
			temp = temp->link;
		}
		printf("\n");
	}
}

/*history*/

/*���α׷� ���۽� history node�� init �ϴ� �Լ�*/
void i_history() {
	head = malloc(sizeof(HI_NODE));
	head->link = NULL;
	pPre = head;
	return;
}



//history�� store�ϴ� �Լ�
void s_history(char* cmd) {


	pNew = malloc(sizeof(HI_NODE));//��ɾ ������ ���
	pNew->command = malloc(sizeof(cmd));
	strcpy(pNew->command, cmd);
	pNew->link = NULL;
	pPre->link = pNew;
	pPre = pPre->link;


}

//history ���
void p_history() {
	HI_NODE* temp = head->link;
	int idx = 1;
	while (temp != NULL) {
		printf("\t%d\t%s\n", idx, temp->command);
		temp = temp->link;
		idx++;
	}
}
//history node �����Ҵ� ���� 
void f_history() {
	HI_NODE* temp = head, * temp2 = head;
	while (temp != NULL) {
		temp = temp->link;
		free(temp2->command);
		free(temp2);
		temp2 = temp;
	}
	return;
}

/*
	return: -1(error)
			0 (du[mp])//dump�� �� ������ command�� ����
			1({command} start)
			2({command} start, end)
			3({command} start, end,value)
	command�� �ùٸ� ��������, �ùٸ� ���¶�� start�� end ,(value)�� �����Ѵ�.


	comma: ','�� ����, num:������ ����
	ans1: ª�� command (du)
	ans2: �� command (dump)
*/
int check_command(char* command, char* ans1, char* ans2, int* start, int* end, int* value) {
	int comma = 0, num = 0;//comma: ','�� ����, num:������ ����
	int check_idx;//Ȯ���� index�� ������ġ�� ��Ÿ��
	int idx_a, idx_b;//��� 16����(����)�� ���ڿ��ȿ��� ��ġ�ϴ� ����/�� index

	if ((strlen(command) == strlen(ans1) || strcmp(command, ans2) == 0) && command[0] == 'd')
	{//dump�� start,end�� ��� �ùٸ� command�̴�.
		if (strlen(command) == strlen(ans1))
			is_l_s = 1;
		else
			is_l_s = 2;

		/*start ����*/

		*start = last_address + 1; // dump()���� ������ ����� fffff�� ��쿡 last_address�� -1���� update����

		/*end ����*/
		if (*start + 159 > MEMORY_SIZE - 1) {
			*end = MEMORY_SIZE - 1;
		}
		else {
			*end = *start + 159;//160�� ���
		}
		return 0;
	}
	else if (strlen(command) == strlen(ans1) || strcmp(command, ans2) == 0)
		return -1;//�߸��� �Է�

	else if (strncmp(command, ans2, strlen(ans2)) == 0) {//�� command�� ���
		check_idx = strlen(ans2);
		is_l_s = 2;
	}
	else {//ª�� command�� ���
		check_idx = strlen(ans1);
		is_l_s = 1;
	}

	if (command[check_idx] != ' ') {
		//command �ڿ� ������ ���� ��� (�ݵ�� �־����)
		//Ȥ�� edit��� edxxó�� �� command�� �߸��� ��쵵 �ش�� 

		return -1;
	}


	/*{command}+' '������ Ȯ�� �� ����
	���� ���ڿ� ','�� ������ ������ �˻��Ѵ�.
	*/


	idx_a = -1;
	idx_b = -1;//�ʱ�ȭ



	for (int i = check_idx + 1; i < strlen(command); i++) {
		if (command[i] == ',') {//comma

			if (command[i - 1] != ' ') {//����(����)�� �������( 16������ �´��� Ȯ���ؾ���)
				idx_b = i - 1;
				if (check_hex2(command, idx_a, idx_b, start, end, value, num) == -1)
					return -1;

				num++;//���� ���� ����
				idx_a = -1;
				idx_b = -1;//�ʱ�ȭ
			}

			if (num - 1 != comma)//���� ���� �ĸ����� �ִ� ���,comma�� AA,,BBó�� �ߺ��Ǿ� �����ϴ� ���
			{
				return -1;
			}
			comma++;//comma ���� ����
		}
		else if (command[i] == ' ') {//����
			if (command[i - 1] == ' ' || command[i - 1] == ',')
				continue;

			else {//16���� ����(����)�� ���� ���
				idx_b = i - 1;
				if (check_hex2(command, idx_a, idx_b, start, end, value, num) == -1)
					return -1;
				num++;
				idx_a = -1;
				idx_b = -1;//�ʱ�ȭ
			}
		}

		else {//�� �� (���ڿ��� �������� ��쿡�� �˻��ؾ���)

			if (idx_a == -1)//���ڰ� ������ �������� ���� ���
				idx_a = i;//���� ������

			if (i == strlen(command) - 1) {
				idx_b = i;
				if (check_hex2(command, idx_a, idx_b, start, end, value, num) == -1)
					return -1;
				num++;
				idx_a = -1;
				idx_b = -1;//�ʱ�ȭ
			}
		}
	}//end for

	if (num == 1) {//dump�� start�� ������ ��쿡�� end�� ���� ���ؾ��Ѵ�.
		/*end ����*/
		if (*start + 159 > MEMORY_SIZE - 1) {
			*end = MEMORY_SIZE - 1;
		}
		else {
			*end = *start + 159;

		}
	}

	if (comma != num - 1)
		return -1;
	else if (command[0] == 'e' && (num != 2||*end>255))//edit {},{}�� ���, end(=value)�� ff���� ���
		return -1;
	else if (command[0] == 'f' && (num != 3 || *start > *end||*value>255))
		return -1;//Start �ּҰ� end �ּҺ��� ū ���� ���� ���, ���� ó��.
	else if (command[0] == 'd' && *start > *end)
		return -1;//Start �ּҰ� end �ּҺ��� ū ���� ���� ���, ���� ó��.


	return num;

}
/*�޸𸮿� ����Ǿ��ִ� ������ ����Ѵ�.
�ܼ��� ��°� last_address update���� ����ϸ�,
end,start�� ��������, ����� �������� ���� ���δ� �˻����� ����
*/
void dump(int start, int end) {
	for (int i = start / 16 * 16; i < (end / 16 + 1) * 16; i++) {
		if (i % 16 == 0)
			printf("%05X ", i);//�ּ� ���

		/*�޸� ���� ���*/
		if (i < start || i>end)
			printf("   ");
		else
			printf("%02X ", memory[i]);

		/*�޸� ���� ���*/
		if (i % 16 == 15) {
			printf("; ");
			for (int j = i - 15; j <= i; j++) {
				if (j<start || j>end || memory[j] < 32 || memory[j]>126)//�־��� ���� �Ѵ°��
					printf(".");
				else
					printf("%1c", memory[j]);
			}

			printf("\n");
		}
	}
	last_address = end; //update last_address 
	if (last_address == MEMORY_SIZE - 1)
		last_address = -1;
}



/*	�޸��� ����� ���� �����ϴ� �Լ�*/
/*	address�� �������δ� �˻����� ����(check_hex���� �̸� �˻�)

*/
void edit(int address, int value) {
	
	memory[address] = value;
	return;
	
}

/*
	value�� start~end������ memory�� ä��.
*/
void fill(int start, int end, int value) {

	for (int i = start; i <= end; i++)
		memory[i] = value;
	return ;
}
/*�޸��� ��� ������ 0���� �ٲ�*/
void reset() {
	fill(0, MEMORY_SIZE - 1, 0);
}
/* {len}�ڸ��� 16�������� Ȯ��
	return:
	>=0: hex�� 10������ ��ȯ
	-1: not hex
*/
int check_hex(char* wrd, int len) {
	int dec = 0;//10������ ��ȯ�� ����� �����ϴ� ����
	if (strlen(wrd) != len)
		return -1;
	for (int i = 0; i < len; i++)
	{
		if (wrd[i] >= '0' && wrd[i] <= '9') {
			dec += (wrd[i] - '0') * pow(16, len - i - 1);
		}
		else if (wrd[i] >= 'a' && wrd[i] <= 'f') {
			dec += (wrd[i] - 'a' + 10) * pow(16, len - i - 1);
		}
		else if (wrd[i] >= 'A' && wrd[i] <= 'F') {
			dec += (wrd[i] - 'A' + 10) * pow(16, len - i - 1);
		}
		else
			return -1;
	}
	if (dec>=MEMORY_SIZE)//fffff������ ���� 
	{
		return -1;
	}

	return dec;

}

/*check_command()���� ���� Ȯ���� �� ���*/
int check_hex2(char* command, int idx_a, int idx_b, int* start, int* end, int* value, int num) {
	int num_a;
	char* ishex = malloc(sizeof(char) * (idx_b - idx_a + 2));
	ishex[idx_b - idx_a + 1] = '\0';
	strncpy(ishex, command + idx_a, idx_b - idx_a + 1);
	num_a = check_hex(ishex, strlen(ishex));
	if (num_a == -1)//16���� ���� �ƴ�
	{
		return -1;
	}
	else {//�ùٸ� ����

		if (num == 0)
			*start = num_a;
		else if (num == 1)
			*end = num_a;
		else if (num == 2 && command[0] == 'f')//fill�ΰ�츸 ���� 3�� 
			*value = num_a;
		else {
			return -1;//���ڰ� �ʹ� ���� ���� ���
		}

	}
	return 1;
}


/*command �� �����ؼ� return*/
/*ex) {command} num[0], num[1] (len�� ���� �޶���)
	len: num�� ���� (len>=0)
*/
char* clear_command(char* command, int* num, int len) {
	char* clear = malloc(sizeof(char) * 100);
	if (len == 0) 
		sprintf(clear, "%s", command);
	else if (len == 1)
		sprintf(clear, "%s %X", command, num[0]);
	else if (len == 2)
		sprintf(clear, "%s %X, %X", command, num[0], num[1]);
	else//3(3���� ū ���� ����)
		sprintf(clear, "%s %X, %X, %X", command, num[0], num[1], num[2]);
	return clear;
}

/*�յ� ���� ����*/
char* trim(char* command) {
	char* trim_cmd;

	int front = 0, rear = strlen(command) - 1;

	while (command[front] == ' '|| command[front] == '\t') {
		front++;
	}
	while (command[rear] == ' ' || command[rear] == '\t') {
		rear--;
	}

	trim_cmd = malloc(sizeof(char) * (rear - front + 2));
	for (int i = front; i <= rear; i++) {
		if (command[i] == '\t')
			command[i] = ' ';//�����ؼ� �����ϱ� ������ ���⼭ ó���ص� �Լ� ���࿡ ��������.
		trim_cmd[i - front] = command[i];
	}
	trim_cmd[rear - front + 1] = '\0';

	return trim_cmd;
}
