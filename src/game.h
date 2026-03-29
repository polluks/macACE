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
    TEAM_ATTACKER = 2, //explictly set to match the values in baordState
    TEAM_DEFENDER = 1
} Team;
typedef struct g_piece{
    UBYTE pos; //pos in board array
    PieceType type;  // KING, DEFENDER, ATTACKER
    Team team;       // 0 = Defneder LIGHT, Anything else = Attacker DARK
    UBYTE captured;  //0 not captured, 1, captured, 2, captured but needs cleared from the board.
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
void drawBoard(void);
void drawPieces(void);
void drawSquareHighlight(void);
void resetGame(void);
void checkForWin(void);
void checkGameEnd(void);
void checkForCaptures(void);
void checkShieldWallCaptures(void);
void something(void);

#endif // _GAME_H_