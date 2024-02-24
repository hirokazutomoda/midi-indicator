//
// Description:    Read ALSA Raw MIDI input and write Raw MIDI output
// rawinout.c
//
#include <alsa/asoundlib.h>  /* for alsa interface   */
#include <unistd.h>    /* for sleep() function */
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>

#define TEST_OK  0     /* テスト関数戻り値(正常)*/
#define TEST_NG -1     /* テスト関数戻り値(異常)*/
#define LOG_FILE     "/home/pi/LOG/log.txt"        /* ログディレクトリ(通常)  */
#define ERR_LOG_FILE "/home/pi/LOG/err_log.txt"    /* ログディレクトリ(エラー)*/

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
  int status;
  int mode = SND_RAWMIDI_SYNC;
  const char *inportname = "hw:1,0,0";
  const char *outportname = "hw:1,0,0";

  LOG_PRINT(">>> TEST START !!! \n");

  if(ac>3){
    inportname = av[1];
    outportname = av[1];
  }
  fprintf(stderr,"input poart=%s\n",inportname);
  fprintf(stderr,"output poart=%s\n",outportname);

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
    rawmidithrough();
  }

  return 0;
}

/* RAW MIDI IN -> RAW MIDI OUT */
void rawmidithrough()
{

  unsigned char buffer[16];
  unsigned short reevents;
  int err;

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
      fprintf(stderr,"%02x ",buffer[0]);
	  LOG_PRINT((char *)&buffer[0]);
	  LOG_PRINT("\n");	  
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
	strftime(str, sizeof(str), "[%Y/%x %H:%M:%S] ", date);

	if ((log_file = fopen(LOG_FILE, "a")) == NULL) {
		ERR_LOG_PRINT("file open error!!\n");
		exit(EXIT_FAILURE);        /* エラーの場合は通常、異常終了する */
	}

	/* 文字列結合 */
	strcat(str, log_txt);

	fputs(str, log_file);
	fclose(log_file);

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

