# JOS Operating System Experiments

https://pdos.csail.mit.edu/6.828/2018/overview.html

Use qemu emulator to start the system, if flash the OS img to hard disk, it can actually boot.

## Lab1 Booting a PC
- PC booting process, from BIOS to bootloader to kernel
- objdump, load ELF file
- Physical memory address, where BIOS, bootloader, kernel loaded
- Stack, calling convention, push arguments and eip(call instruction), esp

## Lab2 Memory management
- Physical page management: struct PageInfo* pages array
- virtual memory, linear memory, physical memory
- Two level page lookup: MMU; pde & pte
- setup memory layout, where is the kernel, kernel stack, PageInfo pages, page table(uvpt, uvpd), legacy memory hole for compatibility(BIOS, VGA...)
- Refer to inc/memlayout.h to have a better understanding
- Initialize kernel address space: kern_pgdir

## Lab3 User Environment
- Environment is like process linux
- User memory space vs. Kernel memory space: env_pgdir vs. kern_pgdir
- Creating and running user environment: copy kernel memory space, setting RPL(Requstor Privilege Level), load executable and run
- Interrupts and exceptions: protected control transfer: IDT(Interrupt descriptor tabe) and TSS(Task state segment), CPU will do the stack switch to the exception stack and auto push Trapframe to save user env state; Here we must config the exception stack for the trap(struct Trapframe *tf) function call in C code, assemble code do most of the stuff here. To sum up, save state in user env, then switch stack, modify stack, call the interrupt handler code.
- System call and page fault, for syscall, notice the RPL should equal to DPL to avoid general protection exception. It will provide system API for user env;
- Page fault handler is used for next lab where we can do COW(copy on write)
- uvpd, uvpt: trick to access the page table linearly & read only

## Lab4 Preemptive multitasking
- MultiProcessor support where we will have per CPU kernel stack, TSS, environment pointer, system registers
- Round-robin scheduling, where each environment must explicitly call sys_yield to release the cpu resource, otherwise other process cannot run
- fork implementation, copy the entire memory space where parent env has the write permission, share the memory that only has read permission
- Copy on write fork, no need to copy a new memory page that has PTE_W permission. Instead, mark it as PTE_COW, it will trigger page fault interrupt when trying to write to COW page. Kernel will do the work of allocating a new page and copy content over.
- Need to take caution as to how to set up the child env memory. Apart from making COW of write page, we need to set new memory for the exception stack. After all, no one can setup the COW page if a page fault recursively happen in the interrupt's handler which use exception stack
- Preemptive multitasking, handle clock interrupts, so that kernel can gain the power to allocation the cpu resources
- IPC, using the system page share syscall to share memory between envs(processes)

## Lab5 File system, spawn, shell
- disk space management, it is like the physical memory management
- File descriptor => mapped to a page, current content offset of the file.
- A file system env to handle the file read/write, other user env use API to access file. The API(RPC call) is a wrapper of IPC to the file system env.
- Spawning process, fork proces and then load the ELF file, set entry to eip.
- direct map the shared library memory page which has PTE_SHARE, it is like that *.so, *.dll libs are shared by different process.
- Shell & pipe. Shell is with the std input/output file opened with console read/write method instead of the file ops. It is still a file! The shell env will process the input and spawn new processes. Pipe(|), the left & right part of pipe is two spawned process(parent and child), with a shared page used for IPC: with left side output write redirect to the shared page, right side input redirect from the shared page.