set confirm off
set architecture riscv:rv64
symbol-file kernel/kernel
target remote 127.0.0.1:26000
set disassemble-next-line auto
set riscv use-compressed-breakpoints yes
