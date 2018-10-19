@echo off
if exist sd.bin goto run
qemu-img create -f raw sd.bin 64M

:run
qemu-system-arm -M vexpress-a9 -kernel rtthread.elf -serial stdio -sd sd.bin -net nic -net tap,ifname=qemu -gdb tcp::1234 -S
