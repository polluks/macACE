#include "game.h"
#include "input.h"
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
//static tBitMap *pBmClashFX_BG; //for drawing the FX with the background when they move, to prevent leaving trails
static tBitMap *pBmSquareHighlight; //for highlighting valid moves and selected pieces
static tBitMap *pBmSquareHighlight_Mask;
static tBitMap *pBmSquareHighlight_BG[2]; //for drawing the highlight with the background when it moves, to prevent leaving trails
static tBitMap *pBmMouseCursorSrc;
static tBitMap *pBmMouseCursorData;

static tSprite *pSMouseCursor;


tFont *gFontSmall; //global font for screen
tTextBitMap *testingbitmap;

/*-----Global Vars-----*/
ULONG startTime;
UBYTE boardState[169]; // flattened 13x13 board array, each index corresponds to a square on the board. oversized to avoid out of bounds errors, only 169 squares on the board. 0-168 valid indices.
UBYTE boardStateNew[169]; //used to hold the new state of the board after a move, here we can check for captures, wins etc before doing a compare and draw the difference.
UBYTE validMoves[169]; //used to hold the valid moves for a selected piece, indexed the same as the board array, 0 = not valid, 1 = valid move. oversized to avoid out of bounds errors, only 169 squares on the board. 0-168 valid indices.
UBYTE currentPlayer = TEAM_ATTACKER; 
UBYTE s_ubBufferIndex = 0; //for double buffering, keeps track of which buffer we're currently drawing to (I don't think i'm doing this yet)
UBYTE hightlightActive = 0; //whether the highlight for valid moves is currently active, so we know whether to draw it or not in the drawPieces function, and whether to update it when a piece is selected.
UBYTE lastHighlightIndex[2] = {0, 0}; //the index of the last highlighted square, so we can restore the background when the highlight moves to a new square. This is needed because the highlight is drawn directly to the back buffer and not as a sprite, so we have to manually restore the background when it moves.
UBYTE highlightIndex = 0; //the index of the currently highlighted square, so we can update the highlight position when a new piece is selected or a move is made.
UBYTE pBm_hasBGToRestore[2] = {0, 0};//[2] = {0,0};
UBYTE validGeneration = 0; //used for tracking valid moves in the valid moves array. 

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

    drawBoard(); //draw the board to the back buffer before loading the view, so it's visible when the view loads
    
    gFontSmall = fontCreateFromPath("/data/font/myacefont.fnt");

    spriteManagerCreate(s_pView, 0, NULL);
    systemSetDmaBit(DMAB_SPRITE, 1);

    pBmMouseCursorSrc = bitmapCreateFromPath("/data/GFX/mousepointer.bm",0);
    pBmMouseCursorData = bitmapCreate(CURSOR_SPRITE_WIDTH,CURSOR_SPRITE_HEIGHT,2,BMF_INTERLEAVED | BMF_CLEAR);
    pSMouseCursor = spriteAdd(CURSOR_SPRITE_CHANNEL,pBmMouseCursorData);

    spriteSetEnabled(pSMouseCursor, 1);
    
    blitCopy(pBmMouseCursorSrc, 0,0,
    pBmMouseCursorData,0,0,CURSOR_SPRITE_WIDTH,CURSOR_SPRITE_HEIGHT,MINTERM_COOKIE);
    
    loadAssets();
    setupPieces(); //sets up the pieces in their starting positions in the board array and in the piece structs
    setupBoard(); //sets up the draw positions for each square on the board in the draw_pos array
    buildBoard(); //sets up the board array with the pieces in their starting positions and the special squares marked
    drawPieces(); //draws the board and pieces to the screen, will need to be called again every time a piece moves or is captured

    systemUnuse();
    // Load the view
    viewLoad(s_pView);
}


void gameGsLoop(void) {
    // This will loop every frame
    if(keyCheck(KEY_ESCAPE)) {
      gameExit();
    }

    short mouseX = mouseGetX(MOUSE_PORT_1);
    short mouseY = mouseGetY(MOUSE_PORT_1);

    //Do mouse stuff here to select and move pieces, check for captures and wins, etc.
    updateMousepos(mouseX, mouseY);

    //redraw the board and pieces every frame, for now we can optimize this later by only redrawing the squares that changed when a piece moves or is captured, but for now this is simpler 
    //to implement and works fine performance-wise since it's a small board and not many pieces.
    //use a memcmp when the board states is completed.
    drawPieces();
    
    if(mouseCheck(MOUSE_PORT_1, MOUSE_LMB)){
      onClick(mouseX, mouseY);
      getValidMoves();
      movePiece();
    } else if(mouseCheck(MOUSE_PORT_1, MOUSE_RMB)){
      if (hightlightActive) {
        hightlightActive = 0; 
       // drawBoard(); //not for real, just here so I can clear the board easy for testing.
      }
    
    }
    if (hightlightActive){ //if the highlight for valid moves is active, draw it
      drawSquareHighlight();
    }
    s_ubBufferIndex = !s_ubBufferIndex; //toggle the buffer index for double buffering    
     
    //switch to next player
    //currentPlayer = (currentPlayer == 0) ? 1 : 0;

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
  then when the player clicks a valid move, update the piece's position in its struct and update the boardState array, then call drawPieces() to update the screen. 
  After that, check for captures by looking at the squares around the moved piece in the boardState array, if there's an enemy piece there, 
  check if it's surrounded on the other side by a friendly piece or a special square, if it is, mark it as captured in its struct and update the boardState array to remove it from the board, 
  then call drawPieces() again to update the screen. 
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
  pBmSquareHighlight = bitmapCreateFromPath("/data/GFX/squarehighlight.bm",0);
  pBmSquareHighlight_Mask = bitmapCreateFromPath("/data/GFX/squarehighlight_mask.bm",0);
  pBmSquareHighlight_BG[0] = bitmapCreate(32,21,5,0); //size of the highlight sprite, for storing the background when drawing the highlight
  pBmSquareHighlight_BG[1] = bitmapCreate(32,21,5,0); //size of the highlight sprite, for storing the background when drawing the highlight
}
//sets up the pieces in their starting positions in the board array and in the piece structs
void setupPieces(void){
  
  UBYTE attackerPositions[MAX_ATTACKERS] = { //predefined starting positions for attackers
    17,18,19,20,21,32,
    63,76,88,89,102,115,  
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

//sets up the draw positions for each square on the board in the draw_pos array, which is indexed the same as the board array, so we can easily get the screen position for any square on the board when we need to draw pieces or highlights.
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

    //set up the special positions for the corners and throne
    boardState[13] = 4; //top left corner
    boardState[24] = 4; //top right corner
    boardState[144] = 4; //bottom left corner
    boardState[154] = 4; //bottom right corner
    boardState[84] = 4; //throne in the middle

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
}
//draws the pieces to the screen, will need to be called again every time a piece moves or is captured
//Look to make this more efficent later by only redrawing the squares that changed when a piece moves or is captured.
void drawPieces(void){ 
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

void drawBoard(void){
   pBmBoard = bitmapCreateFromPath("/data/GFX/BG1.bm",0);
    //add asserts here later to check all this logic
    for(UWORD x = 0; x < s_pMainBuffer->uBfrBounds.uwX; x+=16){//fills out the background
    for(UWORD y = 0; y < s_pMainBuffer->uBfrBounds.uwY; y+=16){
      blitCopyAligned(pBmBoard,x,y,s_pMainBuffer->pBack,x,y,16,16);
        blitCopyAligned(pBmBoard,x,y,s_pMainBuffer->pFront,x,y,16,16);
        }
    }
}

void updateMousepos(short mouseX, short mouseY){
  pSMouseCursor->wX = mouseX;
  pSMouseCursor->wY = mouseY;
    
  spriteProcess(pSMouseCursor);
  spriteProcessChannel(CURSOR_SPRITE_CHANNEL);
}

void onClick(short mouseX, short mouseY){
  for(UBYTE i = 0; i < 169; i++){
    //check if the mouse is within the bounds of this square
    if(mouseX >= draw_pos[i].x && mouseX <= draw_pos[i].x + SQUARE_X &&
       mouseY >= draw_pos[i].y && mouseY <= draw_pos[i].y + SQUARE_Y){
         logWrite("Clicked on square index %d\n", i); 
         //If a square is already Highlighted, set to zero for it to be restored
         if(hightlightActive){ 
            logWrite("Undraw Highlighted Index = %d\n", highlightIndex);
            hightlightActive = 0; 
         }
         
         highlightIndex = i; //set the highlight index to the square that was clicked

         hightlightActive = 1; //activate the highlight for valid moves
         logWrite("Highlighted Index = %d\n", highlightIndex);
        
         break; //exit the loop once we've found the square that was clicked
    }
  }
}
 //this function will be used to draw the highlight for valid moves and selected pieces, 
 //it will be called in the drawPieces function if the highlightActive variable is true, and the position will be determined by the highlightIndex variable which will be set when a piece is selected or a move is made.
void drawSquareHighlight(void){
 
  if(!hightlightActive) return;
  //First check if there's a background to restore from the last highlighted square, and if the highlight has moved to a new square, restore the background of the old highlighted square before drawing the new one
  if(highlightIndex != lastHighlightIndex[s_ubBufferIndex]){
    logWrite("Restoring background for index %d\n", lastHighlightIndex[s_ubBufferIndex]);
    //redraw the background to erase the old highlight
    
    blitCopy(pBmBoard, draw_pos[lastHighlightIndex[s_ubBufferIndex]].x, draw_pos[lastHighlightIndex[s_ubBufferIndex]].y,
    s_pMainBuffer->pBack, draw_pos[lastHighlightIndex[s_ubBufferIndex]].x, draw_pos[lastHighlightIndex[s_ubBufferIndex]].y,
    32, 21, MINTERM_COOKIE);
    
    pBm_hasBGToRestore[s_ubBufferIndex] = 0;
  }
    
  //Then draw the highlight with the mask for transparency
  blitCopyMask(pBmSquareHighlight,0,0,
  s_pMainBuffer->pBack, draw_pos[highlightIndex].x, draw_pos[highlightIndex].y,
  32,21,pBmSquareHighlight_Mask->Planes[0]);

  pBm_hasBGToRestore[s_ubBufferIndex] = 1;
  lastHighlightIndex[s_ubBufferIndex] = highlightIndex; //update the last highlighted index to the current one
}

/* This function will calculate the valid moves for the currently highlighted piece and populate the validMoves array, which is indexed the same as the board array, with a value over 0 for valid moves and 0 for invalid moves.*/
void getValidMoves(void){
 
  //First check if the square is unoccupid, if it is then stop.
  if(boardState[highlightIndex] == 0){
    return;
  }

  validGeneration++;

  //If generation is 0, it means that the UBYTE has >255 and the array needs reset
  if(validGeneration == 0){
    memset(validMoves, 0, sizeof(validMoves));
    validGeneration = 1;
  }
  
  /* This is no risk of over or under flow. Only 13-154 are valid game squares and only 14-153 are valid for piece movement */

  /*lets check the rows which are +1 and -1 This needs the minus (or plus) so the index doesn't start on the piece selected and auto fails.*/
  for(UBYTE r = (highlightIndex +1); r < 169; r++){ //rows in the + direction
    //if the square is occupied, break
    if(boardState[r] > 0){
      break; //if greater than 0 it means th square is occupied, a special square or out of bounds and invalid
    }
    validMoves[r] = validGeneration; //add the current position to the valid moves array. 
  }
  
  for(UBYTE u = (highlightIndex - 1); u < 169; u--){ //rows in the - direction
    if(boardState[u] > 0){
      break; 
    }
    validMoves[u] = validGeneration; 
  }

  /* **Check Coloums** */

  for(UBYTE c = (highlightIndex +13); c < 169; c=c+13){ 
    if(boardState[c] > 0){
      break;
    }
    validMoves[c] = validGeneration; 
  }
  
  for(UBYTE y = (highlightIndex -13); y < 169; y=y-13){ 
    if(boardState[y] > 0){
      break; 
    }
    validMoves[y] = validGeneration; 
  }
  
  
  
  /* Warning, this will not print the full array if you select a square and then quit, you must select another square (empty or not)*/
  #ifdef OUTPUT_LOGGING
  logWrite("validGeneration = %d\n", validGeneration);
  for(UBYTE i = 0; i < 169; i++){
    if(validMoves[i] == validGeneration){
      logWrite("Valid Moves at Square %d\n",i);
    }
  }
  #endif
}

void movePiece(void){
  //This function will move the currently highlighted piece to a new square if it's a valid move,
  // then update the boardState array and the piece's struct with the new position, and finally call drawPieces() to update the screen. It will also need to check for captures and wins after the move is made.
  
  //check if the selected new square is a valid move by checking the validMoves array at the highlightIndex, if it's not valid, return and do nothing
  if(validMoves[highlightIndex] != validGeneration){
    logWrite("Invalid move attempted to index %d\n", highlightIndex);
    return;
  }
  //This is not working, the if statement isn't right and needs a non tired revision. 
  
  if(currentPlayer == TEAM_DEFENDER){
    //find the piece that is being moved by checking the boardState at the lastHighlightIndex to see if it's a defender or the king, then loop through the defenders array to find the piece with the matching position and update its position to the new highlightIndex
    for(UBYTE j = 0; j < MAX_DEFENDERS; j++){
      if(defenders[j].pos == lastHighlightIndex[s_ubBufferIndex] && !defenders[j].captured){
        defenders[j].pos = highlightIndex; //update the piece's position in its struct
        
        if(defenders[j].type == KING) boardState[highlightIndex] = 3; //update the boardState array with the new position of the piece, 3 for king
        else boardState[highlightIndex] = 1; //update the boardState array with the new position of the piece, 1 for defender
        
        boardState[lastHighlightIndex[s_ubBufferIndex]] = 0; //set the old position to 0 for empty
        currentPlayer = TEAM_ATTACKER; //swap current player here
        break;
      }
    }
  } else {
    //do the same for attackers
    for(UBYTE k = 0; k < MAX_ATTACKERS; k++){
      if(attackers[k].pos == lastHighlightIndex[s_ubBufferIndex] && !attackers[k].captured){
        attackers[k].pos = highlightIndex;
       
        boardState[highlightIndex] = 2; //update the boardState array with the new position of the piece, 2 for attacker
        boardState[lastHighlightIndex[s_ubBufferIndex]] = 0; //set the old position to 0 for empty
        //swap current player here
        currentPlayer = TEAM_DEFENDER;
        break;
      }
    }
  }
  hightlightActive = 0; //deactivate the highlight after a move is made
}