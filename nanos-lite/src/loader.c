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
    Elf_Ehdr elf_header;
    Elf_Phdr program_header;
    int fd = fs_open(filename, 0, 0);
    if (fd == -1)
        printf("file %s does not exist!", filename);
    assert(fd != -1);
    
    fs_read(fd, &elf_header, sizeof(Elf_Ehdr));
    fs_lseek(fd, elf_header.e_phoff, SEEK_SET);
    
    size_t open_offset;

    for (uint16_t i = 0; i < elf_header.e_phnum; i++) {

        fs_read(fd, &program_header, sizeof(Elf_Phdr));
        open_offset = fs_open_offset(fd);
        if (program_header.p_type == PT_LOAD) {
            fs_lseek(fd, program_header.p_offset, SEEK_SET);
            fs_read(fd, (void *) program_header.p_vaddr, program_header.p_filesz);
            memset((void *) (program_header.p_vaddr + program_header.p_filesz), 0, \
                   (program_header.p_memsz - program_header.p_filesz));
        }
        fs_lseek(fd, open_offset, SEEK_SET);

    }
    fs_close(fd);
    return elf_header.e_entry;
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
