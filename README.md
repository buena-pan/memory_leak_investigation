# memory_leak_investigation
Analizing memory leak in RHEL 8,5 after cudaHostRegister/cudaHostUnregister on custom DMA device that implements dma_alloc_coherent, remap_pfn_range

## How to reproduce the issue
Clone the repo 
```
git clone https://github.com/buena-pan/memory_leak_investigation.git
```
Build the simple_device and change the permission 
```
cd memory_leak_investigation
make
sudo chmod og+wr /dev/simple_device
```
Build the userland test
```
cd userland_test
make
```
Now run the binary and check the kernel output on /var/log/messages
```
./userland_test
tail /var/log/messages
```
You should see something like this
```
Nov 21 14:28:41 asm-m1 kernel: Hardware name: Supermicro SYS-4029GP-TRT/X11DPG-OT-CPU, BIOS 2.1a 10/02/2018
Nov 21 14:28:41 asm-m1 kernel: Call Trace:
Nov 21 14:28:41 asm-m1 kernel: dump_stack+0x5c/0x80
Nov 21 14:28:41 asm-m1 kernel: bad_page.cold.111+0x9b/0xa0
Nov 21 14:28:41 asm-m1 kernel: __free_pages_ok+0x376/0x3e0
Nov 21 14:28:41 asm-m1 kernel: __free_pages+0x33/0x70
Nov 21 14:28:41 asm-m1 kernel: vma_close+0x34/0x40 [simple_driver]
Nov 21 14:28:41 asm-m1 kernel: remove_vma+0x30/0x60
Nov 21 14:28:41 asm-m1 kernel: __do_munmap+0x2a2/0x4f0
Nov 21 14:28:41 asm-m1 kernel: __vm_munmap+0x68/0xc0
Nov 21 14:28:41 asm-m1 kernel: __x64_sys_munmap+0x27/0x30
Nov 21 14:28:41 asm-m1 kernel: do_syscall_64+0x5b/0x1a0
Nov 21 14:28:41 asm-m1 kernel: entry_SYSCALL_64_after_hwframe+0x65/0xca
Nov 21 14:28:41 asm-m1 kernel: RIP: 0033:0x7f95143077db
Nov 21 14:28:41 asm-m1 kernel: Code: ff ff 0f 1f 44 00 00 48 8b 15 a9 76 2c 00 f7 d8 64 89 02 48 c7 c0 ff ff ff ff e9 6f ff ff ff f3 0f 1e fa b8 0b 00 00 00 0f 05 <48> 3d 01 f0 ff ff 73 01 c3 48 8b 0d 7d 76 2c 00 f7 d8 64 89 01 48
Nov 21 14:28:41 asm-m1 kernel: RSP: 002b:00007fff65d58cd8 EFLAGS: 00000206 ORIG_RAX: 000000000000000b
Nov 21 14:28:41 asm-m1 kernel: RAX: ffffffffffffffda RBX: 0000000000000000 RCX: 00007f95143077db
Nov 21 14:28:41 asm-m1 kernel: RDX: 000000000007c000 RSI: 000000000007c000 RDI: 00007f951533e000
Nov 21 14:28:41 asm-m1 kernel: RBP: 00007fff65d58d40 R08: 0000000000fe6fd0 R09: 0000000000000000
Nov 21 14:28:41 asm-m1 kernel: R10: 000000000000000a R11: 0000000000000206 R12: 0000000000400af0
Nov 21 14:28:41 asm-m1 kernel: R13: 00007fff65d58e20 R14: 0000000000000000 R15: 0000000000000000
Nov 21 14:28:41 asm-m1 kernel: BUG: Bad page state in process my_program  pfn:2e8040
Nov 21 14:28:41 asm-m1 kernel: page:ffffee84cba01000 refcount:-1023 mapcount:0 mapping:00000000bcb0f23a index:0x0
Nov 21 14:28:41 asm-m1 kernel: flags: 0x17ffffc0000010(dirty)
Nov 21 14:28:41 asm-m1 kernel: raw: 0017ffffc0000010 ffffee84cba01008 ffffee84cba01008 0000000000000000
Nov 21 14:28:41 asm-m1 kernel: raw: 0000000000000000 0000000000000000 fffffc01ffffffff 0000000000000000
Nov 21 14:28:41 asm-m1 kernel: page dumped because: nonzero _refcount
```
