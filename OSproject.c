#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utime.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <zlib.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>

#define FILE_SIZE 256
#define PERM_SIZE 10
#define TIME_SIZE 16
#define COL 1024
#define CHUNK 16384

const int DEVELOPING = 0; //개발하는동안 오류를 확인하는 변수

typedef struct {
  char* name;
  unsigned int size;
} stHeaderEntry;

/*
	각 함수에 대한 설명은 함수 정의 부분에 위치
*/
int cmdDivider(char *argv[],char cmd[]);
void pwd();
void chmod_(int argc, char* argv[]);
void changedir(int argc, char* path);
void list(int argc, char* argv[]);
void ll_(int argc, char* argv[],int aOpt);
void mkdir_(int argc, char *argv[]);
void cp(int argc, char *argv[]);
void mv(int argc, char *argv[]);
void cat_(int argc, char *argv[]);
int rm_(int argc, char* argv[]);
int rmdir_(int argc, char* argv[]);
int touch_(int argc, char* argv[]);
int sizeSum(char filepath[],int aOpt);
char *getperm (char *perm_c, const mode_t mode);
int tar_(int argc, char *argv[]);
char *gettime (char *time_c, const struct tm *tinfo);
void writefile(char *in_f, char *out_f);
void folderCopy(char* SRC_FOLDER, char* DEST_FOLDER);
int fileCopy(const char* src, const char* dst);
int rmFile(const char* reg);//파일 삭제 함수. rm
int rmDir(const char* folder);//파일,디렉토리 및 하위 파일,디렉토리 삭제 함수. rm -r
int fileWcheck(const char* reg);//파일/디렉토리 권한 체크.
int rmForce(const char* reg);//-f 파일 강제삭제 함수.
int fileDown(const char* reg);//쓰기보호된 디렉토리 들어가는 함수.
int removeD(const char* folder);//rmdir수행 함수
int removeDP(const char* folder);//rmdir -p 수행 함수
int GetT(int year, int month, int day, int hour, int minute, int second);
int stat_(int argc, char* argv[]);
int gzip_(char *src, int mode, char *dst);
void zerr(int ret);
int inf(FILE *source, FILE *dest);
int def(FILE *source, FILE *dest, int level);
int extractTarDirectory(char* tarName,char* directory);
int extractTar(char tarName[]);
int createTar(int nFiles, char *fileNames[], char tarName[]);
int readHeader(FILE * tarFile, stHeaderEntry ** header, int *nFiles);
int loadstr(FILE * file, char **buf);
int copynFile(FILE * origin, FILE * destination, int nBytes);
int GetT(int year, int month, int day, int hour, int minute, int second);//시간 구성요소 정수값으로 받아서 tm 구조체 구성요소에 할당.
void mkd(char name[]);
void mkt(int argc, char *argv[]);

char use[]="Usage: tar -c|x|zf mytar.tar [file1 file2 ...]\n";

struct stat sb;//stat 구조체.


/*main
	매개변수 : cmd[] (명령어 입력)
	명령어를 gets로 받는다. scanf로 받으면 공백을 구분하지 못하기 때문에 gets로 받음

	cmdDivider를 통해 명령어를 공백 기준으로 분할
	switch문으로 커맨드 구분하여 명령어 실행함.
*/
void main(int argc, char* argv[]){
	char cmd[1024];
	int i,flag;

	pid_t ret;//프로세스아이디 정의
	ret = fork(); //생성
	while(1){
		if (ret == 0){//자식프로세스 있을때만 명령어실행
			
		argc = 0;
		pwd();
		gets(cmd);
		if(strcmp(cmd,"\r\n")==0)
			continue;
		argc = cmdDivider(argv,cmd);
			
		if(strcmp(argv[0],"chmod")==0){
			chmod_(argc,argv);
		}else if(strcmp(argv[0],"cd")==0){
			changedir(argc,argv[1]);
		}else if((strcmp(argv[0],"ls")==0)){
			list(argc,argv);
		}else if((strcmp(argv[0],"mkdir")==0)){
			mkdir_(argc,argv);
		}else if((strcmp(argv[0],"cp")==0)){
			cp(argc,argv);
		}else if((strcmp(argv[0],"mv")==0)){
			mv(argc,argv);
		}else if((strcmp(argv[0],"cat")==0)){
			cat_(argc,argv);
		}else if((strcmp(argv[0],"rm")==0)){
			rm_(argc,argv);
		}else if((strcmp(argv[0],"rmdir")==0)){
			rmdir_(argc,argv);
		}else if((strcmp(argv[0],"touch")==0)){
			touch_(argc,argv);
		}else if((strcmp(argv[0],"tar")==0)){
			tar_(argc,argv);
		}else if((strcmp(argv[0],"stat")==0)){
			stat_(argc,argv);
		}
	
		}
	}
	return;
}




/*pwd
	이 함수는 프로그램을 쉘 스크립트 모드로 동작시켜주는 함수이다.
	메인문의 루프 가장 첫번째로 실행된다.
	getcwd 함수를 이용해 항상 현재 경로를 표시할 수 있게 한다.
*/
void pwd(){
	char buf[1024];

	getcwd(buf,1024);
	printf("[OS project SHELL");
	printf("%s] ",buf);

}



/*cmdDiveder
	입력값 : argv 2차원 배열과 명령어 cmd
	사용 함수 : strtok(a,b) (a문자열을 b기준으로 잘라낸 문자열의 주소 반환
						     a에 NULL이 들어 올 시 현재 문자열 기준)
	매개 변수 : *result(공백기준으로 문자열을 하나하나 받아옴)
				argv(잘라낸 문자열을 저장함)
				cmd(main에서 입력받은 명령어)
	출력값 : 나눈 문자열 갯수
*/
int cmdDivider(char *argv[],char cmd[]){
	char * result;
	int i=0;
	result = strtok(cmd," ");
	
	//argv초기화
	while(argv[i]){
		argv[i++]=NULL;
	}
	i=0;

	while(result != NULL)
	{
		argv[i] = (char*)malloc(sizeof(char)*strlen(result));
		strcpy(argv[i++],result);
		result = strtok(NULL," ");
	}

	return i;
}



/*chmod_
	입력값 : 문자열갯수 argc, 명령어가 들어있는 2차원배열 argv
	사용 함수 : strtol(a,b,c) (a배열의 값을 c진수로 저장, b는 여기서 사용하지않음)
	매개 변수 : mode (변환된 8진수를 저장할 변수)
				permission[] (심볼릭모드일 때 입력된 rwx를 저장하는 배열)
				oper (연산자를 저장할 변수)
				in_file_stat (file의 상태가 담기는 구조체)
*/
void chmod_(int argc, char *argv[]){

		int i,mode=0;
		char permission[4]="000";
		char oper;
		struct stat in_file_stat;
		int file_permission;

		if(argc<2){
			printf("chmod: 잘못된 연산자\n");
			return;
		}else if(argc<3){
			printf("chmod: '%s'뒤에 명령어가 빠짐\n",argv[1]);
		}

		//argv[1][0]이 숫자로 시작하면 absolute mode 이다.
		if(argv[1][0] >= 48 && argv[1][0] < 56){
			mode = strtol(argv[1],NULL,8);
			//for문을 이용해 입력된 파일 숫자만큼 모든 파일의 권한을 변경한다.
			for(i=2;i<argc;i++){
                if(-1 == chmod(argv[i],mode))
					printf("failed\n");
			}
		//argv[1][0]이 숫자가 아닌 경우 symbolic mode 이다.
		}else{
			i=0;
			//연산자가 나올 때 까지 검색 후 연산자를 oper에 저장
			while(!((argv[1][i]=='+')||(argv[1][i]=='-')||(argv[1][i]=='='))){
				i++;
			}
			oper = argv[1][i++];
			while(argv[1][i]){
				if(argv[1][i]=='r'){
					permission[0] = '1';
				}else if(argv[1][i]=='w'){
					permission[1] = '1';
				}else if(argv[1][i]=='x'){
					permission[2] = '1';
				}else{
					printf("input mode error! '%s'\n",argv[1]);
					return;
				}
					i++;
			}
			
			i=0;

			//a,u,g,o에 권한 다르게 입력해줌
			//S_로 시작하는 것은 권한 값으로 정의돼 있는 상수임.
			//위에서 입력받은 permission 자릿수가 참인지에 따라 
			//or연산통해 8진수 권한값 저장
			while(argv[1][i]!=oper){
				switch(argv[1][i]){
					case 'a':
						if(permission[0] == '1'){
							mode = mode|0444;
						}

						if(permission[1] == '1'){
							mode = mode|0222;
						}

						if(permission[2] == '1'){
							mode = mode|0111;
						}

						break;

					case 'u':
						if(permission[0] == '1'){
							mode = mode|S_IRUSR;
						}

						if(permission[1] == '1'){
							mode = mode|S_IWUSR;
						}

						if(permission[2] == '1'){
							mode = mode|S_IXUSR;
						}
						
						break;

					case 'g':
						if(permission[0] == '1'){
							mode = mode|S_IRGRP;
						}

						if(permission[1] == '1'){
							mode = mode|S_IWGRP;
						}

						if(permission[2] == '1'){
							mode = mode|S_IXGRP;
						}

						break;

					case 'o':
						if(permission[0] == '1'){
							mode = mode|S_IROTH;
						}

						if(permission[1] == '1'){
							mode = mode|S_IWOTH;
						}

						if(permission[2] == '1'){
							mode = mode|S_IXOTH;
						}

						break;

					default:
						printf("input option error! '%s'\n",argv[1]);
						return;
				}

				i++;
			}

			//현재 파일의 권한과 입력받은 권한값을 +,=,- 에 따라서 
			//+면 or연산, =면 그대로입력, -면 &와 ^연산을 통해 권한 입력해줌
			switch(oper){

			case '+':
				for(i=2;i<argc;i++){
					stat(argv[i], &in_file_stat);
					file_permission = in_file_stat.st_mode;
					if(-1 == chmod(argv[i],mode|file_permission))
						perror("chmod");
				}
				break;

			case '=':
				for(i=2;i<argc;i++){
					if(-1 == chmod(argv[i],mode))
						perror("chmod");
				}
				break;
			
			case '-':
				for(i=2;i<argc;i++){
					stat(argv[i], &in_file_stat);
					file_permission = in_file_stat.st_mode;
					if(-1 == chmod(argv[i],file_permission^(file_permission&mode)))
						perror("chmod");
				}
				break;
			}	
		
	}
}




/*changedir
	입력값 : path(이동할 경로)
	chdir 시스템 콜을 통해 이동
*/
void changedir(int argc, char* path){

	int chk;

	if(argc == 1){
		chk=chdir("/");
		if(chk!=0){
			printf("failed\n");
		}
	}else if((strcmp(path,".")==0)){
		chk=chdir("/");
		if(chk!=0){
			printf("failed\n");
		}
	}else{
		chk=chdir(path);
		if(chk!=0){
			printf("failed\n");
		}
	}
}



/*list
	매개변수 : dp(디렉토리 주소를 저장할 변수)
				dir(디렉토리의 구조를 저장하는 구조체)
				cnt(파일 리스트를 정렬시키기 위해 출력 횟수를 저장하는 변수)
				info(파일의 상태를 저장하는 구조체)
	입력값 : argc, *argv[]
	동작 : argc로 입력 값의 숫자를 센다. 경로가 입력되지 않으면(argc == 1) "."(현재 폴더명) 폴더를 오픈
		   argc가 2 이상이고 argv[1]이 '-'부터 시작하면 옵션이 입력된 것으로 파악하여 옵션 처리.
		   -l 옵션이나 -al,-la 옵션은 ll_함수에서 처리하게 함.
*/
void list(int argc, char* argv[]){

	DIR *dp;
	struct dirent *dir;
	struct stat info;
	int cnt = 0, i;

	if(argc == 1){
		if((dp = opendir(".")) == NULL ){
			fprintf(stderr, "directory open error!!\n");
			return;
		}
		while(( dir = readdir(dp)) != NULL){
			if(dir->d_ino == 0){
				continue;
			}
			if(dir->d_name[0]=='.'){
				continue;
			}

			printf("%-15s ",dir->d_name);
			cnt++;
			if(cnt >= 5){
				printf("\n"); 
				cnt = 0; 
			}
			
		}
		closedir(dp);

		if(cnt != 0)
			printf("\n");
		

	}else if(argv[1][0]!='-'){ //무옵션, 경로 입력 시
		for(i=1;i<argc;i++){
			if(stat(argv[i],&info) == -1){
				fprintf(stderr, "'%s' is not exist!!\n",argv[i]);
				return;
			}
			
			if((info.st_mode & S_IFMT) != S_IFDIR){ //디렉토리가 아니면 파일이름만 출력
				printf("%s\n",argv[i]);
				continue;
			}

			if((dp = opendir(argv[i])) == NULL ){
				fprintf(stderr, "'%s' open error!! : permission is not enough.\n",argv[i]);
				return;
			}

		if(argc != 2) //경로가 하나만 입력됐을 때는 상단에 경로를 출력하지 않음
			printf("%s:\n",argv[i]); 
		
		while(( dir = readdir(dp)) != NULL){
			if(dir->d_ino == 0){
				continue;
			}
			if(dir->d_name[0]=='.'){
				continue;
			}

			printf("%-15s ",dir->d_name);
			cnt++;
			if(cnt >= 5){
				printf("\n"); 
				cnt = 0; 
			}
		}

		closedir(dp);

		if(cnt != 0)
			printf("\n");

		cnt = 0;

		}
	}else if((strcmp(argv[1],"-l"))==0){
		ll_(argc,argv,0);
		return;
	}else if((strcmp(argv[1],"-a"))==0){
		if(argc == 2){ //경로 지정 없을 때
		if((dp = opendir(".")) == NULL ){
			fprintf(stderr, "directory open error!!\n");
			return;
		}
		while(( dir = readdir(dp)) != NULL){
			if(dir->d_ino == 0){
				continue;
			}

				printf("%-15s ",dir->d_name);
				cnt++;
				if(cnt >= 5){
					printf("\n"); 
					cnt = 0; 
				}	
			}
		closedir(dp);

		if(cnt != 0)
			printf("\n");

		}else{  //경로 지정 했을 때
			for(i=2;i<argc;i++){
			if(stat(argv[i],&info) == -1){
				fprintf(stderr, "'%s' is not exist!!\n",argv[i]);
				return;
			}
			
			if((info.st_mode & S_IFMT) != S_IFDIR){ //디렉토리가 아니면 파일이름만 출력
				printf("%s\n",argv[i]);
				continue;
			}

			if((dp = opendir(argv[i])) == NULL ){
				fprintf(stderr, "'%s' open error!! : permission is not enough.\n",argv[i]);
				return;
			}


		if(argc != 3) //경로가 하나만 입력됐을 때는 경로 출력하지 않음
			printf("%s:\n",argv[i]); 
		
		while(( dir = readdir(dp)) != NULL){
			if(dir->d_ino == 0){
				continue;
			}

			printf("%-15s ",dir->d_name);
			cnt++;
			if(cnt >= 5){
				printf("\n"); 
				cnt = 0; 
			}
		}

		closedir(dp);

		if(cnt != 0)
			printf("\n");

		cnt = 0;

		printf("\n");
		}

		}
	}else if(((strcmp(argv[1],"-al"))==0)||((strcmp(argv[1],"-la"))==0)){
		ll_(argc,argv,1);
			return;
	}else{
		printf("ls: invalid line width: '%c'\n",argv[1][1]);
	}
}


/*ll_
	입력값 : argc, *argv[], aOpt(a옵션이 입력됐을 때는 1, 아니면 0)
	매개변수 : filename(파일의 이름을 임시저장할 배열)
			   perm_c(getperm 함수를 이용해 파일의 권한정보를 받아올 배열)
			   time_c(gettime 함수를 이용해 파일의 수정된 시간에 대한 정보를 받아올 배열)
	동작 : argc로 경로 지정이 있는지 없는지를 파악하며 경로가 1개만 입력 됐을 경우에는 폴더명을 따로 출력하지 않음.
		   경로가 디렉토리인지 아닌지를 읽고 디렉토리이면 디렉토리 내의 파일정보를 모두 출력하고 디렉토리가 아니면 해당 파일에 대한 정보만 출력.
		   a옵션이 없으면 숨김파일을 보여주지 않음.
*/
void ll_(int argc, char* argv[],int aOpt)
{
	char filename[FILE_SIZE] = "";
    char perm_c[PERM_SIZE + 1] = "";
    char time_c[TIME_SIZE + 1] = "";
    struct stat finfo;
	DIR *dp;
	struct dirent *dir;
	int size, i;
	
	if(aOpt == 0){ //ls -l
		if (argc > 2){ //경로 지정시
			for(i=2;i<argc;i++){
				if(stat(argv[i],&finfo) == -1){
				fprintf(stderr, "'%s' is not exist!!\n",argv[i]);
				return;
				}
			
				if((finfo.st_mode & S_IFMT) != S_IFDIR){ //디렉토리가 아니면 그 파일에 대한 정보만출력
					printf ("%s %ld %s %s %-6d %s %s\n", 
					getperm (perm_c, finfo.st_mode), 
					finfo.st_nlink, 
					getpwuid (finfo.st_uid)->pw_name, 
					getgrgid (finfo.st_gid)->gr_name, 
					(int)finfo.st_size, 
					gettime (time_c, localtime (&finfo.st_mtime)), 
					argv[i]
					);
					continue;
				}

				if((dp = opendir(argv[i])) == NULL ){
					fprintf(stderr, "'%s' open error!! : permission is not enough.\n",argv[i]);
					return;
				}


				if((dp = opendir(argv[i])) == NULL ){
				fprintf(stderr, "directory open error!!\n");
				return;
				}

			size = sizeSum(argv[i], 0); //ls -l
			printf("합 계  %d\n",size);
			while(( dir = readdir(dp)) != NULL){
				if(dir->d_ino == 0){
					continue;
				}
				if(dir->d_name[0]=='.'){
					continue;
				}

				strcpy(filename,argv[i]);
				strcat(filename,"/");
				strcat(filename,dir->d_name);
				if (stat (filename, &finfo) == -1){
					perror ("[*] path error");
					return;
				}

				printf ("%s %ld %s %s %-6d %s %s\n", 
				getperm (perm_c, finfo.st_mode), 
				finfo.st_nlink, 
				getpwuid (finfo.st_uid)->pw_name, 
				getgrgid (finfo.st_gid)->gr_name, 
				(int)finfo.st_size, 
				gettime (time_c, localtime (&finfo.st_mtime)), 
				dir->d_name
				);
			}
			closedir(dp);
			printf("\n");
			}
		
		}else{ //경로 없을 시
			if((dp = opendir(".")) == NULL ){
				fprintf(stderr, "directory open error!!\n");
				return;
			}
			size = sizeSum(".", 0); //ls -l
			printf("합 계  %d\n",size);
			while(( dir = readdir(dp)) != NULL){
				if(dir->d_ino == 0){
					continue;
				}
				if(dir->d_name[0]=='.'){
					continue;
				}

				if (stat (dir->d_name, &finfo) == -1){
					perror ("[*] path error");
					return;
				}

				printf ("%s %ld %s %s %-6d %s %s\n", 
				getperm (perm_c, finfo.st_mode), 
				finfo.st_nlink, 
				getpwuid (finfo.st_uid)->pw_name, 
				getgrgid (finfo.st_gid)->gr_name, 
				(int)finfo.st_size, 
				gettime (time_c, localtime (&finfo.st_mtime)), 
				dir->d_name
				);
			}
			closedir(dp);
		}
	}else{ //ls -al || -la
		if(argc > 2){ //경로 지정시
			for(i=2;i<argc;i++){
				if(stat(argv[i],&finfo) == -1){
				fprintf(stderr, "'%s' is not exist!!\n",argv[i]);
				return;
				}
			
				if((finfo.st_mode & S_IFMT) != S_IFDIR){ //디렉토리가 아니면 그 파일에 대한 정보만출력
					printf ("%s %ld %s %s %-6d %s %s\n", 
					getperm (perm_c, finfo.st_mode), 
					finfo.st_nlink, 
					getpwuid (finfo.st_uid)->pw_name, 
					getgrgid (finfo.st_gid)->gr_name, 
					(int)finfo.st_size, 
					gettime (time_c, localtime (&finfo.st_mtime)), 
					argv[i]
					);
					continue;
				}

				if((dp = opendir(argv[i])) == NULL ){
					fprintf(stderr, "'%s' open error!! : permission is not enough.\n",argv[i]);
					return;
				}

			size = sizeSum(argv[i], 1); //ls -al || -la
			printf("합 계  %d\n",size);
			while(( dir = readdir(dp)) != NULL){
				if(dir->d_ino == 0){
					continue;
				}

				strcpy(filename,argv[i]);
				strcat(filename,"/");
				strcat(filename,dir->d_name);
				if (stat (filename, &finfo) == -1){
					perror ("[*] path error");
					return;
				}

				printf ("%s %ld %s %s %-6d %s %s\n", 
				getperm (perm_c, finfo.st_mode), 
				finfo.st_nlink, 
				getpwuid (finfo.st_uid)->pw_name, 
				getgrgid (finfo.st_gid)->gr_name, 
				(int)finfo.st_size, 
				gettime (time_c, localtime (&finfo.st_mtime)), 
				dir->d_name
				);
			}
			closedir(dp);
			printf("\n");
			}
		
		}else{ //경로 없을 시
			if((dp = opendir(".")) == NULL ){
				fprintf(stderr, "directory open error!!\n");
				return;
			}
			size = sizeSum(".", 1); //ls -al
			printf("합 계  %d\n",size);
			while(( dir = readdir(dp)) != NULL){
				if(dir->d_ino == 0){
					continue;
				}

				if (stat (dir->d_name, &finfo) == -1){
					perror ("[*] path error");
					return;
				}

				printf ("%s %ld %s %s %-6d %s %s\n", 
				getperm (perm_c, finfo.st_mode), 
				finfo.st_nlink, 
				getpwuid (finfo.st_uid)->pw_name, 
				getgrgid (finfo.st_gid)->gr_name, 
				(int)finfo.st_size, 
				gettime (time_c, localtime (&finfo.st_mtime)), 
				dir->d_name
				);
			}
			closedir(dp);
		}
	}
}


/*getperm
	권한 정보를 읽어 'drwxrwxrwx'형태로 리턴하는 함수이다.
*/
char *getperm (char *perm_c, const mode_t mode)
{
    char *ref = "xwr";
    int i;

    memset (perm_c, '-', PERM_SIZE);

    if ((mode & S_IFMT) == S_IFDIR)
        perm_c[0] = 'd';

    for (i = 0; i < 9; i++)
    {
        if (mode & (1 << i))
            perm_c[PERM_SIZE - i - 1] = ref[i % 3];
    }

    return perm_c;
}


/*gettime
	파일 및 디렉토리의 수정시간 정보를 읽고 리턴하는 함수이다.
*/
char *gettime (char *time_c, const struct tm *tinfo)
{
	snprintf (time_c, TIME_SIZE + 1, "%04d-%02d-%02d %02d:%02d", \
              tinfo->tm_year + 1900, \
              tinfo->tm_mon + 1, \
              tinfo->tm_mday, \
              tinfo->tm_hour, \
              tinfo->tm_min
              );

    return time_c;
}



/*sizeSum
	ls -l 입력시 출력되는 파일의 크기정보 합계를 반환하는 함수이다.
	입력값 : filepath[], aOpt(a옵션이 없으면 숨김파일은 더하지 않음
	매개변수 : entry(디렉토리 구조체)
			   dirpt(디렉토리 변수)
			   sum(디렉토리 내의 파일의 크기를 모두 저장한 후 반환할 값을 가지고 있는 변수)
			   buf(파일의 정보를 가지고 있는 구조체)
			   filename(파일 이름을 임시 저장할 배열)
*/
int sizeSum(char filepath[],int aOpt)
{
	struct dirent *entry;
	DIR *dirpt;
	int sum=0;
	struct stat buf;
	char filename[FILE_SIZE];

	dirpt = opendir(filepath);

	while((entry = readdir(dirpt))!='\0')
	{
		if((aOpt == 0) && (entry->d_name[0]=='.'))
			continue;

		strcpy(filename,filepath);
		strcat(filename,"/");
		strcat(filename,entry->d_name);
		if((lstat(filename,&buf))==0)
			sum = sum+buf.st_blocks;
	}
	closedir(dirpt);

	return sum/2;
}



/*mkdir
	입력값 : argc , *argv[]
	매개변수 : filepath(파일 경로를 임시 저장할 배열)
			   filename(파일 이름을 임시 저장할 배열)
			   result(argv를 '/'단위로 자르고 그 주소를 가지기 위해 있는 포인터)
			   mode(설정할 파일 권한 8진수 값을 저장할 변수)
			   oper('+','-','=' 연산자를 저장할 변수)
			   permission[](권한 계산을 도와줄 배열)
	사용함수 : umask(0) (파일을 생성함과 동시에 권한 설정을 제약없이 가능케 하기 위해 umask를 풀어 놓아야 한다.)
			   
	동 작 : 전체적인 동작은 chmod와 비슷하다. 우선 사용자가 입력한 권한에 대해 8진수값으로 바꾸는 작업을 거치고 
			산술연산 시 기본권한인 0775를 대상으로 연산하게 된다. 권한을 계산하고 나면 mkdir(파일명,권한값)으로 
			생성과 동시에 권한을 설정하게 된다.
*/

void mkdir_(int argc, char *argv[])
{
 char filepath[256];
 char filename[128];
 char *result;
 int i, mode = 0;
 char oper;
 char permission[4] = "000";
 umask(0);

 if (argv[1][0] != '-')
 {
  mkt(argc, argv); // 쓰레드생성함수 호출
 }
 else { //옵션부여
  if (strcmp(argv[1], "-pm") == 0) {//-pm일경우
            //argv[2][0]이 숫자로 시작하면 absolute mode 이다.
   if (argv[2][0] >= 48 && argv[2][0] < 56) {
    mode = strtol(argv[2], NULL, 8);

    for (i = 3; i<argc; i++) {
     strcpy(filepath, "");
     strcpy(filename, "");
     strcat(filename, argv[i]);
     if (mkdir(argv[i], mode) != 0) {
      result = strtok(argv[i], "/");
      while (result != NULL) {
       strcat(filepath, result);
       mkdir(filepath, 0775);
       if (mkdir(filename, mode) == 0) {
        break;
       }
       strcat(filepath, "/");
       result = strtok(NULL, "/");
      }
     }
     else {
      continue;
     }
    }

   }
   else { //argv[2][0]이 숫자가 아닌 경우 symbolic mode 이다.
    i = 0;
    //연산자가 나올 때 까지 검색 후 연산자를 oper에 저장
    while (!((argv[2][i] == '+') || (argv[2][i] == '-') || (argv[2][i] == '='))) {
     i++;
    }
    oper = argv[2][i++];
    while (argv[2][i]) {
     if (argv[2][i] == 'r') {
      permission[0] = '1';
     }
     else if (argv[2][i] == 'w') {
      permission[1] = '1';
     }
     else if (argv[2][i] == 'x') {
      permission[2] = '1';
     }
     else {
      printf("input mode error! '%s'\n", argv[2]);
      return;
     }
     i++;
    }
    i = 0;

    //a,u,g,o에 권한 다르게 입력해줌
    //S_로 시작하는 것은 권한 값으로 정의돼 있는 상수임.
    //위에서 입력받은 permission 자릿수가 참인지에 따라
    //or연산통해 8진수 권한값 저장
    while (argv[2][i] != oper) {
     switch (argv[2][i]) {
     case 'a':
      if (permission[0] == '1') {
       mode = mode | 0444;
      }

      if (permission[1] == '1') {
       mode = mode | 0222;
      }

      if (permission[2] == '1') {
       mode = mode | 0111;
      }

      break;

     case 'u':
      if (permission[0] == '1') {
       mode = mode | S_IRUSR;
      }

      if (permission[1] == '1') {
       mode = mode | S_IWUSR;
      }

      if (permission[2] == '1') {
       mode = mode | S_IXUSR;
      }

      break;

     case 'g':
      if (permission[0] == '1') {
       mode = mode | S_IRGRP;
      }

      if (permission[1] == '1') {
       mode = mode | S_IWGRP;
      }

      if (permission[2] == '1') {
       mode = mode | S_IXGRP;
      }

      break;

     case 'o':
      if (permission[0] == '1') {
       mode = mode | S_IROTH;
      }

      if (permission[1] == '1') {
       mode = mode | S_IWOTH;
      }

      if (permission[2] == '1') {
       mode = mode | S_IXOTH;
      }

      break;

     default:
      printf("input option error! '%s'\n", argv[2]);
      return;;
     }

     i++;
    }

    //현재 파일의 권한과 입력받은 권한값을 +,=,- 에 따라서
    //+면 or연산, =면 그대로입력, -면 &와 ^연산을 통해 권한 입력해줌
    switch (oper) {
    case '+':
     for (i = 3; i<argc; i++) {
      strcpy(filepath, "");
      strcpy(filename, "");
      strcat(filename, argv[i]);
      if (mkdir(argv[i], mode | 0775) != 0) {
       result = strtok(argv[i], "/");
       while (result != NULL) {
        strcat(filepath, result);
        mkdir(filepath, 0775);
        if (mkdir(filename, mode | 0775) == 0) {
         break;
        }
        strcat(filepath, "/");
        result = strtok(NULL, "/");
       }
      }
      else {
       continue;
      }
     }
     break;

    case '=':
     for (i = 3; i<argc; i++) {
      strcpy(filepath, "");
      strcpy(filename, "");
      strcat(filename, argv[i]);
      if (mkdir(argv[i], mode) != 0) {
       result = strtok(argv[i], "/");
       while (result != NULL) {
        strcat(filepath, result);
        mkdir(filepath, 0775);
        if (mkdir(filename, mode) == 0) {
         break;
        }
        strcat(filepath, "/");
        result = strtok(NULL, "/");
       }
      }
      else {
       continue;
      }
     }
     break;

    case '-':
     for (i = 3; i<argc; i++) {
      strcpy(filepath, "");
      strcpy(filename, "");
      strcat(filename, argv[i]);
      if (mkdir(argv[i], 0775 ^ (0775 & mode)) != 0) {
       result = strtok(argv[i], "/");
       while (result != NULL) {
        strcat(filepath, result);
        mkdir(filepath, 0775);
        if (mkdir(filename, 0775 ^ (0775 & mode)) == 0) {
         break;
        }
        strcat(filepath, "/");
        result = strtok(NULL, "/");
       }
      }
      else {
       continue;
      }
     }
     break;
    }
   }

  }
  else if (strcmp(argv[1], "-p") == 0) { //-p일경우
   for (i = 2; i<argc; i++) {
    strcpy(filepath, "");
    strcpy(filename, "");
    strcat(filename, argv[i]);
    if (mkdir(argv[i], 0775) != 0) {
     result = strtok(argv[i], "/");
     while (result != NULL) {
      strcat(filepath, result);
      mkdir(filepath, 0775);
      if (mkdir(filename, 0775) == 0) {
       break;
      }
      strcat(filepath, "/");
      result = strtok(NULL, "/");
     }
    }
    else {
     continue;
    }
   }
  }
  else if (strcmp(argv[1], "-m") == 0) { //-m만 입력되어야함
              //argv[2][0]이 숫자로 시작하면 absolute mode 이다.
   if (argv[2][0] >= 48 && argv[2][0] < 56) {
    mode = strtol(argv[2], NULL, 8);

    for (i = 3; i<argc; i++) {
     if (mkdir(argv[i], mode) != 0) {
      perror("mkdir:");
     }
     else {
      continue;
     }
    }
    //argv[2][0]이 숫자가 아닌 경우 symbolic mode 이다.
   }
   else {
    i = 0;
    //연산자가 나올 때 까지 검색 후 연산자를 oper에 저장
    while (!((argv[2][i] == '+') || (argv[2][i] == '-') || (argv[2][i] == '='))) {
     i++;
    }
    oper = argv[2][i++];
    while (argv[2][i]) {
     if (argv[2][i] == 'r') {
      permission[0] = '1';
     }
     else if (argv[2][i] == 'w') {
      permission[1] = '1';
     }
     else if (argv[2][i] == 'x') {
      permission[2] = '1';
     }
     else {
      printf("input mode error! '%s'\n", argv[2]);
      return;
     }
     i++;
    }
    i = 0;

    //a,u,g,o에 권한 다르게 입력해줌
    //S_로 시작하는 것은 권한 값으로 정의돼 있는 상수임.
    //위에서 입력받은 permission 자릿수가 참인지에 따라
    //or연산통해 8진수 권한값 저장
    while (argv[2][i] != oper) {
     switch (argv[2][i]) {
     case 'a':
      if (permission[0] == '1') {
       mode = mode | 0444;
      }

      if (permission[1] == '1') {
       mode = mode | 0222;
      }

      if (permission[2] == '1') {
       mode = mode | 0111;
      }

      break;

     case 'u':
      if (permission[0] == '1') {
       mode = mode | S_IRUSR;
      }

      if (permission[1] == '1') {
       mode = mode | S_IWUSR;
      }

      if (permission[2] == '1') {
       mode = mode | S_IXUSR;
      }

      break;

     case 'g':
      if (permission[0] == '1') {
       mode = mode | S_IRGRP;
      }

      if (permission[1] == '1') {
       mode = mode | S_IWGRP;
      }

      if (permission[2] == '1') {
       mode = mode | S_IXGRP;
      }

      break;

     case 'o':
      if (permission[0] == '1') {
       mode = mode | S_IROTH;
      }

      if (permission[1] == '1') {
       mode = mode | S_IWOTH;
      }

      if (permission[2] == '1') {
       mode = mode | S_IXOTH;
      }

      break;

     default:
      printf("input option error! '%s'\n", argv[2]);
      return;
     }

     i++;
    }

    //현재 파일의 권한과 입력받은 권한값을 +,=,- 에 따라서
    //+면 or연산, =면 그대로입력, -면 &와 ^연산을 통해 권한 입력해줌
    switch (oper) {
    case '+':
     for (i = 3; i<argc; i++) {
      if ((mkdir(argv[i], mode | 0775)) != 0) {
       perror("mkdir");
      }
      else {
       continue;
      }
     }
     break;

    case '=':
     for (i = 3; i<argc; i++) {
      if (mkdir(argv[i], mode) != 0) {
       perror("mkdir");
      }
      else {
       continue;
      }
     }
     break;

    case '-':
     for (i = 3; i<argc; i++) {
      if (mkdir(argv[i], 0775 ^ (0775 & mode)) != 0) {
       perror("mkdir");
      }
      else {
       continue;
      }
     }
    }
   }

  }
  else {
   printf("잘못된 모드 '%s'\n", argv[1]);
   return;
  }
 }
}



 /*writefile
	입 력 값 : *in_f, *out_f
	동 작 : 복사할 파일 in_f와 목적지 out_f를 입력받고 
			write와 read함수를 이용해 복사해준다.
			복사할 파일이 없는 파일이거나 목적지 권한이 없을 경우 에러메시지를 띄운다.
 */
void writefile(char *in_f, char *out_f) //파일 복사 함수
{
	struct stat info;
    int in_o, out_o;
    int read_o;
    char buf[1024];

	if(stat(in_f, &info) == -1){
		printf("cp: '%s'를 설명할 수 없음 : 그런 파일이나 디렉터리가 없습니다.\n",in_f);
		return;
	}

	if((info.st_mode & S_IFMT) == S_IFDIR){
		folderCopy(in_f,out_f);
		return;
	}

    in_o = open(in_f, O_RDONLY);
    out_o = open(out_f, O_WRONLY|O_CREAT,info.st_mode);
 
    while((read_o = read(in_o,buf,sizeof(buf))) > 0)
        write(out_o,buf,read_o);
}


/*cp
	입 력 값 : argc, *argv[]
	매개변수 : path,path2(복사 목적지를 임시 저장하는 변수)
	동 작 : 각종 오동작 조건들을 체크 한 후 이상이 없으면 writefile 함수 호출
*/
void cp(int argc, char *argv[])
{
    int i,chk;
	struct stat info;
	char path[512];
	char path2[256];

	if(argc==1){
		printf("cp: 파일 명령어 누락\n");
		return;
	}else if(argc<3) {
		printf("cp: '%s' 다음에 대상 파일 명령이 누락됨\n",argv[1]);
		return;
	}else if(argc>3){  //여러 파일 입력시 대상경로가 디렉토리가 아니면 종료되어야 함
		if(stat(argv[argc-1],&info) == -1){ 
			fprintf(stderr, "'%s' is not exist!!\n",argv[argc-1]);
			return;
		}
		if((info.st_mode & S_IFMT) != S_IFDIR){ //디렉토리가 아니면 종료
			printf("cp: 대상 '%s'은 디렉터리가 아님\n",argv[argc-1]);
			return;
		}

		//디렉터리가 맞으면 목적지폴더에 파일들을 복사함
		if(argv[argc-1][0]!='/'){ //절대경로가 아닌 경우
			strcat(path,argv[argc-1]);
			strcat(path,"/");
			strcpy(path2,path);
			for(i=1;i<argc-1;i++){
			if((info.st_mode&S_IWUSR)!=S_IWUSR){
				printf("cp: 일반 파일 '%s'을 (를) 생성할 수 없음 : 허가 거부\n",argv[i]);
				continue;
			}
				strcat(path,argv[i]);
				writefile(argv[i],path);
				strcpy(path,path2);
			}
		}else{ //절대경로인 경우
			strcpy(path,argv[argc-1]);

			if(strcmp(argv[argc-1],"/")!=0){
				strcat(path,"/");
			}else{
				for(i=1;i<argc-1;i++){
					printf("cp: 일반 파일 '/%s'을 (를) 생성할 수 없음 : 허가 거부\n",argv[i]);
				}
				return;
			}

			strcpy(path2,path);
			for(i=1;i<argc-1;i++){
			if((info.st_mode&S_IWUSR)!=S_IWUSR){
				printf("cp: 일반 파일 '%s'을 (를) 생성할 수 없음 : 허가 거부\n",argv[i]);
				continue;
			}
				strcat(path,argv[i]);
				writefile(argv[i],path);
				strcpy(path,path2);
			}
		}
	}else{ 
		if(strcmp(argv[1],argv[2])==0){
			printf("cp: '%s'와 '%s'는 같은파일\n",argv[1],argv[2]);
			return;
		}

		chk = stat(argv[2],&info);

		if(chk==-1){  //목적지파일이 존재하지않으면 그냥 복사
			writefile(argv[1],argv[2]);
		}else if((info.st_mode & S_IFMT) != S_IFDIR){ //디렉토리가 아니면 덮어쓰기
			if((info.st_mode&S_IWUSR)!=S_IWUSR){
				printf("cp: 일반 파일 '%s'을 (를) 생성할 수 없음 : 허가 거부\n",argv[2]);
				return;
			}else{
			unlink(argv[2]);
			writefile(argv[1],argv[2]);
			}
		}else if(argv[2][0]!='/'){ //목적파일이 디렉토리이고 파일명 지정하지 않은 경우
			//절대경로가 아닌경우
			if((info.st_mode&S_IWUSR)!=S_IWUSR){
				printf("cp: 일반 파일 '%s'을 (를) 생성할 수 없음 : 허가 거부\n",argv[1]);
				return;
			}
			strcpy(path,"./");
			strcat(path,argv[2]);
			strcat(path,"/");
			strcat(path,argv[1]);
			writefile(argv[1],path);
		}else{
			//절대경로인 경우
			if((info.st_mode&S_IWUSR)!=S_IWUSR){
				printf("cp: 일반 파일 '%s'을 (를) 생성할 수 없음 : 허가 거부\n",argv[1]);
				return;
			}
			
			if(strcmp(argv[2],"/")!=0){
				strcpy(path,argv[2]);
				strcat(path,"/");
				strcat(path,argv[1]);
				writefile(argv[1],path);
			}else{
				printf("cp: 일반 파일 '/%s'을 (를) 생성할 수 없음 : 허가 거부\n",argv[1]);
				return;
			}	
		}
	}
	return;
}

void mv(int argc, char *argv[]){
	int i,chk;
	struct stat finfo;
	char path[512]="";
	char path2[256];
	char perm_c[PERM_SIZE - 1]="";
	char opt[2];

	if(argc==1){
		printf("mv: 파일 명령어 누락\n");
		return;
	}else if(argc<3){
		printf("mv: '%s' 다음에 대상 파일 명령이 누락됨\n",argv[1]);
		return;
	}else if(argc>3){  //여러 파일 입력시 대상경로가 디렉토리가 아니면 종료되어야 함
		if(stat(argv[argc-1],&finfo) == -1){ 
			fprintf(stderr, "'%s' is not exist!!\n",argv[argc-1]);
			return;
		}
		if((finfo.st_mode & S_IFMT) != S_IFDIR){ //디렉토리가 아니면 종료
			printf("mv: 대상 '%s'은 디렉터리가 아님\n",argv[argc-1]);
			return;
		}

		//디렉터리가 맞으면 목적지폴더에 파일들을 복사함
		if(argv[argc-1][0]!='/'){ //절대경로가 아닌 경우
			strcat(path,argv[argc-1]);
			strcat(path,"/");
			strcpy(path2,path);
			for(i=1;i<argc-1;i++){
				strcat(path,argv[i]);
				if((finfo.st_mode&S_IWUSR)!=S_IWUSR){
				printf("mv: 일반 파일 '%s'로 옮길 수 없음 : 허가 거부\n",path);
				strcpy(path,path2);
				continue;
				}
				rename(argv[i],path);
				strcpy(path,path2);
			}
		}else{ //절대경로인 경우
			strcpy(path,argv[argc-1]);
			if(strcmp(argv[argc-1],"/")!=0){
				strcat(path,"/");
			}else{
				for(i=1;i<argc-1;i++){
					printf("mv: '%s'를 '/%s'로 옮길 수 없음 : 허가 거부\n",argv[i],argv[i]);
				}
				return;
			}
			strcat(path,"/");
			strcpy(path2,path);
			for(i=1;i<argc-1;i++){
				if((finfo.st_mode&S_IWUSR)!=S_IWUSR){
				printf("mv: 일반 파일 '%s'로 옮길 수 없음 : 허가 거부\n",argv[argc-1]);
				continue;
				}
				strcat(path,argv[i]);
				rename(argv[i],path);
				strcpy(path,path2);
			}
		}
	}else{
		if(strcmp(argv[1],argv[2])==0){
			printf("mv: '%s'와 '%s'는 같은파일\n",argv[1],argv[2]);
			return;
		}

		chk = stat(argv[2],&finfo);

		if(chk==-1){  //목적지파일이 존재하지않으면 그냥 복사
			rename(argv[1],argv[2]);
		}else if((finfo.st_mode & S_IFMT) != S_IFDIR){ //디렉토리가 아니면 덮어쓰기
			if((finfo.st_mode&S_IWUSR)!=S_IWUSR){
				printf("mv: '%s' 파일이 이미 존재합니다. 덮어 쓰시겠습니까?",argv[2]);
				gets(opt);
				if(opt[0]=='y'){
					rename(argv[1],argv[2]);
					return;
				}else{
					return;
				}
			}else{
				rename(argv[1],argv[2]);
			}
		}else if(argv[2][0]!='/'){ //목적파일이 디렉토리이고 파일명 지정하지 않은 경우
			//절대경로가 아닌경우
			if((finfo.st_mode&S_IWUSR)!=S_IWUSR){
				printf("mv: 일반 파일 '%s'로 옮길 수 없음 : 허가 거부\n",argv[argc-1]);
				return;
			}
			strcat(path,argv[2]);
			strcat(path,"/");
			strcat(path,argv[1]);
			rename(argv[1],path);
		}else{
			//절대경로인 경우
			if(strcmp(argv[2],"/")!=0){
				if((finfo.st_mode&S_IWUSR)!=S_IWUSR){
					printf("mv: 일반 파일 '%s'로 옮길 수 없음 : 허가 거부\n",argv[argc-1]);
					return;
				}
				strcpy(path,argv[2]);
				strcat(path,"/");
				strcat(path,argv[1]);
				rename(argv[1],path);
			}else{
				printf("mv: '%s'를 '/%s'로 옮길 수 없음 : 허가 거부\n",argv[1],argv[1]);
				return;
			}	
		}
	}
}


void cat_(int argc, char *argv[])
{
 char s[COL];
 char txt[1024];
 int count = 1;
 char c = 0;
 int i = 0;
 struct stat finfo;

 if(!strcmp(argv[1], "-n"))  //줄번호포함출력
 {
  FILE* fp = fopen(argv[2], "r");
  if (fp == NULL)
  {
   puts("파일열기실패\n");
   return;
  }
  while (fgets(s, COL, fp) != NULL)
  {
   printf("%6d  %s", count, s);
   count++;
  }
  fclose(fp);
  return;
 }
 else if ((strcmp(argv[1],"q")==0))  //파일명입력받고 내용입력
 { 

	if((stat(argv[2],&finfo))!= -1)
		unlink(argv[2]);

	while(1)
	{
		FILE* fp = fopen(argv[2], "a");
	fgets(txt, sizeof(txt),stdin);
   if (strcmp(txt, "end\n") == 0)  //end를 치면 종료.
   {
    //printf("@@@%s", txt);
    return;
   }
   else
   {
    //printf("###%s", txt);
    fputs(txt, fp); 
	fclose(fp);
   }
  
  }  

 }else  //그냥출력
 {
  FILE* fp = fopen(argv[1], "r");
  if (fp == NULL)
  {
   perror("cat: ");
   return;
  }
  while (fgets(s, COL, fp) != NULL)
  {
   printf("%s", s);
  }
  fclose(fp);
 
  return;
 }
}

void folderCopy(char* SRC_FOLDER, char* DEST_FOLDER){
    DIR            *dir_info;
    struct dirent  *dir_entry;
    struct  stat   statBuf;
	struct stat info;
	struct stat info2;
    char src[512];
    char dest[512];
	umask(0);
	
	stat(SRC_FOLDER,&info2);

	if(stat(DEST_FOLDER,&info) == -1){
		mkdir(DEST_FOLDER,info2.st_mode);
	}


    dir_info = opendir(SRC_FOLDER);              // 현재 디렉토리를 열기
    if ( NULL != dir_info){
       while( (dir_entry = readdir( dir_info)) != NULL)  // 디렉토리 안에 있는 모든 파일과 디렉토리 출력
       {
           if(!strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, "..")){
               continue;
           }

           sprintf(src, "%s/%s", SRC_FOLDER, dir_entry->d_name);
           sprintf(dest, "%s/%s", DEST_FOLDER, dir_entry->d_name);

		   lstat(src, &statBuf);

           if(S_ISDIR(statBuf.st_mode)){
               folderCopy(src, dest);
           }
           else {
               fileCopy(src, dest);
           }
       }
       closedir( dir_info);
    }
}

// 파일 복사 함수의 본체

int fileCopy(const char* src, const char* dst) {
    FILE *in, *out;
    char* buf;
    size_t len;
    //if (!strcmpi(src, dst)) return 4; // 원본과 사본 파일이 동일하면 에러
    if ((in  = fopen(src, "rb")) == NULL) return 1; // 원본 파일 열기
    if ((out = fopen(dst, "wb")) == NULL)
    {
        fclose(in);
        return 2;
    } // 대상 파일 만들기
    if ((buf = (char *) malloc(1024*1024)) == NULL) { fclose(in); fclose(out); return 10; } // 버퍼 메모리 할당
    while ( (len = fread(buf, sizeof(char), sizeof(buf), in)) != NULL )
    if (fwrite(buf, sizeof(char), len, out) == 0) {
        fclose(in); fclose(out);
        free(buf);
        remove(dst); // 에러난 파일 지우고 종료
        return 3;
    }
    fclose(in); fclose(out);
    free(buf); // 메모리 할당 해제
    return 0;
}

int rm_(int argc, char* argv[])
{
    int i;
    int ret;//stat 저장되는 함수.


    if(argc < 2){
        printf("rm: 잘못된 연산자\nTry 'rm --help' for more information.\n");
        return 1;//rm 인자 입력 없을 경우. --help는 구현 x
    }

		if(strcmp(argv[1], "-r")==0){
        for(i=2;i<argc;i++){
            ret = stat(argv[i], &sb);
            if(ret){
                printf("rm: '%s'를 지울 수 없음: 그런 파일이나 디렉터리가 없습니다\n",argv[i]);
                return 1;
            }
        rmDir(argv[i]);
        }
		}
    else if(strcmp(argv[1], "-f")==0){
        for(i=2;i<argc;i++){
            ret = stat(argv[i], &sb);
            if(ret){
                return 1;
            }
        rmForce(argv[i]);
        }
	}
	else{
        if(strcmp(argv[1],"-f")==0)
        {
            return 0;
        }
        if(strcmp(argv[1],"-r")==0)
        {
            printf("rm: 잘못된 연산자\nTry 'rm --help' for more information.\n");
            return 0;
        }
        for(i=1;i<argc;i++){
            ret = stat(argv[i], &sb);
            if(ret){
                printf("rm: '%s'를 지울 수 없음: 그런 파일이나 디렉터리가 없습니다\n",argv[i]);
                return 1;
            }
            rmFile(argv[i]);
        }
	}
    return 0;
}

int rmFile(const char* reg)//파일 삭제
{
    switch(sb.st_mode&S_IFMT){
    case S_IFBLK:
        printf("block device node\n");
        break;
    case S_IFCHR:
        printf("character device node\n");
        break;
    case S_IFIFO:
        printf("FIFO\n");
        break;
    case S_IFDIR:
        printf("rm: '%s'를 지울 수 없음: 디렉터리입니다\n",reg);
        break;
    case S_IFREG:
        if(fileWcheck(reg)==0)
        {
            unlink(reg);
        }
        break;
    default:
        printf("rm: '%s'를 지울 수 없음 : 그런 파일이나 디렉터리가 없습니다\n",reg);
        break;
    }
    return 0;
}

//-r 구현위한 rmDir함수
int rmDir(const char* folder)//폴더이름 인자로 받음
{
    DIR* dp=NULL; //DIR-디렉토리를 나타내는 구분자, 구조체로서 디렉토리 정보 저장. 디렉토리 관련 작업시 포인터로 선언.
    struct dirent *entry=NULL; //디렉터리의 entry정보가 저장되는 구조체.
    struct stat buf; //open된 디렉토리/파일 정보가 저장되는 stat 구조체.
    char target[512]; //작업중인 파일의 경로 저장

    switch(sb.st_mode&S_IFMT){
    case S_IFREG://파일인 경우 삭제.
        if(fileWcheck(folder)==0)
        {
            unlink(folder);
        }
        break;
    case S_IFDIR://디렉토리인 경우.
        if((dp = opendir(folder))==NULL)//opendir-디렉토리 open함수, open실패에 대한 예외처리.
        {
            //디렉토리 open 실패시
             perror("file open error");
            return -1;
        }
        while(1)//while문 이용, 디렉토리의 마지막 하위 디렉토리 및 파일 까지 탐색.
        {
            //readdir-open된 디렉토리의 entry(항목)를 읽는 함수
            if((entry=readdir(dp))==NULL)//빈 디렉토리.
            {
                if(fileWcheck(folder)==0){//쓰기 권한 체크.
                    closedir(dp);
                    remove(folder);//해당 폴더 삭제.
                    return -1;//탕색 종료.
                }
            }

            if(strcmp(entry->d_name, ".")==0||strcmp(entry->d_name, "..")==0)//'.'=자기자신, '..'=상위폴더는 스킵.
            {
                continue;
            }

            if(fileDown(folder)!=0)//읽기 보호된 폴더 내려가기 여부 묻기.
            {
                return -1;
            }

            //디렉토리안에 여러개의 파일,디렉토리가 존재할 수 있으므로 while문 사용해 모두 읽음.
            sprintf(target, "%s/%s", folder, entry->d_name);//경로 업데이트.
            //대상이 디렉토리인지, 파일인지 구분후 작업.
            // stat-readdir로부터 읽어낸 entry에서 구체적인 정보 읽어냄.
            stat(target, &buf);

            if(access(folder, W_OK)==-1&&access(target, W_OK)==0&&rmDir(target) == -1)
            {//상위 폴더 쓰기 권한 없고, 자기 자신은 쓰기 권한 있고, rmDir 함수 -1리턴.
                printf("rm: '%s'를 지울 수 없음: 허가 거부\n", target);
                return 0;
            }

            if(S_ISDIR(buf.st_mode))//디렉토리 일 경우. st_mode : 해당 파일 정보
            {
                //rmDir함수 재귀호출
                if(rmDir(target) == -1)
                {
                    if(access(folder, W_OK)==-1)//상위 폴더 쓰기 권한 없는 경우.
                    {
                        return -1;//탐색 종료.
                    }
                    continue;
                }
                continue;
            }

            if(fileWcheck(target)==0)//폴더 내부에 파일인 경우. 권한 체크
            {
            unlink(target);
            }
            continue;//현재 디렉토리 다시 읽음.
        }
        break;
    }
}

int rmForce(const char* reg)//-f옵션.
{
    switch(sb.st_mode&S_IFMT){
    case S_IFBLK:
        printf("block device node\n");
        break;
    case S_IFCHR:
        printf("character device node\n");
        break;
    case S_IFIFO:
        printf("FIFO\n");
        break;
    case S_IFDIR:
        printf("rm: '%s'를 지울 수 없음: 디렉터리입니다\n",reg);
        break;
    case S_IFREG:
        unlink(reg);
        break;
    default:
        break;
    }
    return 0;
}

int fileWcheck(const char* reg)//파일/디렉토리 읽기 권한 검사
{
    struct stat ff;
    stat(reg, &ff);
    char answer[512];
    int i;
    if(access(reg, W_OK) != 0)
    {
        switch(ff.st_mode&S_IFMT)//파일 타입이 디렉토리일 경우,
        {
        case S_IFDIR:
            printf("rm: 쓰기 보호된 디렉토리 '%s'를(을) 지울까요? ", reg);
            break;
        case S_IFREG:
            printf("rm: 쓰기 보호된 일반 파일 '%s'를(을) 지울까요? ", reg);
            break;
        default:
            printf("rm: '%s'를 지울 수 없음 : 그런 파일이나 디렉터리가 없습니다\n",reg);
            break;
        }
        scanf("%s", answer);

        if((answer[0] == 'y')||(answer[0] == 'Y'))
        {
            return 0;//삭제 승인, 0 반환.
        }
        return -1;//삭제 거부, -1 반환.
    }
    return 0;//삭제 승인, 0 반환.
}

int fileDown(const char* reg)
{
    struct stat ff;
    stat(reg, &ff);
    char answer[512];
    int i;
    if(access(reg, W_OK) != 0)
    {
        switch(ff.st_mode&S_IFMT)//파일 타입이 디렉토리일 경우,
        {
        case S_IFDIR:
            printf("rm: 쓰기 방지된 '%s' 디렉터리로 내려가겠습니까? ", reg);
            break;
        default:
            printf("rm: '%s'를 지울 수 없음 : 그런 파일이나 디렉터리가 없습니다\n",reg);
            break;
        }
        scanf("%s", answer);

        if((answer[0] == 'y')||(answer[0] == 'Y'))
        {
            return 0;//삭제 승인, 0 반환.
        }
        return -1;//삭제 거부, -1 반환.
    }
    return 0;//삭제 승인, 0 반환.
}



int rmdir_(int argc, char* argv[])
{

    int i;
    int ret;

    if(argc < 2){
        printf("rmdir: 잘못된 연산자\n");
        return 1;//rm 인자 입력 없을 경우.
    }

	if(strcmp(argv[1],"-p")==0){
        for(i=2;i<argc;i++){
            ret=stat(argv[i], &sb);//파일or 디렉토리 정보 저장
            if(ret){
                printf("rmdir: failed to remove '%s': 그런 파일이나 디렉터리가 없습니다\n",argv[i]);//존재하지 않는 파일/디렉토리 일 경우.
                return 1;
            }
            removeDP(argv[i]);//rmdir -p 수행.
        }
	}else{	//기본 rmdir
        for(i=1;i<argc;i++){
            ret=stat(argv[i], &sb);//파일or 디렉토리 정보 저장
            if(ret){
                printf("rmdir: failed to remove '%s': 그런 파일이나 디렉터리가 없습니다\n",argv[i]);//존재하지 않는 파일/디렉토리 일 경우.
                return 1;
            }
            removeD(argv[i]);//rmdir 수행
		}
	}

    return 0;
}

int removeD(const char* folder)
{
        DIR* dp=NULL; //DIR-디렉토리를 나타내는 구분자, 구조체로서 디렉토리 정보 저장. 디렉토리 관련 작업시 포인터로 선언.
        struct dirent *entry=NULL; //디렉터리의 entry정보가 저장되는 구조체.

        switch(sb.st_mode&S_IFMT){//st_mode : 대상이 파일인지 디렉토리인지 구분.
        case S_IFREG://파일인 경우
            printf("rmdir: failed to remove '%s': 디렉터리가 아닙니다\n",folder);//삭제 불가.
            break;
        case S_IFDIR://디렉토리인 경우.
            if((dp = opendir(folder))==NULL)//opendir-디렉토리 open함수, open실패에 대한 예외처리.
            {
                return -1;//종료.
            }
                //->빈디렉토리이면 삭제, 아닐경우 삭제 못함.
            while(1){
                if((entry=readdir(dp))==NULL)//빈 디렉토리인 경우.
                {
                    closedir(dp);
                    rmdir(folder);//해당 빈 폴더 삭제.
                    return -1;//탕색 종료.
                }
                if(strcmp(entry->d_name, ".")==0||strcmp(entry->d_name, "..")==0)//'.'=자기자신, '..'=상위폴더는 스킵.
                {
                    continue;
                }
                printf("rmdir: failed to remove '%s': 디렉터리가 비어있지 않음\n", folder);//디렉토리 비어있지 않은 경우 삭제 안하고 종료. 
                return -1;
                }
                closedir(dp);
                break;
    }
}

int removeDP(const char* folder){
    char DI[512];
    strcpy(DI, folder);//입력받은 경로 문자열로 저장.
    char *dir[512]={NULL};//경로를 폴더/파일 단위로 나누어 저장되는 배열.
    char target[512];//삭제 대상.
    int i=0;
    int j,k;
    char *ptr = strtok(DI,"/");//최초 입력 받은 경로를 /단위로 나눔.
    while(ptr != NULL)//전부 나누어 dir배열에 각각 저장.
    {
        dir[i] = ptr;
        i++;
        ptr = strtok(NULL,"/");
    }
    strcpy(target,dir[0]);
    
    for(k=i;k>0;k--){//경로 더하기 반복, 하위 디렉토리는 다음 더하기시 잘림.
    for(j=1;j<k;j++)
    {
        sprintf(target, "%s/%s", target,dir[j]);
    }
    removeD(target);//removeD함수 수행.
    strcpy(target,dir[0]);
    }
    return 0;
}


int tar_(int argc, char *argv[]){
    
    int opt=0, i;
	int nExtra=0, retCode=EXIT_SUCCESS;
	int option = 0;
    char *tarName=NULL;
    
    if(argc < 2){
        fprintf(stderr,"%s",use);
        return EXIT_FAILURE;
    }


		nExtra = argc - 3;

	tarName = argv[2];
		
	if(strcmp(argv[1],"-xzf")==0){
		gzip_(tarName, 0, argv[argc-1]);
		retCode=extractTar(argv[argc-1]);
	}else if(strcmp(argv[1],"-xfz")==0){
		gzip_(tarName, 0, argv[argc-1]);
		retCode=extractTar(argv[argc-1]);
	}else if(strcmp(argv[1],"-zxf")==0){
		gzip_(tarName, 0, argv[argc-1]);
		retCode=extractTar(argv[argc-1]);
	}else if(strcmp(argv[1],"-zfx")==0){
		gzip_(tarName, 0, argv[argc-1]);
		retCode=extractTar(argv[argc-1]);
	}else if(strcmp(argv[1],"-fxz")==0){
		gzip_(tarName, 0, argv[argc-1]);
		retCode=extractTar(argv[argc-1]);
	}else if(strcmp(argv[1],"-fzx")==0){
		gzip_(tarName, 0, argv[argc-1]);
		retCode=extractTar(argv[argc-1]);
	}else if(strcmp(argv[1],"-xf")==0){    //옵션 x,f에 대한 처리
		if (nExtra==0)
            retCode=extractTar(tarName);
        else if (nExtra==1)
            retCode=extractTarDirectory(tarName,argv[argc-nExtra]);
        else {
			fprintf(stderr,"%s",use);
			return EXIT_FAILURE;
			}
	}else if(strcmp(argv[1],"-fx")==0){ 
		if (nExtra==0)
            retCode=extractTar(tarName);
        else if (nExtra==1)
            retCode=extractTarDirectory(tarName,argv[argc-nExtra]);
        else {
            fprintf(stderr,"%s",use);
            return EXIT_FAILURE;
			}
	}else if(strcmp(argv[1],"-czf")==0){  ///옵션 c,z,f에 대한 처리
		retCode=createTar(nExtra, &argv[argc-nExtra], tarName);
		gzip_(tarName, 1, tarName);
	}else if(strcmp(argv[1],"-cfz")==0){ 
		retCode=createTar(nExtra, &argv[argc-nExtra], tarName);
		gzip_(tarName, 1, tarName);
	}else if(strcmp(argv[1],"-fzc")==0){ 
		retCode=createTar(nExtra, &argv[argc-nExtra], tarName);
		gzip_(tarName, 1, tarName);
	}else if(strcmp(argv[1],"-fcz")==0){ 
		retCode=createTar(nExtra, &argv[argc-nExtra], tarName);
		gzip_(tarName, 1, tarName);
	}else if(strcmp(argv[1],"-zfc")==0){ 
		retCode=createTar(nExtra, &argv[argc-nExtra], tarName);
		gzip_(tarName, 1, tarName);
	}else if(strcmp(argv[1],"-zcf")==0){ 
		retCode=createTar(nExtra, &argv[argc-nExtra], tarName);
		gzip_(tarName, 1, tarName);
	}else if(strcmp(argv[1],"-cf")==0){    //옵션 c,f에 대한 처리
		retCode=createTar(nExtra, &argv[argc-nExtra], tarName);
	}else if(strcmp(argv[1],"-fc")==0){    
		retCode=createTar(nExtra, &argv[argc-nExtra], tarName);
	}else{
		printf("%s\n",use);
        retCode=EXIT_FAILURE;
	}
	return retCode;

}


int copynFile(FILE * origin, FILE * destination, int nBytes)
{
    int totalBytes = 0; 
    int outputByte = 0; 
    int readByte = 0;  


    if (origin == NULL) { 
		return -1; 
	} 
    
    while((totalBytes < nBytes) && (readByte = getc(origin)) != EOF) { 
        if ((ferror(origin) != 0)) { return -1; } 
        outputByte = putc(readByte, destination);
        if (outputByte == EOF) { return -1; } 
        totalBytes++;
    }

    return totalBytes;
}


int loadstr(FILE * file, char **buf) {
    int filenameLength = 0, index = 0;
    char *name;
    char bit;

    while((bit = getc(file) != '\0')) {
        filenameLength++;
        if (bit == 0) {}
    }

    name =  malloc(sizeof(char) * (filenameLength + 1)); 
    fseek(file, -(filenameLength + 1), SEEK_CUR);

    for (index = 0; index < filenameLength+1; index++) {
        name[index] = getc(file);
       
    }    

    (*buf)=name; 
    return 0;
}


int readHeader(FILE * tarFile, stHeaderEntry ** header, int *nFiles)
{
    int nr_files = 0, index = 0, size = 0; 
    stHeaderEntry *stHeader=NULL; 

    if (fread( &nr_files, sizeof(int), 1, tarFile) == 0) { 
        if(DEVELOPING == 1) printf("Wasn't possible to read the mtar file\n");
        return EXIT_FAILURE; 
	}
    stHeader=malloc(sizeof(stHeaderEntry)*nr_files); 
   
    for (index = 0; index < nr_files; index++) {

        if (loadstr(tarFile, &stHeader[index].name) != 0) {
            if(DEVELOPING == 1) printf("Wasn't possible to read the mtar file\n");
            return EXIT_FAILURE;
        }

        fread(&size, sizeof(unsigned int), 1, tarFile); 
        stHeader[index].size = size;
    }

    (*nFiles)=nr_files; 
    (*header)=stHeader;

    return (EXIT_SUCCESS);
}

/* tar로 파일을 묶는 함수 */
int createTar(int nFiles, char *fileNames[], char tarName[])
{
    FILE * inputFile; 
    FILE * outputFile; 

    int copiedBytes = 0, stHeaderBytes = 0, index = 0;
    stHeaderEntry *stHeader; 

    stHeader      =  malloc(sizeof(stHeaderEntry) * nFiles); 
    stHeaderBytes += sizeof(int); 
    stHeaderBytes += nFiles*sizeof(unsigned int); 

    for (index=0; index < nFiles; index++) {
        stHeaderBytes+=strlen(fileNames[index])+1; 
    }

    outputFile =  fopen(tarName, "w"); 
    fseek(outputFile, stHeaderBytes, SEEK_SET); 

    for (index=0; index < nFiles; index++) {

        if ((inputFile = fopen(fileNames[index], "r")) == NULL) {
            if(DEVELOPING == 1) printf("Yo! File: %s doesn't exist\n", fileNames[index]);
            return (EXIT_FAILURE);
        } 

        copiedBytes = copynFile(inputFile, outputFile, INT_MAX); 
    
    if (copiedBytes == -1) {
        if (DEVELOPING == 1) printf("Copied bytes in the create tar is: %d\n", copiedBytes);        
        return EXIT_FAILURE;    
    }
    else {
            stHeader[index].size = copiedBytes; 
            stHeader[index].name = malloc(sizeof(fileNames[index]) + 1); 
            strcpy(stHeader[index].name, fileNames[index]); 
        if (DEVELOPING == 1) printf("File %s size is %d bytes\n", stHeader[index].name, stHeader[index].size);  
    }        
    
    if (fclose(inputFile) == EOF) return EXIT_FAILURE; 
    }

    if (fseek(outputFile, 0, SEEK_SET) != 0)
        return (EXIT_FAILURE); 
    else
        fwrite(&nFiles, sizeof(int), 1, outputFile); 
    

    for (index = 0; index < nFiles; index++) {
        fwrite(stHeader[index].name, strlen(stHeader[index].name)+1, 1, outputFile); 
        fwrite(&stHeader[index].size, sizeof(unsigned int), 1, outputFile); 
      
    }

    
    for (index=0; index < nFiles; index++) {
        free(stHeader[index].name); 
    }

    free(stHeader); 

    if (fclose(outputFile) == EOF) { return (EXIT_FAILURE); } 

    if(DEVELOPING ==1) printf("mtar file created successfully\n");

    return (EXIT_SUCCESS);
}


/* tar로 묶인 파일을 풀어주는 함수*/
int extractTar(char tarName[])
{ 
    FILE *tarFile = NULL; 
    FILE *destinationFile = NULL;
    stHeaderEntry *stHeader; 
    int nr_files = 0, index = 0, copiedBytes = 0;

    if((tarFile = fopen(tarName, "r") ) == NULL) {
        if(DEVELOPING == 1) printf("Yo! %s file doesn't exit \n", tarName);
        return (EXIT_FAILURE); 
    }

    if (readHeader(tarFile, &stHeader, &nr_files) == -1) {
        if(DEVELOPING == 1) printf("We couldn't load the header \n");
        return (EXIT_FAILURE); 
    } 

    for (index = 0; index < nr_files; index++) {

        if ((destinationFile = fopen(stHeader[index].name, "w")) == NULL) { return EXIT_FAILURE; } 
        else {
            copiedBytes = copynFile(tarFile, destinationFile, stHeader[index].size);
        if (copiedBytes == -1) { return EXIT_FAILURE; }     
    }
        
        if(fclose(destinationFile) != 0) { return EXIT_FAILURE; } 
    }
    
    if(DEVELOPING == 1) {
        printf("Number of files is %d\n", nr_files);
        for (index = 0; index <nr_files;index++) {
            printf("File %s size is %d bytes\n", stHeader[index].name, stHeader[index].size);
        }
    }

    for (index = 0; index <nr_files; index++) {
        free(stHeader[index].name);
    }

    free(stHeader); 
    if (fclose(tarFile) == EOF) { return (EXIT_FAILURE); } 

    return (EXIT_SUCCESS);
}

/*tar로 묶인 폴더를 풀어주는함수 */

int extractTarDirectory(char* tarName,char* directory) {
    
    char *filepath = malloc(strlen(directory) + 1 + strlen(tarName) + 2);
    filepath = strcpy(filepath, directory);
    filepath = strcat(filepath, "/");
    filepath = strcat(filepath, tarName);

    if(DEVELOPING == 1) printf("Filepath is %s \n", filepath);
 
    /*디렉토리 존재하는지 검사*/
    if (0 != access(directory, F_OK)) {
        if(DEVELOPING == 1) printf("Can't create file at %s\n", directory);
        return EXIT_FAILURE;
    }
    

    if(DEVELOPING == 1) printf("Oh yes. The directory exists :-)\n");

    FILE *tarFile = NULL; 
    stHeaderEntry *stHeader; 
    int nr_files = 0, index = 0, copiedBytes = 0;

    if((tarFile = fopen(tarName, "r")) == NULL) {
        if(DEVELOPING == 1) printf("Yo! %s file doesn't exit \n", tarName);
        return (EXIT_FAILURE); 
    }

    if (readHeader(tarFile, &stHeader, &nr_files) == -1) {
        if(DEVELOPING == 1) printf("We couldn't load the header \n");
        return (EXIT_FAILURE); 
    }


    for (index = 0; index < nr_files; index++) {
    
        char *destFilename = malloc(strlen(directory) + 1 + strlen(tarName) + 2);
        destFilename = strcpy(filepath, directory);
        destFilename = strcat(filepath, "/");
        destFilename = strcat(filepath, stHeader[index].name);
        
        FILE *destinationFile = NULL;

        if ((destinationFile = fopen(destFilename, "w")) == NULL) { return EXIT_FAILURE; } 
        else {
            copiedBytes = copynFile(tarFile, destinationFile, stHeader[index].size); 
            if (copiedBytes == -1) { return EXIT_FAILURE; }     
        }  
    
        if(fclose(destinationFile) != 0) { return EXIT_FAILURE; } 
    }
    
    if (fclose(tarFile) == EOF) { return (EXIT_FAILURE); } 

    if (DEVELOPING == 1) printf("Yeah. Extract a directory\n");
    return 0;
}

/* 실질적으로 압축을 하는 함수 */
int def(FILE *source, FILE *dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

	/*압축과정*/
    do {
		strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

		
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;

			ret = deflate(&strm, flush);    
            assert(ret != Z_STREAM_ERROR);  

			 have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0);     

		 
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        

	
    (void)deflateEnd(&strm);
    return Z_OK;
}

/* 실질적으로 압축을 푸는 함수 */
int inf(FILE *source, FILE *dest)
{
	int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

	
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

	
    do {
		 strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
		
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;

			ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
			have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
		} while (strm.avail_out == 0);

		
    } while (ret != Z_STREAM_END);

    (void)inflateEnd(&strm);
    
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}


/* 에러 메시지 출력 */
void zerr(int ret)
{
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* -z옵션
	mode가 1이면 압축, 0이면 해제*/
int gzip_(char *src, int mode, char *dst)
{
    int ret;
	struct stat finfo;
	FILE *fp, *fp2;

	fp = fopen(src,"rwb");
	fp2 = fopen(dst,"rwb");

	//압축
    if (mode == 1) {
        ret = def(fp, fp2, Z_BEST_COMPRESSION);

        if (ret != Z_OK)
            zerr(ret);

        return ret;
	}

	//압축해제
    else if (mode == 0) {
		ret = inf(fp, fp);

		ret= ret^Z_DATA_ERROR; //예외처리

        if (ret != Z_OK)
            zerr(ret);

        return ret;
    }

    else {
        printf("코딩오류: gzip호출 시 mode 값 확인\n");
        return 1;
    }
}

int stat_(int argc, char* argv[])
{
    int i;

    for(i=1;i<argc;i++){
        struct timeval val;
        struct tm *tm;
        struct stat sb;
        stat(argv[i], &sb);
        struct passwd *pw = getpwuid(sb.st_uid);
        struct group  *grp = getgrgid(sb.st_gid);
        int a =(sb.st_mode & S_IFMT)+(sb.st_mode & S_IRWXU)+(sb.st_mode & S_IRWXG)+(sb.st_mode & S_IRWXO);
        char type[512];

        if(sb.st_mode & S_IFREG)
        {
            if(sb.st_size==0)
            {
                strcpy(type, "일반 빈 파일");
            }
            else
            strcpy(type, "일반 파일");
        }
        else if(sb.st_mode & S_IFDIR)
        {
            strcpy(type, "디렉토리");
        }

        printf("  File: '%s'\n", argv[i]);
        printf("  Size: %ld\t\tBlocks: %ld\tIO Blocks: %ld\t %s\n",sb.st_size,sb.st_blocks,sb.st_blksize,type);
        printf("Device: %ld/%ld\tInode: %ld\tLinks: %ld\n",sb.st_rdev,sb.st_dev,sb.st_ino,sb.st_nlink);
        printf("Access: (");
        printf("%04o/", a%4096);
        printf((S_ISDIR(sb.st_mode))  ? "d" : "-");
        printf((sb.st_mode & S_IRUSR) ? "r" : "-");
        printf((sb.st_mode & S_IWUSR) ? "w" : "-");
        printf((sb.st_mode & S_IXUSR) ? "x" : "-");
        printf((sb.st_mode & S_IRGRP) ? "r" : "-");
        printf((sb.st_mode & S_IWGRP) ? "w" : "-");
        printf((sb.st_mode & S_IXGRP) ? "x" : "-");
        printf((sb.st_mode & S_IROTH) ? "r" : "-");
        printf((sb.st_mode & S_IWOTH) ? "w" : "-");
        printf((sb.st_mode & S_IXOTH) ? "x" : "-");
        printf(")  ");
        printf("Uid: ( %d/ %s)   Gid: ( %d/ %s)\t", sb.st_uid, pw->pw_name,sb.st_gid, grp->gr_name);
        printf("\n");

        printf("Access: ");
        tm = localtime(&sb.st_atime);
        printf("%d-",tm->tm_year+1900);
        printf("%d-",tm->tm_mon+1);
        printf("%d ",tm->tm_mday);
        printf("%d:",tm->tm_hour);
        printf("%d:",tm->tm_min);
        printf("%d\n",tm->tm_sec);

        printf("Modify: ");
        tm = localtime(&sb.st_mtime);
        printf("%d-",tm->tm_year+1900);
        printf("%d-",tm->tm_mon+1);
        printf("%d ",tm->tm_mday);
        printf("%d:",tm->tm_hour);
        printf("%d:",tm->tm_min);
        printf("%d\n",tm->tm_sec);
        printf("Change: ");
        tm = localtime(&sb.st_ctime);
        printf("%d-",tm->tm_year+1900);
        printf("%d-",tm->tm_mon+1);
        printf("%d ",tm->tm_mday);
        printf("%d:",tm->tm_hour);
        printf("%d:",tm->tm_min);
        printf("%d\n",tm->tm_sec);
        printf(" Birth: -\n");
    }
}


int touch_(int argc, char* argv[])
{
    struct stat st;//파일 stat정보 구조체.
    int i;
    char *ptr;
    char dat1[50], dat2[50];
    struct utimbuf ut;//파일 접근시각 및 수정 시각 정보 저장된 구조체.

    if(argc<2){
        printf("touch: 파일 명령어 누락\n");
        return 1;
    }
      if(strcmp(argv[1],"-t")==0){
            ptr = strtok(argv[2], ".");//소수점 분리.
            strcpy(dat1, ptr);
            double num1 = atof(dat1);//문자열을 double형으로 변환.
            ptr = strtok(NULL, ".");
            strcpy(dat2, ptr);
            double num2 = atoi(dat2);
            int year, month, day, hour, min, sec;
            for(i=3;i<argc;i++){        //입력값 년,월,일 등 순으로 분리.
               year = num1/100000000;
               month = (num1/1000000)-(year*100);
               day = ((num1 - (month*1000000))/10000)-year*10000;
               hour = ((num1 - (day*10000))/100)-(year*1000000)-(month*10000);
               min = (long int)num1%100;
               sec = num2;

                time_t base = GetT(year,month,day,hour,min,sec);
                ut.actime = ut.modtime = base;//timestamp 변경.
                utime(argv[i], &ut);//변경된 시간 적용.
            }
      }
      else if(strcmp(argv[1],"-c")==0){
            for(i=2;i<argc;i++){
                    stat(argv[i], &st);
                st.st_ctime = time((time_t *)0);
                utime(argv[i], NULL);
            }
   }
      else{
            for(i=1;i<argc;i++){
                    stat(argv[i], &st);
                FILE *wfp = fopen(argv[i], "w");//입력받은 파일명이 존재하지 않는 경우 파일 생성.
                st.st_ctime = time((time_t *)0);//현재 시간으로 설정.
                utime(argv[i], NULL);//현재시간으로 설정.
            }
      }
    
}

int GetT(int year, int month, int day, int hour, int minute, int second) {
    struct tm t = {0};//tm 구조체.
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;//해당하는 값 저장.

    return mktime(&t);//mktime 리턴.timestamp값 리턴.
}

void mkd(char name[]) //디렉토리 만드는 함수
{
 if (mkdir(name, 0775) == -1)
  perror("mkdir");
}

void mkt(int argc, char *argv[]) //쓰레드 만드는 함수
{

 pthread_t p_thread[argc]; //쓰레드ID 저장할 변수
 int thr_id; //쓰레드 에러체크
 int k;

 for (k = 1; k < argc; k++) //입력에서 mkdir을 제외한 값의 수
 {
  thr_id = pthread_create(&p_thread[0], NULL, &mkd, (void*)argv[k]); //쓰레드생성/쓰레드에서 실행할 함수(mkd),전달인자값
  if (thr_id < 0)
  {
   perror("thread create error : ");
   exit(0);
  }
 }
 sleep(0.5);
 return;
}
