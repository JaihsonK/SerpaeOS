#ifndef ISR80H_IO_H
#define ISR80H_IO_H
#define func(x) void* x (struct interrupt_frame *frame)

struct interrupt_frame;
void* isr80h_command1_print(struct interrupt_frame* frame);
void* isr80h_command2_getkey(struct interrupt_frame* frame);
void* isr80h_command3_putchar(struct interrupt_frame* frame);
void* isr80h_command9_fopen(struct interrupt_frame* frame);
void* isr80h_command9_clrscr(struct interrupt_frame* frame);
void* isr80h_command10_fopen(struct interrupt_frame* frame);
void* isr80h_command11_fclose(struct interrupt_frame* frame);
void* isr80h_command12_fread(struct interrupt_frame* frame);
void *isr80h_command18_write_sector(struct interrupt_frame *frame);
void *isr80h_command19_read_sector(struct interrupt_frame *frame);
void *isr80h_command13_fwrite(struct interrupt_frame *frame);
void *isr80h_command20_fstat(struct interrupt_frame *frame);
void *isr80h_command21_ftell(struct interrupt_frame *frame);
void *isr80h_command22_fseek(struct interrupt_frame *frame);
void *isr80h_command23_remove(struct interrupt_frame *frame);
func(isr80h_command24_set_cursor);
func(isr80h_command25_draw_block);
func(isr80h_command26_play_sound);
func(isr80h_command27_quit_sound);

#endif