#include "game.h"
#include "mouse.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/blit.h> // Blitting fns
#include <ace/managers/state.h>
#include <ace/managers/rand.h>
#include <ace/managers/sprite.h>
#include <ace/managers/mouse.h>
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
#define BOARD_ORIGIN_X 28 //offset from left edge of screen to start of board
#define BOARD_ORIGIN_Y 3 //offset from top edge of screen to start of board
#define SQUARE_Y 23 //size of each square on the board in pixels
#define SQUARE_X 23
#define PIECE_SPRITE_WIDTH 32 //size of each piece sprite in pixels
#define PIECE_SPRITE_HEIGHT 21
#define CURSOR_SPRITE_WIDTH 16
#define CURSOR_SPRITE_HEIGHT 18
#define CURSOR_SPRITE_CHANNEL 5
//#define OUTPUT_LOGGING //uncomment to enable more logging on arrays and positions in the debug.txt file.

/*------Setting Up Viewports-------*/
static tView *s_pView; // View containing all the viewports
static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer; //only a main screen in this, no score ribbon
static tRandManager *s_pRandManager;

/*-----Game Piece Setup-----*/
g_piece attackers[MAX_ATTACKERS];
g_piece defenders[MAX_DEFENDERS];

/*-----GFX Setup-----*/
static tBitMap *pBmBoard;
static tBitMap *pBmAttackers[MAX_ATTACKERS];
static tBitMap *pBmAttackers_Mask[MAX_ATTACKERS];
static tBitMap *pBmAttackers_BG[MAX_ATTACKERS]; //for drawing the pieces with the background when they move, to prevent leaving trails
static tBitMap *pBmDefenders[MAX_DEFENDERS];
static tBitMap *pBmDefenders_Mask[MAX_DEFENDERS];
static tBitMap *pBmDefenders_BG[MAX_DEFENDERS]; //for drawing the pieces with the background when they move, to prevent leaving trails
static tBitMap *pBmKing; //since the king is a different graphic
static tBitMap *pBmKing_Mask;
static tBitMap *pBmKing_BG; 
static tBitMap *pBmClashFX; //FX to flash when one piece takes another
static tBitMap *pBmClashFX_Mask;
static tBitMap *pBmClashFX_BG; //for drawing the FX with the background when they move, to prevent leaving trails
static tBitMap *pBmSquareHighlight; //for highlighting valid moves and selected pieces
static tBitMap *pBmSquareHighlight_Mask;
static tBitMap *pBmSquareHighlight_BG; //for drawing the highlight with the background when it moves, to prevent leaving trails
static tBitMap *pBmMouseCursorSrc;
static tBitMap *pBmMouseCursorData;

static tSprite *pBmMouseCursor;


tFont *gFontSmall; //global font for screen
tTextBitMap *testingbitmap;

/*-----Global Vars-----*/
ULONG startTime;
UBYTE boardState[169]; // flattened 13x13 board array, each index corresponds to a square on the board. oversized to avoid out of bounds errors, only 169 squares on the board. 0-168 valid indices.
UBYTE boardStateNew[169]; //used to hold the new state of the board after a move, here we can check for captures, wins etc before doing a compare and draw the difference.
UBYTE validMoves[169]; //used to hold the valid moves for a selected piece, indexed the same as the board array, 0 = not valid, 1 = valid move. oversized to avoid out of bounds errors, only 169 squares on the board. 0-168 valid indices.
UBYTE specialpos[5]; //four corners and the throne are only for the King
UBYTE currentPlayer = 1; //0 = defender, 1 = attacker : games start with attacker turn so this is initialised to 1
UBYTE s_ubBufferIndex = 0; //for double buffering, keeps track of which buffer we're currently drawing to

ScreenPos draw_pos[169];

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
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_INTERLEAVED,
    TAG_SIMPLEBUFFER_IS_DBLBUF, 1, //add this line in for double buffering
    TAG_END);

    paletteLoadFromPath("/data/palette/GamePalettev2.plt", s_pVpMain->pPalette, 32);

    pBmBoard = bitmapCreateFromPath("/data/GFX/BG1.bm",0);
    //add asserts here later to check all this logic
    for(UWORD x = 0; x < s_pMainBuffer->uBfrBounds.uwX; x+=16){//fills out the background
    for(UWORD y = 0; y < s_pMainBuffer->uBfrBounds.uwY; y+=16){
      blitCopyAligned(pBmBoard,x,y,s_pMainBuffer->pBack,x,y,16,16);
        blitCopyAligned(pBmBoard,x,y,s_pMainBuffer->pFront,x,y,16,16);
        }
    }
    
    gFontSmall = fontCreateFromPath("/data/font/myacefont.fnt");

    spriteManagerCreate(s_pView, 0, NULL);
    systemSetDmaBit(DMAB_SPRITE, 1);

    pBmMouseCursorSrc = bitmapCreateFromPath("/data/GFX/mousepointer.bm",0);
    pBmMouseCursorData = bitmapCreate(CURSOR_SPRITE_WIDTH,CURSOR_SPRITE_HEIGHT,2,BMF_INTERLEAVED | BMF_CLEAR);
    pBmMouseCursor = spriteAdd(CURSOR_SPRITE_CHANNEL,pBmMouseCursorData);

    spriteSetEnabled(pBmMouseCursor, 1);
    
    // blitCopyMask(pBmMouseCursorSrc,0,0,
    // pBmMouseCursorData,0,0,CURSOR_SPRITE_WIDTH,16,pBmMouseCursorSrc->Planes[0]);
    blitCopy(pBmMouseCursorSrc, 0,0,
    pBmMouseCursorData,0,0,CURSOR_SPRITE_WIDTH,16,MINTERM_COOKIE);
    
    loadAssets();
    setupPieces(); //sets up the pieces in their starting positions in the board array and in the piece structs
    setupBoard(); //sets up the draw positions for each square on the board in the draw_pos array
    buildBoard(); //sets up the board array with the pieces in their starting positions and the special squares marked
    drawBoard(); //draws the board and pieces to the screen, will need to be called again every time a piece moves or is captured

    systemUnuse();
    // Load the view
    viewLoad(s_pView);
}


void gameGsLoop(void) {
    // This will loop every frame
    if(keyCheck(KEY_ESCAPE)) {
      gameExit();
    }
    //Do mouse stuff here to select and move pieces, check for captures and wins, etc.
  
    UWORD mouseX = mouseGetX(MOUSE_PORT_1);
    UWORD mouseY = mouseGetY(MOUSE_PORT_1);
    
    pBmMouseCursor->wX = mouseX;
    pBmMouseCursor->wY = mouseY;
    
    spriteProcess(pBmMouseCursor);
    spriteProcessChannel(CURSOR_SPRITE_CHANNEL);
    //redraw the board and pieces every frame, for now we can optimize this later by only redrawing the squares that changed when a piece moves or is captured, but for now this is simpler 
    //to implement and works fine performance-wise since it's a small board and not many pieces.
    drawBoard();
    
    s_ubBufferIndex = !s_ubBufferIndex; //toggle the buffer index for double buffering    
     
    //switch to next player
    currentPlayer = (currentPlayer == 0) ? 1 : 0;

    viewProcessManagers(s_pView);
    copProcessBlocks();
    systemIdleBegin();
    vPortWaitForEnd(s_pVpMain);
    systemIdleEnd();
}
  /* On click
  find the board position given the mouse x/y and the draw_pos array
  check if there's a piece in that position that belongs to the current player using the boardState array
  if there is, calculate the valid moves for that piece and populate the validMoves array
  then when the player clicks a valid move, update the piece's position in its struct and update the boardState array, then call drawBoard() to update the screen. 
  After that, check for captures by looking at the squares around the moved piece in the boardState array, if there's an enemy piece there, 
  check if it's surrounded on the other side by a friendly piece or a special square, if it is, mark it as captured in its struct and update the boardState array to remove it from the board, 
  then call drawBoard() again to update the screen. 
  Finally, check for wins by seeing if the king is captured or if he reaches a corner, and if so, end the game and show a win screen.
  
  */
void gameGsDestroy(void) {
    systemUse();
    bitmapDestroy(pBmBoard);
    viewDestroy(s_pView);
}
//loads in the game assets, including the piece sprites and their masks for blitting with transparency.
void loadAssets(void){
  for(UBYTE i = 0; i < MAX_ATTACKERS; i++){
    pBmAttackers[i] = bitmapCreateFromPath("/data/GFX/attackers.bm",0);
    pBmAttackers_Mask[i] = bitmapCreateFromPath("/data/GFX/attackers_mask.bm",0);
  }
  for(UBYTE j = 0; j < MAX_DEFENDERS; j++){
    pBmDefenders[j] = bitmapCreateFromPath("/data/GFX/defenders.bm",0);
    pBmDefenders_Mask[j] = bitmapCreateFromPath("/data/GFX/defenders_mask.bm",0);
  }
  pBmKing = bitmapCreateFromPath("/data/GFX/king.bm",0);
  pBmKing_Mask = bitmapCreateFromPath("/data/GFX/king_mask.bm",0);
  pBmClashFX = bitmapCreateFromPath("/data/GFX/clashFX.bm",0);
  pBmClashFX_Mask = bitmapCreateFromPath("/data/GFX/clashFX_mask.bm",0);
}
//sets up the pieces in their starting positions in the board array and in the piece structs
void setupPieces(void){
  
  UBYTE attackerPositions[MAX_ATTACKERS] = { //predefined starting positions for attackers
    17,18,19,20,21,32,
    63,76,88,89,102,115, //something in this  row isn't working. 
    79,66,53,92,105,80,
    147,148,149,150,151,136
  };

  UBYTE defenderPositions[MAX_DEFENDERS] = { //predefined starting positions for defenders, including the king
    84,58,70,71,72,82,83,85,86,96,97,98,110
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

void setupBoard(void){
  for(UBYTE y = 0; y < 13; y++){
    for(UBYTE x = 0; x < 13; x++){
      UBYTE i = y * 13 + x; //calculate the index in the board array for this position
      draw_pos[i].x = BOARD_ORIGIN_X + (x - 1) * SQUARE_X; //calculate the screen position for this square based on the origin and square size
      draw_pos[i].y = BOARD_ORIGIN_Y + (y - 1) * SQUARE_Y;
    }
  }

  //DEBUG: print the draw positions to the log to check they're correct
  #ifdef OUTPUT_LOGGING
  for(UBYTE i = 0; i < 169; i++){
    logWrite("Draw POS: %d is x: %d y: %d \n", i, draw_pos[i].x, draw_pos[i].y);
  }
  #endif
}
//sets up the board array with the pieces in their starting positions and the special squares marked
void buildBoard(void){
    //0 = empty square, 1 = occupied by defender, 2 = occupied by attacker, 3 = occupied by king, 4 = special square ,99 = dead zone for out of bounds
    //initialize the board array to all 0s
    for(int i = 0; i < 169; i++){
        boardState[i] = 0;
    }
    //place the pieces on the board according to their positions in the piece structs
    for(UBYTE j = 0; j < MAX_DEFENDERS; j++){
        if(defenders[j].type == KING){
            boardState[defenders[j].pos] = 3; //king
        } else {
            boardState[defenders[j].pos] = 1; //defender
        }
    }
    //place the attackers on the board according to their positions in the piece structs
    for(UBYTE k = 0; k < MAX_ATTACKERS; k++){
        boardState[attackers[k].pos] = 2; //attacker
        //logWrite("Placing attacker at index %d\n", attackers[k].pos);
    }
    //sets the border squares to 99 to prevent out of bounds errors when checking for moves and captures
    for(UBYTE i = 0; i < 169; i++){
        if(i < 13 || i >= 156 || i % 13 == 0 || i % 13 == 12){
            boardState[i] = 99; //dead zone
            //logWrite("Setting board index %d to 99\n", i);
        }
    }

    //DEBUG: print the board to the log to check it's set up correctly
    #ifdef OUTPUT_LOGGING
    for(UBYTE i = 0; i < 169; i++){
        logWrite("Board POS: %d is %d \n", i, boardState[i]);
    }
    #endif

    //set up the special positions for the corners and throne
    specialpos[0] = 13; //top left corner
    specialpos[1] = 24; //top right corner
    specialpos[2] = 144; //bottom left corner
    specialpos[3] = 154; //bottom right corner
    specialpos[4] = 84; //throne in the middle
}
//draws the pieces to the screen, will need to be called again every time a piece moves or is captured
//Look to make this more efficent later by only redrawing the squares that changed when a piece moves or is captured.
void drawBoard(void){ 
  for (UBYTE i = 0; i < 169; i++){
    if(boardState[i] == 1){ //defender
      for(UBYTE j = 0; j < MAX_DEFENDERS; j++){
        if(defenders[j].pos == i && !defenders[j].captured){ //find the defender that is in this position and hasn't been captured, then draw it
            //copy the background to the piece's BG bitmap for later restoration when it moves
            blitCopy(s_pMainBuffer->pBack, draw_pos[i].x, draw_pos[i].y, 
            pBmDefenders_BG[j], 0, 0, 
            PIECE_SPRITE_WIDTH, PIECE_SPRITE_HEIGHT, MINTERM_COOKIE); 
              
            //Then draw the piece with the mask for transparency
            blitCopyMask(pBmDefenders[j],0,0, 
            s_pMainBuffer->pBack, draw_pos[i].x, draw_pos[i].y,
            PIECE_SPRITE_WIDTH,PIECE_SPRITE_HEIGHT,pBmDefenders_Mask[j]->Planes[0]);
        }
      }
    } else if(boardState[i] == 2){ //attacker
      for(UBYTE k = 0; k < MAX_ATTACKERS; k++){
        if(attackers[k].pos == i && !attackers[k].captured){
            //copy the background to the piece's BG bitmap for later restoration when it moves
            blitCopy(s_pMainBuffer->pBack, draw_pos[i].x, draw_pos[i].y, 
            pBmAttackers_BG[k], 0, 0, 
            PIECE_SPRITE_WIDTH, PIECE_SPRITE_HEIGHT, MINTERM_COOKIE); 
              
            //Then draw the piece with the mask for transparency
            blitCopyMask(pBmAttackers[k],0,0,
            s_pMainBuffer->pBack, draw_pos[i].x, draw_pos[i].y,
            PIECE_SPRITE_WIDTH,PIECE_SPRITE_HEIGHT,pBmAttackers_Mask[k]->Planes[0]);
        }
      }
    } else if(boardState[i] == 3){ //king
      //copy the background to the king's BG bitmap for later restoration when it moves
      blitCopy(s_pMainBuffer->pBack, draw_pos[i].x, draw_pos[i].y, 
      pBmKing_BG, 0, 0,
      PIECE_SPRITE_WIDTH, PIECE_SPRITE_HEIGHT, MINTERM_COOKIE);
      
      //Then draw the king with the mask for transparency
      blitCopyMask(pBmKing,0,0,
        s_pMainBuffer->pBack, draw_pos[i].x, draw_pos[i].y,
        PIECE_SPRITE_WIDTH,PIECE_SPRITE_HEIGHT,pBmKing_Mask->Planes[0]);
    }
  }
}

void createMouseCursor(void){
  //initialise mouse cursor sprite and mask here, then set it as the active cursor with mouseSetCursor() so we can use it for piece selection and movement.
}