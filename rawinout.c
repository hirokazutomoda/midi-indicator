//
// Description:    Read ALSA Raw MIDI input and write Raw MIDI output
// rawinout.c
//
#include <alsa/asoundlib.h>  /* for alsa interface   */
#include <unistd.h>    /* for sleep() and execv() function */
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TEST_OK  0     /* テスト関数戻り値(正常)*/
#define TEST_NG -1     /* テスト関数戻り値(異常)*/
#define LOG_FILE     "/home/pi/LOG/log.txt"        /* ログディレクトリ(通常)  */
#define ERR_LOG_FILE "/home/pi/LOG/err_log.txt"    /* ログディレクトリ(エラー)*/
#define N 256 // 1行の最大文字数(バイト数)

FILE *log_file;        /* 通常ログ */
FILE *err_log_file;    /* 異常ログ */


int test_function(int num);                    /* テスト関数        */
void LOG_PRINT(char log_txt[256], ...);        /* 通常ログ出力関数  */
void ERR_LOG_PRINT(char err_txt[256], ...);    /* エラーログ出力関数*/
void rawmidithrough();

int npfds;
struct pollfd *pfds;
struct pollfd qfds;
snd_rawmidi_t *midiinport = NULL;
snd_rawmidi_t *midioutport = NULL;


int main(int ac, char *av[])
{

  LOG_PRINT(">>> reading color map \n");

  FILE *fp; // FILE型構造体


  char fname[] = "color_map";
  //int chr;
  char strColorMap[256][N];

  fp = fopen(fname, "r"); // ファイルを開く。失敗するとNULLを返す。
  if (fp == NULL) {
	  printf("cannot open file %s\n", fname);
	  return -1;
  }


  int i = 0;
  char *ret;
  do {
	//printf("i %d\n", i);
	//char s[N];
	ret = fgets(strColorMap[i], N, fp);
	//printf("s %s", s);
	//strColorMap[i] = s;
	//printf("c %x %x %x", strColorMap[0], strColorMap[1], strColorMap[2]);
	i++;
  } while (ret != NULL);
  i = 0;


  //while (fgets(strColorMap[i], N, fp) != NULL) {
	 // printf("%s", strColorMap[i]);
  //}

  //一文字ずつ
  //while ((chr = fgetc(fp)) != EOF) {
	 // putchar(chr);
	 // //fprintf(stdout, "reading %02x\n", chr);
  //}

  fclose(fp); // ファイルを閉じる

  //fprintf(stdout, "size %d", sizeof(strColorMap));
  //fprintf(stderr, "size %d", sizeof(strColorMap));
  //for (int i = 0; i < N ; ++i) {
	 // fprintf(stderr, "looping %d: %s \n", i, strColorMap[i]);
  //};



  LOG_PRINT(">>> opening midi ports \n");

  int status;
  int mode = SND_RAWMIDI_SYNC;
  const char *inportname = "hw:1,0,0";
  const char *outportname = "hw:1,0,0";

  if(ac>3){
    inportname = av[1];
    outportname = av[1];
  }
  fprintf(stderr,"input port=%s\n",inportname);
  fprintf(stderr,"output port=%s\n",outportname);

  if ((status = snd_rawmidi_open(&midiinport, NULL, inportname, mode)) < 0)
  {
	  fprintf(stderr, "Problem opening MIDI input: %s, result=%d",
		  snd_strerror(status), status);
	  exit(1);
  }
  if ((status = snd_rawmidi_open(NULL, &midioutport, outportname, mode)) < 0)
  {
	  fprintf(stderr, "Problem opening MIDI input: %s",
		  snd_strerror(status));
	  exit(1);
  }

  /* デバイスがすでに別のアプリケーションで占有されている場合に
     -EBUSYエラーを返します */
  snd_rawmidi_nonblock(midiinport, 1);
  snd_rawmidi_nonblock(midioutport, 1);
  /* RawMidiハンドルのポーリング記述子の数を取得します */
  npfds = snd_rawmidi_poll_descriptors_count(midiinport);
  pfds = (struct pollfd *)alloca(npfds * sizeof(struct pollfd));
  /* ポーリング記述子を取得します */
  status = snd_rawmidi_poll_descriptors(midiinport, pfds, POLLIN);

  while (1)
  {
    rawmidithrough(strColorMap);
  }

  return 0;
}

/* RAW MIDI IN -> RAW MIDI OUT */
void rawmidithrough(char strColorMap[256][N])
{

  unsigned char buffer[16];
  unsigned short reevents;
  int err;
  bool isProgramChange = false;

  while (1)
  {

    /* ポーリング記述子から返されたイベントを取得します */
    /* 成功した場合はゼロ、負の場合はエラーコード */
    if ((err = snd_rawmidi_poll_descriptors_revents(midiinport,
     &qfds, npfds, &reevents)) < 0)
    {
      fprintf(stderr, "polling error =%d\n", err);
      exit(1);
    }

    /* MIDIストリームからMIDIバイトを読み取る */
    /* 成功した場合はゼロ、負の場合はエラーコード */
    /* 受信バッファーが空だと-11が返ってくる */
    if ((err = snd_rawmidi_read(midiinport, buffer, 1)) < 0)
    {
      return;
    }
    /* MIDIバイトをMIDIストリームに書き込みます */
    if ((err = snd_rawmidi_write(midioutport,buffer,1)) < 0)
    {
      fprintf(stderr, "raw midi write error =%d\n", err);
      exit(1);
    } else {
		//fprintf(stderr, "02x: %02x \n", buffer[0]);
		//fprintf(stderr, "08X: %08X \n", &buffer[0]);
		//fprintf(stderr, "d: %d \n", (int)buffer[0]);
		//fprintf(stderr, "c: %c \n", (int)buffer[0]);
		//fprintf(stderr, "s: %16s \n", (char*)&buffer[0]);
		//fprintf(stderr, "s: %16s \n", (char*)(intptr_t)(int)buffer[0]);

		//fprintf(stderr, "sizeof: %d \n", sizeof buffer);
		//fprintf(stderr, "sizeof 0: %d \n", sizeof buffer[0]);
		//fprintf(stderr, "sizeof /: %d \n", sizeof buffer / sizeof buffer[0]);

		char strLog[0];
		snprintf(strLog, 100, "%d", buffer[0]);
		//fprintf(stderr, "s: %s \n", strLog);
		
		//for (int i = 0; i < 10; i += 1) {
		//	printf("%d: %08X: %d\n", i, &buffer[i], buffer[i]);
		//}


		// 192 = c0 = program change /MIDI_CMD_PGM_CHANGE
		if (isProgramChange) {
			fprintf(stderr, "Program Change received: %d \n", buffer[0]);



			pid_t pid = fork();
			if (pid < 0) {
				perror("cannot fork process");
				exit(-1);
			}
			else if (pid == 0) {
				char *rgb = strColorMap[(int)buffer[0]];
				fprintf(stderr, "rgb: %s \n", rgb);

				// 子プロセスで別プログラムを実行
				char *path = "/usr/bin/python";
				char *_argv[] = {
						"python",
						"./triggerLED.py",
						rgb,
						//"234,5,6",
						">",
						"/dev/null",
						NULL
				};
				int ret = execv(path, _argv);
				perror("?");
				exit(-1);
			}

			// 親プロセス
			int status;
			pid_t r = waitpid(pid, &status, 0); //子プロセスの終了待ち
			if (r < 0) {
				perror("failed to change LED");
				exit(-1);
			}
			if (WIFEXITED(status)) {
				// 子プロセスが正常終了の場合
				printf("crystal exit-code=%d\n", WEXITSTATUS(status));
			}
			else {
				printf("child status=%04x\n", status);
			}


			fprintf(stdout, "\n");
			//fprintf(stderr, "exec api: %d \n", ret);
		}
	  if (buffer[0] == MIDI_CMD_PGM_CHANGE) {
		  isProgramChange = true;
/*
		  for (int i = 0; i < sizeof(buffer) / sizeof(buffer[0]); ++i) {
			  fprintf(stderr, "looping %x: %02x \n", i, buffer[i]);
		  };
*/
	  }
	  else {
		  isProgramChange = false;
	  }
	  LOG_PRINT("midi message ");
	  LOG_PRINT(strLog);
	  //LOG_PRINT((char*)&buffer[0]);	  
	  //LOG_PRINT((char*)(intptr_t)(int)buffer[0]);
	  LOG_PRINT("");
    }
  }
  /* rawmidi I/O のリングバッファのすべてのバイトを排出します */
  snd_rawmidi_drain(midioutport);
  return;
}


void LOG_PRINT(char log_txt[256], ...)
{

	time_t timer;
	struct tm *date;
	char str[256];

	/* 時間取得 */
	timer = time(NULL);
	date = localtime(&timer);
	//strftime(str, sizeof(str), "[%Y/%M/%D %H:%M:%S] ", date);
	strftime(str, sizeof(str), "[%x %H:%M:%S] ", date);

	if ((log_file = fopen(LOG_FILE, "a")) == NULL) {
		ERR_LOG_PRINT("file open error!!\n");
		exit(EXIT_FAILURE);        /* エラーの場合は通常、異常終了する */
	}

	/* 文字列結合 */
	strcat(str, log_txt);

	fputs(str, log_file);
	fclose(log_file);
	printf("%s\n", log_txt);

	return;

}

void ERR_LOG_PRINT(char err_txt[256], ...)
{
	time_t timer;
	struct tm *date;
	char str[256];

	/* 時間取得 */
	timer = time(NULL);
	date = localtime(&timer);
	strftime(str, sizeof(str), "[%Y/%x %H:%M:%S] ", date);

	if ((err_log_file = fopen("LOG/err_log.txt", "a")) == NULL) {
		printf("ERROR !!\n");
		exit(EXIT_FAILURE);        /* エラーの場合は通常、異常終了する */
	}

	/* 文字列結合 */
	strcat(str, err_txt);

	fputs(str, err_log_file);
	fclose(err_log_file);

	return;

}
