#include "stdio.h"
#include "string.h"
#include "serpaeos.h"

#define C 262
#define Cs 277
#define Db 277
#define D 294
#define Ds 311
#define Eb 311
#define E 330
#define F 349
#define Fs 370
#define Gb 370
#define G 392
#define Gs 415
#define Ab 415
#define A 440
#define As 466
#define Bb 466
#define B 494

FILE *music;

void error(char *msg)
{
    printf("musicbox: ERROR: %s\nPress any key to terminate...", msg);
    getchar();
    exit(1);
}

long unsigned int next_note()
{
    char note = getc(music);
    if(note == 0 || note == EOF)
        error("end of file");
    if(note == 'r')
        return 0; //rest note
    if(note < 'a' && note > 'g')
        error("invalid note");


    char accidental = getc(music);
    if(accidental == 0 || note == EOF)
        error("end of file");
    if(accidental != 'b' && accidental != '#' && accidental != 'n')
        error("invalid accidental");
    
    switch(note)
    {
        case 'a':
            if(accidental == 'n')
                return A;
            if(accidental == '#')
                return As;
            return Ab;
        case 'b':
            if(accidental == 'n')
                return B;
            return Bb;
        case 'c':
            if(accidental == 'n')
                return C;
            return Cs;
        case 'd':
            if(accidental == 'n')
                return D;
            if(accidental == '#')
                return Ds;
            return Db;
        case 'e':
            if(accidental == 'n')
                return E;
            return Eb;
        case 'f':
            if(accidental == 'n')
                return F;
            return Fs;
        case 'g':
            if(accidental == 'n')
                return G;
            if(accidental == '#')
                return Gs;
            return Gb;
        default:
            error("invalid note");
    }
    return 0;
}

int main(int argc, char **argv)
{
    if(argc != 2)
        error("invalid usage");
    char path[108] = "0:/usr/files/musicbox/";
    strcpy(path + 22, argv[1]);
    music = fopen(path, "r");
    if(!music)
        error("file provided is inaccessable for reading");

    while(1)
    {
        long unsigned int note = next_note();
        if(note >= C)
            play(note);
        for(int i = 0 ; i < 400000000; i++);
        quit_sound();
    }
}