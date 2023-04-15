#ifndef __TETRIS_H__
#define __TETRIS_H__

#define Xsize 10
#define Ysize 20

char Blocks[7][4][2];
float Colors[7][4];

char Table[Xsize][Ysize];
char OldTable[Xsize][Ysize];
char LineElim[Ysize];

int CurrentBlockType, NextBlockType;
char CurrentBlock[4][2], NextBlock[4][2];

int Xpos, Ypos;
int Speed, Score;
int GameState;
int IsDropping;

void NewAGame(void);
void NewBGame(void);
void DrawCurrentBlock(int x, int y);
void DrawNextBlock(int x, int y);
void DrawTable(void);
int IsValid(int x, int y);
void Rotate(void);
void Move(int x);
void Place(void);
int ClearLines(void);
void RunStep(void);

#endif