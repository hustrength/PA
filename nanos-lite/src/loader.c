#include "proc.h"
#include <elf.h>

/* PA 3.3 */
#include "fs.h"


#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
    /* PA 3.2 */
    /* PA 3.3 */
    Elf_Ehdr ehdr;
    Elf_Phdr phdr;
    int fd = fs_open(filename, 0, 0);
    if (fd == -1)
        Log("file %s does not exist!", filename);
    assert(fd != -1);
    
    fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
    fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
    
    size_t open_offset;

    for (uint16_t i = 0; i < ehdr.e_phnum; i++) {

        fs_read(fd, &phdr, sizeof(Elf_Phdr));
        open_offset = fs_open_offset(fd);
        if (phdr.p_type == PT_LOAD) {
            fs_lseek(fd, phdr.p_offset, SEEK_SET);
            fs_read(fd, (void *) phdr.p_vaddr, phdr.p_filesz);
            memset((void *) (phdr.p_vaddr + phdr.p_filesz), 0, \
                   (phdr.p_memsz - phdr.p_filesz));
        }
        fs_lseek(fd, open_offset, SEEK_SET);
    }

    fs_close(fd);
    return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %x", entry);
    ((void (*)()) entry)();
}

void context_kload(PCB *pcb, void *entry) {
    _Area stack;
    stack.start = pcb->stack;
    stack.end = stack.start + sizeof(pcb->stack);

    pcb->cp = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);

    _Area stack;
    stack.start = pcb->stack;
    stack.end = stack.start + sizeof(pcb->stack);

    pcb->cp = _ucontext(&pcb->as, stack, stack, (void *) entry, NULL);
}
