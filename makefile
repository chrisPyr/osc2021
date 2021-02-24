CC:= aarch64-linux-gnu
Source:= a.S
LINKER:= linker.ld
QEMUL:= qemu-system-aarch64

.PHONY: all
all: a.o kernel8.elf kernel8.img
	$(QEMUL) -M raspi3 -kernel kernel8.img -display none -d in_asm

.PHONY: a.o
a.o: $(Source)
	$(CC)-gcc -c $(Source)

.PHONY: kernel8.elf
kernel8.elf: a.o
	$(CC)-ld -T $(LINKER) -o kernel8.elf a.o 

.PHONY: kernel8.img
kernel8.img: kernel8.elf
	$(CC)-objcopy -O binary kernel8.elf kernel8.img

.PHONY: clean
clean:
	rm -f a.o kernel8.elf kernel8.img a.out


