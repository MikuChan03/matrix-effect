#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <inttypes.h>
#include "conf.h"

#if !defined( __POSIX ) && !defined( __WIN ) && !defined(__POSIWIX) && !defined(__WINWIN)
static_assert( 0, "We need either __POSIX, __POSIWIX, __WIN or __WINWIN" );
#endif

#if defined(__POSIX) || defined(__POSIWIX)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#if defined(__WIN) || defined(__WINWIN)
#include <windows.h>
#endif

sig_atomic_t keepRunnywun = true;

typedef struct sizeySize {
  int width;
  int height;
} sizeySize;

typedef struct snakeySnake {
  int length;
  int tip;
} snakeySnake;

enum {
  SNAKEY_STATEY_STARTY = 3,
  SNAKEY_STATEY_MIDY = 4,
  SNAKEY_STATEY_MIDY_CLEAN = 5,
  SNAKEY_STATEY_ENDY = 6,
  SNAKEY_STATEY_ENDY_CLEAN = 7,
  SNAKEY_STATEY_PASTY = 0,
  SNAKEY_STATEY_PASTY_CLEAN = 1,
  SNAKEY_STATEY_PASTY_DEATH = 2
};

#if defined(__POSIX) || defined(__POSIWIX)
sizeySize getTermSizeySize() {
  struct winsize w;
  ioctl(STDIN_FILENO, TIOCGWINSZ, &w);
  return (sizeySize){ w.ws_col, w.ws_row };
}

void sleepySleep_ms( int ms ){
  usleep( ms * 1000 );
}
#endif

#if defined(__WIN) || defined(__WINWIN)
sizeySize getTermSizeySize() {
  CONSOLE_SCREEN_BUFFER_INFO buf;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buf);
  return (sizeySize){buf.srWindow.Right - buf.srWindow.Left + 1,
                buf.srWindow.Bottom - buf.srWindow.Top + 1 };
}

void sleepySleep_ms( int ms ){
  Sleep( ms );
}
#endif

long int getNanosecsyNanosecs(){
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  return ts.tv_nsec;
}

int chanceyChance( int percentyPercent ){
           /* 0 - 99 */
  return ( rand() % 100 ) < percentyPercent;
}

char getRandChar(){
  return ( rand() % 94 ) + 33;
}

void userHadEnough(int ja){
  (void)ja;
  keepRunnywun = false;
  signal(SIGINT, SIG_IGN);
}

int main(){
  sizeySize termSize = getTermSizeySize();
  char * str = malloc( 1024 * 1024 ); /* One MwegaByte mowe or less */
  char * strWhite = malloc( 1024 * 1024 ); /* One MwegaByte mowe or less */
  char * field = malloc( termSize.width * termSize.height );
  snakeySnake * snakeySnakes = malloc( termSize.width * termSize.height * sizeof(snakeySnake) );
  int * snakesySnakesInColyCol = calloc( termSize.width, sizeof(int) );
  int * snakesySnakesInColyColIndyInd = calloc( termSize.width, sizeof(int) );
  int * snakeySnakeTipyTipMap = calloc( ( termSize.height * 5 ) + 1, sizeof(int) );
  int * snakeySnakeEndyEndMap = calloc( ( termSize.height * 5 ) + 1, sizeof(int) );

  if( termSize.width == 0 || termSize.height == 0 ){
    fprintf( stderr, "pwease don't wediwect anything, othewise we don't know the tewminal dimensions UwU\n" );
    return 1;
  }

  srand( getNanosecsyNanosecs() );
  signal(SIGINT, userHadEnough);
  printf( "\033[2J\033[?25l" );

  strcpy( str, "\033[38;2;8;202;40m" );
  strcpy( strWhite, "\033[38;2;255;255;255m" );

  snakeySnakeTipyTipMap[0] = SNAKEY_STATEY_STARTY;
  snakeySnakeTipyTipMap[termSize.height] = SNAKEY_STATEY_ENDY;

  for( int i = 1; i < termSize.height; i++ ){
    snakeySnakeTipyTipMap[i] = SNAKEY_STATEY_MIDY;
  }

  snakeySnakeEndyEndMap += ( termSize.height * 4 );
  for( int i = 0; i < termSize.height; i++ ){
    snakeySnakeEndyEndMap[i] = 1;
  }
  snakeySnakeEndyEndMap[termSize.height] = 2;

  while( keepRunnywun ){
    int strInd = sizeof( "\033[38;2;8;202;40m" ) - 1;
    int strWhtInd = sizeof( "\033[38;2;255;255;255m" ) - 1;
    char c;

    for( int i = 0; i < termSize.width; i++ ){
      char * fld = field + ( i * termSize.height );
      snakeySnake * snks = snakeySnakes + ( i * termSize.height );
      int newSnakeInd = (snakesySnakesInColyColIndyInd[i] + snakesySnakesInColyCol[i]) % termSize.height;
      snakeySnake * newSnake = snks + newSnakeInd;
      snakeySnake * highestSnake = NULL;
      int lowestSnakeDied = 0;

      if(newSnakeInd == 0){
        highestSnake = snks + termSize.height - 1;
      }else{
        highestSnake = snks + newSnakeInd - 1;
      }

      if( (snakesySnakesInColyCol[i]==0 || (highestSnake->tip - highestSnake->length >= SNAKES_MIN_GAP))
          && chanceyChance(SNAKES_LIKELIHOOD) ){

        /* both snakeySnakes lengths are inclusive */
        int lengthPercent = (rand() % (SNAKES_LENGTH_MAX - SNAKES_LENGTH_MIN + 1))
                            + SNAKES_LENGTH_MIN;

        newSnake->length = termSize.height * (lengthPercent / 100.0);
        if(newSnake->length < 2){
          newSnake->length = 2;
        }
        newSnake->tip = 0;
        snakesySnakesInColyCol[i]++;
      }

      for( int j = 0; j < snakesySnakesInColyCol[i]; j++ ){
        snakeySnake * currentSnakeySnake = snks + ((snakesySnakesInColyColIndyInd[i] + j) % termSize.height);

        switch(snakeySnakeTipyTipMap[currentSnakeySnake->tip] + snakeySnakeEndyEndMap[currentSnakeySnake->tip - currentSnakeySnake->length]){
          case SNAKEY_STATEY_STARTY:
            c = getRandChar();
            fld[currentSnakeySnake->tip] = c;
            strWhtInd += sprintf(strWhite + strWhtInd, "\033[%i;%iH%c", currentSnakeySnake->tip+1, i+1, c);
          break;

          case SNAKEY_STATEY_MIDY_CLEAN:
            strInd += sprintf(str + strInd, "\033[%i;%iH%c", (currentSnakeySnake->tip - currentSnakeySnake->length)+1, i+1, ' ');
          case SNAKEY_STATEY_MIDY:
            c = getRandChar();
            fld[currentSnakeySnake->tip] = c;
            strWhtInd += sprintf(strWhite + strWhtInd, "\033[%i;%iH%c", currentSnakeySnake->tip+1, i+1, c);
            strInd += sprintf(str + strInd, "\033[%i;%iH%c", currentSnakeySnake->tip, i+1, fld[currentSnakeySnake->tip-1]);
          break;

          case SNAKEY_STATEY_ENDY_CLEAN:
            strInd += sprintf(str + strInd, "\033[%i;%iH%c", (currentSnakeySnake->tip - currentSnakeySnake->length)+1, i+1, ' ');
          case SNAKEY_STATEY_ENDY:
            strInd += sprintf(str + strInd, "\033[%i;%iH%c", currentSnakeySnake->tip, i+1, fld[currentSnakeySnake->tip-1]);
          break;

          case SNAKEY_STATEY_PASTY_CLEAN:
            strInd += sprintf(str + strInd, "\033[%i;%iH%c", (currentSnakeySnake->tip - currentSnakeySnake->length)+1, i+1, ' ');
          break;

          case SNAKEY_STATEY_PASTY_DEATH:
            /* The snakeySnake has finally outlived it's usefulness ... */
            lowestSnakeDied = 1;
          break;
        }

        currentSnakeySnake->tip++;
      }

      if(lowestSnakeDied){
        snakesySnakesInColyColIndyInd[i] = (snakesySnakesInColyColIndyInd[i] + 1) % termSize.height;
        snakesySnakesInColyCol[i]--;
      }
    }

    fwrite( str, 1, strInd, stdout );
    fwrite( strWhite, 1, strWhtInd, stdout );
    fflush( stdout );
    sleepySleep_ms( SNAKES_SPEED );
  }

  printf( "\033[1;1H\033[2J\033[?25h\033[m" );

  free(str);
  free(strWhite);
  free(field);
  free(snakeySnakes);
  free(snakesySnakesInColyCol);
  free(snakesySnakesInColyColIndyInd);
  free(snakeySnakeTipyTipMap);
  free(snakeySnakeEndyEndMap - ( termSize.height * 4 ));
  return 0;
}
