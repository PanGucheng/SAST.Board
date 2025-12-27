#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <stdint.h>

void countdown_init(void);
void countdown_tick(void);
void countdown_display(void);
void countdown_control(int key);

#endif // COUNTDOWN_H

