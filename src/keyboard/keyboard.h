#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_CAPSLOCK_ON 1
#define KEYBOARD_CAPSLOCK_OFF 0

#define KEY_DOWN 220
#define KEY_UP 221
#define KEY_RIGHT 223
#define KEY_LEFT 224

struct process;

typedef int (*KEYBOARD_INIT_FUNCTION)();
struct keyboard
{
    KEYBOARD_INIT_FUNCTION init;
    char name[20];
    struct keyboard* next;
};

void keyboard_init();
void keyboard_backspace(struct process* process);
void keyboard_push(char c);
char keyboard_pop();
int keyboard_insert(struct keyboard* keyboard);


#endif