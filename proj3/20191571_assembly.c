#include "20191571.h"


int LOCCTR = 0;//location counter


char* base_symbol = NULL;
SORT_SYMTAB* ssym = NULL;
int ssym_len = 0;


SYMTAB** tempsym = NULL;//hash table형태로 사용
//tempsym으로 만들다가 정상적으로 다 만들어지면 정렬한 새로운 자료구조를 만들고 tempsym의 할당을 해제

LST* lstf[linesize];
TEMPFILE* tempf[linesize];//배열로 사용
int prog_len = 0;
int prog_len_idx = 0;
/*hash_table*/

//-1: 파일 없음(main에서 에러 메시지 출력)
//1: 파일 내용 출력
int type(char* filename) {
	//fopen(),fclose()사용 (library call function)
	FILE* pFile = fopen(filename, "r"); //read mode
	char temp;
	if (pFile == NULL)
		return -1;
	while (1) {
		temp = fgetc(pFile);
		if (temp == EOF)
			break;
		printf("%c", temp);
	}

	fclose(pFile);
	return 1;
}

FILE* open_file(char* str) {
	FILE* f = fopen(str, "r");
	if (f == NULL)
		return NULL;
	else
		return f;
}

/*return -1:error*/
int pass1(FILE* pFile, HASH** hash_table, char* filename) {
	init_symtab();
	char buf[101];//한줄에 올 수 있는 단어
	int start_address = 0;

	char* a, * b, * c;
	int num = 0;
	int idx = 0;//tempf의 index
	base_symbol = NULL;//초기화
	a = malloc(sizeof(char) * 31);
	b = malloc(sizeof(char) * 31);
	c = malloc(sizeof(char) * 31);

	fgets(buf, sizeof(buf), pFile);// 첫줄 읽기
	get_line(buf, &num, a, b, c); // 읽기

	if (num == 3 && strcmp(b, "START") == 0) {

		int temp = is_num(c, 16);
		if (temp == -1) {
			free_symtab();
			printf("error: line 5\n");
			free(a);
			free(b);
			free(c);
			fclose(pFile);
			return -1;
		}
		else {
			tempf[0] = malloc(sizeof(TEMPFILE));
			start_address = temp;
			LOCCTR = start_address;

			tempf[0]->loc = LOCCTR;
			tempf[0]->strnum = 3;
			tempf[0]->str1 = malloc(sizeof(char) * 31);
			tempf[0]->str2 = malloc(sizeof(char) * 31);
			tempf[0]->str3 = malloc(sizeof(char) * 31);
			strcpy(tempf[0]->fullstr, buf);
			tempf[0]->fullstr[strlen(buf) - 1] = '\0';
			strcpy(tempf[0]->str1, a);
			strcpy(tempf[0]->str2, b);
			strcpy(tempf[0]->str3, c);
		}
	}
	else {//start가 없음
		LOCCTR = 0;//0으로 초기화

		if (get_address(0, num, a, b, c, buf, hash_table) == -1) {//여기서 동적할당 함
			printf("error: line 5\n");
			free_symtab();
			free(tempf[0]);
			fclose(pFile);
			free(a);
			free(b);
			free(c);
			return -1;
		}

		tempf[0]->loc = LOCCTR;
	}

	idx++;

	while (fgets(buf, sizeof(buf), pFile) != NULL) {


		if (buf[0] == '.') {//comment -num:1,str1에 전부 저장
			tempf[idx] = malloc(sizeof(TEMPFILE));
			tempf[idx]->strnum = 1;
			tempf[idx]->loc = LOCCTR;
			tempf[idx]->str1 = malloc(sizeof(char) * 101);
			strcpy(tempf[idx]->str1, buf);
			tempf[idx]->str1[strlen(tempf[idx]->str1) - 1] = '\0';

		}
		else {
			get_line(buf, &num, a, b, c); // 읽기

			if (get_address(idx, num, a, b, c, buf, hash_table) == -1) {
				printf("error: line %d\n", (idx + 1) * 5);
				free_symtab();
				prog_len_idx = idx + 1;
				fclose(pFile);
				free_tempf();
				free(a);
				free(b);
				free(c);
				return -1;
			}
			if (strcmp(a, "END") == 0) {
				tempf[idx] = malloc(sizeof(TEMPFILE));
				tempf[idx]->strnum = 1;
				tempf[idx]->loc = LOCCTR;
				tempf[idx]->str1 = malloc(sizeof(char) * 101);
				strcpy(tempf[idx]->str1, buf);
				tempf[idx]->str1[strlen(tempf[idx]->str1) - 1] = '\0';
				strcpy(tempf[idx]->fullstr, buf);
				tempf[idx]->fullstr[strlen(buf) - 1] = '\0';
				break;
			}
		}
		strcpy(tempf[idx]->fullstr, buf);
		tempf[idx]->fullstr[strlen(buf) - 1] = '\0';

		idx++;
	}
	fclose(pFile);
	/* 동적할당 해제 */
	free(a);
	free(b);
	free(c);
	prog_len = LOCCTR - start_address;
	prog_len_idx = idx + 1;


	if (pass2(hash_table, filename, start_address) == -1)//에러메시지는 pass2에서 출력
	{
		return -1;
	}
	return 1;
}


int pass2(HASH** hash_table, char* filename, int start_address) {
	char* prog_name = NULL;
	init_lst();
	char* fname = malloc(sizeof(char) * 20);//obj
	char* fname2 = malloc(sizeof(char) * 20);//lst
	strncpy(fname, filename, strlen(filename) - 4);//.asm 포함되어있으므로 빼야함
	strcpy(fname + strlen(filename) - 4, ".obj");

	FILE* fObj;
	FILE* fLst;
	strncpy(fname2, filename, strlen(filename) - 4);//.asm 포함되어있으므로 빼야함(.asm인지 여부는 main()에서 판단)
	strcpy(fname2 + strlen(filename) - 4, ".lst");


	int start_idx = 0;//for loop를 시작할 idx

	int var_num;//변수의 object code 작성에서 필요한 정수 변수
	int start_address2 = start_address;//start_address의 값이 object code 작성 과정에서 변하는데 이떄 초기값을 저장할 변수 
	char* var_str;

	if (tempf[0]->strnum == 3 && strcmp(tempf[0]->str2, "START") == 0) {//start로 시작
		prog_name = malloc(sizeof(char) * 10);
		strcpy(prog_name, tempf[0]->str1);
		start_idx++;
		lstf[0]->comment_flag = 2;//loc 출력
		lstf[0]->loc = tempf[0]->loc;
		strcpy(lstf[0]->line, tempf[0]->fullstr);
	}

	for (int i = start_idx; i < prog_len_idx; i++) {

		strcpy(lstf[i]->line, tempf[i]->fullstr);
		lstf[i]->loc = tempf[i]->loc;
		if (tempf[i]->str1[0] == '.' || strcmp(tempf[i]->str1, "BASE") == 0 || i == prog_len_idx - 1) {//object code가 만들어지지 않는 경우1(마지막에는 end가 주어지기 때문에)
			lstf[i]->comment_flag = 1;//loc도 출력 안됨

		}
		else if (tempf[i]->strnum == 3 && (strcmp(tempf[i]->str2, "RESW") == 0 || strcmp(tempf[i]->str2, "RESB") == 0)) {//object code가 만들어지지 않는 경우1
			lstf[i]->comment_flag = 2;//loc는 출력됨
		}
		else if (tempf[i]->strnum == 3 && strcmp(tempf[i]->str2, "WORD") == 0) {
			lstf[i]->comment_flag = 0;
			if (tempf[i]->str3[0] != '-') {
				var_num = is_num(tempf[i]->str3, 10);
				if (var_num == -1)
				{
					pass2_close(fname2, fname, 1, i);
					return -1;
				}
			}
			else {
				var_num = is_num(tempf[i]->str3 + 1, 10);
				if (var_num == -1)
				{
					pass2_close(fname2, fname, 1, i);
					return -1;
				}
				var_num *= -1;
			}
			var_str = malloc(sizeof(char) * 10);
			sprintf(var_str, "%06X", var_num);
			strcpy(lstf[i]->obj, var_str + 2);
			free(var_str);
		}
		else if (tempf[i]->strnum == 3 && strcmp(tempf[i]->str2, "BYTE") == 0) {
			lstf[i]->comment_flag = 0;

			if (tempf[i]->str3[0] == 'C') {
				for (int j = 2; j < strlen(tempf[i]->str3) - 1; j++) {
					sprintf(lstf[i]->obj + (j - 2) * 2, "%02X", tempf[i]->str3[j]);

				}
			}
			else {//X(이때 숫자에 에러가 있는지 아닌지는 pass1때 검사함)
				var_str = malloc(sizeof(char) * 10);
				strncpy(var_str, tempf[i]->str3 + 2, strlen(tempf[i]->str3) - 3);
				var_str[strlen(tempf[i]->str3) - 3] = '\0';

				var_num = is_num(var_str, 16);
				if (var_num == -1)
				{
					pass2_close(fname2, fname, 1, i);
					return -1;
				}
				sprintf(lstf[i]->obj, "%02X", var_num);//음수 출력 확인
				free(var_str);
			}
		}

		else if (tempf[i]->strnum == 3 && tempf[i]->str2[0] == '+') {//+있으면 무조건 4형식
			lstf[i]->comment_flag = 4;//object 코드 작성때 M 때문에 별도의 flag로 분류 
			if (format4(i, hash_table) == -1) {

				pass2_close(fname2, fname, 1, i);
				return -1;
			}
		}
		else if (tempf[i]->strnum == 2 && tempf[i]->str1[0] == '+') {
			lstf[i]->comment_flag = 4;//주석아님 
			if (format4(i, hash_table) == -1) {
				pass2_close(fname2, fname, 1, i);
				return -1;
			}
		}
		else if (pc(i, hash_table) == 1) {//return값이 1이면 lst[]가 만들어진 상태
			lstf[i]->comment_flag = 0;//주석아님 
		}
		else if (base(i, hash_table) == 1) {
			lstf[i]->comment_flag = 0;//주석아님 
		}
		else {//4형식이면 (이번 과제에서는) 무조건 +표시 있어야함

			pass2_close(fname2, fname, 1, i);

			return -1;
		}


	}

	//object code end

	//lst file write start
	fObj = fopen(fname, "w");
	fLst = fopen(fname2, "w");
	if (fObj == NULL || fLst == NULL)
	{
		//error()
		printf("Object(listing) file open error");
		free_symtab();
		free_lst();
		free_tempf();
		return -1;
	}

	for (int i = 0; i < prog_len_idx; i++) {
		if (lstf[i]->comment_flag == 0 || lstf[i]->comment_flag == 4) {
			fprintf(fLst, "%d\t%04X\t%s", (i + 1) * 5, lstf[i]->loc, lstf[i]->line);
			for (int j = 0; j < 25 - strlen(lstf[i]->line); j++)
				fprintf(fLst, " ");
			fprintf(fLst, "\t%s\n", lstf[i]->obj);
		}
		else if (lstf[i]->comment_flag == 1) {//loc 출력 x
			fprintf(fLst, "%d\t    \t%s\n", (i + 1) * 5, lstf[i]->line);
		}
		else {//loc 출력
			fprintf(fLst, "%d\t%04X\t%s\n", (i + 1) * 5, lstf[i]->loc, lstf[i]->line);
		}
	}
	fclose(fLst);

	//object file
	if (prog_name == NULL) {
		fprintf(fObj, "H      %06X%06X\n", start_address, prog_len);

	}
	else
	{
		fprintf(fObj, "H%-6s%06X%06X\n", prog_name, start_address, prog_len);

	}

	int str_idx = 0;

	char* obj_str;
	obj_str = malloc(sizeof(char) * 61);

	for (int i = start_idx; i < prog_len_idx; i++) {
		if (lstf[i]->comment_flag == 0 || lstf[i]->comment_flag == 4) {
			strcpy(obj_str + str_idx, lstf[i]->obj);
			str_idx += strlen(lstf[i]->obj);
		}
		if (lstf[i]->comment_flag == 2) {//resw,resb
			if (i != prog_len_idx - 1 && lstf[i + 1]->comment_flag != 2) {
				write_obj(obj_str, str_idx, start_address, fObj);
				str_idx = 0;
				start_address = lstf[i + 1]->loc;

			}
			else if (i == prog_len_idx - 1) {
				write_obj(obj_str, str_idx, start_address, fObj);

			}


		}
		else {
			if (i == prog_len_idx - 1) {
				write_obj(obj_str, str_idx, start_address, fObj);

			}
			else if ((lstf[i + 1]->comment_flag == 0 || lstf[i + 1]->comment_flag == 4) && str_idx + strlen(lstf[i + 1]->obj) > 60) {//1E*2를 넘는 경우 
				write_obj(obj_str, str_idx, start_address, fObj);
				str_idx = 0;
				start_address = lstf[i + 1]->loc;
			}
		}
	}

	//T end

	str_idx = 0;
	for (int i = start_idx; i < prog_len_idx; i++) {

		if (lstf[i]->comment_flag == 4) {// 4형식일때만(sicxe) modified record에 기록되므로 5 half byte는 고정으로 출력
			//indirect나 immediate인 경우는 해당되지 않음
			if (tempf[i]->strnum == 2 && (tempf[i]->str2[0] != '#' && tempf[i]->str2[0] != '@'))
				fprintf(fObj, "M%06X05\n", str_idx + 1);//2를 더해야하는데 idx가 0부터 시작하므로 2-1=1을 더함
			else if (tempf[i]->strnum == 3 && (tempf[i]->str3[0] != '#' && tempf[i]->str3[0] != '@'))
				fprintf(fObj, "M%06X05\n", str_idx + 1);//2를 더해야하는데 idx가 0부터 시작하므로 2-1=1을 더함
		}
		if (lstf[i]->comment_flag == 4 || lstf[i]->comment_flag == 0)
			str_idx += strlen(lstf[i]->obj) / 2;

	}

	fprintf(fObj, "E%06X\n", start_address2);

	fclose(fObj);
	free(obj_str);

	sort_symtab();
	pass2_close(fname2, fname, 0, 0);
	free(fname2);
	free(fname);
	return 1;
}

void write_obj(char* str, int str_idx, int start_address, FILE* fObj) {
	str[str_idx] = '\0';
	fprintf(fObj, "T%06X%02X%s\n", start_address, str_idx / 2, str);
}


// 종료하기 전 파일들을 삭제하고 동적할당을 해제하는 함수
//flag: 1, error, 0 정상 종료
void  pass2_close(char* lst, char* obj, int error, int idx) {


	free_lst();
	free_tempf();
	if (error == 1) {
		printf("error: line %d\n", (idx + 1) * 5);
		free_symtab();
	}
	else {//정상종료
		printf("[%s], [%s]\n", lst, obj);
	}
}
/*assemble 작업에 필요한 listing 을 초기화*/
void init_lst() {

	for (int i = 0; i < prog_len_idx; i++) {
		lstf[i] = malloc(sizeof(LST));
		lstf[i]->line = malloc(sizeof(char) * 100);
		lstf[i]->obj = malloc(sizeof(char) * 10);
	}
}
void free_lst() {
	for (int i = 0; i < prog_len_idx; i++) {
		free(lstf[i]->line);
		free(lstf[i]->obj);
		free(lstf[i]);

	}

}

//임시파일 동적할당 해제
void free_tempf() {
	for (int i = 0; i < prog_len_idx; i++) {
		if (tempf[i]->strnum >= 1)
			free(tempf[i]->str1);
		if (tempf[i]->strnum >= 2)
			free(tempf[i]->str2);
		if (tempf[i]->strnum == 3)
			free(tempf[i]->str3);
		free(tempf[i]);
	}
}
//lstf[]->obj 에 내용을 복사하는 것 까지 마치고 성공하면 1, 실패(에러)하면 -1을 return
int pc(int idx, HASH** hash_table) {
	int num = tempf[idx]->strnum;
	HASH* tHash;
	int pc_idx = idx + 1;//pc index는 기존 idx의 다음이다.
	int pc_loc = 0;
	int address = -1;
	char* obj, * tStr;
	int opcode;
	int i = 1;
	int n = 1;
	int x = 0;
	int format_num;
	int sym_flag = 1;//숫자와 symbol을 구별하기 위한 flag, sym:1 num:0

	if (num == 1) {//simple로 처리
		tHash = s_hashtable2(tempf[idx]->str1, hash_table);
		if (tHash == NULL)
			return -1;
		format_num = tHash->num[0] - '0';
		obj = malloc(sizeof(char) * (format_num * 2 + 1));


		if (format_num == 3)
			sprintf(obj, "%02X", tHash->op + 3);//simple이라 3을 더함
		else
			sprintf(obj, "%02X", tHash->op);
		for (int i = 2; i < format_num * 2; i++)
			obj[i] = '0';
		obj[format_num * 2] = '\0';
		strcpy(lstf[idx]->obj, obj);
		free(obj);
		return 1;
	}


	//pc index 구하기(comment 면 건너뛰어야함.)
	for (; pc_idx < prog_len_idx; pc_idx++) {
		if (tempf[pc_idx]->loc > tempf[idx]->loc)
			break;
	}
	pc_loc = tempf[pc_idx]->loc;
	format_num = get_opcode(idx, hash_table, &n, &i, &x, &address, &opcode, pc_loc, &sym_flag);//작성해야할 object code가 몇형식인지
	if (format_num < 0)
		return -1;

	if (format_num == 3) {//format 2 나 1에서는 pc를 쓰지 않음
		address = address - pc_loc;

		if (address > 2047 || address < -2048)
			return -1;//signed 에서 disp가 주어진 범위를 넘는 경우
	}
	obj = malloc(sizeof(char) * (2 * (format_num)+1));

	if (format_num == 3)
		sprintf(obj, "%02X", opcode + n * 2 + i);
	else
		sprintf(obj, "%02X", opcode);


	if (format_num == 3) {//3형식이면 opcode가 6비트이다. 
		sprintf(obj + 2, "%01X", x * 8 + sym_flag * 2);

		if (address >= 0)
			sprintf(obj + 3, "%03X", address);
		else {
			tStr = malloc(sizeof(char) * 10);
			sprintf(tStr, "%X", address);

			strncpy(obj + 3, tStr + 5, 3);
			free(tStr);
		}
	}
	else if (format_num == 2) {
		sprintf(obj + 2, "%02X", address);
	}
	obj[format_num * 2] = '\0';
	//format_num이 1인경우는 opcode만 존재하므로 추가 작업을 하지 않아도 된다. 


	strcpy(lstf[idx]->obj, obj);
	free(obj);
	return 1;
}

int base(int idx, HASH** hash_table) {
	int i = 1, n = 1, x = 0, address = -1;
	char* obj, * tStr;
	int opcode;
	int sym_flag = 1;

	if (base_symbol == NULL)//base가 없는 상태
		return -1;
	int base_num = search_symtab(base_symbol);
	if (base_num == -1)
		return -1;

	int format_num = get_opcode(idx, hash_table, &n, &i, &x, &address, &opcode, base_num, &sym_flag);
	if (format_num < 0)
		return -1;


	if (format_num == 3) {

		address = address - base_num;

		if (address >= 4096 || address < 0)//unsigned
			return -1;
	}
	obj = malloc(sizeof(char) * (2 * (format_num)+1));
	if (format_num == 3)
		sprintf(obj, "%02X", opcode + n * 2 + i);
	else
		sprintf(obj, "%02X", opcode);


	if (format_num == 3) {//3형식이면 opcode가 6비트이다. 
		sprintf(obj + 2, "%01X", x * 8 + sym_flag * 4);//4:base

		if (address >= 0)
			sprintf(obj + 3, "%03X", address);
		else {
			tStr = malloc(sizeof(char) * 10);
			sprintf(tStr, "%X", address);

			strncpy(obj + 3, tStr + 5, 3);
			free(tStr);
		}
	}
	else if (format_num == 2) {
		sprintf(obj + 2, "%02X", address);
	}
	obj[format_num * 2] = '\0';
	//format_num이 1인경우는 opcode만 존재하므로 추가 작업을 하지 않아도 된다. 


	strcpy(lstf[idx]->obj, obj);
	free(obj);
	return 1;
}
int format4(int idx, HASH** hash_table) {
	//+ 있는 경우만 입력으로 들어옴
	//format4에서는 pc,base가 사용되지 않는다.
	int opcode, address;
	int num = tempf[idx]->strnum;
	char* obj;

	int n = 1;
	int i = 1;
	int x = 0;
	int temp2 = 0;//사용되지 않는 변수 ,parameter에 사용

	if (num == 3 && tempf[idx]->str2[0] == '+')
		strcpy(tempf[idx]->str2, tempf[idx]->str2 + 1);//+기호를 없앤다(출력할때는 fullstr사용)
	else///+기호가 있기 때문에 입력으로 들어온 것
		strcpy(tempf[idx]->str1, tempf[idx]->str1 + 1);//+기호를 없앤다(출력할때는 fullstr사용)
	int format_num = get_opcode(idx, hash_table, &n, &i, &x, &address, &opcode, 0, &temp2);
	if (format_num != 3)//'3/4'가 아닌 경우
		return -1;

	opcode += n * 2 + i;//n,i를 더한다
	obj = malloc(sizeof(char) * 9);//확장

	sprintf(obj, "%02X", opcode);
	obj[2] = '1' + x * 8;//x가 1이어도 9이다
	obj[3] = '0';

	sprintf(obj + 4, "%04X", address);//address를 obj의 뒤쪽에 작성, address는 양수
	obj[8] = '\0';

	strcpy(lstf[idx]->obj, obj);
	free(obj);
	return 1;
}
//알파벳순 정렬을 위해 별도의 struct 도입
void sort_symtab() {
	if (ssym != NULL)
		free_sort();//기존에 생성한 것이 있으면 free한다.
	int tempnum;
	char* tempstr = malloc(sizeof(char) * 31);
	int idx = 0;
	ssym = malloc(sizeof(SORT_SYMTAB) * 100);
	SYMTAB* sTemp;
	for (int i = 0; i < 17; i++) {
		sTemp = tempsym[i]->link;
		while (sTemp != NULL) {
			ssym[idx].num = sTemp->loc;
			ssym[idx].name = malloc(sizeof(char) * 10);
			strcpy(ssym[idx].name, sTemp->name);

			sTemp = sTemp->link;
			idx++;
		}
	}

	//O(n^2) sort
	for (int i = 0; i < idx - 1; i++) {
		for (int j = i + 1; j < idx; j++) {
			if (strcmp(ssym[i].name, ssym[j].name) > 0) {//바꿔야함

				strcpy(tempstr, ssym[i].name);
				strcpy(ssym[i].name, ssym[j].name);
				strcpy(ssym[j].name, tempstr);

				tempnum = ssym[i].num;
				ssym[i].num = ssym[j].num;
				ssym[j].num = tempnum;
			}
		}
	}

	ssym_len = idx;

	free(tempstr);
	free_symtab();
}
void free_sort() {
	for (int i = 0; i < ssym_len; i++) {
		free(ssym[i].name);
	}
	free(ssym);
	ssym = NULL;
	ssym_len = 0;
}
//print symbol
void p_symbol() {
	if (ssym_len == 0) {
		printf("symtab is NULL\n");
		return;//이때에도 history에 기록
	}
	for (int i = 0; i < ssym_len; i++) {
		printf("\t%s\t%04X\n", ssym[i].name, ssym[i].num);
	}
	//정상적으로 출력되기 위해 마지막 줄에 엔터가 있다
}

//opcode를 찾아서 format number를 return , -1 은 error
/*주석 및 objectcode 로 만들어지지 않는 부분은 입력으로 주어지지 않음*/
//address: symbol의 address가 필요한 경우 symbol의 address가 할당됨 
//pc_base: pc나 base의 주소값. #숫자일 경우 pc_base를 값에 더해준다(호출한 함수로 돌아갔을때 base(혹은 pc)값을 빼기 때문.
int get_opcode(int idx, HASH** hash_table, int* n, int* i, int* x, int* address, int* opcode, int pc_base, int* sym_flag) {
	int num = tempf[idx]->strnum;

	int obj_num;

	HASH* tHash;
	*address = 0;//우선 초기화
	if (num == 3) {
		tHash = s_hashtable2(tempf[idx]->str2, hash_table);//BASE나 word등은 pass2()에서 처리되어서 입력으로 주어지지 않음
		if (tHash != NULL) {
			*opcode = tHash->op;
			if (tempf[idx]->str3[0] == '#') {//immediate
				*address = is_num(tempf[idx]->str3 + 1, 10);// 숫자인지 확인
				if (*address == -1)
					*address = search_symtab(tempf[idx]->str3 + 1);//symbol에 있는지 확인
				else//숫자이면
				{
					*address += pc_base;
					*sym_flag = 0;
				}
				if (*address == -1)
					*address = is_register(tempf[idx]->str3 + 1, x);
				if (*address == -1)//둘다 아닌 경우
					return -1;

				*n = 0;//i만 1

			}
			else if (tempf[idx]->str3[0] == '@') {//indirect

				*address = search_symtab(tempf[idx]->str3 + 1);//symbol에 있는지 확인
				if (*address == -1)
					*address = is_register(tempf[idx]->str3 + 1, x);
				if (*address == -1)//아닌 경우
					return -1;
				*i = 0;
			}
			else {
				*address = search_symtab(tempf[idx]->str3);
				if (*address == -1)
					*address = is_register(tempf[idx]->str3, x);
				if (*address == -1)
					return -1;
			}
		}
		else
			return -1;

		obj_num = tHash->num[0] - '0';//1,2,3 중 하나

		return obj_num;
	}
	else if (num == 2) {
		tHash = s_hashtable2(tempf[idx]->str2, hash_table);
		if (tHash != NULL) {//str2가 opcode 인 경우 (str1은 symbol)
			*opcode = tHash->op;
			*address = search_symtab(tempf[idx]->str1);//str1이 symbol에 있는지 확인
			if (*address == -1)
				return -1;
			obj_num = tHash->num[0] - '0';//1,2,3 중 하나
			return obj_num;
		}
		else {//str1 이 opcode인 경우 
			tHash = s_hashtable2(tempf[idx]->str1, hash_table);
			if (tHash == NULL)
				return -1;

			*opcode = tHash->op;
			if (tempf[idx]->str2[0] == '#') {//immediate
				*address = is_num(tempf[idx]->str2 + 1, 10);// 숫자인지 확인
				if (*address == -1) {
					*address = search_symtab(tempf[idx]->str2 + 1);//symbol에 있는지 확인

				}
				else {
					*address += pc_base;

					*sym_flag = 0;
				}
				if (*address == -1)
					*address = is_register(tempf[idx]->str2 + 1, x);
				if (*address == -1)//둘다 아닌 경우
					return -1;
				*n = 0;
			}
			else if (tempf[idx]->str2[0] == '@') {//indirect

				*address = search_symtab(tempf[idx]->str2 + 1);//symbol에 있는지 확인
				if (*address == -1)
					*address = is_register(tempf[idx]->str2 + 1, x);
				if (*address == -1)//아닌 경우
					return -1;
				*i = 0;
			}
			else {
				*address = search_symtab(tempf[idx]->str2);
				if (*address == -1)
					*address = is_register(tempf[idx]->str2, x);
				if (*address == -1)
					return -1;
			}

			obj_num = tHash->num[0] - '0';//1,2,3 중 하나
			return obj_num;


		}
	}
	else {//num==1
		tHash = s_hashtable2(tempf[idx]->str1, hash_table);
		if (tHash == NULL)
			return -1;
		*opcode = tHash->op;
		obj_num = tHash->num[0] - '0';//1,2,3 중 하나
		return obj_num;
	}
}
//register 라면 해당하는 num 을 return, 아니면 -1 을 return 
//extended 라면 해당하는 주소을 return
int is_register(char* str, int* x) {
	char* temp, * temp2;
	int return_address;
	int re, re2;//return 값을 저장할 변수


	if (strcmp(str, "A") == 0)
		return 0 * 16;
	else if (strcmp(str, "X") == 0)
		return 1 * 16;
	else if (strcmp(str, "L") == 0)
		return 2 * 16;
	else if (strcmp(str, "B") == 0)
		return 3 * 16;
	else if (strcmp(str, "S") == 0)
		return 4 * 16;
	else if (strcmp(str, "T") == 0)
		return 5 * 16;
	else if (strcmp(str, "F") == 0)
		return 6 * 16;
	else if (strcmp(str, "PC") == 0)
		return 8 * 16;
	else if (strcmp(str, "SW") == 0)
		return 9 * 16;
	else if (strlen(str) > 4 && str[strlen(str) - 1] == 'X' && str[strlen(str) - 2] == ',')// ,X 인 경우 
	{
		temp = malloc(sizeof(char) * 31);
		strncpy(temp, str, strlen(str) - 2);
		temp[strlen(str) - 2] = '\0';

		return_address = search_symtab(temp);
		if (return_address == -1)
			return -1;
		*x = 1;

		free(temp);
		return return_address;//extended도 여기서 찾는다. 띄어쓰기는 주어진 .asm파일을 따른다
	}
	else if (strlen(str) >= 3 && str[1] == ',') {//두 레지스터 
		temp = malloc(sizeof(char) * 2);
		temp2 = malloc(sizeof(char) * 2);

		temp[0] = str[0], temp2[0] = str[2];
		temp[1] = '\0', temp2[1] = '\0';

		re = is_register(temp, x);
		re2 = is_register(temp2, x);
		free(temp);
		free(temp2);
		if (re != -1 && re2 != -1)
		{

			return (re * 16 + re2) / 16;//{re}{re2}
		}
		else
			return -1;
	}
	else
		return -1;
}

//-1 error 1 success
//기존 문자열과 분리된 문자열들을 받아서 symtab에 symbol을 넣고 temp file을 작성한다.
int get_address(int idx, int  num, char* a, char* b, char* c, char* buf, HASH** hash_table) {
	int len;
	HASH* hTemp;

	tempf[idx] = malloc(sizeof(TEMPFILE));
	tempf[idx]->strnum = num;
	if (num >= 1) {
		tempf[idx]->str1 = malloc(sizeof(char) * 31);
		strcpy(tempf[idx]->str1, a);

	}
	if (num >= 2) {
		tempf[idx]->str2 = malloc(sizeof(char) * 31);
		strcpy(tempf[idx]->str2, b);
	}
	if (num == 3) {//최대 3
		tempf[idx]->str3 = malloc(sizeof(char) * 31);
		strcpy(tempf[idx]->str3, c);
	}
	tempf[idx]->loc = LOCCTR;//현재의 loc 값이 해당 line 의 loc 값
	//동적할당 end

	//symbol이 있는 경우 symtab에 넣는다.
	if (buf[0] != ' ' && buf[0] != '\t')
	{
		if (insert_symtab(a, hash_table) == -1) {
			return -1;
		}

	}

	/*symbol 확인 및 생성
	loc update
	*/

	if (num == 3) {
		if (strcmp(b, "BYTE") == 0) {


			if (c[0] == 'C') {
				len = strlen(c) - 3;
				LOCCTR += len;
			}
			else {//X:hex
				len = strlen(c) - 3;
				LOCCTR += (len + 1) / 2;
			}

		}
		else if (strcmp(b, "WORD") == 0) {

			LOCCTR += 3;//3byte
		}
		else if (strcmp(b, "RESW") == 0) {

			len = is_num(c, 10);
			if (len == -1)
				return -1;
			LOCCTR += len * 3;
		}
		else if (strcmp(b, "RESB") == 0) {

			len = is_num(c, 10);
			if (len == -1)
				return -1;
			LOCCTR += len;
		}
		else if (b[0] == '+') {//extended
			hTemp = s_hashtable2(b + 1, hash_table);
			if (hTemp == NULL)
				return -1;
			LOCCTR += 4;
		}
		else {

			hTemp = s_hashtable2(b, hash_table);
			if (hTemp != NULL) {
				if (hTemp->num[0] == '1')
					len = 1;
				else if (hTemp->num[0] == '2')
					len = 2;
				else
					len = 3;
				LOCCTR += len;
			}
			else {

				return -1;//없는 opcode 사용, 이외의 directive는 num==3일때 사용되지 않음
			}
		}
	}
	else if (num == 2) {
		if (a[0] == '+') {
			hTemp = s_hashtable2(a + 1, hash_table);
			if (hTemp == NULL)
				return -1;
			LOCCTR += 4;
		}
		else {

			hTemp = s_hashtable2(a, hash_table);

			if (hTemp != NULL) {
				if (hTemp->num[0] == '1')
					len = 1;
				else if (hTemp->num[0] == '2')
					len = 2;
				else
					len = 3;

			}
			else if (strcmp("END", a) == 0 || strcmp("BASE", a) == 0) {//directives
				if (strcmp("BASE", a) == 0)
				{
					base_symbol = malloc(sizeof(char) * 31);
					strcpy(base_symbol, b);
				}
				len = 0;

			}
			else {
				return -1;
			}
			LOCCTR += len;
		}
	}
	else if (num == 1) {
		hTemp = s_hashtable2(a, hash_table);
		if (hTemp != NULL)
			len = hTemp->num[0] - '0';
		else if (strcmp("+RSUB", a) == 0)
			len = 4;
		else if (s_hashtable2(a + 1, hash_table) != NULL) {
			hTemp = s_hashtable2(a + 1, hash_table);
			if (hTemp != NULL && a[0] == '+' && hTemp->num[0] - '0' == 3)
				len = 4;
			else
				return -1;
		}
		else
			return -1;
		LOCCTR += len;

	}
	return 1;
}
/*search hashtable*/
//return : null(error) 
//else 해당하는 hash* 
HASH* s_hashtable2(char* mnemonic, HASH** hash_table) {

	int flag = hash_function(mnemonic);//hash_table을 위한 function
	if (flag == -1) {
		return NULL;
	}

	HASH* temp = hash_table[flag]->link;
	while (temp != NULL) {
		if (strcmp(mnemonic, temp->mnemonic) == 0) {//opcode  존재
			return temp;
		}
		temp = temp->link;
	}

	return NULL;
}


/*숫자로 추정되는 문자열을 입력받아 그 숫자를 변환해서 return
* error 일경우 -1  return
	flag: n진수인지 나타냄
	음이아닌 입력만 가정!
*/
int is_num(char* num, int flag) {
	int len = strlen(num);
	int dec = 0;
	for (int i = 0; i < len; i++)
	{
		if (num[i] >= '0' && num[i] <= '9') {
			dec += (num[i] - '0') * pow(flag, len - i - 1);
		}
		else if (num[i] >= 'a' && num[i] <= 'f') {
			if (flag == 10)//10진수에는 알파벳 입력이 안됨
				return -1;
			dec += (num[i] - 'a' + 10) * pow(flag, len - i - 1);
		}
		else if (num[i] >= 'A' && num[i] <= 'F') {
			if (flag == 10)//10진수에는 알파벳 입력이 안됨
				return -1;
			dec += (num[i] - 'A' + 10) * pow(flag, len - i - 1);
		}
		else
			return -1;
	}

	return dec;

}
int search_symtab(char* str) {
	if (str == NULL)
		return -1;
	int idx = hash_function2(str);
	if (idx == -1)
		return -1;
	SYMTAB* sTemp = tempsym[idx]->link;
	while (sTemp != NULL) {
		if (strcmp(sTemp->name, str) == 0) {
			return sTemp->loc;
		}
		sTemp = sTemp->link;
	}
	return -1;
}

//tempsym 초기화
void init_symtab() {
	tempsym = malloc(sizeof(SYMTAB*) * 17);
	for (int i = 0; i < 17; i++)
	{
		tempsym[i] = malloc(sizeof(SYMTAB));
		tempsym[i]->error = 0;
		tempsym[i]->link = NULL;
		tempsym[i]->name = NULL;
		
	}
	return;
}

//tempsym(혹은 다른 symtab)의 동적할당 해제

void free_symtab() {
	SYMTAB* sTemp2, * sTemp;
	for (int i = 0; i < 17; i++) {
		sTemp = tempsym[i]->link;
		sTemp2 = tempsym[i]->link;
		free(tempsym[i]);
		while (sTemp != NULL) {
			sTemp = sTemp->link;
			if (sTemp2->name != NULL)
				free(sTemp2->name);
			free(sTemp2);
			sTemp2 = sTemp;
		}
	}
	free(tempsym);
}

//hash function을 이용해서 symtab에 동일한 symbol이 있는지 찾고, 있다면 error flag를 set한 다음에 return 한다.
// 동일한 symbol이 없으면 linked list로 기존 symtab에 연결한다.
//return: -1(error) 1(success)
int insert_symtab(char* str, HASH** hash_table) {
	int idx = hash_function2(str);
	SYMTAB* sTemp = tempsym[idx];
	SYMTAB* sNew;

	if (s_hashtable2(str, hash_table) != NULL)
		return -1;
	if (sTemp->link != NULL) {
		sTemp = sTemp->link;
		while (sTemp->link != NULL) {
			if (strcmp(sTemp->name, str) == 0) {
				sTemp->error = 1;
				return -1;
			}
			sTemp = sTemp->link;
		}
	}
	sNew = malloc(sizeof(SYMTAB));
	sTemp->link = sNew;
	sNew->name = malloc(sizeof(char) * 31);
	strcpy(sNew->name, str);
	sNew->error = 0;
	sNew->loc = LOCCTR;
	sNew->link = NULL;

	return 1;
}
//symtab을 위한 hash function
int hash_function2(char* str) {
	int idx = strlen(str);
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] < 'A' || str[i]>'Z')
			return -1;
		idx += (str[i] - 'A') * pow(3, i);
	}
	idx = idx % 17;
	return idx;
}

//단순히 한 줄의 문자열을 분리해서 저장
//comment인 경우에는 입력으로 들어오지 않는다.

void get_line(char* buf, int* num, char* a, char* b, char* c) {

	*num = 0;
	char* d;

	for (int i = 0; i < strlen(buf) - 1; i++) {
		if ((buf[i] != ' ' && buf[i] != '\t') && (buf[i + 1] == ' ' || buf[i + 1] == '\t'))
			(*num)++;
	}
	if (buf[strlen(buf) - 1] != ' ' && buf[strlen(buf) - 1] != '\t')
		(*num)++;

	if (*num == 3) {
		sscanf(buf, "%s %s %s", a, b, c);
		//	printf("%s%d %s%d %s%d", a, strlen(a), b, strlen(b), c, strlen(c));
	}
	else if (*num == 2) {
		sscanf(buf, "%s %s", a, b);
		//	printf("%s%d %s%d ", a, strlen(a), b, strlen(b));
	}
	else if (*num == 1) {
		sscanf(buf, "%s", a);
		//	printf("%s%d  ", a, strlen(a));
	}
	else if (*num > 3) {
		d = malloc(sizeof(char) * 31);
		sscanf(buf, "%s %s %s %s", a, b, c,d);
		
		strcpy(c + strlen(c), d);
		*num = 3;
		free(d);
	}
	if (*num == 3 && b[strlen(b) - 1] == ',') {//컴마로 끝남, b와 c를 합치고 *num=2가 된다
		strcpy(b + strlen(b), c);
		*num = 2;
	}

}
