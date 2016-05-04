


// IO Includes
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <ctime>

// String and Char Includes
#include <string>
#include <ctype.h>

//Windows
#include <windows.h>

// SDL Includes
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>

// Constants
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define SCREEN_BPP 32

// Surfaces (do wyświetlania okna i grafiki w SDL-u musimy zadeklarować powierzchnię)
// Main Surface (zmienna typu surface (powierzchnia) może pełnić funkcje związane z grafiką)
SDL_Surface *MainSurface = NULL;
// Sprites Surfaces  (Obrazki 2D, które wyświetlamy w SDL-u nazywane są sprite'ami)
SDL_Surface *SPRITE_BORDER = NULL;
SDL_Surface *SPRITE_BLUE = NULL;
SDL_Surface *SPRITE_CYAN = NULL;
SDL_Surface *SPRITE_GRAY = NULL;
SDL_Surface *SPRITE_GREEN = NULL;
SDL_Surface *SPRITE_ORANGE = NULL;
SDL_Surface *SPRITE_RED = NULL;
SDL_Surface *SPRITE_YELLOW = NULL;
// Misc
SDL_Surface *LOGO = NULL;
SDL_Surface *NastepnaFigura = NULL;
SDL_Surface *EkranStartowy = NULL;
SDL_Surface *KoniecGry = NULL;
// Levels Surfaces
SDL_Surface *L1 = NULL;
SDL_Surface *L2 = NULL;
SDL_Surface *L3 = NULL;
SDL_Surface *L4 = NULL;
SDL_Surface *L5 = NULL;

// Misc Vars
bool Running = true;
SDL_Event event;
int CRoom = 1;
int CLevel = 1;

// Timing
int VCount = 0;
int VCOUNT_MAX = 500;

// Color Constants
#define TCOLOR_BLUE 0
#define TCOLOR_CYAN 1
#define TCOLOR_GREEN 2
#define TCOLOR_ORANGE 3
#define TCOLOR_RED 4
#define TCOLOR_YELLOW 5
#define TCOLOR_GRAY 6

// Plansza gry o wymiarach 20 wierszy na 10 kolumn(zw. matrix lub tetrion)
#define MATRIX_DRAW_POS_X 100
#define MATRIX_DRAW_POS_Y 0
#define MATRIX_PIECES_X 10
#define MATRIX_PIECES_Y 20

// TFigura Class
class TFigura
{
public:
    int Color = 0; // Represents the blue color
    bool Used = false;
};

// Create the Matrix
TFigura FiguraMatrix[MATRIX_PIECES_X][MATRIX_PIECES_Y]; // this is the all tetris pieces - TFigura[x][y]
TFigura TmpMatrix[4][4]; // Klocki (tetrimino) zlozone z czterech malych kwadratow zw. blokami
TFigura TmpNextMatrix[4][4]; //klocek następny (wyświetlany po prawej stronie)

int XPosTmp = 4;
int YPosTmp = 0;
int WidthTmp = 0;
int HeightTmp = 0;

// Buttons States
bool DownButton = false;

// Points
int Points = 0;

// Music
Mix_Music *TetrisMusic = NULL;

// Generator klockow
int CRAND = 0; //kształt bieżącej figury
int DIF = 4; //zmienna nie używana, wcześniej użyta jako skok inkrementacji zmiany kształtu kolejnej figury
int NextFigura = 4; //kształt następnej figury



// Function Prototipes
SDL_Surface *LoadSurfaceFromFile(std::string filename);
void         DrawSurface(int x, int y, SDL_Surface* source, SDL_Surface* destination);
void         GenerateTmpMatrix(int FigIndex, TFigura figura[4][4]);
void         GenerateTmpMatrix(int FigIndex);
void         DrawTetrisMatrix();
bool         LoadFiles();
void         UnloadFiles();
bool         Update();
bool         TestBlockColision();
void         PutInMatrix();
int          GetNewFiguraIndex();
void         CalculateTmpSize();
void         RotateTmpMatrix();
void         VerifyLines();
void         DrawTmpFigura(int x, int y);
void         ResetFiguraMatrix();

using namespace std;

int main(int argc, char* args[])
{
    srand(time(NULL));
    //Hide Console
    FreeConsole();
    // std out file write
    cout << "Tetris" << endl;
    // Inicjalizacja SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
        // Blad inicjalizacji
        cout << "Blad inicjalizacji SDL!" << endl;
        return 1;
    }
    // Inicjalizacja SDL Mixer
    if(Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
    {
        // blad
        cout << "Blad inicjalizacji sdl mix!" << endl;
        return true;
    }
    // Tworzenie glownej powierzchni (width=600, height=600, bpp=32, ?????????????)
    MainSurface = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
    // Test if MainSurface was created
    if (MainSurface == NULL)
    {
        // error
        cout << "Blad tworzenia MainSurface!" << endl;
        return 1;
    }
    // Set Window Caption
    SDL_WM_SetCaption("Tetris", NULL);
    // Load Files
    if (LoadFiles() == true)
    {
        // error
        cout << "Error loading Files!" << endl;
        return 1;
    }
    // Generate one tmp startup matrix
    GetNewFiguraIndex();
    GenerateTmpMatrix(GetNewFiguraIndex(), TmpMatrix);//losowanie klocka startowego
    GenerateTmpMatrix(NextFigura, TmpNextMatrix); //tworzenie klocka następnego (na potrzeby wyświetlania go po prawej stronie ekranu)
    // Start Music
    if (Mix_PlayingMusic() == 0)
    {
        if (Mix_PlayMusic(TetrisMusic, -1) == -1)
        {
            cout << "Error playing music!" << endl;
            return true;
        }
    }
    // Start the Game Loop  (klawisze sterujace: LEFT,RIGHT,UP - rotacja,DOWN)
    while (Running)
    {
        if (Update() == 1)
        {
            // Error ocurrent in update
            Running = false;
        }
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_LEFT:
                    if (XPosTmp > 0) XPosTmp -= 1;
                    break;
                case SDLK_RIGHT:
                    //przed przesunięciem w prawo sprawdzane jest czy klocek nie wyjdzie poza planszę
                    if (XPosTmp < MATRIX_PIECES_X - (WidthTmp + 1)) XPosTmp += 1;
                    break;
                case SDLK_DOWN:
                    DownButton = true;
                    break;
                case SDLK_UP:
                    // Rotate
                    RotateTmpMatrix();

                    //po obróceniu klocka trzeba sprawdzić czy nie wyszedł poza planszę (prawa strona)
                    if (XPosTmp >= MATRIX_PIECES_X - (WidthTmp + 1))
                    {
                        XPosTmp = MATRIX_PIECES_X - (WidthTmp+1);
                    }
                    break;
                case SDLK_RETURN:
                    if (CRoom == 1)
                    {
                        CRoom = 0;
                    }
                    else if (CRoom == 2)
                    {
                        ResetFiguraMatrix();
                        YPosTmp = 0;
                        XPosTmp = 5;
                        GenerateTmpMatrix(0, TmpMatrix);
                        CRoom = 1;
                    }
                    break;
                }

            }
            if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_DOWN:
                    DownButton = false;
                    break;
                }
            }

            // Event Close
            if (event.type == SDL_QUIT)
            {
                // Turn running to false for close the program
                Running = false;
            }
        }
    }
    // Stop Music
    Mix_HaltMusic();
    // Unload Files
    UnloadFiles();
    // Quit SDL
    SDL_Quit();
    return 0;
}

bool Update()
{
    // Clear the Screen
    SDL_FillRect(MainSurface, NULL, 0x000000);
    // Draw just the Selected Room
    if (CRoom == 2)
    {
        DrawSurface(100, 100, KoniecGry, MainSurface);

    }
    if (CRoom == 1)
    {
        DrawSurface(100, 100, EkranStartowy, MainSurface);

    }
    if (CRoom == 0)
    {
        // Draw Logo
        DrawSurface(350, 10, LOGO, MainSurface);

        // Draw Txt Info
        DrawSurface(350, 140, NastepnaFigura, MainSurface);

        // Draw Level
        int lx = 350;
        int ly = 350;
        if (CLevel == 1)
        {
            DrawSurface(lx, ly, L1, MainSurface);
        }
        if (CLevel == 2)
        {
            DrawSurface(lx, ly, L2, MainSurface);
        }
        if (CLevel == 3)
        {
            DrawSurface(lx, ly, L3, MainSurface);
        }
        if (CLevel == 4)
        {
            DrawSurface(lx, ly, L4, MainSurface);
        }
        if (CLevel == 5)
        {
            DrawSurface(lx, ly, L5, MainSurface);
        }

        if (DownButton == true)
        {
            VCount += 20;
        }
        if (VCount >= VCOUNT_MAX)
        {
            if (TestBlockColision() == true)
            {
                // put in matrix
                PutInMatrix();
                // Verify if is game over
                if (YPosTmp < 2)
                {
                    // Game Over
                    CRoom = 2;
                }
                // Reset Pos
                XPosTmp = 5;
                YPosTmp = 0;
                //
                int pInd = GetNewFiguraIndex();
                GenerateTmpMatrix(pInd, TmpMatrix);
                GenerateTmpMatrix(NextFigura, TmpNextMatrix);
                //
                CalculateTmpSize();
                //
                VerifyLines();
            }
            else
            {
                YPosTmp += 1;
            }
            VCount = 0;
        }
        else
        {
            VCount++;
        }


        DrawTetrisMatrix();
    }
    // Update the Screen
    if (SDL_Flip(MainSurface) == -1)
    {
        // Error
        cout << "Error updating Screen!" << endl;
        return true;
    }
    return false;
}

void ResetFiguraMatrix()
{
    for (int x = 0; x < MATRIX_PIECES_X; x++)
    {
        for (int y = 0; y < MATRIX_PIECES_Y; y++)
        {
            FiguraMatrix[x][y].Used = false;
        }
    }
}

int GetNewFiguraIndex()
{
    //int n_figura = rand()%7;

    CRAND = NextFigura;

    NextFigura = rand()%7;
    //
    return CRAND;
}

void DrawTetrisMatrix()
{
    // vals
    int px_start = MATRIX_DRAW_POS_X + 20; // +block size
    int py_start = MATRIX_DRAW_POS_Y + 100; // +Space
    //
    int rightBorderPosX = px_start + (MATRIX_PIECES_X * 20);
    int leftBorderPosX = MATRIX_DRAW_POS_X;
    //
    int DownBorderMaxX = px_start + (MATRIX_PIECES_X * 20);

    // Draw Borders
    // Left Border
    int YLimit = (MATRIX_PIECES_Y * 20) + 20 + py_start;
    for (int yp = 0; yp < YLimit; yp+=20) // Add +1 Border Figura // +1 = Limit
    {
        DrawSurface(MATRIX_DRAW_POS_X, yp, SPRITE_BORDER, MainSurface);
    }
    // Right Border
    for (int yp = 0; yp < YLimit; yp+=20) // Add +1 Border Figura // +1 = Limit
    {
        DrawSurface(rightBorderPosX, yp, SPRITE_BORDER, MainSurface);
    }
    // Down Border
    for (int xp = MATRIX_DRAW_POS_X; xp < DownBorderMaxX; xp += 20)
    {
        DrawSurface(xp, YLimit - 20, SPRITE_BORDER, MainSurface);
    }

    // Draw Pieces
    for (int xp = 0; xp < MATRIX_PIECES_X; xp++)
    {
        for (int yp = 0; yp < MATRIX_PIECES_Y; yp++)
        {
            if (FiguraMatrix[xp][yp].Used == true) // False for a test
            {
                //
                int PosXG = px_start + (xp * 20);
                int PosYG = py_start + (yp * 20);
                //
                if (FiguraMatrix[xp][yp].Color == TCOLOR_BLUE)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_BLUE, MainSurface);
                }
                if (FiguraMatrix[xp][yp].Color == TCOLOR_CYAN)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_CYAN, MainSurface);
                }
                if (FiguraMatrix[xp][yp].Color == TCOLOR_GRAY)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_GRAY, MainSurface);
                }
                if (FiguraMatrix[xp][yp].Color == TCOLOR_GREEN)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_GREEN, MainSurface);
                }
                if (FiguraMatrix[xp][yp].Color == TCOLOR_ORANGE)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_ORANGE, MainSurface);
                }
                if (FiguraMatrix[xp][yp].Color == TCOLOR_RED)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_RED, MainSurface);
                }
                if (FiguraMatrix[xp][yp].Color == TCOLOR_YELLOW)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_YELLOW, MainSurface);
                }
                //
            }
        }
    }

    // Draw Figura Tmp // animacja spadania klocka
    for (int xp = 0; xp < 4; xp++)
    {
        for (int yp = 0; yp < 4; yp++)
        {
            if (TmpMatrix[xp][yp].Used == true) // False for a test
            {
                //
                int PosXG = px_start + ((xp + XPosTmp) * 20);
                int PosYG = py_start + ((yp + YPosTmp) * 20);
                //
                if (TmpMatrix[xp][yp].Color == TCOLOR_BLUE)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_BLUE, MainSurface);
                }
                if (TmpMatrix[xp][yp].Color == TCOLOR_CYAN)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_CYAN, MainSurface);
                }
                if (TmpMatrix[xp][yp].Color == TCOLOR_GRAY)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_GRAY, MainSurface);
                }
                if (TmpMatrix[xp][yp].Color == TCOLOR_GREEN)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_GREEN, MainSurface);
                }
                if (TmpMatrix[xp][yp].Color == TCOLOR_ORANGE)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_ORANGE, MainSurface);
                }
                if (TmpMatrix[xp][yp].Color == TCOLOR_RED)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_RED, MainSurface);
                }
                if (TmpMatrix[xp][yp].Color == TCOLOR_YELLOW)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_YELLOW, MainSurface);
                }
                //
            }
        }
    }

    //

    DrawTmpFigura(450, 180); //pokazuje nastepny klocek z prawej strony

}
//rysowanie klocka z prawej
void DrawTmpFigura(int x, int y)
{
    // Draw Figura Tmp
    for (int xp = 0; xp < 4; xp++)
    {
        for (int yp = 0; yp < 4; yp++)
        {
            if (TmpNextMatrix[xp][yp].Used == true) // False for a test
            {
                //
                int PosXG = x + (xp * 20);
                int PosYG = y + (yp * 20);
                //
                if (TmpNextMatrix[xp][yp].Color == TCOLOR_BLUE)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_BLUE, MainSurface);
                }
                if (TmpNextMatrix[xp][yp].Color == TCOLOR_CYAN)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_CYAN, MainSurface);
                }
                if (TmpNextMatrix[xp][yp].Color == TCOLOR_GRAY)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_GRAY, MainSurface);
                }
                if (TmpNextMatrix[xp][yp].Color == TCOLOR_GREEN)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_GREEN, MainSurface);
                }
                if (TmpNextMatrix[xp][yp].Color == TCOLOR_ORANGE)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_ORANGE, MainSurface);
                }
                if (TmpNextMatrix[xp][yp].Color == TCOLOR_RED)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_RED, MainSurface);
                }
                if (TmpNextMatrix[xp][yp].Color == TCOLOR_YELLOW)
                {
                    DrawSurface(PosXG, PosYG, SPRITE_YELLOW, MainSurface);
                }
                //
            }
        }
    }

}

void PutInMatrix()
{
    for (int xpt = 0; xpt < 4; xpt++)
    {
        for (int ypt = 0; ypt < 4; ypt++)
        {
            if (TmpMatrix[xpt][ypt].Used == true)
            {
                if (xpt + XPosTmp > -1 && xpt + XPosTmp < MATRIX_PIECES_X + 1 &&
                    ypt + YPosTmp > -1 && ypt + YPosTmp < MATRIX_PIECES_Y + 1)
                {
                    FiguraMatrix[xpt + XPosTmp][ypt + YPosTmp] = TmpMatrix[xpt][ypt];
                }
            }
        }
    }


}

//Oblicza maksymalne rozmiary (szerokość, wysokość) figury
//Rozmiar jest podany "informatyczne", tzn. WidthTmp = 2 oznacza, że szerokość = 3 :)
//Obliczon wymiary umieszczane są w zmiennych globalnych: WidthTmp, HeightTmp
void CalculateTmpSize()
{
    int width = 0;
    int height = 0;
    //
    for (int xp = 0; xp < 4; xp++)
    {
        for (int yp = 0; yp < 4; yp++)
        {
            if (TmpMatrix[xp][yp].Used == true)
            {
                if (xp > width)
                {
                    width = xp;
                }
                if (yp > height)
                {
                    height = yp;
                }
            }
        }
    }
    //
    WidthTmp = width;
    HeightTmp = height;
}

bool TestBlockColision()
{
    if (YPosTmp == 20 - (HeightTmp + 1)) // -msize
    {
        return true;
    }
    for (int xpt = 0; xpt < 4; xpt++)
    {
        for (int ypt = 0; ypt < 4; ypt++)
        {
            if (TmpMatrix[xpt][ypt].Used == true)
            {
                //
                //
                for (int xp = 0; xp < MATRIX_PIECES_X; xp++)
                {
                    for (int yp = 0; yp < MATRIX_PIECES_Y; yp++)
                    {
                        if (FiguraMatrix[xp][yp].Used == true)
                        {
                            //
                            if (xpt + XPosTmp == xp && ypt + (YPosTmp + 1) == yp)
                            {
                                return true;
                            }
                            //
                        }
                    }
                }
                //
            }
        }
    }
    return false;
}





void ClearTmpMatrix(TFigura figura[4][4])
{
    for (int xp = 0; xp < 4; xp++)
    {
        for (int yp = 0; yp < 4; yp++)
        {
            figura[xp][yp].Used = false;
        }
    }
}




void FillTmpColor(int color, TFigura figura[4][4])
{
    for (int xp = 0; xp < 4; xp++)
    {
        for (int yp = 0; yp < 4; yp++)
        {
            figura[xp][yp].Color = color;
        }
    }
}




void VerifyLines()
{

    for (int yP = 20; yP > -1; yP--)
    {
        bool hU = false;
        for (int xP = 0; xP < MATRIX_PIECES_X; xP++)
        {
            if (FiguraMatrix[xP][yP].Used == false)
            {
                hU = true;
            }
        }
        if (hU == false)
        {
            // Complete Line
            for (int xp = 0; xp < MATRIX_PIECES_X; xp++)
            {
                FiguraMatrix[xp][yP].Used = false;
            }
            // Get Down Matrix
            for (int yPv = yP; yPv > -1; yPv--)
            {
                for (int xp = 0; xp < MATRIX_PIECES_X; xp++)
                {
                    if (yPv - 1 > -1)
                    {
                        FiguraMatrix[xp][yPv] = FiguraMatrix[xp][yPv - 1];
                    }
                }
            }
            //
            Points += 1;
            if (Points < 10)
            {
                CLevel = 1;
                VCOUNT_MAX = 500;
            }
            if (Points == 10)
            {
                CLevel = 2;
                VCOUNT_MAX = 400;
            }
            if (Points == 20)
            {
                CLevel = 3;
                VCOUNT_MAX = 300;
            }
            if(Points == 30)
            {
                CLevel = 4;
                VCOUNT_MAX = 200;
            }
            if (Points == 50)
            {
                CLevel = 5;
                VCOUNT_MAX = 100;
            }
        }
    }

}

void SetMatrixInCorner()
{
    TFigura MatrixBackup[5][5];
    // Left Side
    while (true)
    {
        for (int xp = 0; xp < 4; xp++)
        {
            for (int yp = 0; yp < 4; yp++)
            {
                MatrixBackup[xp][yp] = TmpMatrix[xp][yp];
            }
        }
        //
        bool h = false;
        for (int yTmp = 0; yTmp < 4; yTmp++)
        {
            if (TmpMatrix[0][yTmp].Used == true)
            {
                h = true;
            }
        }
        if (h == true)
        {
            break;
        }
        else
        {
            for (int xp = 0; xp < 4; xp++)
            {
                for (int yp = 0; yp < 4; yp++)
                {
                    TmpMatrix[xp][yp] = MatrixBackup[xp + 1][yp];
                }
            }
        }
    }
    // Up Side
    while (true)
    {
        for (int xp = 0; xp < 4; xp++)
        {
            for (int yp = 0; yp < 4; yp++)
            {
                MatrixBackup[xp][yp] = TmpMatrix[xp][yp];
            }
        }
        //
        bool h = false;
        for (int xTmp = 0; xTmp < 4; xTmp++)
        {
            if (TmpMatrix[xTmp][0].Used == true)
            {
                h = true;
            }
        }
        if (h == true)
        {
            break;
        }
        else
        {
            for (int xp = 0; xp < 4; xp++)
            {
                for (int yp = 0; yp < 4; yp++)
                {
                    TmpMatrix[xp][yp] = MatrixBackup[xp][yp + 1];
                }
            }
        }
    }

}

void RotateTmpMatrix()
{
    TFigura MatrixBackup[4][4];
    for (int xp = 0; xp < 4; xp++)
    {
        for (int yp = 0; yp < 4; yp++)
        {
            MatrixBackup[xp][yp] = TmpMatrix[xp][yp];
        }
    }
    //
    TmpMatrix[0][3] = MatrixBackup[3][3];
    TmpMatrix[0][2] = MatrixBackup[2][3];
    TmpMatrix[0][1] = MatrixBackup[1][3];
    TmpMatrix[0][0] = MatrixBackup[0][3];
    //
    TmpMatrix[3][3] = MatrixBackup[3][0];
    TmpMatrix[2][3] = MatrixBackup[3][1];
    TmpMatrix[1][3] = MatrixBackup[3][2];
    TmpMatrix[0][3] = MatrixBackup[3][3];
    //
    TmpMatrix[3][3] = MatrixBackup[3][0];
    TmpMatrix[3][2] = MatrixBackup[2][0];
    TmpMatrix[3][1] = MatrixBackup[1][0];
    TmpMatrix[3][0] = MatrixBackup[0][0];
    //
    TmpMatrix[0][0] = MatrixBackup[0][3];
    TmpMatrix[1][0] = MatrixBackup[0][2];
    TmpMatrix[2][0] = MatrixBackup[0][1];
    TmpMatrix[3][0] = MatrixBackup[0][0];
    //
    TmpMatrix[1][1] = MatrixBackup[1][2];
    TmpMatrix[1][2] = MatrixBackup[2][2];
    TmpMatrix[2][2] = MatrixBackup[2][1];
    TmpMatrix[2][1] = MatrixBackup[1][1];
    //
    SetMatrixInCorner();
    CalculateTmpSize();
}





void GenerateTmpMatrix(int FigIndex, TFigura  figura[4][4])
{
    // 0 = I, 1 = J, 2 = L, 3 = O, 4 = Z, 5 = T, 6 = S
    // Clear the old Matrix
    ClearTmpMatrix(figura);
    //
    if (FigIndex == 0)
    {
        // I
        FillTmpColor(TCOLOR_BLUE, figura);
        figura[0][0].Used = true;
        figura[0][1].Used = true;
        figura[0][2].Used = true;
        figura[0][3].Used = true;
    }
    if (FigIndex == 1)
    {
        // J
        FillTmpColor(TCOLOR_CYAN, figura);
        figura[1][0].Used = true;
        figura[1][1].Used = true;
        figura[1][2].Used = true;
        figura[0][2].Used = true;
    }
    if (FigIndex == 2)
    {
        // L
        FillTmpColor(TCOLOR_GRAY, figura);
        figura[0][0].Used = true;
        figura[0][1].Used = true;
        figura[0][2].Used = true;
        figura[1][2].Used = true;
    }
    if (FigIndex == 3)
    {
        // O
        FillTmpColor(TCOLOR_GREEN, figura);
        figura[0][0].Used = true;
        figura[0][1].Used = true;
        figura[1][1].Used = true;
        figura[1][0].Used = true;
    }
    if (FigIndex == 4)
    {
        // Z
        FillTmpColor(TCOLOR_ORANGE, figura);
        figura[0][0].Used = true;
        figura[1][0].Used = true;
        figura[1][1].Used = true;
        figura[2][1].Used = true;
    }
    if (FigIndex == 5)
    {
        // T
        FillTmpColor(TCOLOR_RED, figura);
        figura[1][0].Used = true;
        figura[0][1].Used = true;
        figura[1][1].Used = true;
        figura[2][1].Used = true;
    }
    if (FigIndex == 6)
    {
        // S
        FillTmpColor(TCOLOR_YELLOW, figura);
        figura[0][1].Used = true;
        figura[1][1].Used = true;
        figura[1][0].Used = true;
        figura[2][0].Used = true;
    }
    //
    CalculateTmpSize();
}





bool LoadFiles()
{
    // Start Loading Sprites Surfaces
    SPRITE_BORDER = LoadSurfaceFromFile("k_ramka.png");
    SPRITE_BLUE = LoadSurfaceFromFile("k_blue.png");
    SPRITE_CYAN = LoadSurfaceFromFile("k_cyan.png");
    SPRITE_GRAY = LoadSurfaceFromFile("k_gray.png");
    SPRITE_GREEN = LoadSurfaceFromFile("k_green.png");
    SPRITE_ORANGE = LoadSurfaceFromFile("k_orange.png");
    SPRITE_RED = LoadSurfaceFromFile("k_red.png");
    SPRITE_YELLOW = LoadSurfaceFromFile("k_yellow.png");

    // LOGO, NastepnaFigura, EkranStartowy, KoniecGry
    LOGO = LoadSurfaceFromFile("logo.png");
    NastepnaFigura = LoadSurfaceFromFile("nf.png");
    EkranStartowy = LoadSurfaceFromFile("tetris_logo.jpg");
    KoniecGry = LoadSurfaceFromFile("koniecgry.png");

    // Load Level Images
    L1 = LoadSurfaceFromFile("l1.png");
    L2 = LoadSurfaceFromFile("l2.png");
    L3 = LoadSurfaceFromFile("l3.png");
    L4 = LoadSurfaceFromFile("l4.png");
    L5 = LoadSurfaceFromFile("l5.png");

    // Load Music
    TetrisMusic = Mix_LoadMUS("Tetris.mp3");


    return false;
}

void UnloadFiles()
{
    // Unload Sprites Surfaces
    SDL_FreeSurface(SPRITE_BORDER);
    SDL_FreeSurface(SPRITE_BLUE);
    SDL_FreeSurface(SPRITE_CYAN);
    SDL_FreeSurface(SPRITE_GRAY);
    SDL_FreeSurface(SPRITE_GREEN);
    SDL_FreeSurface(SPRITE_ORANGE);
    SDL_FreeSurface(SPRITE_RED);
    SDL_FreeSurface(SPRITE_YELLOW);

    // LOGO, NastepnaFigura, EkranStartowy, KoniecGry
    SDL_FreeSurface(LOGO);
    SDL_FreeSurface(NastepnaFigura);
    SDL_FreeSurface(EkranStartowy);
    SDL_FreeSurface(KoniecGry);

    // Free Level Images
    SDL_FreeSurface(L1);
    SDL_FreeSurface(L2);
    SDL_FreeSurface(L3);
    SDL_FreeSurface(L4);
    SDL_FreeSurface(L5);

    // Free Music
    Mix_FreeMusic(TetrisMusic);

}

void DrawSurface(int x, int y, SDL_Surface* source, SDL_Surface* destination)
{
    // Create Surface Rectangle
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;

    // Blit
    SDL_BlitSurface(source, NULL, destination, &offset);
}

SDL_Surface *LoadSurfaceFromFile(std::string filename)
{
    // Surfaces
    SDL_Surface* LoadedImage = NULL;
    SDL_Surface* OptimizedImage = NULL;

    // Loaded Image
    LoadedImage = IMG_Load(filename.c_str());

    // if image loaded
    if (LoadedImage != NULL)
    {
        // Create a new optimized image
        OptimizedImage = SDL_DisplayFormat(LoadedImage);

        // Free the old surface[
        SDL_FreeSurface(LoadedImage);
    }

    // return the new optimized image
    return OptimizedImage;
}

// END
