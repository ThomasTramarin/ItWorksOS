set architecture i386
set disassembly-flavor intel
set pagination off
set confirm off
set disassemble-next-line on

target remote localhost:1234

break _start
break kmain