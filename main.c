
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <inttypes.h>
#include "conf.h"

#if !defined( __POSIX ) && !defined( __WIN )
static_assert( 0, "We need either __POSIX or __WIN" );
#endif

#ifdef __POSIX
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#ifdef __WIN
#include <windows.h>
#endif

sig_atomic_t keepRunning = true;

typedef struct size {
  int width;
  int height;
} size;

typedef struct snake {
  int length;
  int tip;
} snake;

enum {
  SNAKE_STATE_START = 3,
  SNAKE_STATE_MID = 4,
  SNAKE_STATE_MID_CLEAN = 5,
  SNAKE_STATE_END = 6,
  SNAKE_STATE_END_CLEAN = 7,
  SNAKE_STATE_PAST = 0,
  SNAKE_STATE_PAST_CLEAN = 1,
  SNAKE_STATE_PAST_DEATH = 2
};

#ifdef __POSIX
size getTermSize() {
  struct winsize w;
  ioctl(STDIN_FILENO, TIOCGWINSZ, &w);
  return (size){ w.ws_col, w.ws_row };
}

void sleep_ms( int ms ){
  usleep( ms * 1000 );
}
#endif

#ifdef __WIN
size getTermSize() {
  CONSOLE_SCREEN_BUFFER_INFO buf;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buf);
  return (size){buf.srWindow.Right - buf.srWindow.Left + 1,
                buf.srWindow.Bottom - buf.srWindow.Top + 1 };
}

void sleep_ms( int ms ){
  Sleep( ms );
}
#endif

long int getNanosecs(){
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  return ts.tv_nsec;
}

int chance( int percent ){
           /* 0 - 99 */
  return ( rand() % 100 ) < percent;
}

char getRandChar(){
  return ( rand() % 94 ) + 33;
}

void userHadEnough(int ja){
  (void)ja;
  keepRunning = false;
  signal(SIGINT, SIG_IGN);
}

int main(){
  size termSize = getTermSize();
  char * str = malloc( 1024 * 1024 ); /* One Megabyte more or less */
  char * strWhite = malloc( 1024 * 1024 ); /* One Megabyte more or less */
  char * field = malloc( termSize.width * termSize.height );
  snake * snakes = malloc( termSize.width * termSize.height * sizeof(snake) );
  int * snakesInCol = calloc( termSize.width, sizeof(int) );
  int * snakesInColInd = calloc( termSize.width, sizeof(int) );
  int * snakeTipMap = calloc( ( termSize.height * 5 ) + 1, sizeof(int) );
  int * snakeEndMap = calloc( ( termSize.height * 5 ) + 1, sizeof(int) );

  if( termSize.width == 0 || termSize.height == 0 ){
    fprintf( stderr, "pwease don't wediwect anything, othewise we don't know the tewminal dimensions UwU\n" );
    return 1;
  }

  srand( getNanosecs() );
  signal(SIGINT, userHadEnough);
  printf( "\033[2J\033[?25l" );

  strcpy( str, "\033[38;2;8;202;40m" );
  strcpy( strWhite, "\033[38;2;255;255;255m" );

  snakeTipMap[0] = SNAKE_STATE_START;
  snakeTipMap[termSize.height] = SNAKE_STATE_END;

  for( int i = 1; i < termSize.height; i++ ){
    snakeTipMap[i] = SNAKE_STATE_MID;
  }

  snakeEndMap += ( termSize.height * 4 );
  for( int i = 0; i < termSize.height; i++ ){
    snakeEndMap[i] = 1;
  }
  snakeEndMap[termSize.height] = 2;

  while( keepRunning ){
    int strInd = sizeof( "\033[38;2;8;202;40m" ) - 1;
    int strWhtInd = sizeof( "\033[38;2;255;255;255m" ) - 1;
    char c;

    for( int i = 0; i < termSize.width; i++ ){
      char * fld = field + ( i * termSize.height );
      snake * snks = snakes + ( i * termSize.height );
      int newSnakeInd = (snakesInColInd[i] + snakesInCol[i]) % termSize.height;
      snake * newSnake = snks + newSnakeInd;
      snake * highestSnake = NULL;
      int lowestSnakeDied = 0;

      if(newSnakeInd == 0){
        highestSnake = snks + termSize.height - 1;
      }else{
        highestSnake = snks + newSnakeInd - 1;
      }

      if( (snakesInCol[i]==0 || (highestSnake->tip - highestSnake->length >= SNAKES_MIN_GAP))
          && chance(SNAKES_LIKELIHOOD) ){

        /* both snakes lengths are inclusive */
        int lengthPercent = (rand() % (SNAKES_LENGTH_MAX - SNAKES_LENGTH_MIN + 1))
                            + SNAKES_LENGTH_MIN;

        newSnake->length = termSize.height * (lengthPercent / 100.0);
        if(newSnake->length < 2){
          newSnake->length = 2;
        }
        newSnake->tip = 0;
        snakesInCol[i]++;
      }

      for( int j = 0; j < snakesInCol[i]; j++ ){
        snake * currentSnake = snks + ((snakesInColInd[i] + j) % termSize.height);

        switch(snakeTipMap[currentSnake->tip] + snakeEndMap[currentSnake->tip - currentSnake->length]){
          case SNAKE_STATE_START:
            c = getRandChar();
            fld[currentSnake->tip] = c;
            strWhtInd += sprintf(strWhite + strWhtInd, "\033[%i;%iH%c", currentSnake->tip+1, i+1, c);
          break;

          case SNAKE_STATE_MID_CLEAN:
            strInd += sprintf(str + strInd, "\033[%i;%iH%c", (currentSnake->tip - currentSnake->length)+1, i+1, ' ');
          case SNAKE_STATE_MID:
            c = getRandChar();
            fld[currentSnake->tip] = c;
            strWhtInd += sprintf(strWhite + strWhtInd, "\033[%i;%iH%c", currentSnake->tip+1, i+1, c);
            strInd += sprintf(str + strInd, "\033[%i;%iH%c", currentSnake->tip, i+1, fld[currentSnake->tip-1]);
          break;

          case SNAKE_STATE_END_CLEAN:
            strInd += sprintf(str + strInd, "\033[%i;%iH%c", (currentSnake->tip - currentSnake->length)+1, i+1, ' ');
          case SNAKE_STATE_END:
            strInd += sprintf(str + strInd, "\033[%i;%iH%c", currentSnake->tip, i+1, fld[currentSnake->tip-1]);
          break;

          case SNAKE_STATE_PAST_CLEAN:
            strInd += sprintf(str + strInd, "\033[%i;%iH%c", (currentSnake->tip - currentSnake->length)+1, i+1, ' ');
          break;

          case SNAKE_STATE_PAST_DEATH:
            /* The snake has finally outlived it's usefulness ... */
            lowestSnakeDied = 1;
          break;
        }

        currentSnake->tip++;
      }

      if(lowestSnakeDied){
        snakesInColInd[i] = (snakesInColInd[i] + 1) % termSize.height;
        snakesInCol[i]--;
      }
    }

    fwrite( str, 1, strInd, stdout );
    fwrite( strWhite, 1, strWhtInd, stdout );
    fflush( stdout );
    sleep_ms( SNAKES_SPEED );
  }

  printf( "\033[1;1H\033[2J\033[?25h\033[m" );

  free(str);
  free(strWhite);
  free(field);
  free(snakes);
  free(snakesInCol);
  free(snakesInColInd);
  free(snakeTipMap);
  free(snakeEndMap - ( termSize.height * 4 ));
  return 0;
}


