#include "opengl.h"
#include <AL/al.h>
#include "tetris.h"
#include "3ds.h"

extern void DrawModel3DS(Model3DS_t *Model);

extern Model3DS_t Model[3];
extern int SwitchObject;

typedef enum
{
	VBO_SKYBOX,
	VBO_SCREENQUAD,
	VBO_INSTANCEDDATA,

	TEXTURE_REFLECT,
	TEXTURE_SPECULAR,

	TEXTURE_ORIGINAL,
	TEXTURE_ORIGINAL1,
	TEXTURE_ORIGINAL2,
	TEXTURE_ORIGINAL3,
	TEXTURE_ORIGINAL4,
	TEXTURE_ORIGINAL5,
	TEXTURE_ORIGINAL6,
	TEXTURE_ORIGINAL7,
	TEXTURE_BLUR0,
	TEXTURE_BLUR1,

	BUFFER_ORIGINAL,
	BUFFER_BLUR,
	BUFFER_DEPTH,

	GLSL_SKYBOX,
	GLSL_SKYBOX_MVP,
	GLSL_SKYBOX_TEXTURE,

	GLSL_GLASS,
	GLSL_GLASS_MVP,
	GLSL_GLASS_EYE,
	GLSL_GLASS_SKYBOX,

	GLSL_MILKGLASS,
	GLSL_MILKGLASS_MVP,
	GLSL_MILKGLASS_EYE,
	GLSL_MILKGLASS_SKYBOX,

	GLSL_LIGHTING,
	GLSL_LIGHTING_MVP,
	GLSL_LIGHTING_EYE,
	GLSL_LIGHTING_SPECULAR,
	GLSL_LIGHTING_MATRIX,

	GLSL_BLUR,
	GLSL_BLUR_COORDS,
	GLSL_BLUR_DIRECTION,
	GLSL_BLUR_TEXTURE,

	GLSL_COMPOSITE,
	GLSL_COMPOSITE_COORDS,
	GLSL_COMPOSITE_EXPOSURE,
	GLSL_COMPOSITE_ORIGINAL,
	GLSL_COMPOSITE_ORIGINAL1,
	GLSL_COMPOSITE_ORIGINAL2,
	GLSL_COMPOSITE_ORIGINAL3,
	GLSL_COMPOSITE_ORIGINAL4,
	GLSL_COMPOSITE_ORIGINAL5,
	GLSL_COMPOSITE_ORIGINAL6,
	GLSL_COMPOSITE_ORIGINAL7,
	GLSL_COMPOSITE_BLUR,

	SOUND_LINE,
	SOUND_LINE4,
	SOUND_DROP,
	SOUND_MOVE,
	SOUND_ROTATE,
	SOUND_LEVELUP,

	NUM_OBJECTS
} ObjectNames;

extern unsigned int Objects[NUM_OBJECTS];

extern float MusicSpeed;

int randrange(int min, int max);

int DropCount;

void NewAGame(void)
{
	// ****
	Blocks[0][0][0]=-2; Blocks[0][0][1]= 0;
	Blocks[0][1][0]=-1; Blocks[0][1][1]= 0;
	Blocks[0][2][0]= 0; Blocks[0][2][1]= 0;
	Blocks[0][3][0]= 1; Blocks[0][3][1]= 0;

	// ***
	//   *
	Blocks[1][0][0]=-1; Blocks[1][0][1]= 0;
	Blocks[1][1][0]= 0; Blocks[1][1][1]= 0;
	Blocks[1][2][0]= 1; Blocks[1][2][1]= 0;
	Blocks[1][3][0]= 1; Blocks[1][3][1]=-1;

	// ***
	// *
	Blocks[2][0][0]=-1; Blocks[2][0][1]= 0;
	Blocks[2][1][0]= 0; Blocks[2][1][1]= 0;
	Blocks[2][2][0]= 1; Blocks[2][2][1]= 0;
	Blocks[2][3][0]=-1; Blocks[2][3][1]=-1;

	// ***
	//  *
	Blocks[3][0][0]=-1; Blocks[3][0][1]= 0;
	Blocks[3][1][0]= 0; Blocks[3][1][1]= 0;
	Blocks[3][2][0]= 1; Blocks[3][2][1]= 0;
	Blocks[3][3][0]= 0; Blocks[3][3][1]=-1;

	// **
	//  **
	Blocks[4][0][0]=-1; Blocks[4][0][1]= 0;
	Blocks[4][1][0]= 0; Blocks[4][1][1]= 0;
	Blocks[4][2][0]= 0; Blocks[4][2][1]=-1;
	Blocks[4][3][0]= 1; Blocks[4][3][1]=-1;

	//  **
	// **
	Blocks[5][0][0]= 1; Blocks[5][0][1]= 0;
	Blocks[5][1][0]= 0; Blocks[5][1][1]= 0;
	Blocks[5][2][0]= 0; Blocks[5][2][1]=-1;
	Blocks[5][3][0]=-1; Blocks[5][3][1]=-1;

	// **
	// **
	Blocks[6][0][0]=-1; Blocks[6][0][1]= 0;
	Blocks[6][1][0]= 0; Blocks[6][1][1]= 0;
	Blocks[6][2][0]=-1; Blocks[6][2][1]=-1;
	Blocks[6][3][0]= 0; Blocks[6][3][1]=-1;

	Colors[0][0]=1.0f;	Colors[0][1]=0.0f;	Colors[0][2]=0.0f;
	Colors[1][0]=1.0f;	Colors[1][1]=1.0f;	Colors[1][2]=0.0f;
	Colors[2][0]=0.0f;	Colors[2][1]=1.0f;	Colors[2][2]=0.0f;
	Colors[3][0]=0.0f;	Colors[3][1]=1.0f;	Colors[3][2]=1.0f;
	Colors[4][0]=0.0f;	Colors[4][1]=0.0f;	Colors[4][2]=1.0f;
	Colors[5][0]=1.0f;	Colors[5][1]=0.0f;	Colors[5][2]=1.0f;
	Colors[6][0]=1.0f;	Colors[6][1]=1.0f;	Colors[6][2]=1.0f;

	memset(&Table, -1, Xsize*Ysize*sizeof(char));
	memset(&LineElim, 0, Ysize*sizeof(char));

	memcpy(CurrentBlock, Blocks[CurrentBlockType=randrange(0, 6)], sizeof(char)*4*2);
	memcpy(NextBlock, Blocks[NextBlockType=randrange(0, 6)], sizeof(char)*4*2);

	Xpos=Xsize/2;
	Ypos=Ysize-1;
	Speed=10;
	Score=0;
	GameState=1;

	IsDropping=0;

	MusicSpeed=1.0f;
}

void NewBGame(void)
{
	int i;

	NewAGame();

	for(i=0;i<50;i++)
		Table[rand()%Xsize][rand()%(Ysize/2)]=rand()%7;
}

int IsValid(int x, int y)
{
	int i;

	for(i=0;i<4;i++)
	{
		if(!(Table[x+CurrentBlock[i][0]][y+CurrentBlock[i][1]]==-1))
			return 0;

		if(!(x+CurrentBlock[i][0]>=0)||!(x+CurrentBlock[i][0]<=Xsize-1))
			return 0;

		if(!(y+CurrentBlock[i][1]>=0)||!(y+CurrentBlock[i][1]<=Ysize-1))
			return 0;
	}

	return 1;
}

void Rotate(void)
{
	int i, temp;

	switch(GameState)
	{
		case -1:
		case 0:
			break;

		default:
			if(CurrentBlockType==6)
				break;

			for(i=0;i<4;i++)
			{
				temp=CurrentBlock[i][0];
				CurrentBlock[i][0]=-CurrentBlock[i][1];
				CurrentBlock[i][1]=temp;
			}

			if(!IsValid(Xpos, Ypos))
			{
				for(i=0;i<4;i++)
				{
					temp=CurrentBlock[i][1];
					CurrentBlock[i][1]=-CurrentBlock[i][0];
					CurrentBlock[i][0]=temp;
				}
			}
			else
				alSourcePlay(Objects[SOUND_ROTATE]);
			break;
	}
}

void Move(int x)
{
	switch(GameState)
	{
		case -1:
		case 0:
			break;

		default:
			if(IsValid(Xpos+x, Ypos))
			{
				Xpos+=x;
				alSourcePlay(Objects[SOUND_MOVE]);
			}
			break;
	}
}

void Place(void)
{
	switch(GameState)
	{
		case -1:
		case 0:
			break;

		default:
			while(IsValid(Xpos, Ypos-1))
				Ypos--;
			break;
	}
}

int ClearLines(void)
{
	int x, y, i, lines=0;

	memcpy(&OldTable, &Table, Xsize*Ysize);

	for(y=Ysize-1;y>=0;y--)
	{
		for(x=0;x<Xsize;x++)
		{
			if(Table[x][y]==-1)
				break;
		}

		if(x==Xsize)
		{
			LineElim[y]=1;
			lines++;

			for(i=y;i<Ysize;i++)
			{
				for(x=0;x<Xsize;x++)
					Table[x][i]=Table[x][i+1];
			}

			for(x=0;x<Xsize;x++)
				Table[x][Ysize-1]=-1;
		}
	}

	return lines;
}

void RunStep(void)
{
	switch(GameState)
	{
		case -1:
		case 0:
			break;

		case 1:
			DropCount++;

			if(IsDropping||DropCount==Speed)
			{
				DropCount=0;

				if(!IsValid(Xpos, Ypos-1))
				{
					int OldScore=Score;

					Table[Xpos+CurrentBlock[0][0]][Ypos+CurrentBlock[0][1]]=CurrentBlockType;
					Table[Xpos+CurrentBlock[1][0]][Ypos+CurrentBlock[1][1]]=CurrentBlockType;
					Table[Xpos+CurrentBlock[2][0]][Ypos+CurrentBlock[2][1]]=CurrentBlockType;
					Table[Xpos+CurrentBlock[3][0]][Ypos+CurrentBlock[3][1]]=CurrentBlockType;

					Xpos=Xsize/2;
					Ypos=Ysize-1;

					memcpy(CurrentBlock, Blocks[CurrentBlockType=NextBlockType], sizeof(char)*4*2);
					memcpy(NextBlock, Blocks[NextBlockType=randrange(0, 6)], sizeof(char)*4*2);

					switch(ClearLines())
					{
						case 1:
							Score+=100;
							alSourcePlay(Objects[SOUND_LINE]);
							break;

						case 2:
							Score+=400;
							alSourcePlay(Objects[SOUND_LINE]);
							break;

						case 3:
							Score+=900;
							alSourcePlay(Objects[SOUND_LINE]);
							break;

						case 4:
							Score+=2000;
							alSourcePlay(Objects[SOUND_LINE4]);
							break;

						default:
							alSourcePlay(Objects[SOUND_DROP]);
							break;
					}

					if(OldScore/2000!=Score/2000)
					{
						if(Speed>1)
						{
							Speed--;
							MusicSpeed+=0.025f;

							alSourcePlay(Objects[SOUND_LEVELUP]);
						}
					}

					if(!IsValid(Xpos, Ypos))
						GameState=-1;
				}
				else
					Ypos--;
			}
			break;
	}
}