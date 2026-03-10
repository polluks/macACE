#ifndef _MOUSE_H_
#define _MOUSE_H_

#include <ace/managers/system.h>

void inputOpen();
void inputClose();
void inputProcess();
void updateMousepos(short mouseX, short mouseY);
void onClick(short mouseX, short mouseY);

#endif // _MOUSE_H_