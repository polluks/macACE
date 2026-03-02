#include "game.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/blit.h> // Blitting fns
#include <ace/managers/state.h>
#include <ace/managers/rand.h>
#include <ace/managers/sprite.h>
#include <ace/utils/file.h>
#include <ace/utils/font.h>
#include <ace/utils/string.h>
#include <ace/utils/palette.h>

/*------DEFINES Setup------*/
//Amiga Pal LOWRES is 320x256
#define SCREEN_HEIGHT 256 //32 for the top viewport height
#define SCREEN_WIDTH 320
#define MAX_DEFENDERS 13  //12 Defenders + King
#define MAX_ATTACKERS 24  //24 Attackers
#define BOARD_WIDTH 256 
#define BOARD_HEIGHT 256
#define BOARD_ORIGIN_X 32 //offset from left edge of screen to start of board
#define BOARD_ORIGIN_Y 0 //offset from top edge of screen to start of board
#define SQUARE_WIDTH 32 //size of each square on the board in pixels
#define SQUARE_HEIGHT 25 //22 +3 for the line width
#define PIECE_SPRITE_WIDTH 32 //size of each piece sprite in pixels
#define PIECE_SPRITE_HEIGHT 22

/*------Setting Up Viewports-------*/
static tView *s_pView; // View containing all the viewports
static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer; //only a main screen in this, no score ribbon
static tRandManager *s_pRandManager;

/*-----Game Piece Setup-----*/
g_piece attackers[MAX_ATTACKERS];
g_piece defenders[MAX_DEFENDERS];

/*-----GFX Setup-----*/
static tBitMap *pBmBackground;
static tBitMap *pBmAttackers;
static tBitMap *pBmDefenders;
static tBitMap *pBmKing; //since the king is a different graphic
static tBitMap *pBmKing_Mask;
static tBitMap *pBmClashFX; //FX to flash when one piece takes another

tFont *gFontSmall; //global font for screen
tTextBitMap *testingbitmap;

/*-----Global Vars-----*/
ULONG startTime;
UBYTE board[169]; // flattened 13x13 board array, each index corresponds to a square on the board. oversized to avoid out of bounds errors, only 169 squares on the board. 0-168 valid indices.
UBYTE specialpos[5]; //four corners and the throne are only for the King


void gameGsCreate(void) {

    tRayPos sRayPos = getRayPos();

    s_pRandManager = randCreate(1+(sRayPos.bfPosY << 8), 1 + sRayPos.bfPosX);

    s_pView = viewCreate(0,TAG_VIEW_GLOBAL_PALETTE, 1,TAG_END);

    s_pVpMain = vPortCreate(0,
    TAG_VPORT_VIEW, s_pView,
    TAG_VPORT_BPP, 5, //needed 5 for 32 colours
    TAG_END);

    s_pMainBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpMain,
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
    TAG_SIMPLEBUFFER_IS_DBLBUF, 1, //add this line in for double buffering
    TAG_END);

    paletteLoadFromPath("/data/palette/GamePalette.plt", s_pVpMain->pPalette, 32);

    pBmBackground = bitmapCreateFromPath("/data/GFX/BG1.bm",0);
    //add asserts here later to check all this logic
    for(UWORD x = 0; x < s_pMainBuffer->uBfrBounds.uwX; x+=16){//fills out the background
    for(UWORD y = 0; y < s_pMainBuffer->uBfrBounds.uwY; y+=16){
      blitCopyAligned(pBmBackground,x,y,s_pMainBuffer->pBack,x,y,16,16);
        blitCopyAligned(pBmBackground,x,y,s_pMainBuffer->pFront,x,y,16,16);
        }
    }
    
    gFontSmall = fontCreateFromPath("/data/font/myacefont.fnt");
    
    //tTextBitMap *testingbitmap = fontCreateTextBitMapFromStr(gFontSmall, "THIS IS A TEST");
    //fontDrawTextBitMap(s_pMainBuffer->pBack, testingbitmap, 100, 100, 9,FONT_COOKIE);

    setupPieces(); //sets up the pieces in their starting positions in the board array and in the piece structs
    buildBoard(); //sets up the board array with the pieces in their starting positions and the special squares marked

    pBmKing = bitmapCreateFromPath("/data/GFX/King.bm",0);
    pBmKing_Mask = bitmapCreateFromPath("/data/GFX/King_Mask.bm",0);
    
    systemUnuse();
    // Load the view
    viewLoad(s_pView);
}

void gameGsLoop(void) {
    // This will loop every frame
  if(keyCheck(KEY_ESCAPE)) {
    gameExit();
  }
}

void gameGsDestroy(void) {
    systemUse();
    bitmapDestroy(pBmBackground);
    viewDestroy(s_pView);
}

void setupPieces(void){
  
  UBYTE attackerPositions[MAX_ATTACKERS] = { //predefined starting positions for attackers
    17,18,19,20,21,32,
    63,76,88,89,102,115,
    79,66,53,92,105,80,
    147,148,149,150,151,136
  };

  UBYTE defenderPositions[MAX_DEFENDERS] = { //predefined starting positions for defenders, including the king
    84, 58,70,71,72,82,83,85,86,96,97,98,110
  };

  // Set up the defenders
  for(int i = 0; i < MAX_DEFENDERS; i++){
    defenders[i].type = (i == 0) ? KING : DEFENDER; // First piece is the king, the rest are defenders
    defenders[i].team = TEAM_DEFENDER;
    defenders[i].captured = 0;
    defenders[i].pos = defenderPositions[i]; // Assign starting positions from the predefined array First position is the throne in the middle, the rest are around it
  }

  // Set up the attackers
  for(int i = 0; i < MAX_ATTACKERS; i++){
    attackers[i].type = ATTACKER;
    attackers[i].team = TEAM_ATTACKER;
    attackers[i].captured = 0;
    attackers[i].pos = attackerPositions[i]; // Assign starting positions from the predefined array
  }
}
void buildBoard(void){
    //0 = empty square, 1 = occupied by defender, 2 = occupied by attacker, 3 = occupied by king, 4 = special square ,99 = dead zone for out of bounds
    //initialize the board array to all 0s
    for(int i = 0; i < 169; i++){
        board[i] = 0;
    }
    //place the pieces on the board according to their positions in the piece structs
    for(UBYTE j = 0; j < MAX_DEFENDERS; j++){
        if(defenders[j].type == KING){
            board[defenders[j].pos] = 3; //king
        } else {
            board[defenders[j].pos] = 1; //defender
        }
    }
    //place the attackers on the board according to their positions in the piece structs
    for(UBYTE k = 0; k < MAX_ATTACKERS; k++){
        board[attackers[k].pos] = 2; //attacker
    }
    //sets the border squares to 99 to prevent out of bounds errors when checking for moves and captures
    for(UBYTE i = 0; i < 169; i++){
        if(i < 13 || i >= 156 || i % 13 == 0 || i % 13 == 12){
            board[i] = 99; //dead zone
            //logWrite("Setting board index %d to 99\n", i);
        }
    }

    //DEBUG: print the board to the log to check it's set up correctly
    // for(UBYTE i = 0; i < 169; i++){
    //     logWrite("%d ", board[i]);
    // }
    //set up the special positions for the corners and throne
    specialpos[0] = 13; //top left corner
    specialpos[1] = 24; //top right corner
    specialpos[2] = 144; //bottom left corner
    specialpos[3] = 154; //bottom right corner
    specialpos[4] = 84; //throne in the middle
}

void drawBoard(void){

}