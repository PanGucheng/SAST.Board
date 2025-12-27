#ifndef KEYBOARD_H
#define KEYBOARD_H

void key_scan(void);
void Key_Scan_Tick(void);
int Key_Get_Event(void);

typedef void (*KeyCallback_t)(void);
void Key_Register_Callback(int key, KeyCallback_t callback);
void Key_Execute_Callback(int key);

#endif //KEYBOARD_H
