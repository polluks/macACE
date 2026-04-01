#include "menu.h"
#include "input.h"
#include "states.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/blit.h> // Blitting fns
#include <ace/managers/state.h>
#include <ace/managers/sprite.h>
#include <ace/utils/file.h>
#include <ace/utils/font.h>
#include <ace/utils/string.h>
#include <ace/utils/palette.h>
#include <stdbool.h>
#include <stdlib.h>

#define MENU_HEIGHT	256 //y
#define MENU_WIDTH	320 //x

/*-----Seeting Up Viewports-----*/
static tView *s_pMenuView; //view for all viewports
static tVPort *s_pVpMain;//viewport for the menu
static tSimpleBufferManager *s_pMainBuffer;

/*-----GFX Setup-----*/
tFont *menufont;    //font
tTextBitMap *menutextbitmap; //bitmap for the font
static tBitMap *pBDefwin;
static tBitMap *pBmAckwin;

void menuGsCreate(void){
    // //create view port and the display buffer for the main viewport
    s_pMenuView = viewCreate(0, TAG_VIEW_GLOBAL_PALETTE, 1, TAG_END);

    s_pVpMain = vPortCreate(0, TAG_VPORT_VIEW, s_pMenuView, TAG_VPORT_BPP, 5, TAG_END);
    s_pMainBuffer = simpleBufferCreate(0,TAG_SIMPLEBUFFER_VPORT, s_pVpMain, TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_INTERLEAVED, BMF_CLEAR, TAG_END);
    
    //colour palette for the menu
    paletteLoadFromPath("data/palette/menuPalette.plt", s_pVpMain->pPalette, 32);
    
    pBDefwin = bitmapCreateFromPath("data/GFX/defwin.bm",0);
    pBmAckwin = bitmapCreateFromPath("data/GFX/ackwin.bm",0);
    
    menufont = fontCreateFromPath("data/font/myacefont.fnt");

    if(gameWinner == 1){
        //draw attackers win background
        for(UWORD x = 0; x < s_pMainBuffer->uBfrBounds.uwX; x+=16){
        for(UWORD y = 0; y < s_pMainBuffer->uBfrBounds.uwY; y+=16){
        blitCopyAligned(pBmAckwin,x,y,s_pMainBuffer->pBack,x,y,16,16);
            blitCopyAligned(pBmAckwin,x,y,s_pMainBuffer->pFront,x,y,16,16);
            }
        }

        menutextbitmap = fontCreateTextBitMapFromStr(menufont, "ATTACKERS WIN");
        fontDrawTextBitMap(s_pMainBuffer->pBack, menutextbitmap, MENU_WIDTH / 2 - 36, MENU_HEIGHT / 2 + 95, 8, FONT_COOKIE);
        fontDestroyTextBitMap(menutextbitmap);
    }
    else if(gameWinner == 2){

        for(UWORD x = 0; x < s_pMainBuffer->uBfrBounds.uwX; x+=16){//fills out the background
        for(UWORD y = 0; y < s_pMainBuffer->uBfrBounds.uwY; y+=16){
        blitCopyAligned(pBDefwin,x,y,s_pMainBuffer->pBack,x,y,16,16);
            blitCopyAligned(pBDefwin,x,y,s_pMainBuffer->pFront,x,y,16,16);
            }
        }

        menutextbitmap = fontCreateTextBitMapFromStr(menufont, "DEFENDERS WIN");
        fontDrawTextBitMap(s_pMainBuffer->pBack, menutextbitmap, MENU_WIDTH / 2 - 36, MENU_HEIGHT / 2 + 95, 8, FONT_COOKIE);
        fontDestroyTextBitMap(menutextbitmap); 
    }
    menutextbitmap = fontCreateTextBitMapFromStr(menufont, "Play Again Y N");
    fontDrawTextBitMap(s_pMainBuffer->pBack, menutextbitmap, MENU_WIDTH / 2 - 36, MENU_HEIGHT / 2 + 84, 8, FONT_COOKIE);
    fontDestroyTextBitMap(menutextbitmap);

    systemUnuse();
    viewLoad(s_pMenuView);
}

void menuGsLoop(void){
    if(keyCheck(KEY_N)){
        logWrite("getting outta here!\n");
        gameExit();
    }
    
    //logWrite("Looking at that else!\n");
    if(keyCheck(KEY_Y)){
        logWrite("Going back to the Game!\n");
       // gameWinner = 3; //set the reset signal for the game loop to reset the game.
        stateChange(g_pStateManager, g_pGameState);
        logWrite("Switching!\n");
        return;
    }

    // //may induce lag having to destroy the text every frame?
    // menutextbitmap = fontCreateTextBitMapFromStr(menufont, "GAME OVER");
    // fontDrawTextBitMap(s_pMainBuffer->pBack, menutextbitmap, MENU_WIDTH / 2 - 36, MENU_HEIGHT / 2 + 105, 8, FONT_COOKIE);
    // //fontDestroyTextBitMap(menutextbitmap);

    copProcessBlocks();
    systemIdleBegin();
    vPortWaitUntilEnd(s_pVpMain);
}

void menuGsDestroy(void){
    systemUse();
    bitmapDestroy(pBmAckwin);
    bitmapDestroy(pBDefwin);
    fontDestroyTextBitMap(menutextbitmap);
    viewDestroy(s_pMenuView);
}