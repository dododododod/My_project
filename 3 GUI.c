#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <conio.h> // _getch 
#include <windows.h> // gotoxy, hideCursor 


typedef long unsigned int u32;
typedef  unsigned char u8;
#define push_enter 9999
//리아 블록 암호 선택

#if 1
#define _LEA128_
#else
#if 1
#define _LEA192_
#else
#define _LEA256_
#endif
#endif



#if defined(_LEA128_)
#define LEA_NUM_RNDS    24
#define LEA_KEY_BYTE_LEN  16
#endif

#if defined(_LEA192_)
#define LEA_NUM_RNDS    28
#define LEA_KEY_BYTE_LEN  24
#endif

#if defined(_LEA256_)
#define LEA_NUM_RNDS    32
#define LEA_KEY_BYTE_LEN  32
#endif

#define LEA_BLK_BYTE_LEN  16
#define LEA_RNDKEY_WORD_LEN 6

#if defined _MSC_VER
#define ROR(W,i) _lrotr(W, i)
#define ROL(W,i) _lrotl(W, i)
#else
#define ROR(W,i) (((W)>>(i)) | ((W)<<(32-(i))))
#define ROL(W,i) (((W)<<(i)) | ((W)>>(32-(i))))
#endif

#define u32_in(x)            (*(u32*)(x))
#define u32_out(x, v)        {*((u32*)(x)) = (v);}






//junfun
MYSQL mysql;
void gotoxy(int x, int y);

int push_banghangkey(int menucount);
void first_printmenu();
void ShowRndKeys(u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN]);
void resign();
void login();
void member_service();
void find_reservation();
void hideCursor();
void find_coupon();
void reservation();
void payment();
int reserve_menu_value;
void Show(u32 *src, int word_len);
int count_string(char *string);

int menuPos = 0;
void member_join();
int next_state=0;
int patien_id=NULL;
u32 rndkeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN] = {0x0,};

void LEA_Keyschedule(u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN],const u8 pbKey[LEA_KEY_BYTE_LEN]);

void LEA_EncryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN], const u8 pbSrc[LEA_BLK_BYTE_LEN],const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN]);

void LEA_DecryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN], const u8 pbSrc[LEA_BLK_BYTE_LEN], const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN]);


#define first_menu 0
#define memer_join_menu 1
#define resign_menu 2
#define login_menu 3
#define member_service_menu 4
#define reservation_menu 5
#define find_reservation_menu 6
#define find_coupon_menu 7
#define payment_menu 8


int main(void) {


	MYSQL_RES *res;
	char *host = "localhost";
	char *user = "root";
	char *pwd = "tiger1234";
	char *db = "db_hw2";
	MYSQL_ROW row;
	char temp[LEA_BLK_BYTE_LEN];
	u8 key[32] = {  
		0x0f, 0x1e, 0x2d, 0x3c, 0x4b, 0x5a, 0x69, 0x78, 0x87, 0x96, 0xa5, 0xb4, 0xc3, 0xd2, 0xe1, 0xf0,
		0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87, 0x78, 0x69, 0x5a, 0x4b, 0x3c, 0x2d, 0x1e, 0x0f 
	};

	LEA_Keyschedule(rndkeys, key);
	//system("chcp 949");
	mysql_init(&mysql);
	if(!mysql_real_connect(&mysql, host, user, pwd, db, 3306, NULL, 0)) 

		printf("Connection FAIL\n");
	else
		printf("Connection Success\n");


	mysql_query(&mysql,"SET character_set_results = 'euckr', character_set_client = 'euckr', character_set_connection = 'euckr', character_set_database = 'euckr', character_set_server = 'euckr' "); //한글깨짐현상 수정


	while(1){
		switch(next_state)
		{
		case first_menu:

			first_printmenu();
			break;
		case memer_join_menu:
			member_join();
			break;

		case resign_menu:
			resign();
			break;

		case login_menu:
			login();
			break;

		case member_service_menu:
			member_service();
			break;


		case reservation_menu:
			reservation();
			break;

		case find_reservation_menu:
			find_reservation();
			break;
		case find_coupon_menu:
			find_coupon();
			break;
		case payment_menu:
			payment();
			break;
		}
	}
	

	mysql_query(&mysql,
		"INSERT INTO payment(appointmentnumber,patient_id) values(1, 10);"); 



	//mysql_commit(&mysql);
	mysql_free_result(res); //결과값 변수 할당해제

	mysql_close(&mysql); 
	return 0;

}

void first_printmenu()
{
	const int menucount=4;
	system("cls");
	hideCursor();

	gotoxy(0, 0);
	puts("  1.회원가입");
	puts("  2.로그인");
	puts("  3.탈퇴");
	puts("  4.종료");
	gotoxy(0, menuPos);

	puts(">");
	if(push_enter==push_banghangkey(menucount))// next state
	{
		switch(menuPos)
		{
		case 0:
			system("cls");
			menuPos=0;
			next_state=memer_join_menu;

			break;
		case 1:
			system("cls");
			menuPos=0;
			next_state=login_menu;


			break;
		case 2:
			system("cls");
			menuPos=0;
			next_state=resign_menu;

			break;
		case 3:
			system("cls");
			menuPos=0;
			system("cls");
			printf("이용해 주셔서 감사합니다\n");

			exit(0);
			break;
		}


	}



}


void member_service()
{

	const int menucount=4;
	system("cls");
	hideCursor();

	gotoxy(0, 0);
	puts("  1.예약하기");
	puts("  2.예약조회");
	puts("  3.쿠폰조회");
	puts("  4.로그아웃");
	gotoxy(0, menuPos);

	puts(">");
	if(push_enter==push_banghangkey(menucount))// next state
	{
		switch(menuPos)
		{
		case 0:
			system("cls");
			menuPos=0;
			next_state=reservation_menu;
			break;

		case 1:
			system("cls");
			menuPos=0;
			next_state=find_reservation_menu;


			break;
		case 2:
			system("cls");
			menuPos=0;
			next_state=find_coupon_menu;

			break;

		case 3:
			system("cls");
			patien_id=NULL;
			next_state=first_menu;
			break;


		}


	}


}


void find_coupon()
{

	MYSQL_RES *res;
	char sjQuery[1024] = "\0";
	int result_curry;
	MYSQL_ROW row;
	system("cls");





	sprintf(sjQuery,"select c.couponnumber 쿠폰번호, c.coupontype 쿠폰타입, c.availableservice 적용가능서비스, c.discountrate 할인율, c.couponstate 사용가능여부 from coupon c where c.Patient_Id = %d",patien_id);
	mysql_query(&mysql, sjQuery);
	res = mysql_use_result(&mysql); 

	printf("쿠폰번호\t쿠폰타입\t가능서비스\t할인율\t사용가능\n");
	while ((row = mysql_fetch_row(res)) != NULL) 
		printf("%s\t\t%s\t%s\t\t%s\t%s\t\n", row[0],row[1],row[2],row[3], row[4]);
	
	printf("\n\n5초 후 이전 화면으로 이동합니다.....\n");
	Sleep(5000);
	menuPos=0;
	next_state=member_service_menu;

}
void find_reservation()
{
	MYSQL_RES *res;
	char sjQuery[1024] = "\0";
	int result_curry;
	MYSQL_ROW row;
	system("cls");





	sprintf(sjQuery,"select a.AppointmentNumber 예약번호, s.ServiceName 서비스이름, a.AppointmentTime 예약시간, a.appointmentstate 예약상태, s.doctorname 담당의사, s.TreatmentFee 진료비, a.Hospitallocation 지점,  s.MedicalDepartment 진료부서 from appointment a, service s where patient_id = %d and a.servicenumber = s.ServiceNumber;",patien_id);
	mysql_query(&mysql, sjQuery);
	res = mysql_use_result(&mysql); 

	printf("서비스이름\t예약시간\t\t\t예약상태\t\t진료비\t\t\n");
	while ((row = mysql_fetch_row(res)) != NULL) 
		printf("%s\t%s\t\t%s\t\t%s%s\n", row[1],row[2],row[3],row[4], row[5]);

	printf("\n\n5초 후 이전 화면으로 이동합니다.....\n");
	Sleep(5000);
	menuPos=0;
	next_state=member_service_menu;

}

void login()
{
	MYSQL_RES *res;
	char  c_name[20], c_telephone[15];
	char sjQuery[1024] = "\0";
	int result_curry;
	int patient_id_return=0;

	MYSQL_ROW row;
	system("cls");
	puts("Name:");
	puts("Phone number:");
	gotoxy(15, 0); gets(c_name);
	gotoxy(15, 1); gets(c_telephone);

	//쿼 필요
	//sprintf(sjQuery,"INSERT INTO patient(name, birthday, phonenumber, address) VALUES('%s','%s','%s','%s');", c_name, c_birthday, c_telephone, c_address);
	sprintf(sjQuery,"select patient_login('%s', '%s');", c_name,c_telephone);
	mysql_query(&mysql, sjQuery);

	res = mysql_use_result(&mysql); //결과값 저장
	while ((row = mysql_fetch_row(res)) != NULL) 
		printf("%d\n", patien_id=atoi(row[0])); //결과값 출력

	patient_id_return=patien_id;




	//mysql_query(&mysql, sjQuery);
	//결과 값이 1이상이면 로그인성공
	//1번 아이디를 admin으로 바꿀수도 있음

	if(patient_id_return==0)
	{
		system("cls");
		printf("로그인 아이디와 비밀번호가 맞지 않습니다.\n");
		printf("3초후 처음 페이지로넘어갑니다.");
		Sleep(3000);
		next_state=first_menu;

	}

	else if(patient_id_return>0)
	{
		system("cls");
		printf("로그인 성공.\n");
		printf("1초후 사용자 페이지로넘어갑니다.");
		Sleep(1000);
		next_state=member_service_menu;
	}
}

void reservation()
{
	char sjQuery[1024] = "\0";
	char sjQuery2[1024] = "\0";
	char sjQuery3[1024] = "\0";
	char yanghan[10];
	int service_num;
	char hospital_location[20];
	char reservation_time[30];
	int appointnumber;
	char creditcard_num[30];
	MYSQL_ROW row;
	MYSQL_RES *res;

	system("cls");


	sprintf(sjQuery,"select *from service where availablestate = 'Y';");
	mysql_query(&mysql, sjQuery);
	res = mysql_use_result(&mysql); 

	printf("%10s %20s %10s %10s\n","서비스번호","서비스종류","의사","진료비");
	printf("----------------------------------------------------------------\n");
	while ((row = mysql_fetch_row(res)) != NULL) 
		printf("%10s %20s %10s %10s\n\n", row[0],row[1],row[2],row[4]);
	printf("\n\n----------------------------------------------------------------\n");
	puts("양의원 한의원 선택(양 또는 한 입력)");
	gets(yanghan);
	puts("예약하고 싶은 서비스의 번호 입력");
	scanf("%d",&service_num);
	fflush(stdin);
	puts("병원위치입력:");
	gets(hospital_location);
	puts("예약 시간 입력");
	gets(reservation_time);
	puts("카드 번호 입력");
	gets(creditcard_num);

	sprintf(sjQuery2,"insert into appointment(patient_id,hospitaltype,servicenumber,hospitallocation,appointmenttime,appointmentstate,creditcardnumber)   values (%d,'%s',%d,'%s','%s','reserved','%s');",patien_id,yanghan,service_num,hospital_location,reservation_time,creditcard_num);
	//sprintf(sjQuery2,"call reserve(%d,'%s',%d,'%s','%s')",patien_id,yanghan,service_num,hospital_location,reservation_time);

	//sprintf(sjQuery2,"insert into appointment(patient_id,hospitaltype,servicenumber,hospitallocation,appointmenttime,appointmentstate) values (%d,%s,%d,%s,%s,'reserving');",patien_id,yanghan,service_num,hospital_location,reservation_time);

	mysql_query(&mysql, sjQuery2);
	sprintf(sjQuery3,"select appointmentnumber from appointment order by appointmentnumber desc limit 1;");
	mysql_query(&mysql, sjQuery3);

	res = mysql_use_result(&mysql); 


	if ((row = mysql_fetch_row(res)) != NULL) 
		appointnumber=atoi(row[0]) ;//결과값 출력
	reserve_menu_value=appointnumber;

	next_state=member_service_menu;
	printf("예약이 완료되었습니다.\n");
	Sleep(2000);


}





void resign()
{

	char  c_name[20], c_telephone[15];
	char sjQuery[1024] = "\0";
	int result_curry;
	system("cls");

	puts("Name:");
	puts("Phone number:");

	gotoxy(15, 0); gets(c_name);
	gotoxy(15, 1); gets(c_telephone);



	sprintf(sjQuery,"delete from patient where name = '%s'and phonenumber = '%s'", c_name, c_telephone);

	mysql_query(&mysql, sjQuery);
	system("cls");
	printf("%s님 탈퇴가성공적으로 완료되었습니다.\n",c_name);
	Sleep(3000);
	next_state=first_menu;

}
void payment()
{
	char criditcard_num[30];	
	int cridit_num_lenght;

	char sjQuery[1024] = "\0";
	fflush(stdin);
	system("cls");
	puts("카드번호 입력: ");
	gets(criditcard_num);


	system("cls");
	cridit_num_lenght=count_string(criditcard_num);
	if(cridit_num_lenght!=16)
	{

		printf("카드번호가 올바르지 않습니다.\n");
		printf("선택 화면으로 넘어갑니다.\n");
		sprintf(sjQuery,"delete from appointment where appointmentnumber = %d",reserve_menu_value );
		mysql_query(&mysql, sjQuery);
		reserve_menu_value=0;
		next_state=member_service_menu;


		Sleep(2000);
	}

	else
	{
		printf("예약이 완료 되었습니다.\n");
		printf("선택 화면으로 넘어갑니다.\n");

		sprintf(sjQuery,"call input_card_number('%s',%d);",criditcard_num,reserve_menu_value);
		//mysql_query(&mysql, "update appointment set CreditcardNumber = '1234 1234 1234 1234' where appointmentnumber = 25;");
		//sprintf(sjQuery,"update appointment set appointmentstate = 'reserved' where appointmentnumber = %d;",reserve_menu_value);
		mysql_query(&mysql, sjQuery);
		next_state=member_service_menu;
		reserve_menu_value=NULL;
		Sleep(2000);
	}

}


void member_join()
{

	char  c_name[20], c_birthday[20], c_telephone[15], c_address[20];
	u8 plaintxt_jun[20];
	char sjQuery[1024] = "\0";
	int result_curry;
	u8 src2[LEA_BLK_BYTE_LEN] = { 0x0, };
	u8 src3[LEA_BLK_BYTE_LEN] = { 0x0, };
	char database_cipher[40]={0,};
	char decry_test[20];
	int i;
	char blockhex[3];

	system("cls");

	puts("Name:");
	puts("Birthday:");
	puts("Phone number:");
	puts("Address:");

	gotoxy(15, 0); gets(c_name);
	gotoxy(15, 1); gets(c_birthday);
	gotoxy(15, 2); gets(c_telephone);
	gotoxy(15, 3); gets(c_address);
	printf("[%s],[%s],[%s],[%s]",  c_name, c_birthday, c_telephone, c_address);


	for(i=0;i<16;i++)
	{
		plaintxt_jun[i]=c_address[i];

	}


	//ox로 설계 예정  
	for (i=0; i<LEA_BLK_BYTE_LEN; i++)
	{
		plaintxt_jun[i] += (LEA_KEY_BYTE_LEN-16)*2;
	}

	LEA_EncryptBlk(src2, plaintxt_jun, rndkeys);

	//전송 패킷 제작
	for(i=0;i<16;i++)
	{
		sprintf(blockhex,"%02x",src2[i]);
		database_cipher[2*i]=blockhex[0];
		database_cipher[2*i+1]=blockhex[1];
	}
	database_cipher[32]='\0';

	printf("\n다음과 같이 데이터베이스에 암호화 되어 저장되었습니다.\n암호문:%s\n",database_cipher);


	// LEA_DecryptBlk(src3, src2, rndkeys);

	/*  
	for(i=0;i<20;i++)
	{

	if(src3[i]==0xff)
	src3[i]='\0';
	}


	for(i=0;i<20;i++)
	{
	decry_test[i]=src3[i];

	}

	*/
	//system("cls");
	//printf("\n복호화 결과 %s\n",decry_test);




	Sleep(3000);
	sprintf(sjQuery,"INSERT INTO patient(name, birthday, phonenumber, address) VALUES('%s','%s','%s','%s');", c_name, c_birthday, c_telephone, database_cipher);

	mysql_query(&mysql, sjQuery);




	next_state=first_menu;

}

void gotoxy(int x, int y)
{
	COORD Cur;

	Cur.X = x;
	Cur.Y = y;

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Cur);
}



int push_banghangkey(int menucount)
{

	char ch; 

	ch= _getch();
	menucount=menucount--;	
	if (ch == 13){
		return push_enter;
	}


	else if (ch == 80 || ch == 72)
	{


		switch (ch)
		{
		case 72:
			menuPos--;
			if (menuPos < 0)
				menuPos = 0;
			break;
		case 80:
			menuPos++;
			if (menuPos > menucount) //예외처리
				menuPos = menucount;
			break;
		}
	}
}

void hideCursor()
{
	CONSOLE_CURSOR_INFO CurInfo;

	CurInfo.dwSize = 1;
	CurInfo.bVisible = FALSE;

	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &CurInfo);
}



void Show(u32 *src, int word_len)
{
	int i;
	for (i=0; i<word_len; i++)
		printf("%08x ", src[i]);
	printf("\n");

}


void ShowRndKeys(u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN])
{
	int i;
	for ( i=0; i<LEA_NUM_RNDS; i++)
	{
		printf("RndKey[%2d]: ", i);
		Show(pdRndKeys[i], LEA_RNDKEY_WORD_LEN);
	}
}


void LEA_Keyschedule(u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN],const u8 pbKey[LEA_KEY_BYTE_LEN])
{

#if defined(_LEA128_)

	u32 delta[4] = {0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec};
	u32 T[4] = {0x0,};
	int i;
	T[0] = u32_in(pbKey);
	T[1] = u32_in(pbKey + 4);
	T[2] = u32_in(pbKey + 8);
	T[3] = u32_in(pbKey + 12);

	for(i=0; i<LEA_NUM_RNDS; i++) 
	{
		T[0] = ROL(T[0] + ROL(delta[i&3], i), 1);
		T[1] = ROL(T[1] + ROL(delta[i&3], i+1), 3);
		T[2] = ROL(T[2] + ROL(delta[i&3], i+2), 6);
		T[3] = ROL(T[3] + ROL(delta[i&3], i+3), 11);

		pdRndKeys[i][0] = T[0];
		pdRndKeys[i][1] = T[1];
		pdRndKeys[i][2] = T[2];
		pdRndKeys[i][3] = T[1];
		pdRndKeys[i][4] = T[3];
		pdRndKeys[i][5] = T[1];
	}
#endif

#if defined(_LEA192_)

	u32 delta[6] = { 0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec, 0x715ea49e, 0xc785da0a};
	u32 T[6] = {0x0,};

	T[0] = u32_in(pbKey);
	T[1] = u32_in(pbKey + 4);
	T[2] = u32_in(pbKey + 8);
	T[3] = u32_in(pbKey + 12);
	T[4] = u32_in(pbKey + 16);
	T[5] = u32_in(pbKey + 20);

	for(int i=0; i<LEA_NUM_RNDS; i++) 
	{
		T[0] = ROL(T[0] + ROL(delta[i%6], i&0x1f), 1);
		T[1] = ROL(T[1] + ROL(delta[i%6], (i+1)&0x1f), 3);
		T[2] = ROL(T[2] + ROL(delta[i%6], (i+2)&0x1f), 6);
		T[3] = ROL(T[3] + ROL(delta[i%6], (i+3)&0x1f), 11);
		T[4] = ROL(T[4] + ROL(delta[i%6], (i+4)&0x1f), 13);
		T[5] = ROL(T[5] + ROL(delta[i%6], (i+5)&0x1f), 17);

		pdRndKeys[i][0] = T[0];
		pdRndKeys[i][1] = T[1];
		pdRndKeys[i][2] = T[2];
		pdRndKeys[i][3] = T[3];
		pdRndKeys[i][4] = T[4];
		pdRndKeys[i][5] = T[5];
	}

#endif

#if defined(_LEA256_)

	u32 delta[8] = {0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec, 0x715ea49e, 0xc785da0a, 0xe04ef22a, 0xe5c40957};
	u32 T[8] = {0x0,};

	T[0] = u32_in(pbKey);
	T[1] = u32_in(pbKey + 4);
	T[2] = u32_in(pbKey + 8);
	T[3] = u32_in(pbKey + 12);
	T[4] = u32_in(pbKey + 16);
	T[5] = u32_in(pbKey + 20);
	T[6] = u32_in(pbKey + 24);
	T[7] = u32_in(pbKey + 28);

	for(int i=0; i<LEA_NUM_RNDS; i++)
	{
		T[(6*i)&7] = ROL(T[(6*i)&7] + ROL(delta[i&7], i&0x1f), 1);
		T[(6*i + 1)&7] = ROL(T[(6*i + 1)&7] + ROL(delta[i&7], (i+1)&0x1f), 3);
		T[(6*i + 2)&7] = ROL(T[(6*i + 2)&7] + ROL(delta[i&7], (i+2)&0x1f), 6);
		T[(6*i + 3)&7] = ROL(T[(6*i + 3)&7] + ROL(delta[i&7], (i+3)&0x1f), 11);
		T[(6*i + 4)&7] = ROL(T[(6*i + 4)&7] + ROL(delta[i&7], (i+4)&0x1f), 13);
		T[(6*i + 5)&7] = ROL(T[(6*i + 5)&7] + ROL(delta[i&7], (i+5)&0x1f), 17);

		pdRndKeys[i][0] = T[(6*i)&7];
		pdRndKeys[i][1] = T[(6*i+1)&7];
		pdRndKeys[i][2] = T[(6*i+2)&7];
		pdRndKeys[i][3] = T[(6*i+3)&7];
		pdRndKeys[i][4] = T[(6*i+4)&7];
		pdRndKeys[i][5] = T[(6*i+5)&7];
	}
#endif

}

/*******************************************************************************/

void LEA_EncryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN], const u8 pbSrc[LEA_BLK_BYTE_LEN],const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN])
{
	u32 X0,X1,X2,X3;
	u32 temp;
	int i;
	X0 = u32_in(pbSrc);
	X1 = u32_in(pbSrc + 4);
	X2 = u32_in(pbSrc + 8);
	X3 = u32_in(pbSrc + 12);

	for( i=0; i<LEA_NUM_RNDS; i++)
	{
		X3 = ROR((X2 ^ pdRndKeys[i][4]) + (X3 ^ pdRndKeys[i][5]), 3);
		X2 = ROR((X1 ^ pdRndKeys[i][2]) + (X2 ^ pdRndKeys[i][3]), 5);
		X1 = ROL((X0 ^ pdRndKeys[i][0]) + (X1 ^ pdRndKeys[i][1]), 9);
		temp = X0;
		X0 = X1; X1 = X2; X2 = X3; X3 = temp;
	}

	u32_out(pbDst, X0);
	u32_out(pbDst + 4, X1);
	u32_out(pbDst + 8, X2);
	u32_out(pbDst + 12, X3);
}

int count_string(char *string)
{
	int i,cnt=0;
	for(i=0; string[i]!='\0'; i++)   
	{
		if(string[i]!=' ' && string[i]!='.' && string[i]!=',')  //구두점과 공백이 아니면..
			cnt++;  //cnt를 1증가시킵니다.
	}
	return cnt;  //cnt를 리턴합니다.
}

void LEA_DecryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN], const u8 pbSrc[LEA_BLK_BYTE_LEN], const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN])
{
	u32 X0,X1,X2,X3;
	u32 temp;
	int i;
	X0 = u32_in(pbSrc);
	X1 = u32_in(pbSrc + 4);
	X2 = u32_in(pbSrc + 8);
	X3 = u32_in(pbSrc + 12);


	for(i=0; i<LEA_NUM_RNDS; i++)
	{
		temp = X3;
		X3 = X2;
		X2 = X1;
		X1 = X0;
		X0 = temp;

		X1 = (ROR(X1,9) - (X0 ^ pdRndKeys[LEA_NUM_RNDS-1-i][0])) ^ pdRndKeys[LEA_NUM_RNDS-1-i][1];
		X2 = (ROL(X2,5) - (X1 ^ pdRndKeys[LEA_NUM_RNDS-1-i][2])) ^ pdRndKeys[LEA_NUM_RNDS-1-i][3];
		X3 = (ROL(X3,3) - (X2 ^ pdRndKeys[LEA_NUM_RNDS-1-i][4])) ^ pdRndKeys[LEA_NUM_RNDS-1-i][5];

	}

	u32_out(pbDst, X0);
	u32_out(pbDst + 4, X1);
	u32_out(pbDst + 8, X2);
	u32_out(pbDst + 12, X3);
}
