#include "20191571.h"

int A, X, L, PC, B, S, T; //출력되는 register
int SW;                   //출력은 안되지만 사용되는 register
int progaddr = 0;         //program_load address
int csaddr = 0;           //control section address
int run_start = 0;
int run_prog_len = 0;
int* bp = NULL;   //breakpoint를 저장하는 배열
int bp_idx = 0;   //breakpoint배열에 채워진 index수
ESTAB** estab, * eNew, * eTemp;

extern int memory[MEMORY_SIZE];

extern int last_address; //dump address
extern int run_idx;


//progaddr 을 변경하는 함수
//-1:error 1: success
//address는 trim()이 끝난 상태
int set_progaddr(char* address)
{
	int address_num = is_num(address, 16);
	if (address_num == -1 || address_num > MEMORY_SIZE - 1)
		return -1;
	progaddr = address_num;
	return 1;
}

//link 시 파일 개수를 알아내는 함수
int file_len(char* str)
{
	int blank = 0;

	for (int i = 0; i < strlen(str) - 1; i++)
	{ //str의 맨 앞/뒤 공백은 제거된 상태(trim 사용)
		if (str[i] == ' ' && str[i + 1] != ' ')
		{
			blank++;
		}
	}
	return blank + 1; //blank+1이 파일 개수
}

//- obj 파일에 에러가 존재할 경우 및 정상적으로 link, loader가 동작하지 않는 경우는 고려하지 않는다.
//return -1:error 1:success
int loader(char* str)
{

	int flag;
	int file_num = file_len(str);
	char* str1 = NULL, * str2 = NULL, * str3 = NULL;
	str1 = malloc(sizeof(char) * 20);
	str2 = malloc(sizeof(char) * 20);
	str3 = malloc(sizeof(char) * 20);
	if (file_num > 3) //파일 개수가 3개보다 많은경우(에러)
		return -1;
	csaddr = progaddr;


	if (file_num == 1)
	{
		sscanf(str, "%s", str1);

	}
	else if (file_num == 2)
	{
		sscanf(str, "%s %s", str1, str2);

	}
	else
	{ //3
		sscanf(str, "%s %s %s", str1, str2, str3);


	}



	if (strcmp(str1 + strlen(str1) - 4, ".obj") != 0) //확장자명이 obj가 아닌경우
		return -1;
	if (file_num >= 2 && strcmp(str2 + strlen(str2) - 4, ".obj") != 0) //확장자명이 obj가 아닌경우
		return -1;
	if (file_num == 3 && strcmp(str3 + strlen(str3) - 4, ".obj") != 0) //확장자명이 obj가 아닌경우
		return -1;


	//파일 확인 end
	init_estab();

	flag = load_pass1(str1);
	if (flag != -1 && file_num >= 2) {

		flag = load_pass1(str2);

	}
	if (flag != -1 && file_num == 3) {
		flag = load_pass1(str3);
	}


	if (flag == -1)
		return -1;

	//pass1 end

	print_estab(); //load map 출력

	flag = load_pass2(str1);

	if (flag != -1 && file_num >= 2) {
		flag = load_pass2(str2);

	}
	if (flag != -1 && file_num == 3) {
		flag = load_pass2(str3);

	}

	if (flag == -1)
		return -1;


	//pass2 end
	free(str1);
	free(str2);
	free(str3);

	//register 초기화
	A = 0, X = 0, L = run_prog_len, PC = progaddr, B = 0, S = 0, T = 0, SW = 0;



	free_estab();

	return 1;
}

//파일 없음: -1

int load_pass1(char* str)
{
	FILE* pFile = fopen(str, "r");

	char* tempstr;      //이름
	char* temp_address; //address(의 str형) 을 저장


	char buf[101];
	int cslth = 0;
	int flag, address;

	if (pFile == NULL) //파일 없음
		return -1;
	tempstr = malloc(sizeof(char) * 10);

	temp_address = malloc(sizeof(char) * 10);

	while (fgets(buf, sizeof(buf), pFile) != NULL)
	{
		if (buf[0] == 'H')
		{ //header record
			flag = 0;

			strncpy(tempstr, buf + 1, 6); //이름
			for (int i = 0; i < 6; i++)
			{
				if (tempstr[i] == ' ')
				{
					tempstr[i] = '\0';
					break;
				}
				if (i == 5)
				{
					tempstr[6] = '\0';
				}
			}
			address = get_num(buf + 7, 6);

			address = address + csaddr;
			csaddr = address; //update csaddr,csaddr+address

			cslth = get_num(buf + 13, 6);

			if (insert_estab(address, tempstr, flag, cslth) == -1)
			{
				free(tempstr);
				free(temp_address);
				fclose(pFile);

				return -1;
			}
		}
		else if (buf[0] == 'D')
		{

			for (int idx = 1; idx < strlen(buf) - 1;)
			{ //마지막은 \n이라서 strlen()-1까지
				flag = 1;
				strncpy(tempstr, buf + idx, 6);

				for (int i = 0; i < 6; i++)
				{
					if (tempstr[i] == ' ')
					{
						tempstr[i] = '\0';
						break;
					}
					if (i == 5)
					{
						tempstr[6] = '\0';
					}
				}
				idx += 6;

				address = get_num(buf + idx, 6);

				address = csaddr + address;
				idx += 6;
				if (insert_estab(address, tempstr, flag, cslth) == -1)
				{
					free(tempstr);
					fclose(pFile);
					free(temp_address);

					return -1;
				}
			}
		}
		else if (buf[0] == 'E')
		{
			csaddr += cslth; //next control section 의 시작주소..
		}
	}
	free(tempstr);
	free(temp_address);
	fclose(pFile);

	return 1;
}

void init_estab()
{
	estab = malloc(sizeof(ESTAB*) * 17);
	for (int i = 0; i < 17; i++)
	{
		estab[i] = malloc(sizeof(ESTAB));
		estab[i]->link = NULL;
		estab[i]->name = NULL;
	}
}

/*return -1: error 1:success
 주소와 이름 flag을 parameter로 전달받아 estab에 저장.
 이미 존재하는 경우 -1을 return.
*/
int insert_estab(int address, char* name, int flag, int length)
{
	int idx = hash_function2(name);

	eTemp = estab[idx];

	if (eTemp->link != NULL)
	{
		eTemp = eTemp->link;
		while (eTemp->link != NULL)
		{
			if (strcmp(name, eTemp->name) == 0)
			{ //기존에 존재하면
				return -1;
			}
			eTemp = eTemp->link;
		}
	}

	eNew = malloc(sizeof(ESTAB));
	eTemp->link = eNew;
	eNew->flag = flag;
	eNew->name = malloc(sizeof(char) * 8);
	strcpy(eNew->name, name);
	eNew->address = address;
	eNew->length = length; // 다 저장하지만 control section만 사용됨

	eNew->link = NULL;

	return 1;
}
/* load map for debugging*/
void print_estab()
{
	int total = 0;
	printf("control symbol address length\n");
	printf("section name\n");
	printf("------------------------------\n");
	for (int i = 0; i < 17; i++)
	{

		eTemp = estab[i]->link;
		while (eTemp != NULL)
		{
			if (eTemp->flag == 0)
			{
				printf("%-6s          %04X  %04X\n", eTemp->name, eTemp->address, eTemp->length);
				total += eTemp->length;
			}
			else
			{
				printf("        %-6s  %04X\n", eTemp->name, eTemp->address);
			}
			eTemp = eTemp->link;
		}
	}
	printf("------------------------------\n");
	printf("\t  total length %04X\n", total);
	run_prog_len = total;
	return;
}

//estab에서 해당 symbol을 찾는 함수
//return ; 찾으면 해당 symbol의 address, error -1
int search_estab(char* str)
{
	int idx = hash_function2(str);

	eTemp = estab[idx];

	if (eTemp->link != NULL)
	{
		eTemp = eTemp->link;

		while (eTemp != NULL)
		{

			if (strcmp(str, eTemp->name) == 0)
			{ //기존에 존재하면
				return eTemp->address;
			}
			eTemp = eTemp->link;
		}
	}
	return -1;
}

void free_estab()
{
	ESTAB* eTemp2;
	for (int i = 0; i < 17; i++)
	{
		eTemp2 = estab[i]->link;
		eTemp = estab[i]->link;
		free(estab[i]);
		while (eTemp != NULL) {
			eTemp = eTemp->link;
			if (eTemp2->name != NULL)
				free(eTemp2->name);
			free(eTemp2);
			eTemp2 = eTemp;
		}
	}

	free(estab);
}

/*performs actual loading.relocation, linking of the program*/
/*return ; -1: error
	else: return address
*/
int load_pass2(char* str)
{

	FILE* pFile = fopen(str, "r"); //파일 열기
	if (pFile == NULL) {
		printf("pass2 file open error\n");
		return -1;
	}
	char* tempstr; //버퍼에서 일부분을 읽을때 사용

	char buf[100]; //버퍼 저장
	int ref[100] = {
		0,
	}; //reference

	int start, num, address, modif_len, modif_num; //start: 시작주소 num:문자열에서 변환된 숫자 address: 주소 modif_len: 수정할 길이(half byte)
	//modif_num: 변경할 숫자
	int temp_num;

	int execaddr = progaddr;

	tempstr = malloc(sizeof(char) * 20);

	fgets(buf, sizeof(buf), pFile); //첫줄 읽기 (header record)
	strncpy(tempstr, buf + 1, 6);   //이름
	for (int i = 0; i < 6; i++)
	{
		if (tempstr[i] == ' ')
		{
			tempstr[i] = '\0';
			break;
		}
		if (i == 5)
		{
			tempstr[6] = '\0';
		}
	}
	csaddr = search_estab(tempstr);
	ref[1] = csaddr; //control section은 01

	strncpy(tempstr, buf + 13, 6);
	tempstr[6] = '\0';

	while (fgets(buf, sizeof(buf), pFile) != NULL)
	{ //한줄씩 읽기

		if (buf[0] == 'T')
		{ //text record
			start = get_num(buf + 1, 6);

			start += csaddr;

			for (int i = 9; i < strlen(buf) - 1; i += 2)
			{
				num = get_num(buf + i, 2);

				edit(start, num);
				start++;
			}
		}

		else if (buf[0] == 'M')
		{ //modification record

			num = get_num(buf + 10, 2);
			address = ref[num]; //reference 주소
			if (buf[9] == '-')  //- 인 경우
				address *= -1;
			//address 만큼 변경한다.

			num = get_num(buf + 1, 6); //starting location
			num += csaddr;

			modif_len = get_num(buf + 7, 2); //half byte
			modif_num = 0;                   //초기화
			temp_num = 1;

			//기존 값 구하기
			for (int i = (modif_len + 1) / 2 - 1; i >= 0; i--)
			{

				modif_num += memory[num + i] * temp_num;
				temp_num *= 16 * 16;
			}

			if (modif_len % 2 == 0 && memory[num] >= 0x80)
			{ //음수인 경우(len:6)

				modif_num = (0xffffff - modif_num + 1) * -1; //음수로 바꾸기
			}
			else if (modif_len % 2 == 1 && memory[num] % 16 >= 0x8)
			{ //len:5

				temp_num /= 16;
				modif_num -= (memory[num] / 16) * temp_num; //맨 앞글자를 뺀 숫자를 저장
				temp_num = memory[num] / 16;                //맨 앞글자(숫자)를 저장
				modif_num = (0xfffff - modif_num + 1) * -1; //음수로 바꾸기
			}

			modif_num += address; //변경되는 주소 넣기

			sprintf(tempstr, "%08X", modif_num); //문자열로 변환(음수일때도 있기 때문)

			//해당하는 길이만큼 자름
			strncpy(tempstr, tempstr + strlen(tempstr) - (modif_len + 1) / 2 * 2, (modif_len + 1) / 2 * 2);
			tempstr[(modif_len + 1) / 2 * 2] = '\0';

			if (modif_len % 2 == 1 && memory[num] % 16 == 0xf)
			{
				if (temp_num < 10)
					tempstr[0] = temp_num + '0';
				else
					tempstr[0] = temp_num - 10 + 'A';
			}

			//다시 넣기
			for (int i = 0; i < (modif_len + 1) / 2; i++)
			{
				temp_num = get_num(tempstr + i * 2, 2);

				edit(num + i, temp_num);
			}
		}
		else if (buf[0] == 'E')
		{ //end record
			if (strlen(buf) > 2)
			{
				temp_num = get_num(buf + 1, 6);
				execaddr = csaddr + temp_num;
				run_start = execaddr;
			}

			break;
		}
		else if (buf[0] == 'R')
		{ //reference record

			for (int i = 1; i < strlen(buf) - 1; i += 8)
			{ //8:2(길이)+6(이름)
				num = get_num(buf + i, 2);

				strncpy(tempstr, buf + i + 2, 6); //symbol
				for (int j = 0; j < 6; j++)
				{
					if (tempstr[j] == ' ' || tempstr[j] == '\n')
					{
						tempstr[j] = '\0';
						break;
					}
					if (j == 5)
						tempstr[6] = '\0';
				}

				address = search_estab(tempstr);

				if (address == -1)
				{

					free(tempstr);
					fclose(pFile);



					return -1;
				}
				ref[num] = address;
			}
		}
	}
	fclose(pFile);
	free(tempstr);

	return run_start;
}

int get_num(char* buf, int len)
{
	char tempstr[10];

	strncpy(tempstr, buf, len);

	for (int i = 0; i < len; i++)
	{
		if (tempstr[i] == ' ' || tempstr[i] == '\n')
		{
			tempstr[i] = '\0';
			break;
		}
	}
	tempstr[len] = '\0';
	return is_num(tempstr, 16);
}

//- 입력되는 bp의 범위는 메모리에 올라가는 프로그램 길이와 동일하다고 가정한다.(범위 체크를 따로 하지 않음)
void set_bp(char* str)
{ //str은 trim()된 상태
	if (bp == NULL)
		bp = malloc(sizeof(int) * run_prog_len);

	int num = is_num(str, 16);
	bp[bp_idx] = num + progaddr;
	bp_idx++;
	printf("\t\t[\033[32mok\033[0m] create breakpoint %s\n", str);
}

void clear_bp()
{
	free(bp);
	bp_idx = 0;
	bp = NULL;
	printf("\t\t[\033[32mok\033[0m] clear all breakpoints\n");
}
void print_bp()
{
	printf("\t\tbreakpoint\n");
	printf("\t\t----------\n");

	for (int i = 0; i < bp_idx; i++)
	{
		printf("\t\t%X\n", bp[i]);
	}
}

//pc,base를 고려하여 address(disp)의 원래 값을 알아내는 함수
//n.i는 다른 함수에서 고려
int get_address2(int num2, int num3, int num4, int n, int i, int b, int p, int e)
{

	int address = -1; //return 값

	if (p == 1 && b == 0 && e == 0 && num2 % 16 < 8)
	{                                                  //pc이고 양수
		address = PC + num2 % 16 * 16 * 16 + num3 + 3; //다음 loc가 PC이므로 3을 더해주어야한다. 이때의 pc는 현재의 loc
	}
	else if (p == 1 && b == 0 && e == 0 && num3 % 16 >= 8)
	{                                         //pc, 음수
		address = num2 % 16 * 16 * 16 + num3; //양수로 가정했을때의 수
		address = (0xfff - address + 1) * -1; //음수로 바꾸기

		address += PC + 3;
	}
	else if (p == 0 && b == 1 && e == 0)
	{ //base
		address = B + num2 % 16 * 16 * 16 + num3;
	}
	else if (p == 0 && b == 0 && e == 1)
	{ //format4
		address = num3 * 16 * 16 + num2 % 16 * 16 * 16 * 16 * 16 + num4;
	}

	if (address == -1 && n == 0 && i == 1)
	{ //immediate인 경우
		address = num2 % 16 * 16 * 16 + num3;
	}

	return address;
	//copy.obj는 위의 3가지 경우만 고려됨
	//위의 경우가 아닐때는 byte 이다.
}

void find_register(int* reg1, int* reg2)
{
	if (*reg1 == 0)
		*reg1 = A;
	else if (*reg1 == 1)
		*reg1 = X;
	else if (*reg1 == 2)
		*reg1 = L;
	else if (*reg1 == 3)
		*reg1 = B;
	else if (*reg1 == 4)
	{
		*reg1 = S;
	}
	else if (*reg1 == 5)
	{
		*reg1 = T;
	}
	else if (*reg1 == 8)
	{
		*reg1 = PC;
	}

	if (*reg2 == 0)
		*reg2 = A;
	else if (*reg2 == 1)
		*reg2 = X;
	else if (*reg2 == 2)
		*reg2 = L;
	else if (*reg2 == 3)
		*reg2 = B;
	else if (*reg2 == 4)
	{
		*reg2 = S;
	}
	else if (*reg2 == 5)
	{
		*reg2 = T;
	}
	else if (*reg2 == 8)
	{
		*reg2 = PC;
	}
}
void clear_register(int reg)
{
	if (reg == 0)
		A = 0;
	else if (reg == 1)
		X = 0;
	else if (reg == 2)
		L = 0;
	else if (reg == 3)
		B = 0;
	else if (reg == 4)
	{
		S = 0;
	}
	else if (reg == 5)
	{
		T = 0;
	}
	else if (reg == 8)
	{
		PC = 0;
	}
}
//load,값 비교  할때 사용
int update_num(int num, int n, int i)
{

	if (n == 1 && i == 1)
	{ //simple
		num = memory[num];
	}
	else if (n == 1 && i == 0)
	{ //indirect,word 일때 가정
		num = memory[num] * 16 * 16 * 16 * 16 + memory[num + 1] * 16 * 16 + memory[num + 2];

		//이 값이 pc값으로 사용
	}
	else if (n == 0 && i == 1)
	{ //immediate
		num = num;
	}
	return num;
}
//store 사용
int update_num2(int num, int n, int i)
{
	if (n == 1 && i == 1)
	{ //simple
		num = num;
	}
	else if (n == 1 && i == 0)
	{ //indirect
		num = memory[num];
	}

	return num;
}
//store, n,i를 고려하여 value를 저장
void store_num(int num, int value, int n, int i)
{

	edit(update_num2(num, n, i), value);
	return;
}
//opcode 를 확인하여 관련 작업을 수행하는 함수 (copy.obj에서 사용되는 opcode)
//copy.obj는 항상 0부터 시작하기 때문에 csaddr 체크 필요 x
int check_op(int num1, int num2, int num3, int num4, int* idx)
{
	int op = num1 / 4 * 4;
	int num, temp_num;
	int n, i, x, b, p, e;
	int reg1, reg2;
	n = num1 % 4 / 2;
	i = num1 % 2;
	x = num2 / 16 / 8;
	b = num2 / 16 % 8 / 4;
	p = num2 / 16 % 4 / 2;
	e = num2 / 16 % 2;

	num = get_address2(num2, num3, num4, n, i, b, p, e);
	if (op != 0xB4 && op != 0xA0 && op != 0xB8 && op != 0x4C && !(i == 1 && n == 0) && num == -1) //word,resw/b,2형식 아님,immediate아님
		return -1;                                                                                //index 증가 없음, register 변화 없음

	if (x)
		num += X;

	switch (op)
	{
	case 0x14: //STL(3/4)

		store_num(num, L / 0x10000, n, i);
		store_num(num + 1, L % 0x10000 / 0x100, n, i);
		store_num(num + 2, L % 0x100, n, i);

		if (e)
			PC += 4;
		else
			PC += 3;

		break;
	case 0x0C: //STA(3/4)

		store_num(num, A / 0x10000, n, i);
		store_num(num + 1, A % 0x10000 / 0x100, n, i);
		store_num(num + 2, A % 0x100, n, i);

		if (e)
			PC += 4;
		else
			PC += 3;
		break;
	case 0x10: //STX(3/4)

		store_num(num, X / 0x10000, n, i);
		store_num(num + 1, X % 0x10000 / 0x100, n, i);
		store_num(num + 2, X % 0x100, n, i);

		if (e)
			PC += 4;
		else
			PC += 3;
		break;
	case 0x54: //STCH(3/4)
		store_num(num, A % 0x100, n, i);
		if (e)
			PC += 4;
		else
			PC += 3;
		break;
	case 0x00: //LDA(3/4)
		if (n == 0 && i == 1)
		{
			A = num;
		}
		else
		{ //word

			A = update_num(num, n, i) * 16 * 16 * 16 * 16;
			A += update_num(num + 1, n, i) * 16 * 16;
			A += update_num(num + 2, n, i);
		}

		if (e)
			PC += 4;
		else
			PC += 3;
		break;
	case 0x68: //LDB(3/4)
		if (n == 0 && i == 1)
		{ //immediate
			B = num;
		}
		else
		{
			B = update_num(num, n, i) * 16 * 16 * 16 * 16;
			B += update_num(num + 1, n, i) * 16 * 16;
			B += update_num(num + 2, n, i);
		}
		if (e)
			PC += 4;
		else
			PC += 3;
		break;
	case 0x74: //LDT(3/4)
		if (n == 0 && i == 1)
		{
			T = num;
		}
		else
		{

			T = update_num(num, n, i) * 16 * 16 * 16 * 16;
			T += update_num(num + 1, n, i) * 16 * 16;
			T += update_num(num + 2, n, i);
		}
		if (e)
			PC += 4;
		else
			PC += 3;
		break;
	case 0x50: //LDCH(3/4)
		A = update_num(num, n, i) % 0x100;
		if (e)
			PC += 4;
		else
			PC += 3;
		break;
	case 0x4C: //RSUB(3/4)

		PC = L;
		break;
	case 0x48: //JSUB(3/4)
		if (e)
			L = PC + 4; //다음 loc저장
		else
			L = PC + 3;
		PC = num;
		break;
	case 0x30: //JEQ(3/4)
		if (SW == 0)
			PC = num;
		else
		{
			if (e)
				PC += 4;
			else
				PC += 3;
		}
		break;
	case 0x38: //JLT(3/4)
		if (SW < 0)
			PC = num;
		else
		{
			if (e)
				PC += 4;
			else
				PC += 3;
		}
		break;
	case 0x3C: //J(3/4)
		PC = update_num(num, n, i);
		break;
	case 0x28: //COMP(3/4)
		if (n == 0 && i == 1)
		{
			temp_num = num;
		}
		else
		{
			temp_num = update_num(num, n, i) * 16 * 16 * 16 * 16;
			temp_num += update_num(num + 1, n, i) * 16 * 16;
			temp_num += update_num(num + 2, n, i);
		}
		SW = temp_num - A;
		if (e)
			PC += 4;
		else
			PC += 3;
		break;
	case 0xA0: //COMPR(2)
		reg1 = num2 / 16;
		reg2 = num2 % 16;

		find_register(&reg1, &reg2);

		SW = reg1 - reg2;
		PC += 2;
		break;

	case 0xB4: //CLEAR(2)
		reg1 = num2 / 16;

		clear_register(reg1);
		PC += 2;
		break;
	case 0xE0: //TD(3/4)
		if (e)
			PC += 4;
		else
			PC += 3;
		SW = -1; //CC : <
		break;
	case 0xDB: //RD(3/4)
		A = S; //다음 명령어에서 A==S여야한다.
		break;
	case 0xDC: //WD(3/4)
		if (e)
			PC += 4;
		else
			PC += 3;
		break;
	case 0xB8: //TIXR(2)
		X++;
		reg1 = num2 / 16;
		reg2 = 0; //임시로 설정(parameter로 사용)
		find_register(&reg1, &reg2);
		SW = X - reg1;

		PC += 2;
		break;

	default:
		return -1;
		break;
	}

	return 1;
}
//pc가 breakpoint와 일치하는지 확인
int check_bp()
{
	for (int i = 0; i < bp_idx; i++)
	{
		if (PC == bp[i])
			return i;
	}
	return -1;
}
//run을 수행하는 함수,
//copy.obj 에서 작동
int run(int run_bp)
{

	int idx = progaddr;

	int num1, num2, num3, num4; //extended까지 있기 때문에 4개
	int flag;
	while (memory[PC] == 0)
		PC++;//시작할 때 위치조정
	//idx는 memory값이 0이 아닌 index

	while (1)
	{

		num1 = memory[PC], num2 = memory[PC + 1], num3 = memory[PC + 2], num4 = memory[PC + 3];

		flag = check_op(num1, num2, num3, num4, &idx);
		if (flag == -1)
		{
			PC += 3;
		}

		flag = check_bp();
		if (bp_idx != 0 && flag != -1) //breakpoint가 있을 때,  다음 주소가 breakpoint에 위치한 상태
		{

			progaddr = PC;
			break;
		}
		if (PC == run_prog_len)
		{ //csaddr고려 x
			break;
		}
	}

	printf("A : %06X  X : %06X\n", A, X);
	printf("L : %06X PC : %06X\n", L, PC);
	printf("B : %06X  S : %06X\n", B, S);
	printf("T : %06X\n", T);
	if (bp_idx == 0 || bp_idx == run_bp)
	{ //마지막까지 돌린 상태
		printf("\tEnd Program\n");
		run_idx = -1;//함수 종료후에 main()에서 +1 하기 때문
	}
	else
		printf("\tStop at checkpoint[%X]\n", bp[flag]);

	if (bp_idx != 0)
		run_bp++;

	return 1;
}