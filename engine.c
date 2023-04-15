#include <windows.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "opengl.h"
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math.h"
#include <malloc.h>
#include "image.h"
#include "font.h"
#include "3ds.h"
#include "Tremor/ivorbisfile.h"
#include "tetris.h"

#ifndef FREE
#define FREE(p) { if(p) { free(p); p=NULL; } }
#endif

#ifndef BUFFER_OFFSET
#define BUFFER_OFFSET(x) ((char *)NULL+(x))
#endif

#ifdef __GNUC__
#ifndef __int64
#define __int64 long long
#endif
#endif

GLContext_t Context;

ALCcontext *ALContext=NULL;
ALCdevice *ALDevice=NULL;

char *szAppName="HDR Tetris";

int Width=480, Height=640;
int WidthP2, HeightP2;

int Done=0, Key[256];
int SwitchMode=0, SwitchObject=0;
int ToggleMusic=1, ShowFPS=0;
char shader[256];

unsigned __int64 Frequency, StartTime, EndTime;
float FPS, FPSTemp=0.0f, fTimeStep, fTetris=0.0f, fTime=0.0f, fPartTime=0.0f;
int Frames=0;
const float OneOver15=1.0f/15.0f, OneOver30=1.0f/30.0f;

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

unsigned int Objects[NUM_OBJECTS];

float RotateX, RotateY, PanX, PanY, Zoom=-300.0f;

float Projection[16], ModelView[16], MVIT[16], MVP[16];
float QuatX[4], QuatY[4], Quat[4];

Model3DS_t Model[4];

float Exposure=5.0f;

float Matrix[12][4];

typedef struct
{
	int active;
	float life, ttd;
	float pos[3], vel[3], color[3];
} Particle_t;

float PartGrav[3]={ 0.0f, -0.8f, 0.0f };

Particle_t particle[4*Xsize];

float instanced[6*Xsize*Ysize];

typedef struct
{
	char String[256];
} String_t;

String_t *MusicList=NULL, *SceneList=NULL;
int NumMusic=0, NumScene=0;
int CurrentScene=0;
int CurrentMusic=0;
float MusicSpeed=1.0f;

#define BUFFER_SIZE (4096*8)

FILE *oggFile;
OggVorbis_File oggStream;
vorbis_info *oggInfo=NULL;

unsigned int MusicBuffer[2];
unsigned int MusicSource;

void Render(void);
int Init(void);
unsigned int CreateTextureObject(unsigned int Target, unsigned int InternalFormat, unsigned int FilterMode, unsigned int WrapMode, int Width, int Height);
void LoadScene(char *Filename);
GLuint CreateProgram(char *VertexFilename, char *FragmentFilename);

int randrange(int min, int max)
{
	return (int)(((float)rand()/RAND_MAX)*(max+1)+min);
}

int InitSound(void)
{
	if((ALDevice=alcOpenDevice(NULL))==NULL)
	{
		MessageBox(NULL, "alcOpenDevice failed.", "Error", MB_OK);
		return 0;
	}

	if((ALContext=alcCreateContext(ALDevice, NULL))==NULL)
	{
		MessageBox(NULL, "alcCreateContext failed.", "Error", MB_OK);
		return 0;
	}


	alcMakeContextCurrent(ALContext);

	return 1;
}

unsigned int LoadStaticSound(char *Filename, int Loop)
{
	unsigned int BufferID, SoundID, ALformat;
	FILE *stream=NULL;
	unsigned long riff, wave, fmt, data;
	unsigned short format;
	unsigned short channels;
	unsigned long frequency;
	unsigned short sample;
	unsigned long length;
	unsigned char *buffer=NULL;

	if((stream=fopen(Filename, "rb"))==NULL)
		return 0;

	fread(&riff, sizeof(unsigned long), 1, stream);
	fseek(stream, sizeof(unsigned long), SEEK_CUR);
	fread(&wave, sizeof(unsigned long), 1, stream);
	fread(&fmt, sizeof(unsigned long), 1, stream);
	fseek(stream, sizeof(unsigned long), SEEK_CUR);

	if(riff!=0x46464952||wave!=0x45564157||fmt!=0x20746d66)
	{
		fclose(stream);
		return 0;
	}

	fread(&format, sizeof(unsigned short), 1, stream);

	if(format!=1)
	{
		// Only support PCM streams
		fclose(stream);
		return 0;
	}

	fread(&channels, sizeof(unsigned short), 1, stream);
	fread(&frequency, sizeof(unsigned long), 1, stream);
	fseek(stream, sizeof(unsigned long), SEEK_CUR);
	fseek(stream, sizeof(unsigned short), SEEK_CUR);
	fread(&sample, sizeof(unsigned short), 1, stream);

	if(channels==2)
	{
		if(sample==16)
			ALformat=AL_FORMAT_STEREO16;
		else
			ALformat=AL_FORMAT_STEREO8;
	}
	else
	{
		if(sample==16)
			ALformat=AL_FORMAT_MONO16;
		else
			ALformat=AL_FORMAT_MONO8;
	}

	fread(&data, sizeof(unsigned long), 1, stream);

	if(data!=0x61746164)
	{
		fclose(stream);
		return 0;
	}

	fread(&length, sizeof(unsigned long), 1, stream);

	buffer=(unsigned char *)malloc(length);

	if(buffer==NULL)
	{
		fclose(stream);
		return 0;
	}

	fread(buffer, sizeof(unsigned char), length, stream);

	alGenBuffers(1, &BufferID);
	alBufferData(BufferID, ALformat, buffer, length, frequency);

	FREE(buffer);

	alGenSources(1, &SoundID);
	alSourcef(SoundID, AL_PITCH, 1.0f);
	alSourcei(SoundID, AL_LOOPING, Loop);
	alSourcei(SoundID, AL_BUFFER, BufferID);

	return SoundID;
}

int Stream(unsigned int buffer)
{
	char data[BUFFER_SIZE];
	int size=0;

	while(size<BUFFER_SIZE)
	{
		int section, result;

		result=ov_read(&oggStream, data+size, BUFFER_SIZE-size, &section);

		if(result>0)
			size+=result;
		else
			break;
	}

	if(size==0)
		return 0;

	alBufferData(buffer, oggInfo->channels==1?AL_FORMAT_MONO16:AL_FORMAT_STEREO16, data, size, oggInfo->rate);

	return 1;
}

int Update(void)
{
	int processed;
	int active=1;

	if(ToggleMusic)
	{
		ALenum state;

		alGetSourcei(MusicSource, AL_SOURCE_STATE, &state);

		if(state!=AL_PLAYING)
			alSourcePlay(MusicSource);
	}

	alGetSourcei(MusicSource, AL_BUFFERS_PROCESSED, &processed);

	while(processed--)
	{
		unsigned int buffer;

		alSourceUnqueueBuffers(MusicSource, 1, &buffer);
		active=Stream(buffer);
		alSourceQueueBuffers(MusicSource, 1, &buffer);
	}

	return active;
}

int Playback(void)
{
	ALenum state;

	alGetSourcei(MusicSource, AL_SOURCE_STATE, &state);

	if(state==AL_PLAYING)
		return 1;

	if(!Stream(MusicBuffer[0]))
		return 0;

	if(!Stream(MusicBuffer[1]))
		return 0;

	alSourceQueueBuffers(MusicSource, 2, MusicBuffer);
	alSourcePlay(MusicSource);

	return 1;
}

int MusicOpen(char *path)
{
	int result;

	if(!(oggFile=fopen(path, "rb")))
		return 0;

	if((result=ov_open(oggFile, &oggStream, NULL, 0))<0)
	{
		fclose(oggFile);
		return 0;
	}

	oggInfo=ov_info(&oggStream, -1);

	alGenBuffers(2, MusicBuffer);
	alGenSources(1, &MusicSource);
	alSourcef(MusicSource, AL_PITCH, 1.0f);

	return 1;
}

void MusicClose(void)
{
	if(oggFile!=NULL)
	{
		alSourceStop(MusicSource);

		ov_clear(&oggStream);

		fclose(oggFile);
		oggFile=NULL;

		alDeleteSources(1, &MusicSource);
		alDeleteBuffers(2, MusicBuffer);
	}
}

String_t *BuildFileList(char *DirName, char *Filter, int *NumFiles)
{
	HANDLE hList;
	char szDir[MAX_PATH+1];
	WIN32_FIND_DATA FileData;
	String_t *Ret=NULL;

	sprintf(szDir, "%s\\*", DirName);

	if((hList=FindFirstFile(szDir, &FileData))==INVALID_HANDLE_VALUE)
		return NULL;

	for(;;)
	{
		if(!(FileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{
			char *ptr=strrchr(FileData.cFileName, '.');

			if(ptr!=NULL)
			{
				if(!strcmp(ptr, Filter))
				{
					if(Ret==NULL)
						Ret=(String_t *)malloc(sizeof(String_t));

					Ret=(String_t *)realloc(Ret, sizeof(String_t)*(*NumFiles+1));
					sprintf(Ret[(*NumFiles)++].String, "%s", FileData.cFileName);
				}
			}
		}

		if(!FindNextFile(hList, &FileData))
		{
			if(GetLastError()==ERROR_NO_MORE_FILES)
				break;
		}
	}

	FindClose(hList);

	return Ret;
}

unsigned __int64 rdtsc(void)
{
	unsigned long l, h;

#ifdef __GNUC__
	__asm__ __volatile__ ("rdtsc" : "=a" (l), "=d" (h));
#else
	__asm
	{
		rdtsc
		mov l, eax
		mov h, edx
	}
#endif

	return ((unsigned __int64)l|((unsigned __int64)h<<32));
}

unsigned __int64 GetFrequency(void)
{
	unsigned __int64 TimeStart, TimeStop, TimeFreq;
	unsigned __int64 StartTicks, StopTicks;
	volatile unsigned __int64 i;

	QueryPerformanceFrequency((LARGE_INTEGER *)&TimeFreq);

	QueryPerformanceCounter((LARGE_INTEGER *)&TimeStart);
	StartTicks=rdtsc();

	for(i=0;i<1000000;i++);

	StopTicks=rdtsc();
	QueryPerformanceCounter((LARGE_INTEGER *)&TimeStop);

	return (StopTicks-StartTicks)*TimeFreq/(TimeStop-TimeStart);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static POINT old;
	POINT pos, delta;

	switch(uMsg)
	{
		case WM_CREATE:
			break;

		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_DESTROY:
			break;

		case WM_SIZE:
			Width=LOWORD(lParam);
			Height=HIWORD(lParam);
			break;

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			SetCapture(hWnd);
			ShowCursor(FALSE);

			GetCursorPos(&pos);
			old.x=pos.x;
			old.y=pos.y;
			break;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			ShowCursor(TRUE);
			ReleaseCapture();
			break;

		case WM_MOUSEMOVE:
			GetCursorPos(&pos);

			if(!wParam)
			{
				old.x=pos.x;
				old.y=pos.y;
				break;
			}

			delta.x=pos.x-old.x;
			delta.y=old.y-pos.y;

			if(!delta.x&&!delta.y)
				break;

			SetCursorPos(old.x, old.y);

			switch(wParam)
			{
				case MK_LBUTTON:
					RotateX+=delta.x;
					RotateY-=delta.y;
					break;

				case MK_MBUTTON:
					PanX+=delta.x;
					PanY+=delta.y;
					break;

				case MK_RBUTTON:
					Zoom+=delta.y;
					break;
			}
			break;

		case WM_KEYDOWN:
			Key[wParam]=TRUE;

			switch(wParam)
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

				case VK_PAUSE:
				case 'P':
					if(GameState!=-1)
						GameState^=1;
					break;

				case 'N':
					NewAGame();
					break;

				case 'B':
					NewBGame();
					break;

				case 'A':
					Exposure+=0.125f;
					break;

				case 'Z':
					Exposure-=0.125f;
					break;

				case 'M':
					ToggleMusic^=1;
					break;

				case '1':
					SwitchObject++;

					if(SwitchObject>2)
						SwitchObject=0;
					break;

				case '2':
					SwitchMode++;

					if(SwitchMode>2)
						SwitchMode=0;
					break;

				case VK_PRIOR:
					CurrentScene++;

					if(CurrentScene>(NumScene-1))
						CurrentScene=0;

					LoadScene(SceneList[CurrentScene].String);
					break;

				case VK_NEXT:
					CurrentScene--;

					if(CurrentScene<0)
						CurrentScene=NumScene-1;

					LoadScene(SceneList[CurrentScene].String);
					break;

				case VK_HOME:
					if(MusicList)
					{
						MusicClose();

						CurrentMusic++;

						if(CurrentMusic>(NumMusic-1))
							CurrentMusic=0;

						if(MusicOpen(MusicList[CurrentMusic].String))
							Playback();
					}
					break;

				case VK_END:
					if(MusicList)
					{
						MusicClose();

						CurrentMusic--;

						if(CurrentMusic<0)
							CurrentMusic=NumMusic-1;

						if(MusicOpen(MusicList[CurrentMusic].String))
							Playback();
					}
					break;

				case VK_UP:
					Rotate();
					break;

				case VK_DOWN:
					IsDropping=1;
					break;

				case VK_LEFT:
					Move(-1);
					break;

				case VK_RIGHT:
					Move(1);
					break;

				case VK_SPACE:
					IsDropping=1;
					Place();
					RunStep();
					IsDropping=0;
					break;

				default:
					break;
			}
			break;

		case WM_KEYUP:
			Key[wParam]=FALSE;

			switch(wParam)
			{
				case VK_DOWN:
					IsDropping=0;
					break;
			}
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	WNDCLASS wc;
	MSG msg;
	RECT Rect;

	srand(_getpid());

	memset(Objects, 0, NUM_OBJECTS);

	wc.style=CS_VREDRAW|CS_HREDRAW|CS_OWNDC;
	wc.lpfnWndProc=WndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInstance;
	wc.hIcon=LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor=LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground=GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName=NULL;
	wc.lpszClassName=szAppName;

	RegisterClass(&wc);

	Height=GetSystemMetrics(SM_CYSCREEN)-50;
	Width=Height/4*3;

	WidthP2=NextPower2(Width);
	HeightP2=NextPower2(Height);

	SetRect(&Rect, 0, 0, Width, Height);
	AdjustWindowRect(&Rect, WS_SYSMENU|WS_CAPTION|WS_MINIMIZEBOX|WS_OVERLAPPED|WS_CLIPSIBLINGS, FALSE);

	Context.hWnd=CreateWindow(szAppName, szAppName, WS_SYSMENU|WS_CAPTION|WS_MINIMIZEBOX|WS_OVERLAPPED|WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, Rect.right-Rect.left, Rect.bottom-Rect.top, NULL, NULL, hInstance, NULL);

	ShowWindow(Context.hWnd, SW_SHOW);
	SetForegroundWindow(Context.hWnd);

	SceneList=BuildFileList(".", ".scn", &NumScene);

	if(SceneList==NULL)
	{
		MessageBox(NULL, "Could not find any scenes.", "Error", MB_OK);
		return -1;
	}

	CurrentScene=randrange(0, NumScene-1);

	if(!InitSound())
		return -1;

	MusicList=BuildFileList(".", ".ogg", &NumMusic);

	if(MusicList)
	{
		CurrentMusic=randrange(0, NumMusic-1);

		if(MusicOpen(MusicList[CurrentMusic].String))
			Playback();
	}

	Objects[SOUND_LINE]=LoadStaticSound("line.wav", 0);
	Objects[SOUND_LINE4]=LoadStaticSound("line4.wav", 0);
	Objects[SOUND_DROP]=LoadStaticSound("drop.wav", 0);
	Objects[SOUND_MOVE]=LoadStaticSound("move.wav", 0);
	Objects[SOUND_ROTATE]=LoadStaticSound("rotate.wav", 0);
	Objects[SOUND_LEVELUP]=LoadStaticSound("levelup.wav", 0);

	if(!CreateContext(&Context, 32, 24, 0, 0, OGL_DOUBLEBUFFER|OGL_CORE33))
	{
		DestroyWindow(Context.hWnd);

		return -1;
	}

	if(!Init())
	{
		DestroyContext(&Context);
		DestroyWindow(Context.hWnd);

		return -1;
	}

	NewAGame();

	Frequency=GetFrequency();
	StartTime=rdtsc();

	while(!Done)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message==WM_QUIT)
				Done=1;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			StartTime=EndTime;
			EndTime=rdtsc();

			if(MusicList)
			{
				if(!Update())
				{
					MusicClose();

					if(MusicOpen(MusicList[randrange(0, NumMusic-1)].String))
						Playback();
				}
			}

			fTimeStep=(float)(EndTime-StartTime)/Frequency;
			fTime+=fTimeStep;
			fTetris+=fTimeStep;
			fPartTime+=fTimeStep;

			/* Average 100 framerate samples */
			FPSTemp+=1.0f/fTimeStep;

			if(Frames++>100)
			{
				FPS=FPSTemp/Frames;
				FPSTemp=0.0f;
				Frames=0;
			}
			/* --- */

			if(fTetris>OneOver15)
			{
				RunStep();
				fTetris=0.0f;
			}

			alSourcef(MusicSource, AL_PITCH, MusicSpeed);

			Render();
			SwapBuffers(Context.hDC);
		}
	}

	Free3DS(&Model[0]);
	Free3DS(&Model[1]);
	Free3DS(&Model[2]);
	Free3DS(&Model[3]);

	DestroyContext(&Context);
	DestroyWindow(Context.hWnd);

	FREE(SceneList);

	if(MusicList)
		MusicClose();

	FREE(MusicList);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(ALContext);
	alcCloseDevice(ALDevice);

	return 0;
}

void InitParticles(void)
{
	int i;

	memset(particle, 0, sizeof(Particle_t)*Xsize*4);

	for(i=0;i<Xsize*4;i++)
	{
		particle[i].active=0;
		particle[i].life=-1.0f;
	}
}

void RunParticles(void)
{
	int i;

	for(i=0;i<Xsize*4;i++)
	{
		if(particle[i].active)
		{
			particle[i].pos[0]+=particle[i].vel[0]*(fTimeStep*8);
			particle[i].pos[1]+=particle[i].vel[1]*(fTimeStep*8);
			particle[i].pos[2]+=particle[i].vel[2]*(fTimeStep*8);

			particle[i].vel[0]+=PartGrav[0];
			particle[i].vel[1]+=PartGrav[1];
			particle[i].vel[2]+=PartGrav[2];

			particle[i].life-=particle[i].ttd;

			if(particle[i].life<0.0f)
				particle[i].active=0;
		}
    }
}

void DrawModel3DS(Model3DS_t *Model)
{
	int i;

	for(i=0;i<Model->NumMesh;i++)
	{
		glBindBuffer(GL_ARRAY_BUFFER, Model->Mesh[i].VertID);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*8, BUFFER_OFFSET(sizeof(float)*0));	//Vertex
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, BUFFER_OFFSET(sizeof(float)*4));	//NORMAL
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Model->Mesh[i].ElemID);
		glDrawElements(GL_TRIANGLES, Model->Mesh[i].NumFace*3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DrawModel3DSInstanced(Model3DS_t *Model, int count)
{
	int i;

	for(i=0;i<Model->NumMesh;i++)
	{
		glBindBuffer(GL_ARRAY_BUFFER, Model->Mesh[i].VertID);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*8, BUFFER_OFFSET(sizeof(float)*0));	//Vertex
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, BUFFER_OFFSET(sizeof(float)*4));	//NORMAL
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, Objects[VBO_INSTANCEDDATA]);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, BUFFER_OFFSET(0));
		glVertexAttribDivisor(2, 1);
		glEnableVertexAttribArray(2);

		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, BUFFER_OFFSET(sizeof(float)*3));
		glVertexAttribDivisor(3, 1);
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Model->Mesh[i].ElemID);
		glDrawElementsInstanced(GL_TRIANGLES, Model->Mesh[i].NumFace*3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0), count);

		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BuildVBO3DS(Model3DS_t *Model)
{
	int i, j;

	for(i=0;i<Model->NumMesh;i++)
	{
		void *data=NULL;
		float *fPtr=NULL;
		unsigned short *sPtr=NULL;

		if(!Model->Mesh[i].NumVertex)
			continue;

		glGenBuffers(1, &Model->Mesh[i].VertID);
		glBindBuffer(GL_ARRAY_BUFFER, Model->Mesh[i].VertID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*Model->Mesh[i].NumVertex*8, NULL, GL_STATIC_DRAW);

		if(glGetError()==GL_NO_ERROR)
			data=glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

		if(data==NULL)
		{
			char temp[1024];

			sprintf(temp, "VBO data buffer memory map failed or out of memory for object: %s", Model->Mesh[i].Name);
			MessageBox(NULL, temp, "Error", MB_OK);

			glDeleteBuffers(1, &Model->Mesh[i].VertID);
			Model->Mesh[i].VertID=0;
			break;
		}

		for(j=0, fPtr=(float *)data;j<Model->Mesh[i].NumVertex;j++)
		{
			*fPtr++=Model->Mesh[i].Vertex[3*j+0];
			*fPtr++=Model->Mesh[i].Vertex[3*j+1];
			*fPtr++=Model->Mesh[i].Vertex[3*j+2];
			*fPtr++=1.0f; // Padding
			*fPtr++=Model->Mesh[i].Normal[3*j+0];
			*fPtr++=Model->Mesh[i].Normal[3*j+1];
			*fPtr++=Model->Mesh[i].Normal[3*j+2];
			*fPtr++=1.0; // Padding
		}

		glUnmapBuffer(GL_ARRAY_BUFFER);

		glGenBuffers(1, &Model->Mesh[i].ElemID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Model->Mesh[i].ElemID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*Model->Mesh[i].NumFace*3, Model->Mesh[i].Face, GL_STATIC_DRAW);
	}
}

void DrawSkyBox(void)
{
	glBindBuffer(GL_ARRAY_BUFFER, Objects[VBO_SKYBOX]);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 12*3);

	glDisableVertexAttribArray(0);
}

void DrawScreenQuad(void)
{
	glBindBuffer(GL_ARRAY_BUFFER, Objects[VBO_SCREENQUAD]);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 2*3);

	glDisableVertexAttribArray(0);
}

void Blur(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, Objects[BUFFER_BLUR]);

	// Blur
	glUseProgram(Objects[GLSL_BLUR]);

	// Multiplier for generating texture coords from vertex position
	glUniform2f(Objects[GLSL_BLUR_COORDS], (float)(Width<<2)/WidthP2, (float)(Height<<2)/HeightP2);

	// X
	glUniform2f(Objects[GLSL_BLUR_DIRECTION], 1.0f/(WidthP2>>2), 0.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL]);

	glDrawBuffer(GL_COLOR_ATTACHMENT1);

	DrawScreenQuad();

	// Y
	glUniform2f(Objects[GLSL_BLUR_DIRECTION], 0.0f, 1.0f/(HeightP2>>2));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_BLUR1]);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	DrawScreenQuad();
	// *Blur

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int count;

// Multisample FSAA jitter offsets
float j8[8][2]=
{
	{ -0.334818f, 0.435331f },
	{  0.286438f,-0.393495f },
	{  0.459462f, 0.141540f },
	{ -0.414498f,-0.192829f },
	{ -0.183790f, 0.082102f },
	{ -0.079263f,-0.317383f },
	{  0.102254f, 0.299133f },
	{  0.164216f,-0.054399f }
};

void Render(void)
{
	int s, i, x, y;

	RunParticles();

	glBindFramebuffer(GL_FRAMEBUFFER, Objects[BUFFER_ORIGINAL]);

	for(s=0;s<8;s++)
	{
		glDrawBuffer(GL_COLOR_ATTACHMENT0+s);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		// Set viewport and calculate a projection matrix (perspective, with infinite z-far plane)
		glViewport(0, 0, Width, Height);
		MatrixIdentity(Projection);
		InfPerspectiveDxDy(90.0f, (float)Width/Height, Width, Height, 0.01f, j8[s][0], j8[s][1], Projection);

		// Set up model view matrix (translate and rotation)
		MatrixIdentity(ModelView);
		MatrixTranslate(PanX, PanY, Zoom, ModelView);

		QuatAngle(RotateX, 0.0f, 1.0f, 0.0f, QuatX);
		QuatAngle(RotateY, 1.0f, 0.0f, 0.0f, QuatY);
		QuatMultiply(QuatX, QuatY, Quat);
		QuatMatrix(Quat, ModelView);

		MatrixInverse(ModelView, MVIT);

		MatrixMult(ModelView, Projection, MVP);

		//*** Render skybox
		glUseProgram(Objects[GLSL_SKYBOX]);

		glUniformMatrix4fv(Objects[GLSL_SKYBOX_MVP], 1, GL_FALSE, MVP);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, Objects[TEXTURE_REFLECT]);
		glUniform1i(Objects[GLSL_SKYBOX_TEXTURE], 0);

		DrawSkyBox();
		//***

		switch(SwitchMode)
		{
			case 0:
				switch(SwitchObject)
				{
					case 0:
						sprintf(shader, "Rendering spheres as glass");
						break;

					case 1:
						sprintf(shader, "Rendering smooth cubes as glass");
						break;

					case 2:
						sprintf(shader, "Rendering flat cubes as glass");
						break;
				};

				glUseProgram(Objects[GLSL_GLASS]);

				glUniformMatrix4fv(Objects[GLSL_GLASS_MVP], 1, GL_FALSE, MVP);
				glUniform4fv(Objects[GLSL_GLASS_EYE], 1, &MVIT[3*4]);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, Objects[TEXTURE_REFLECT]);
				glUniform1i(Objects[GLSL_GLASS_SKYBOX], 0);
				break;

			case 1:
				switch(SwitchObject)
				{
					case 0:
						sprintf(shader, "Rendering spheres as milk glass");
						break;

					case 1:
						sprintf(shader, "Rendering smooth cubes as milk glass");
						break;

					case 2:
						sprintf(shader, "Rendering flat cubes as milk glass");
						break;
				};

				glUseProgram(Objects[GLSL_MILKGLASS]);

				glUniformMatrix4fv(Objects[GLSL_MILKGLASS_MVP], 1, GL_FALSE, MVP);
				glUniform4fv(Objects[GLSL_MILKGLASS_EYE], 1, &MVIT[3*4]);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, Objects[TEXTURE_SPECULAR]);
				glUniform1i(Objects[GLSL_MILKGLASS_SKYBOX], 0);
				break;

			case 2:
				switch(SwitchObject)
				{
					case 0:
						sprintf(shader, "Rendering spheres with lighting");
						break;

					case 1:
						sprintf(shader, "Rendering smooth cubes with lighting");
						break;

					case 2:
						sprintf(shader, "Rendering flat cubes with lighting");
						break;
				};

				glUseProgram(Objects[GLSL_LIGHTING]);

				glUniformMatrix4fv(Objects[GLSL_LIGHTING_MVP], 1, GL_FALSE, MVP);
				glUniform4fv(Objects[GLSL_LIGHTING_EYE], 1, &MVIT[3*4]);
				glUniform4fv(Objects[GLSL_LIGHTING_MATRIX], 12, &Matrix[0][0]);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, Objects[TEXTURE_SPECULAR]);
				glUniform1i(Objects[GLSL_LIGHTING_SPECULAR], 0);
				break;
		}

		count=0;

		for(i=0;i<4;i++)
		{
			instanced[count++]=((float)CurrentBlock[i][0]+Xpos)*20.0f-90.0f;
			instanced[count++]=((float)CurrentBlock[i][1]+Ypos)*20.0f-200.0f;
			instanced[count++]=0.0f;
			instanced[count++]=Colors[CurrentBlockType][0];
			instanced[count++]=Colors[CurrentBlockType][1];
			instanced[count++]=Colors[CurrentBlockType][2];
		}

		for(i=0;i<4;i++)
		{
			instanced[count++]=((float)NextBlock[i][0]+(Xsize+3))*20.0f-90.0f;
			instanced[count++]=((float)NextBlock[i][1]+(Ysize-3))*20.0f-200.0f;
			instanced[count++]=0.0f;
			instanced[count++]=Colors[NextBlockType][0];
			instanced[count++]=Colors[NextBlockType][1];
			instanced[count++]=Colors[NextBlockType][2];
		}

		for(y=0;y<Ysize;y++)
		{
			for(x=0;x<Xsize;x++)
			{
				if(Table[x][y]!=-1)
				{
					instanced[count++]=(float)x*20.0f-90.0f;
					instanced[count++]=(float)y*20.0f-200.0f;
					instanced[count++]=0.0f;
					instanced[count++]=Colors[Table[x][y]][0];
					instanced[count++]=Colors[Table[x][y]][1];
					instanced[count++]=Colors[Table[x][y]][2];
				}
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, Objects[VBO_INSTANCEDDATA]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*count, instanced, GL_DYNAMIC_DRAW);

		DrawModel3DSInstanced(&Model[SwitchObject], count/6);

		count=0;
		instanced[count++]=-90.0f;
		instanced[count++]=-200.0f;
		instanced[count++]=0.0f;
		instanced[count++]=1.0f;
		instanced[count++]=1.0f;
		instanced[count++]=1.0f;

		glBindBuffer(GL_ARRAY_BUFFER, Objects[VBO_INSTANCEDDATA]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*count, instanced, GL_DYNAMIC_DRAW);

		DrawModel3DSInstanced(&Model[3], 1);

		// Check if any lines were eliminated
		count=0;
		for(y=0;y<Ysize;y++)
		{
			// If that line was eliminated...
			if(LineElim[y])
			{
				for(x=0;x<Xsize;x++)
				{
					particle[count].active=1;

					particle[count].life=1.0f;
					particle[count].ttd=0.005f;

					particle[count].pos[0]=(float)x*20.0f-90.0f;
					particle[count].pos[1]=(float)y*20.0f-200.0f;
					particle[count].pos[2]=0.0f;

					particle[count].vel[0]=(float)randrange(0, 6)-3;
					particle[count].vel[1]=(float)randrange(0, 10);
					particle[count].vel[2]=(float)randrange(0, 6)-3;

					particle[count].color[0]=Colors[OldTable[x][y]][0];
					particle[count].color[1]=Colors[OldTable[x][y]][1];
					particle[count].color[2]=Colors[OldTable[x][y]][2];

					count++;
				}
	
				// Clear the eliminated flag
				LineElim[y]=0;
			}
		}

		count=0;
		for(i=0;i<Xsize*4;i++)
		{
			if(particle[i].active)
			{
				instanced[count++]=particle[i].pos[0];
				instanced[count++]=particle[i].pos[1];
				instanced[count++]=particle[i].pos[2];
				instanced[count++]=particle[i].color[0];
				instanced[count++]=particle[i].color[1];
				instanced[count++]=particle[i].color[2];
			}
		}

		if(count)
		{
			glBindBuffer(GL_ARRAY_BUFFER, Objects[VBO_INSTANCEDDATA]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*count, instanced, GL_DYNAMIC_DRAW);

			glDisable(GL_DEPTH_TEST);
			DrawModel3DSInstanced(&Model[SwitchObject], count/6);
			glEnable(GL_DEPTH_TEST);
		}
	}

	Blur();

	glDisable(GL_DEPTH_TEST);
	// Composite
	glUseProgram(Objects[GLSL_COMPOSITE]);

	glUniform2f(Objects[GLSL_COMPOSITE_COORDS], (float)Width/WidthP2, (float)Height/HeightP2);
	glUniform1f(Objects[GLSL_COMPOSITE_EXPOSURE], powf(2.0f, Exposure));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL]);
	glUniform1i(Objects[GLSL_COMPOSITE_ORIGINAL], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_BLUR0]);
	glUniform1i(Objects[GLSL_COMPOSITE_BLUR], 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL1]);
	glUniform1i(Objects[GLSL_COMPOSITE_ORIGINAL1], 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL2]);
	glUniform1i(Objects[GLSL_COMPOSITE_ORIGINAL2], 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL3]);
	glUniform1i(Objects[GLSL_COMPOSITE_ORIGINAL3], 4);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL4]);
	glUniform1i(Objects[GLSL_COMPOSITE_ORIGINAL4], 5);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL5]);
	glUniform1i(Objects[GLSL_COMPOSITE_ORIGINAL5], 6);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL6]);
	glUniform1i(Objects[GLSL_COMPOSITE_ORIGINAL6], 7);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL7]);
	glUniform1i(Objects[GLSL_COMPOSITE_ORIGINAL7], 8);

	DrawScreenQuad();

	glActiveTexture(GL_TEXTURE0);

	// Stats
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	Font_Print(0.0f, (float)Height-16.0f, "%s\nExposure: %0.3f\nFPS: %0.1f", shader, Exposure, FPS);

	switch(GameState)
	{
		case 1:
			Font_Print(0.0f, 24.0f, "Score: %d\nLevel: %d", Score, 11-Speed);
			break;

		case 0:
			Font_Print(0.0f, 24.0f, "Paused\nScore: %d\nLevel: %d", Score, 11-Speed);
			break;

		case -1:
			Font_Print(0.0f, 24.0f, "Game Over\nScore: %d\nLevel: %d", Score, 11-Speed);
			break;
	}

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

GLuint BuildInstanceVBO(void)
{
	GLuint obj;

	memset(&instanced, 0, sizeof(float)*6*Xsize*Ysize);

	glGenBuffers(1, &obj);
	glBindBuffer(GL_ARRAY_BUFFER, obj);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*Xsize*Ysize, instanced, GL_DYNAMIC_DRAW);

	return obj;
}

GLuint BuildSkyboxVBO(void)
{
	GLuint obj;
	float verts[]=
	{
		1.0f, -1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, -1.0f, 0.0f,

		-1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, -1.0f, 0.0f,

		-1.0f, 1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f, 0.0f,

		-1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f, 0.0f,

		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 0.0f,

		-1.0f, -1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, -1.0f, 0.0f
	};

	glGenBuffers(1, &obj);
	glBindBuffer(GL_ARRAY_BUFFER, obj);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*3*12, verts, GL_STATIC_DRAW);

	return obj;
}

GLuint BuildScreenQuadVBO(void)
{
	GLuint obj;
	float verts[]=
	{
		-1.0f, -1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f,
		-1.0f, -1.0f,
		1.0f, 1.0f,
		-1.0f, 1.0f
	};

	glGenBuffers(1, &obj);
	glBindBuffer(GL_ARRAY_BUFFER, obj);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*3*2, verts, GL_STATIC_DRAW);

	return obj;
}

int Init(void)
{
	GLuint VAO;

	InitParticles();

	Objects[GLSL_SKYBOX]=CreateProgram("skybox_v.glsl", "skybox_f.glsl");
	Objects[GLSL_SKYBOX_MVP]=glGetUniformLocation(Objects[GLSL_SKYBOX], "mvp");
	Objects[GLSL_SKYBOX_TEXTURE]=glGetUniformLocation(Objects[GLSL_SKYBOX], "skybox");

	Objects[GLSL_GLASS]=CreateProgram("generic_v.glsl", "glass_f.glsl");
	Objects[GLSL_GLASS_MVP]=glGetUniformLocation(Objects[GLSL_GLASS], "mvp");
	Objects[GLSL_GLASS_EYE]=glGetUniformLocation(Objects[GLSL_GLASS], "eye");
	Objects[GLSL_GLASS_SKYBOX]=glGetUniformLocation(Objects[GLSL_GLASS], "skybox");

	Objects[GLSL_MILKGLASS]=CreateProgram("generic_v.glsl", "milkglass_f.glsl");
	Objects[GLSL_MILKGLASS_MVP]=glGetUniformLocation(Objects[GLSL_MILKGLASS], "mvp");
	Objects[GLSL_MILKGLASS_EYE]=glGetUniformLocation(Objects[GLSL_MILKGLASS], "eye");
	Objects[GLSL_MILKGLASS_SKYBOX]=glGetUniformLocation(Objects[GLSL_MILKGLASS], "skybox");

	Objects[GLSL_LIGHTING]=CreateProgram("generic_v.glsl", "lighting_f.glsl");
	Objects[GLSL_LIGHTING_MVP]=glGetUniformLocation(Objects[GLSL_LIGHTING], "mvp");
	Objects[GLSL_LIGHTING_EYE]=glGetUniformLocation(Objects[GLSL_LIGHTING], "eye");
	Objects[GLSL_LIGHTING_SPECULAR]=glGetUniformLocation(Objects[GLSL_LIGHTING], "specular");
	Objects[GLSL_LIGHTING_MATRIX]=glGetUniformLocation(Objects[GLSL_LIGHTING], "matrix");

	Objects[GLSL_BLUR]=CreateProgram("blur_v.glsl", "blur_f.glsl");
	Objects[GLSL_BLUR_COORDS]=glGetUniformLocation(Objects[GLSL_BLUR], "coords");
	Objects[GLSL_BLUR_DIRECTION]=glGetUniformLocation(Objects[GLSL_BLUR], "direction");
	Objects[GLSL_BLUR_TEXTURE]=glGetUniformLocation(Objects[GLSL_BLUR], "texture");

	Objects[GLSL_COMPOSITE]=CreateProgram("composite_v.glsl", "composite_f.glsl");
	Objects[GLSL_COMPOSITE_COORDS]=glGetUniformLocation(Objects[GLSL_COMPOSITE], "coords");
	Objects[GLSL_COMPOSITE_EXPOSURE]=glGetUniformLocation(Objects[GLSL_COMPOSITE], "exposure");
	Objects[GLSL_COMPOSITE_ORIGINAL]=glGetUniformLocation(Objects[GLSL_COMPOSITE], "original");
	Objects[GLSL_COMPOSITE_ORIGINAL1]=glGetUniformLocation(Objects[GLSL_COMPOSITE], "original1");
	Objects[GLSL_COMPOSITE_ORIGINAL2]=glGetUniformLocation(Objects[GLSL_COMPOSITE], "original2");
	Objects[GLSL_COMPOSITE_ORIGINAL3]=glGetUniformLocation(Objects[GLSL_COMPOSITE], "original3");
	Objects[GLSL_COMPOSITE_ORIGINAL4]=glGetUniformLocation(Objects[GLSL_COMPOSITE], "original4");
	Objects[GLSL_COMPOSITE_ORIGINAL5]=glGetUniformLocation(Objects[GLSL_COMPOSITE], "original5");
	Objects[GLSL_COMPOSITE_ORIGINAL6]=glGetUniformLocation(Objects[GLSL_COMPOSITE], "original6");
	Objects[GLSL_COMPOSITE_ORIGINAL7]=glGetUniformLocation(Objects[GLSL_COMPOSITE], "original7");
	Objects[GLSL_COMPOSITE_BLUR]=glGetUniformLocation(Objects[GLSL_COMPOSITE], "blur");

	LoadScene(SceneList[CurrentScene].String);

	glGenTextures(1, &Objects[TEXTURE_ORIGINAL]);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, WidthP2, HeightP2, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &Objects[TEXTURE_ORIGINAL1]);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, WidthP2, HeightP2, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &Objects[TEXTURE_ORIGINAL2]);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, WidthP2, HeightP2, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &Objects[TEXTURE_ORIGINAL3]);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, WidthP2, HeightP2, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &Objects[TEXTURE_BLUR0]);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_BLUR0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, WidthP2>>2, HeightP2>>2, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &Objects[TEXTURE_BLUR1]);
	glBindTexture(GL_TEXTURE_2D, Objects[TEXTURE_BLUR1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, WidthP2>>2, HeightP2>>2, 0, GL_RGBA, GL_FLOAT, NULL);

	Objects[VBO_SKYBOX]=BuildSkyboxVBO();
	Objects[VBO_SCREENQUAD]=BuildScreenQuadVBO();
	Objects[VBO_INSTANCEDDATA]=BuildInstanceVBO();

	if(Load3DS(&Model[0], "sphere.3ds"))
		BuildVBO3DS(&Model[0]);

	if(Load3DS(&Model[1], "scube.3ds"))
		BuildVBO3DS(&Model[1]);

	if(Load3DS(&Model[2], "fcube.3ds"))
		BuildVBO3DS(&Model[2]);

	if(Load3DS(&Model[3], "border.3ds"))
		BuildVBO3DS(&Model[3]);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glGenRenderbuffers(1, &Objects[BUFFER_DEPTH]);
	glBindRenderbuffer(GL_RENDERBUFFER, Objects[BUFFER_DEPTH]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, WidthP2, HeightP2);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &Objects[BUFFER_ORIGINAL]);
	glBindFramebuffer(GL_FRAMEBUFFER, Objects[BUFFER_ORIGINAL]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL2], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, Objects[TEXTURE_ORIGINAL3], 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Objects[BUFFER_DEPTH]);

	glGenFramebuffers(1, &Objects[BUFFER_BLUR]);
	glBindFramebuffer(GL_FRAMEBUFFER, Objects[BUFFER_BLUR]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Objects[TEXTURE_BLUR0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, Objects[TEXTURE_BLUR1], 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	return 1;
}

void LoadScene(char *Filename)
{
	FILE *stream;
	char Reflect[256];
	char Specular[256];

	if((stream=fopen(Filename, "rt"))==NULL)
		return;

	fscanf(stream, "%s", Reflect);
	fscanf(stream, "%s", Specular);
	fscanf(stream, "%f %f %f %f", &Matrix[0][0], &Matrix[0][1], &Matrix[0][2], &Matrix[0][3]);
	fscanf(stream, "%f %f %f %f", &Matrix[1][0], &Matrix[1][1], &Matrix[1][2], &Matrix[1][3]);
	fscanf(stream, "%f %f %f %f", &Matrix[2][0], &Matrix[2][1], &Matrix[2][2], &Matrix[2][3]);
	fscanf(stream, "%f %f %f %f", &Matrix[3][0], &Matrix[3][1], &Matrix[3][2], &Matrix[3][3]);
	fscanf(stream, "%f %f %f %f", &Matrix[4][0], &Matrix[4][1], &Matrix[4][2], &Matrix[4][3]);
	fscanf(stream, "%f %f %f %f", &Matrix[5][0], &Matrix[5][1], &Matrix[5][2], &Matrix[5][3]);
	fscanf(stream, "%f %f %f %f", &Matrix[6][0], &Matrix[6][1], &Matrix[6][2], &Matrix[6][3]);
	fscanf(stream, "%f %f %f %f", &Matrix[7][0], &Matrix[7][1], &Matrix[7][2], &Matrix[7][3]);
	fscanf(stream, "%f %f %f %f", &Matrix[8][0], &Matrix[8][1], &Matrix[8][2], &Matrix[8][3]);
	fscanf(stream, "%f %f %f %f", &Matrix[9][0], &Matrix[9][1], &Matrix[9][2], &Matrix[9][3]);
	fscanf(stream, "%f %f %f %f", &Matrix[10][0],&Matrix[10][1],&Matrix[10][2],&Matrix[10][3]);
	fscanf(stream, "%f %f %f %f", &Matrix[11][0],&Matrix[11][1],&Matrix[11][2],&Matrix[11][3]);

	fclose(stream);

	if(Objects[TEXTURE_REFLECT])
		glDeleteTextures(1, &Objects[TEXTURE_REFLECT]);

	Objects[TEXTURE_REFLECT]=Image_Upload(Reflect, IMAGE_RGBE|IMAGE_CUBEMAP_ANGULAR|IMAGE_CLAMP|IMAGE_MIPMAP);

	if(Objects[TEXTURE_SPECULAR])
		glDeleteTextures(1, &Objects[TEXTURE_SPECULAR]);

	Objects[TEXTURE_SPECULAR]=Image_Upload(Specular, IMAGE_RGBE|IMAGE_CUBEMAP_ANGULAR|IMAGE_CLAMP|IMAGE_MIPMAP);
}

int LoadShader(GLuint Shader, char *Filename)
{
	FILE *stream;
	char *buffer;
	int length;

	if((stream=fopen(Filename, "rb"))==NULL)
		return 0;

	fseek(stream, 0, SEEK_END);
	length=ftell(stream);
	fseek(stream, 0, SEEK_SET);

	buffer=(char *)malloc(length+1);
	fread(buffer, 1, length, stream);
	buffer[length]='\0';

	glShaderSource(Shader, 1, (const char **)&buffer, NULL);

	fclose(stream);
	free(buffer);

	return 1;
}

GLuint CreateProgram(char *VertexFilename, char *FragmentFilename)
{
	GLuint Program, Vertex, Fragment;
	GLint Status, LogLength;
	char *Log=NULL;

	Program=glCreateProgram();

	if(VertexFilename)
	{
		Vertex=glCreateShader(GL_VERTEX_SHADER);

		if(LoadShader(Vertex, VertexFilename))
		{
			glCompileShader(Vertex);
			glGetShaderiv(Vertex, GL_COMPILE_STATUS, &Status);

			if(!Status)
			{
				glGetShaderiv(Vertex, GL_INFO_LOG_LENGTH, &LogLength);
				Log=(char *)malloc(LogLength);

				if(Log)
				{
					glGetShaderInfoLog(Vertex, LogLength, NULL, Log);
					MessageBox(Context.hWnd, Log, VertexFilename, MB_OK);
					free(Log);
				}
			}
			else
				glAttachShader(Program, Vertex);
		}

		glDeleteShader(Vertex);
	}

	if(FragmentFilename)
	{
		Fragment=glCreateShader(GL_FRAGMENT_SHADER);

		if(LoadShader(Fragment, FragmentFilename))
		{
			glCompileShader(Fragment);
			glGetShaderiv(Fragment, GL_COMPILE_STATUS, &Status);

			if(!Status)
			{
				glGetShaderiv(Fragment, GL_INFO_LOG_LENGTH, &LogLength);
				Log=(char *)malloc(LogLength);

				if(Log)
				{
					glGetShaderInfoLog(Fragment, LogLength, NULL, Log);
					MessageBox(Context.hWnd, Log, FragmentFilename, MB_OK);
					free(Log);
				}
			}
			else
				glAttachShader(Program, Fragment);
		}

		glDeleteShader(Fragment);
	}

	glLinkProgram(Program);
	glGetProgramiv(Program, GL_LINK_STATUS, &Status);

	if(!Status)
	{
		glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &LogLength);
		Log=(char *)malloc(LogLength);

		if(Log)
		{
			glGetProgramInfoLog(Program, LogLength, NULL, Log);
			MessageBox(Context.hWnd, Log, "Link", MB_OK);
			free(Log);
		}
	}

	return Program;
}
