
#ifndef _MENU_H_
#define _MENU_H_

void menu_init(void);
void menu_display(uint8_t menu);
void menu_background_process(void);
void toggle_switch(uint8_t number);
void updateRunning( int status );
void updateSwitchesDisplay();

#endif
