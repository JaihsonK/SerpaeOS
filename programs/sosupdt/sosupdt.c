#include "stdio.h"
#include "memory.h"
#include "string.h"
#include "serpaeos.h"

#define sosupdt_version "23.2.1"

void pause()
{
    while(getkey() != EOF);
    return;
}

int main(int argc, char** argv)
{
    printf("SerpaeOS Update Center version %s\n", sosupdt_version);

    char path[108];

    if(argc != 2)
    {
        printf("\nPlease enter path to the kernel image: ");
        getline(path, 107);
    }
    else
        strcpy(path, argv[1]);

    printf("\nAre you sure you want to use %s as your kernel image? Using a kernel image not produced by SerpaeOS developers can be very dangerous. [Y/n]", path);
    char c;
    while((c = getchar()) != 'Y' && c != 'y' && c != 'N' && c != 'n');
    if(c == 'N' || c == 'n')
        return -1;
    FILE *kernel_image = fopen(path, "r");
    if(!kernel_image)
    {
        printf("\nsosupdt: ERROR: file %s cannot be accessed", path);
        pause();
        return -1;
    }

    printf("\nWorking...");
    char buf[512];
    int sector = 0, res, filesize;

    fseek(kernel_image, 0, SEEK_END);
    filesize = ftell(kernel_image);
    fseek(kernel_image, 0, SEEK_SET);

    while(filesize > 0)
    {
        filesize -= 512;
        fread(kernel_image, 512, buf);
        printf("\nSector %d", sector);
        res = sos_writesector(sector, 0, buf);
        if(res == 0)
            sector++;
        else
        {
            printf("\nsosupdt: Fail on sector %d. SerpaeOS may have been corrupted, and it may be best that you don't boot this copy of SerpaeOS ever again. Your files can probably be backed up before you reinstall SerpaeOS.\nFor help, contact us at \"serpaeos.devers@gmail.com\".Press any key to continue...", sector);
            getchar();
            fclose(kernel_image);
            return -2;
        }
        memset(buf, 0, 512);
    }

    printf("\nDone! Once you reboot, you can enjoy an updated SerpaeOS!\a");
    getchar();
    return 0;
}
