FILES = ./build/kernel.asm.o ./build/kernel.o ./build/time/time.o ./build/graphics/graphics.o ./build/loader/formats/elf.o ./build/power/power.o ./build/isr80h/process.o ./build/sound/sound.o ./build/fs/fat/fat16_write.o ./build/fs/fat32/fat32.o ./build/loader/formats/elfloader.o ./build/isr80h/memory.o ./build/isr80h/isr80h.o ./build/keyboard/keyboard.o ./build/keyboard/classic.o ./build/isr80h/io.o ./build/isr80h/misc.o ./build/disk/disk.o ./build/disk/streamer.o ./build/task/process.o ./build/task/task.o ./build/task/task.asm.o ./build/task/tss.asm.o ./build/fs/pparser.o ./build/fs/file.o ./build/fs/fat/fat16.o ./build/string/string.o ./build/idt/idt.asm.o ./build/idt/idt.o ./build/memory/memory.o ./build/io/io.asm.o ./build/gdt/gdt.o ./build/gdt/gdt.asm.o ./build/memory/heap/heap.o ./build/memory/heap/kheap.o ./build/memory/paging/paging.o ./build/memory/paging/paging.asm.o
INCLUDES = -I./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ./bin/boot.bin ./bin/kernel.bin user_programs
	rm -rf ./bin/SerpaeOS.img
	dd if=./bin/boot.bin >> ./bin/SerpaeOS.img
	dd if=./bin/kernel.bin >> ./bin/SerpaeOS.img
	dd if=./bin/SerpaeOS.img >> ./bin/serpaeos.ki
	dd if=/dev/zero bs=1048576 count=4096 >> ./bin/SerpaeOS.img
	sudo mount -t vfat ./bin/SerpaeOS.img /mnt/d
	sudo cp ./LICENSE ./dirsys/usr/files
	sudo cp -r ./dirsys/sys /mnt/d
	sudo cp -r ./dirsys/usr /mnt/d
	sudo umount /mnt/d

./bin/kernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/kernel.asm
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

./build/kernel.o: ./src/kernel.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o

./build/idt/idt.asm.o: ./src/idt/idt.asm
	nasm -f elf -g ./src/idt/idt.asm -o ./build/idt/idt.asm.o

./build/loader/formats/elf.o: ./src/loader/formats/elf.c
	i686-elf-gcc $(INCLUDES) -I./src/loader/formats $(FLAGS) -std=gnu99 -c ./src/loader/formats/elf.c -o ./build/loader/formats/elf.o

./build/loader/formats/elfloader.o: ./src/loader/formats/elfloader.c
	i686-elf-gcc $(INCLUDES) -I./src/loader/formats $(FLAGS) -std=gnu99 -c ./src/loader/formats/elfloader.c -o ./build/loader/formats/elfloader.o

./build/gdt/gdt.o: ./src/gdt/gdt.c
	i686-elf-gcc $(INCLUDES) -I./src/gdt $(FLAGS) -std=gnu99 -c ./src/gdt/gdt.c -o ./build/gdt/gdt.o

./build/gdt/gdt.asm.o: ./src/gdt/gdt.asm
	nasm -f elf -g ./src/gdt/gdt.asm -o ./build/gdt/gdt.asm.o


./build/isr80h/isr80h.o: ./src/isr80h/isr80h.c
	i686-elf-gcc $(INCLUDES) -I./src/isr80h $(FLAGS) -std=gnu99 -c ./src/isr80h/isr80h.c -o ./build/isr80h/isr80h.o

./build/isr80h/memory.o: ./src/isr80h/memory.c
	i686-elf-gcc $(INCLUDES) -I./src/isr80h $(FLAGS) -std=gnu99 -c ./src/isr80h/memory.c -o ./build/isr80h/memory.o

./build/isr80h/misc.o: ./src/isr80h/misc.c
	i686-elf-gcc $(INCLUDES) -I./src/isr80h $(FLAGS) -std=gnu99 -c ./src/isr80h/misc.c -o ./build/isr80h/misc.o
./build/isr80h/process.o: ./src/isr80h/process.c
	i686-elf-gcc $(INCLUDES) -I./src/isr80h $(FLAGS) -std=gnu99 -c ./src/isr80h/process.c -o ./build/isr80h/process.o


./build/keyboard/keyboard.o: ./src/keyboard/keyboard.c
	i686-elf-gcc $(INCLUDES) -I./src/keyboard $(FLAGS) -std=gnu99 -c ./src/keyboard/keyboard.c -o ./build/keyboard/keyboard.o


./build/keyboard/classic.o: ./src/keyboard/classic.c
	i686-elf-gcc $(INCLUDES) -I./src/keyboard $(FLAGS) -std=gnu99 -c ./src/keyboard/classic.c -o ./build/keyboard/classic.o


./build/isr80h/io.o: ./src/isr80h/io.c
	i686-elf-gcc $(INCLUDES) -I./src/isr80h $(FLAGS) -std=gnu99 -c ./src/isr80h/io.c -o ./build/isr80h/io.o

./build/idt/idt.o: ./src/idt/idt.c
	i686-elf-gcc $(INCLUDES) -I./src/idt $(FLAGS) -std=gnu99 -c ./src/idt/idt.c -o ./build/idt/idt.o

./build/memory/memory.o: ./src/memory/memory.c
	i686-elf-gcc $(INCLUDES) -I./src/memory $(FLAGS) -std=gnu99 -c ./src/memory/memory.c -o ./build/memory/memory.o


./build/task/process.o: ./src/task/process.c
	i686-elf-gcc $(INCLUDES) -I./src/task $(FLAGS) -std=gnu99 -c ./src/task/process.c -o ./build/task/process.o


./build/task/task.o: ./src/task/task.c
	i686-elf-gcc $(INCLUDES) -I./src/task $(FLAGS) -std=gnu99 -c ./src/task/task.c -o ./build/task/task.o

./build/task/task.asm.o: ./src/task/task.asm
	nasm -f elf -g ./src/task/task.asm -o ./build/task/task.asm.o

./build/task/tss.asm.o: ./src/task/tss.asm
	nasm -f elf -g ./src/task/tss.asm -o ./build/task/tss.asm.o

./build/io/io.asm.o: ./src/io/io.asm
	nasm -f elf -g ./src/io/io.asm -o ./build/io/io.asm.o

./build/memory/heap/heap.o: ./src/memory/heap/heap.c
	i686-elf-gcc $(INCLUDES) -I./src/memory/heap $(FLAGS) -std=gnu99 -c ./src/memory/heap/heap.c -o ./build/memory/heap/heap.o

./build/memory/heap/kheap.o: ./src/memory/heap/kheap.c
	i686-elf-gcc $(INCLUDES) -I./src/memory/heap $(FLAGS) -std=gnu99 -c ./src/memory/heap/kheap.c -o ./build/memory/heap/kheap.o

./build/memory/paging/paging.o: ./src/memory/paging/paging.c
	i686-elf-gcc $(INCLUDES) -I./src/memory/paging $(FLAGS) -std=gnu99 -c ./src/memory/paging/paging.c -o ./build/memory/paging/paging.o

./build/memory/paging/paging.asm.o: ./src/memory/paging/paging.asm
	nasm -f elf -g ./src/memory/paging/paging.asm -o ./build/memory/paging/paging.asm.o

./build/disk/disk.o: ./src/disk/disk.c
	i686-elf-gcc $(INCLUDES) -I./src/disk $(FLAGS) -std=gnu99 -c ./src/disk/disk.c -o ./build/disk/disk.o

./build/disk/streamer.o: ./src/disk/streamer.c
	i686-elf-gcc $(INCLUDES) -I./src/disk $(FLAGS) -std=gnu99 -c ./src/disk/streamer.c -o ./build/disk/streamer.o

./build/fs/fat/fat16.o: ./src/fs/fat/fat16.c
	i686-elf-gcc $(INCLUDES) -I./src/fs -I./src/fat $(FLAGS) -std=gnu99 -c ./src/fs/fat/fat16.c -o ./build/fs/fat/fat16.o
./build/fs/fat/fat16_write.o: ./src/fs/fat/fat16_write.c
	i686-elf-gcc $(INCLUDES) -I./src/fs -I./src/fat $(FLAGS) -std=gnu99 -c ./src/fs/fat/fat16_write.c -o ./build/fs/fat/fat16_write.o

./build/fs/fat32/fat32.o: ./src/fs/fat32/fat32.c
	i686-elf-gcc $(INCLUDES) -I./src/fs -I./src/fat32 $(FLAGS) -std=gnu99 -c ./src/fs/fat32/fat32.c -o ./build/fs/fat32/fat32.o

./build/time/time.o: ./src/time/time.c
	i686-elf-gcc $(INCLUDES) -I./src/time $(FLAGS) -std=gnu99 -c ./src/time/time.c -o ./build/time/time.o

./build/fs/file.o: ./src/fs/file.c
	i686-elf-gcc $(INCLUDES) -I./src/fs $(FLAGS) -std=gnu99 -c ./src/fs/file.c -o ./build/fs/file.o
./build/fs/file_o.o: ./src/fs/file_o.c
	i686-elf-gcc $(INCLUDES) -I./src/fs $(FLAGS) -std=gnu99 -c ./src/fs/file_o.c -o ./build/fs/file_o.o

./build/fs/pparser.o: ./src/fs/pparser.c
	i686-elf-gcc $(INCLUDES) -I./src/fs $(FLAGS) -std=gnu99 -c ./src/fs/pparser.c -o ./build/fs/pparser.o

./build/string/string.o: ./src/string/string.c
	i686-elf-gcc $(INCLUDES) -I./src/string $(FLAGS) -std=gnu99 -c ./src/string/string.c -o ./build/string/string.o

./build/power/power.o: ./src/power/power.c
	i686-elf-gcc $(INCLUDES) -I./src/power $(FLAGS) -std=gnu99 -c ./src/power/power.c -o ./build/power/power.o

./build/graphics/graphics.o: ./src/graphics/graphics.c
	i686-elf-gcc $(INCLUDES) -I./src/graphics $(FLAGS) -std=gnu99 -c ./src/graphics/graphics.c -o ./build/graphics/graphics.o

./build/sound/sound.o: ./src/sound/sound.c
	i686-elf-gcc $(INCLUDES) -I./src/sound $(FLAGS) -std=gnu99 -c ./src/sound/sound.c -o ./build/sound/sound.o


user_programs:
	cd ./programs/stdlib && $(MAKE) all

	cd ./programs/blank && $(MAKE) all
	cp ./programs/blank/blank.elf ./dirsys/usr/bin

	cd ./programs/pr && $(MAKE) all
	cp ./programs/pr/pr ./dirsys/sys/bin

	cd ./programs/rm && $(MAKE) all
	cp ./programs/rm/rm ./dirsys/sys/bin

	cd ./programs/view && $(MAKE) all
	cp ./programs/view/view ./dirsys/sys/bin

	cd ./programs/fapp && $(MAKE) all
	cp ./programs/fapp/fapp ./dirsys/sys/bin

	cd ./programs/fwrite && $(MAKE) all
	cp ./programs/fwrite/fwrite ./dirsys/sys/bin

	cd ./programs/fwrite && $(MAKE) all
	cp ./programs/fwrite/fwrite ./dirsys/sys/bin

	cd ./programs/sosupdt && $(MAKE) all
	cp ./programs/sosupdt/sosupdt ./dirsys/sys/bin
	
	cd ./programs/shell && $(MAKE) all
	cp ./programs/shell/shell.elf ./dirsys/sys/bin

	cd ./programs/musicbox && $(MAKE) all
	cp ./programs/musicbox/musicbox ./dirsys/usr/bin
	

user_programs_clean:
	cd ./programs/stdlib && $(MAKE) clean
	cd ./programs/blank && $(MAKE) clean
	cd ./programs/shell && $(MAKE) clean
	cd ./programs/pr && $(MAKE) clean
	cd ./programs/rm && $(MAKE) clean
	cd ./programs/view && $(MAKE) clean
	cd ./programs/fapp && $(MAKE) clean
	cd ./programs/fwrite && $(MAKE) clean
	cd ./programs/sosupdt && $(MAKE) clean

clean: user_programs_clean
	rm -rf ./bin/boot.bin
	rm -rf ./bin/alt_bootsect.bin
	rm -rf ./bin/kernel.bin
	rm -rf ./bin/SerpaeOS.img
	rm -rf ${FILES}
	rm -rf ./build/kernelfull.o
	rm -rf ./bin/serpaeos.ki
comment:
	#add -hdb ./path to either run or test to add a second disk!
run:
	sudo qemu-system-i386 -hda ./bin/SerpaeOS.img  -soundhw pcspk
test:
	sudo kvm -hda ./bin/SerpaeOS.img  -soundhw pcspk