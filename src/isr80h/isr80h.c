#include "isr80h.h"
#include "idt/idt.h"
#include "misc.h"
#include "io.h"
#include "process.h"
#include "memory.h"

void isr80h_register_commands()
{
    isr80h_register_command(SYSTEM_COMMAND0_EXIT, isr80h_command0_exit);
    isr80h_register_command(SYSTEM_COMMAND1_PRINT, isr80h_command1_print);
    isr80h_register_command(SYSTEM_COMMAND2_GETKEY, isr80h_command2_getkey);
    isr80h_register_command(SYSTEM_COMMAND3_PUTCHAR, isr80h_command3_putchar);
    isr80h_register_command(SYSTEM_COMMAND4_MALLOC, isr80h_command4_malloc);
    isr80h_register_command(SYSTEM_COMMAND5_FREE, isr80h_command5_free);
    isr80h_register_command(SYSTEM_COMMAND6_PROCESS_LOAD_START, isr80h_command6_process_load_start);
    isr80h_register_command(SYSTEM_COMMAND8_GET_PROGRAM_ARGUMENTS, isr80h_command8_get_program_arguments);
    isr80h_register_command(SYSTEM_COMMAND7_INVOKE_SYSTEM_COMMAND, isr80h_command7_invoke_system_command);
    isr80h_register_command(SYSTEM_COMMAND9_CLRSCR, isr80h_command9_clrscr);
    isr80h_register_command(SYSTEM_COMMAND10_FOPEN, isr80h_command10_fopen);
    isr80h_register_command(SYSTEM_COMMAND11_FCLOSE, isr80h_command11_fclose);
    isr80h_register_command(SYSTEM_COMMAND12_FREAD, isr80h_command12_fread);
    isr80h_register_command(SYSTEM_COMMAND13_FWRITE, isr80h_command13_fwrite);
    isr80h_register_command(SYSTEM_COMMAND14_REMOTE_TEMINATE, isr80h_command14_remote_terminate);
    isr80h_register_command(SYSTEM_COMMAND15_PROCSS_INF, isr80h_command15_process_inf);
    isr80h_register_command(SYSTEM_COMMAND16_NEW_THREAD, isr80h_command16_new_thread);
    isr80h_register_command(SYSTEM_COMMAND18_WRITE_SECTOR, isr80h_command18_write_sector);
    isr80h_register_command(SYSTEM_COMMAND19_READ_SECTOR, isr80h_command19_read_sector);
    isr80h_register_command(SYSTEM_COMMAND20_FSTAT, isr80h_command20_fstat);
    isr80h_register_command(SYSTEM_COMMAND21_FTELL, isr80h_command21_ftell);
    isr80h_register_command(SYSTEM_COMMAND22_FSEEK, isr80h_command22_fseek);
    isr80h_register_command(SYSTEM_COMMAND23_REMOVE, isr80h_command23_remove);
    isr80h_register_command(SYSTEM_COMMAND24_SET_CURSOR, isr80h_command24_set_cursor);
    isr80h_register_command(SYSTEM_COMMAND25_DRAW_BLOCK, isr80h_command25_draw_block);
    isr80h_register_command(SYSTEM_COMMAND26_PLAY_SOUND, isr80h_command26_play_sound);
    isr80h_register_command(SYSTEM_COMMAND27_QUIT_SOUND, isr80h_command27_quit_sound);
    //isr80h_register_command(SYSTEM_COMMAND28_FORK, isr80h_command28_fork);


    isr80h_register_command(255, isr80h_command255_get_retcode);
}