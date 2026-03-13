#ifndef _GAME_H_
#define _GAME_H_

#include <ace/utils/file.h>

typedef enum 
{
    DEFENDER,
    ATTACKER,
    BESERKER,
    KNIGHT,
    KING
} PieceType;
typedef enum{
    TEAM_ATTACKER,
    TEAM_DEFENDER
} Team;
typedef struct g_piece{
    UBYTE pos; //pos in board array
    PieceType type;  // KING, DEFENDER, ATTACKER
    Team team;       // 0 = Defneder LIGHT, Anything else = Attacker DARK
    UBYTE captured;  //0 not captured, anything else = captured.
} g_piece;

//struct for the x/y positions of the squares on the board for drawing the pieces to the screen, indexed the same as the board array
//needs shorts since the draw positions are bigger than a UBYTE can hold.
typedef struct
{
    short x;
    short y;
} ScreenPos;

void gameGsCreate(void);
void gameGsLoop(void);
void gameGsDestroy(void);
void buildBoard(void);
void setupBoard(void);
void setupPieces(void);
void loadAssets(void);
void getValidMoves(void);
void movePiece(void);
//void checkForCaptures(g_piece *movedPiece);
void checkForWin(void);
void drawBoard(void);
void drawPieces(void);
void drawSquareHighlight(void);

#endif // _GAME_H_