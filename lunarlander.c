#define _XOPEN_SOURCE_EXTENDED 1
#include <curses.h>
#include <time.h>
#include <wchar.h>
#include <ncurses.h>
#include <time.h>
#include <locale.h>
#include <stdlib.h>

//rendering
void stats();
void decideline();
void starsgen();
void printlunar();
void lunargen();
void printstars();

//moving
int moveterrain();
void rocket();
int colldetect();
void orientation();
void printThings(int stars[], int planets[], int fuel, int row, int col, int clock, float time, int y);
int calcAlt(int y);
int calcscore(int clock, float time, int fuel);

//start here

struct Rocket {
  int x;
  int y;
  cchar_t orientation;
  cchar_t thrust;
};

//0  = up
//-1 = left
//1  = right

void orientation(int key, struct Rocket *rocket)
{
  wchar_t head = L' ';
  wchar_t tail = L' ';
  switch(key){
    case 0:
     head = L'\u25B2';
     tail = L'\u25BD';
     break;
    case -1:
     head = L'\u25C0';
     tail = L'\u25B9';
     break;
    case 1:
     head = L'\u25B6';
     tail = L'\u25C3';
    break;
  }
  setcchar(&(rocket->orientation), &head, A_NORMAL, 0, NULL);
  setcchar(&(rocket->thrust), &tail, A_NORMAL, 0, NULL);
  }
  
void moverocket(int x, int y, int cols, int rows, struct Rocket *rocket, int stars[], int moon[], int planets[], int fuel, int clock)
{ 
  rocket->x = x;
  rocket->y = y;
  wchar_t head = 0x25B2; 
  wchar_t tail = 0x0020;
  int drift = 0;
  int j = 1;
  float time = 10;
  int k = 3;
  setcchar(&(rocket->orientation), &head, A_NORMAL, 0, NULL);
  mvadd_wch(rocket->y, rocket->x, &(rocket->orientation));
  refresh();
  while(1)
  { // for altitude
    int t = (j+rocket->x)%600;
    if (t<0)
      t += 600; 
      
    halfdelay(time);
    int ch = getch();
    k = colldetect(rocket->x,rocket->y, k);
    
    if (k==1) {
      if (time <5) {
        printw("Game over! You crashed the lander");
        int score = calcscore(clock, time, fuel);
        mvprintw(rows*0.05, cols*0.01, "score: %d", score);
        cbreak();
        getch();
      }
      else {
        printw("Successful landing!");
        int score = calcscore(clock, time, fuel);
        mvprintw(rows*0.05, cols*0.01, "score: %d", score);
        cbreak();
        getch();
      }
      break;
    }
    else {
    if (k == 2) {
     attron(COLOR_PAIR(2));
    }
    else if (k == 3) {
     attroff(COLOR_PAIR(2));
    }
    clock++;
    if (ch == KEY_RIGHT && fuel>0) {
        fuel -= 3;
        clear();
        orientation(+1, rocket);
        if ((rocket->x) < 90) {
          mvadd_wch((rocket->y), (rocket->x)+1, &(rocket->orientation));
          mvadd_wch((rocket->y), (rocket->x)-1, &(rocket->thrust));
          rocket->x++;
          drift = 1;
          j = moveterrain(moon, 0, j);
          printThings(stars, planets, fuel, rows, cols, clock, time, moon[t] - rocket->y);
        }
        else {
          mvadd_wch((rocket->y), (rocket->x)+1, &(rocket->orientation));
          mvadd_wch((rocket->y), (rocket->x)-1, &(rocket->thrust));
          drift = 1;
          j = moveterrain(moon, +1, j);
          printThings(stars, planets, fuel, rows, cols, clock, time, moon[t] - rocket->y);
        }
      }
      else if (ch == KEY_LEFT && fuel >0) {
          fuel-=3;
          clear();
          orientation(-1, rocket);
          if ((rocket->x) >10) {
            mvadd_wch((rocket->y), (rocket->x)-1, &(rocket->orientation));
            mvadd_wch((rocket->y), (rocket->x)+1, &(rocket->thrust));
            rocket->x = rocket->x-1;
            drift =-1;
            j = moveterrain(moon, 0, j);
            printThings(stars, planets, fuel, rows, cols, clock, time, moon[t] - rocket->y);
        }
        else {
          mvadd_wch((rocket->y), (rocket->x)-1, &(rocket->orientation));
          mvadd_wch((rocket->y), (rocket->x)+1, &(rocket->thrust));
          j = moveterrain(moon, -1, j);
          printThings(stars, planets, fuel, rows, cols, clock, time, moon[t]- rocket->y);
          refresh();
        }
      }
      else if (ch == KEY_UP && fuel >0) {
          fuel-=5;
          clear();
          orientation(0, rocket);
          mvadd_wch((rocket->y)-1, rocket->x, &(rocket->orientation));
          mvadd_wch((rocket->y), (rocket->x), &(rocket->thrust));
          j = moveterrain(moon, 0, j);
          printThings(stars, planets, fuel, rows, cols, clock, time, moon[t] - rocket->y);
          rocket->y--;
          refresh();
          time = 8;
      }
      else {
        if ((rocket->x) > (cols*0.1) && (rocket->x) <60) {
          clear();
          mvadd_wch((rocket->y)+1, (rocket->x) + drift, &(rocket->orientation));
          rocket->y++;
          rocket->x = rocket->x + drift;
          j = moveterrain(moon, 0, j);
          printThings(stars, planets, fuel, rows, cols, clock, time, (moon[t] -rocket->y));
        }
        else if ((rocket->x)<= (cols*0.1)) {
          clear();
          mvadd_wch((rocket->y)+1, (rocket->x) + drift, &(rocket->orientation));
          rocket->y++;
          j = moveterrain(moon, drift, j);
          printThings(stars, planets, fuel, rows, cols, clock, time, (moon[t] -rocket->y));
          refresh();
          j+=drift;
        }
        else if ((rocket->x)>=60){
          clear();
          mvadd_wch((rocket->y)+1, (rocket->x)+drift, &(rocket->orientation));
          rocket->y++;
          j = moveterrain(moon, drift, j);
          printThings(stars, planets, fuel, rows, cols, clock, time, moon[t]-(rocket->y));
          refresh();
          j += drift;
        }
        refresh();
        time = (time)*0.8;
      }
    }
  }
}

//calculates score at the end of the game

int calcscore(int clock, float time, int fuel)
{
  int score = 1000;
  if (time < 5)
    score = 0;
  else if (time >= 5)
    score = score - clock + time + fuel;
  return score;
}


void starsgen(int array[])
{
  int row, col, x,y;
  curs_set(0);
  srand(time(NULL));
  
  getmaxyx(stdscr, row, col);
  for(int j=0; j<30; j+=2){
      y = (rand()%((row/2)));
      x = (rand()%col);
      array[j] = x;
      array[j+1] = y;
  }
}

void printstars(int array[])
{
  for (int j=0; j<30; j+=2) {
    mvaddch(array[j+1],array[j],ACS_BULLET);
    refresh();
  }
}

int moveterrain(int moon[], int k, int j)
{ 
   if (k == -1) {
     j--;
     //clear();
     for (int i = 0; i < 200; i++) {
       int r = (i+j)%600;
       if (r<0)
         r += 600;
       decideline(moon[r], moon[(r+1)%600], i);
     }
   }
   else if (k==1){
     j++;
     //clear();
     for (int i = 0; i < 200; i++) {
       int r = (i+j)%600;
       if (r<0)
         r += 600;
       decideline(moon[r], moon[(r+1)%600], i);
     }
     refresh();
   }
   else {
     for (int i = 0; i < 200; i++) {
       int r = (i+j)%600;
       if (r<0)
         r += 600;
       decideline(moon[r], moon[(r+1)%600], i);
     }
     refresh();
   }
   return j;
}

void lunargen(int array[])
{
  int row, col;
  srand(time(NULL));
  
  getmaxyx(stdscr, row, col);
  int y = row * 0.65;
  
  //generating terrain
  for(int x=0; x<300; x++){
      int r = (rand()%(3));
      if (x%2 == 0)
        y = y + r - 1;
      array[x] = y;
  }
  
  for(int x=0; x<300; x++){
      array[x + 300] = array[300-x];
  }
}

void printlunar(int moon[])
{
  for (int i = 0; i < 200; i++) {
    decideline(moon[i], moon[i+1], i);
  }
  refresh();
}

void decideline(int y1, int y2, int c)
{
  if (y1>y2){
    mvprintw(y2, c, "%c", '/');
  }
  else if (y2>y1)
    mvprintw(y2-1, c, "%c", '\\');
  else
    mvaddch(y2-1, c, '_');
}


int colldetect(int x, int y, int k)
{
  char ch = mvinch(y,x);
  if (ch == '_' || ch == '\\' || ch == '/')
    return 1;
  else if (ch == ('O' & A_CHARTEXT) && k == 2)
    return 3;
  else if (ch == ('O' & A_CHARTEXT) && k ==3)
    return 2;
  else
    return k;
}

void planetgen(int array[])
{
  int row, col, x,y;
  curs_set(0);
  srand(time(NULL));
  
  getmaxyx(stdscr, row, col);
  for(int j=0; j<6; j+=2){
      x = (rand()%(col-30)) + 10;
      array[j] = x;
      array[j+1] = (rand()%10)+5;
  }
}

void planetprint(int array[])
{
  for (int j=0; j<6; j+=2) {
    mvaddch(array[j+1],array[j], 'O');
  }
  refresh();
}

void printThings(int stars[], int planets[] , int fuel, int row, int col, int clock, float time, int y)
{ 
  planetprint(planets);
  attron(A_BLINK);
  printstars(stars);
  attroff(A_BLINK);
  if (fuel <0)
    mvprintw(row*0.03, col*0.01, "Fuel: %d", 0);
  else
    mvprintw(row*0.03, col*0.01, "Fuel: %d", fuel);
  mvprintw(row*0.07, col*0.01, "moves: %d", clock);
  mvprintw(row*0.05, col*0.75, "vertical speed: %.2f", 100/time);
  if (y<6) {
    attron(COLOR_PAIR(1));
    attron(A_BLINK);
    mvprintw(row*0.03, col*0.75, "altitude: DANGER");
    attroff(A_BLINK);
    attroff(COLOR_PAIR(1));
  }
  else {
    attroff(A_BLINK);
    mvprintw(row*0.03, col*0.75, "altitude: %d", y);
  }
  refresh();
}

int main()
{ 
  setlocale(LC_ALL, ""); 
  initscr();
  struct Rocket rocket;
  int y,x, rows, cols;
  getmaxyx(stdscr, rows, cols);
  y = rows*0.1;
  x = cols*0.4;
  int time = 10;
  int moon[600] = {0};
  int stars[60] = {0};
  int planets[6] = {0};
  int fuel = 600;
  int clock = 0;
  
  start_color();
  init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(2, COLOR_CYAN, COLOR_BLACK);
  init_pair(3, 4, COLOR_BLACK);
  
  noecho();
  cbreak();
  keypad(stdscr, TRUE);
  curs_set(0);
  
  mvprintw(rows/2 -2, 10, "Welcome to Lunar Lander!");
  mvprintw(rows/2 -1, 10, "Use the arrow keys to navigate");
  mvprintw(rows/2, 10, "Hit the planets 'O' for a nice surprise, and feel free to explore the landscape");
  mvprintw(rows/2 +1, 10, "...just don't run out of fuel");
  mvprintw(rows/2 +2, 10, "The rocket was a rental, don't crash it please.");
  mvprintw(rows/2 +3, 10, "Press any arrow key to start");
  
  getch();
  clear();
  
  starsgen(stars);
  planetgen(planets);
  planetprint(planets);
  lunargen(moon);
  printThings(stars, planets, fuel, rows, cols, clock, time, y);
  
  moverocket(x, y, cols, rows, &rocket, stars, moon, planets, fuel, clock);
  
  endwin();
  return 0;
}
