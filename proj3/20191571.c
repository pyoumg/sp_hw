#include "20191571.h"		   //user-defined header file
#include "20191571_assembly.c" //sicxe assembler
#include "20191571_linker.c"

int last_address = -1; //dump address

int run_idx = 0;
/*0:default 1:short 2:long */
int is_l_s = 0; //command가 긴 단어인지 짧은 단어인지 구별 (command 정제할 때 사용)

int memory[MEMORY_SIZE] = { 0. }; //memory space, initialized

HI_NODE* pNew, * pPre = NULL, * head = NULL;
HASH* hash_table[20], * hNew, * hPre[20]; //size20인 hash table

int main()
{
	char command[100];					  // 입력받는 명령어
	int start = -1, end = -1, value = -1; //dump,edit,fill에서 parameter로 사용
	char* command_trim;					  //trim()수행한 command
	int flag;							  //다른함수를 호출해서 return값을 저장
	int start_end_value[3] = {
		0,
	}; //start,end,value를 저장해서 parameter로 사용할 배열

	FILE* pFile = fopen("opcode.txt", "r"); // opcode 열기
	if (pFile == NULL)
	{ //파일열기 실패
		printf("opcode file open error\n");
		return -1;
	}
	i_history();		//history node init
	i_hashtable(pFile); //hash table init
	fclose(pFile);		//파일닫기
	while (1)
	{
		printf("sicsim> ");
		fgets(command, 100, stdin); //command 입력받기 (띄어쓰기)

		command[strlen(command) - 1] = '\0'; //개행문자 제거

		command_trim = trim(command); //양쪽 공백 제거

		if (strcmp(command_trim, "h") == 0 || strcmp(command_trim, "help") == 0)
		{ //h[elp]
			s_history(command_trim);
			p_help(); // 명령어 리스트 출력
		}
		else if (strcmp(command_trim, "d") == 0 || strcmp(command_trim, "dir") == 0)
		{ //d[ir]
			s_history(command_trim);
			p_dir(); //디렉터리 출력
		}
		else if (strcmp(command_trim, "q") == 0 || strcmp(command_trim, "quit") == 0)
		{ //q[uit]

			s_history(command_trim);
			break; //종료
		}

		else if (strcmp(command_trim, "hi") == 0 || strcmp(command_trim, "history") == 0)
		{ //hi[story]

			s_history(command_trim);
			p_history();
		}
		else if (strncmp(command_trim, "du", 2) == 0)
		{									  //'du'로 시작하는 경우
			start = -1, end = -1, value = -1; //초기화
			is_l_s = 0;						  //초기화
			flag = check_command(command_trim, "du", "dump", &start, &end, &value);
			if (flag == -1)
			{
				printf("incorrect command\n");
				continue; //history store 하지 않음
			}
			else
			{
				dump(start, end);

				start_end_value[0] = start;
				start_end_value[1] = end;
				//dump는 value가 사용되지 않는다.
				if (is_l_s == 1) //짧은 command
					command_trim = clear_command("du", start_end_value, flag);
				else //2
					command_trim = clear_command("dump", start_end_value, flag);
			}

			s_history(command_trim);
		}
		else if (strncmp(command_trim, "e", 1) == 0)
		{									  //'e'로 시작하는 경우
			start = -1, end = -1, value = -1; //초기화
			is_l_s = 0;						  //초기화
			flag = check_command(command_trim, "e", "edit", &start, &end, &value);
			if (flag == -1)
			{
				printf("incorrect command\n");
				continue;
			}
			else
			{
				edit(start, end);

				start_end_value[0] = start;
				start_end_value[1] = end;
				//edit는 value가 사용되지 않는다.(end가 value대신 사용)
				if (is_l_s == 1)
					command_trim = clear_command("e", start_end_value, flag);
				else //2
					command_trim = clear_command("edit", start_end_value, flag);
			}

			s_history(command_trim);
		}
		else if (strncmp(command_trim, "f", 1) == 0)
		{									  //'f'로 시작하는 경우
			start = -1, end = -1, value = -1; //초기화
			is_l_s = 0;						  //초기화
			flag = check_command(command_trim, "f", "fill", &start, &end, &value);
			if (flag == -1)
			{
				printf("incorrect command\n");
				continue;
			}
			else
			{
				fill(start, end, value);

				start_end_value[0] = start;
				start_end_value[1] = end;
				start_end_value[2] = value;

				if (is_l_s == 1)
					command_trim = clear_command("f", start_end_value, flag);
				else //2
					command_trim = clear_command("fill", start_end_value, flag);
			}

			s_history(command_trim);
		}
		else if (strcmp(command_trim, "reset") == 0)
		{ //reset
			reset();
			s_history(command_trim);
		}
		else if (strncmp(command_trim, "opcode ", 7) == 0)
		{ //opcode mnemonic
			char* mnemonic = trim(command_trim + 7);
			if (s_hashtable(mnemonic) == 1)
			{ //if found
				strcpy(command_trim + 7, mnemonic);
				s_history(command_trim);
				//error message : s_hashtable()
			}
		}
		else if (strcmp(command_trim, "opcodelist") == 0)
		{ //opcodelist
			p_hashtable();
			s_history(command_trim);
		}
		else if (strncmp(command_trim, "type ", 5) == 0)
		{ //type 으로 시작
			char* filename = trim(command_trim + 5);
			flag = type(filename);
			if (flag == -1)
				printf("file doesn't exist\n");
			else
				s_history(command_trim);
		}
		else if (strncmp(command_trim, "assemble ", 9) == 0)
		{ //assemble
			char* filename = trim(command_trim + 9);
			if (strlen(filename) > 4 && strcmp(filename + strlen(filename) - 4, ".asm") == 0)
			{
				FILE* fAsm = open_file(filename);
				if (fAsm == NULL)
					printf("file doesn't exist\n");
				else
				{
					if (pass1(fAsm, hash_table, filename) != -1)
						s_history(command_trim);
				}
			}
			else
				printf("file name error\n");
		}
		else if (strcmp(command_trim, "symbol") == 0)
		{ //symbol
			p_symbol();
			s_history(command_trim);
		}
		else if (strncmp(command_trim, "progaddr ", 9) == 0)
		{ //progaddr
			char* filename = trim(command_trim + 9);
			flag = set_progaddr(filename);
			if (flag == 1)
				s_history(command_trim);
			else
				printf("incorrect range\n");
		}
		else if (strncmp(command_trim, "loader ", 7) == 0)
		{ //loader

			char* filename = trim(command_trim + 7);

			flag = loader(filename);

			if (flag == 1)
			{
				s_history(command_trim);
				run_idx = 0;

			}
			else
				printf("incorrect filename\n");

		}
		else if (strcmp(command_trim, "bp") == 0)//bp print
		{
			print_bp();
			s_history(command_trim);
		}
		else if (strcmp(command_trim, "bp clear") == 0)
		{
			clear_bp();
			s_history(command_trim);
		}
		else if (strncmp(command_trim, "bp ", 3) == 0)
		{ //bp
			char* bp_str = trim(command_trim + 3);
			set_bp(bp_str);
			s_history(command_trim);
		}


		else if (strcmp(command_trim, "run") == 0)
		{
			if (run(run_idx) == 1)
			{
				run_idx++;
				s_history(command_trim);
			}
			else
				printf("run error\n");
		}
		else
		{
			printf("incorrect command\n");
		}

		free(command_trim);
	}

	f_hashtable();
	f_history();
	return 0;
}

/*print help*/

void p_help()
{

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
	printf("assemble filename\n");
	printf("type filename\n");
	printf("symbol\n");

	printf("progaddr [address]\n");
	printf("loader [object filename1] [object filename2] […]\n");
	printf("bp [address]\n");
	printf("bp clear\n");
	printf("bp\n");
	printf("run\n");

	return;
}

/* 디렉토리 출력하는 함수*/

void p_dir()
{
	int idx = 0; //파일 4개당 개행문자 출력하기 위한 변수

	DIR* dir_ptr = opendir("."); //현재 경로 열기
	struct stat st;
	struct dirent* file = NULL;

	if (dir_ptr == NULL)
	{ //디렉토리 열기 실패
		printf("directory open error\n");
		return;
	}

	while ((file = readdir(dir_ptr)) != NULL)
	{

		if (lstat(file->d_name, &st) == -1)
		{
			printf("stat error");
			return;
		}

		if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
		{
			continue; //.나 ..는 출력 안함
		}

		idx++;

		if ((st.st_mode & S_IFMT) == S_IFDIR)
		{ //디렉토리
			printf("\t%s/", file->d_name);
		}
		else if (st.st_mode & S_IXGRP || st.st_mode & S_IXUSR || st.st_mode & S_IXOTH)
		{ // 실행 파일
			printf("\t%s*", file->d_name);
		}
		else
		{
			printf("\t%s", file->d_name);
		}

		if (idx % 4 == 0)
			printf("\n");
	}

	if (idx % 4 != 0)
		printf("\n");

	closedir(dir_ptr); //디렉토리 닫기

	return;
}

/*hash_table*/

//opcode.txt 는 주어지기때문에 정상적인 입력이 들어온다고 가정한다.
/*
	hashtable을 생성하는 함수.
	op, mnemonic,num:opcode
	idx: hash table 에 저장될 index
*/
void i_hashtable(FILE* pFile)
{
	int op, idx;
	char* mnemonic, * num;

	mnemonic = malloc(sizeof(char) * 7);
	num = malloc(sizeof(char) * 4);

	for (int i = 0; i < 20; i++)
	{
		hash_table[i] = malloc(sizeof(HASH));
		hash_table[i]->link = NULL;
		hPre[i] = hash_table[i];
	} //초기화

	while (EOF != fscanf(pFile, "%X\t%s\t\t%s", &op, mnemonic, num)) //파일 끝까지 읽는다.
	{

		hNew = malloc(sizeof(HASH));
		hNew->mnemonic = malloc(sizeof(char) * 7);
		hNew->num = malloc(sizeof(char) * 5);
		strcpy(hNew->mnemonic, mnemonic);
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

/*해시함수*/
int hash_function(char* str)
{
	int idx = strlen(str);
	for (int i = 0; i < strlen(str); i++)
	{
		if (str[i] < 'A' || str[i] > 'Z')
			return -1;
		idx += (str[i] - 'A') * pow(3, i);
	}
	idx = idx % 20;
	return idx;
}

void f_hashtable()
{
	for (int i = 0; i < 20; i++)
	{
		HASH* temp = hash_table[i]->link, * temp2;
		while (temp != NULL)
		{
			temp2 = temp->link;

			free(temp->num);
			free(temp->mnemonic);
			free(temp);
			temp = temp2;
		}
	}
}

/*search hashtable*/
int s_hashtable(char* mnemonic)
{
	int flag = hash_function(mnemonic);
	if (flag == -1)
	{
		printf("opcode search error\n");
		return -1;
	}
	HASH* temp = hash_table[flag]->link;
	while (temp != NULL)
	{
		if (strcmp(mnemonic, temp->mnemonic) == 0)
		{
			printf("opcode is %X\n", temp->op);
			return 1;
		}
		temp = temp->link;
	}
	printf("opcode search error\n");
	return -1;
}

/*print hashtable*/
void p_hashtable()
{
	for (int i = 0; i < 20; i++)
	{
		HASH* temp = hash_table[i]->link;
		printf("%d : ", i);
		while (temp != NULL)
		{
			printf("[%s,%02X]", temp->mnemonic, temp->op);
			if (temp->link != NULL)
				printf(" -> ");
			temp = temp->link;
		}
		printf("\n");
	}
}

/*history*/

/*프로그램 시작시 history node를 init 하는 함수*/
void i_history()
{
	head = malloc(sizeof(HI_NODE));
	head->link = NULL;
	head->command = NULL;
	pPre = head;
	return;
}

//history를 store하는 함수
void s_history(char* cmd)
{

	pNew = malloc(sizeof(HI_NODE)); //명령어를 저장할 노드
	pNew->command = malloc(sizeof(char) * 100);
	strcpy(pNew->command, cmd);
	pNew->link = NULL;
	pPre->link = pNew;
	pPre = pPre->link;
}

//history 출력
void p_history()
{
	HI_NODE* temp = head->link;
	int idx = 1;
	while (temp != NULL)
	{
		printf("\t%d\t%s\n", idx, temp->command);
		temp = temp->link;
		idx++;
	}
}
//history node 동적할당 해제
void f_history()
{
	HI_NODE* temp = head, * temp2 = head;
	while (temp != NULL)
	{
		temp = temp->link;
		if (temp2->command)
			free(temp2->command);
		free(temp2);
		temp2 = temp;
	}
	return;
}


/*
	return: -1(error)
			0 (du[mp])//dump만 이 형태의 command가 허용됨
			1({command} start)
			2({command} start, end)
			3({command} start, end,value)
	command가 올바른 형태인지, 올바른 형태라면 start와 end ,(value)를 지정한다.


	comma: ','의 개수, num:숫자의 개수
	ans1: 짧은 command (du)
	ans2: 긴 command (dump)
*/
int check_command(char* command, char* ans1, char* ans2, int* start, int* end, int* value)
{
	int comma = 0, num = 0; //comma: ','의 개수, num:숫자의 개수
	int check_idx;			//확인할 index의 시작위치를 나타냄
	int idx_a, idx_b;		//어떠한 16진수(추정)가 문자열안에서 위치하는 시작/끝 index

	if ((strlen(command) == strlen(ans1) || strcmp(command, ans2) == 0) && command[0] == 'd')
	{ //dump만 start,end가 없어도 올바른 command이다.
		if (strlen(command) == strlen(ans1))
			is_l_s = 1;
		else
			is_l_s = 2;

		/*start 설정*/

		*start = last_address + 1; // dump()에서 마지막 출력이 fffff인 경우에 last_address를 -1으로 update했음

		/*end 설정*/
		if (*start + 159 > MEMORY_SIZE - 1)
		{
			*end = MEMORY_SIZE - 1;
		}
		else
		{
			*end = *start + 159; //160개 출력
		}
		return 0;
	}
	else if (strlen(command) == strlen(ans1) || strcmp(command, ans2) == 0)
		return -1; //잘못된 입력

	else if (strncmp(command, ans2, strlen(ans2)) == 0)
	{ //긴 command인 경우
		check_idx = strlen(ans2);
		is_l_s = 2;
	}
	else
	{ //짧은 command인 경우
		check_idx = strlen(ans1);
		is_l_s = 1;
	}

	if (command[check_idx] != ' ')
	{
		//command 뒤에 공백이 없을 경우 (반드시 있어야함)
		//혹은 edit대신 edxx처럼 긴 command가 잘못된 경우도 해당됨

		return -1;
	}

	/*{command}+' '까지는 확인 된 상태
	뒤의 숫자와 ','에 오류가 없는지 검사한다.
	*/

	idx_a = -1;
	idx_b = -1; //초기화

	for (int i = check_idx + 1; i < strlen(command); i++)
	{
		if (command[i] == ',')
		{ //comma

			if (command[i - 1] != ' ')
			{ //숫자(추정)가 끝난경우( 16진수가 맞는지 확인해야함)
				idx_b = i - 1;
				if (check_hex2(command, idx_a, idx_b, start, end, value, num) == -1)
					return -1;

				num++; //숫자 개수 증가
				idx_a = -1;
				idx_b = -1; //초기화
			}

			if (num - 1 != comma) //숫자 전에 컴마부터 있는 경우,comma가 AA,,BB처럼 중복되어 등장하는 경우
			{
				return -1;
			}
			comma++; //comma 개수 증가
		}
		else if (command[i] == ' ')
		{ //공백
			if (command[i - 1] == ' ' || command[i - 1] == ',')
				continue;

			else
			{ //16진수 숫자(추정)가 끝난 경우
				idx_b = i - 1;
				if (check_hex2(command, idx_a, idx_b, start, end, value, num) == -1)
					return -1;
				num++;
				idx_a = -1;
				idx_b = -1; //초기화
			}
		}

		else
		{ //그 외 (문자열의 마지막인 경우에도 검사해야함)

			if (idx_a == -1) //숫자가 기존에 시작하지 않은 경우
				idx_a = i;	 //숫자 시작점

			if (i == strlen(command) - 1)
			{
				idx_b = i;
				if (check_hex2(command, idx_a, idx_b, start, end, value, num) == -1)
					return -1;
				num++;
				idx_a = -1;
				idx_b = -1; //초기화
			}
		}
	} //end for

	if (num == 1)
	{ //dump에 start만 지정된 경우에는 end를 따로 구해야한다.
		/*end 설정*/
		if (*start + 159 > MEMORY_SIZE - 1)
		{
			*end = MEMORY_SIZE - 1;
		}
		else
		{
			*end = *start + 159;
		}
	}

	if (comma != num - 1)
		return -1;
	else if (command[0] == 'e' && (num != 2 || *end > 255)) //edit {},{}만 허용, end(=value)는 ff까지 허용
		return -1;
	else if (command[0] == 'f' && (num != 3 || *start > *end || *value > 255))
		return -1; //Start 주소가 end 주소보다 큰 값이 들어온 경우, 에러 처리.
	else if (command[0] == 'd' && *start > *end)
		return -1; //Start 주소가 end 주소보다 큰 값이 들어온 경우, 에러 처리.

	return num;
}
/*메모리에 저장되어있는 내용을 출력한다.
단순한 출력과 last_address update만을 담당하며,
end,start의 에러여부, 저장된 데이터의 에러 여부는 검사하지 않음
*/
void dump(int start, int end)
{
	for (int i = start / 16 * 16; i < (end / 16 + 1) * 16; i++)
	{
		if (i % 16 == 0)
			printf("%05X ", i); //주소 출력

		/*메모리 내용 출력*/
		if (i < start || i > end)
			printf("   ");
		else
			printf("%02X ", memory[i]);

		/*메모리 내용 출력*/
		if (i % 16 == 15)
		{
			printf("; ");
			for (int j = i - 15; j <= i; j++)
			{
				if (j < start || j > end || memory[j] < 32 || memory[j] > 126) //주어진 범위 넘는경우
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

/*	메모리의 저장된 값을 수정하는 함수*/
/*	address의 에러여부는 검사하지 않음(check_hex에서 미리 검사)

*/
void edit(int address, int value)
{

	memory[address] = value;
	return;
}

/*
	value로 start~end까지의 memory를 채움.
*/
void fill(int start, int end, int value)
{

	for (int i = start; i <= end; i++)
		memory[i] = value;
	return;
}
/*메모리의 모든 내용을 0으로 바꿈*/
void reset()
{
	fill(0, MEMORY_SIZE - 1, 0);
}
/* {len}자리의 16진수인지 확인
	return:
	>=0: hex를 10진수로 변환
	-1: not hex
*/
int check_hex(char* wrd, int len)
{
	int dec = 0; //10진수로 변환된 결과를 저장하는 변수
	if (strlen(wrd) != len)
		return -1;
	for (int i = 0; i < len; i++)
	{
		if (wrd[i] >= '0' && wrd[i] <= '9')
		{
			dec += (wrd[i] - '0') * pow(16, len - i - 1);
		}
		else if (wrd[i] >= 'a' && wrd[i] <= 'f')
		{
			dec += (wrd[i] - 'a' + 10) * pow(16, len - i - 1);
		}
		else if (wrd[i] >= 'A' && wrd[i] <= 'F')
		{
			dec += (wrd[i] - 'A' + 10) * pow(16, len - i - 1);
		}
		else
			return -1;
	}
	if (dec >= MEMORY_SIZE) //fffff까지만 사용됨
	{
		return -1;
	}

	return dec;
}

/*check_command()에서 숫자 확인할 때 사용*/
int check_hex2(char* command, int idx_a, int idx_b, int* start, int* end, int* value, int num)
{
	int num_a;
	char* ishex = malloc(sizeof(char) * (idx_b - idx_a + 2));
	ishex[idx_b - idx_a + 1] = '\0';
	strncpy(ishex, command + idx_a, idx_b - idx_a + 1);
	num_a = check_hex(ishex, strlen(ishex));
	if (num_a == -1) //16진수 수가 아님
	{
		return -1;
	}
	else
	{ //올바른 형식

		if (num == 0)
			*start = num_a;
		else if (num == 1)
			*end = num_a;
		else if (num == 2 && command[0] == 'f') //fill인경우만 숫자 3개
			*value = num_a;
		else
		{
			return -1; //숫자가 너무 많이 들어온 경우
		}
	}
	return 1;
}

/*command 를 정제해서 return*/
/*ex) {command} num[0], num[1] (len에 따라 달라짐)
	len: num의 개수 (len>=0)
*/
char* clear_command(char* command, int* num, int len)
{
	char* clear = malloc(sizeof(char) * 100);
	if (len == 0)
		sprintf(clear, "%s", command);
	else if (len == 1)
		sprintf(clear, "%s %X", command, num[0]);
	else if (len == 2)
		sprintf(clear, "%s %X, %X", command, num[0], num[1]);
	else //3(3보다 큰 경우는 없음)
		sprintf(clear, "%s %X, %X, %X", command, num[0], num[1], num[2]);
	return clear;
}

/*앞뒤 공백 제거*/
char* trim(char* command)
{
	char* trim_cmd;
	trim_cmd = malloc(sizeof(char) * 100);
	int front = 0, rear = strlen(command) - 1;


	while (command[front] == ' ' || command[front] == '\t')
	{
		front++;
	}
	while (command[rear] == ' ' || command[rear] == '\t')
	{
		rear--;
	}


	for (int i = front; i <= rear; i++)
	{

		if (command[i] == '\t')
			command[i] = ' '; //정제해서 저장하기 때문에 여기서 처리해도 함수 실행에 문제없다.
		trim_cmd[i - front] = command[i];
	}
	trim_cmd[rear - front + 1] = '\0';

	return trim_cmd;
}
