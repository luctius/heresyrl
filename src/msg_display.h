#pragma once
#ifndef MSG_DISPLAY_H_
#define MSG_DISPLAY_H_


struct msg_win;

struct char_win *cw_create(int height, int width, int starty, int startx);
void cw_destroy(struct char_win *char_window);


#endif /*MSG_DISPLAY_H_*/
