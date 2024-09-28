#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <link.h>

// print_elf_header prints the ELF header content
void print_elf_header(ElfW(Ehdr) *elf_hdr) {
    printf("ELF Header:\n");
    printf("  Magic:   %02x %02x %02x %02x\n", elf_hdr->e_ident[EI_MAG0], elf_hdr->e_ident[EI_MAG1], elf_hdr->e_ident[EI_MAG2], elf_hdr->e_ident[EI_MAG3]);
    printf("  Class:                             %s\n", elf_hdr->e_ident[EI_CLASS] == ELFCLASS32 ? "ELF32" : "ELF64");
    printf("  Entry point address:               0x%lx\n", (unsigned long)elf_hdr->e_entry);
    printf("  Program header offset:             %ld (bytes into file)\n", (long)elf_hdr->e_phoff);
    printf("  Section header offset:             %ld (bytes into file)\n", (long)elf_hdr->e_shoff);
    printf("  Number of program headers:         %d\n", elf_hdr->e_phnum);
    printf("  Number of section headers:         %d\n", elf_hdr->e_shnum);
}

// print_program_headers prints an ELF program headers
void print_program_headers(ElfW(Phdr) *program_hdrs, int size) {
    printf("\nProgram Headers:\n");
    for (int i = 0; i < size; i++) {
        printf("  Type:   %d\n", program_hdrs[i].p_type);
        printf("  Offset: 0x%lx\n", (unsigned long)program_hdrs[i].p_offset);
        printf("  Virtual address: 0x%lx\n", (unsigned long)program_hdrs[i].p_vaddr);
    }
}

// print_section_headers prints the ELF section headers
void print_section_headers(ElfW(Shdr) *section_hdrs, const char *names, int size) {
    printf("\nSection Headers:\n");
    for (int i = 0; i < size; i++) {
        printf("  [%2d] Name: %s\n", i, &names[section_hdrs[i].sh_name]);
        printf("       Type: %d\n", section_hdrs[i].sh_type);
        printf("       Offset: 0x%lx\n", (unsigned long)section_hdrs[i].sh_offset);
        printf("       Address: 0x%lx\n", (unsigned long)section_hdrs[i].sh_addr);
    }
}

// print_symbols prints the ELF symbols
void print_symbols(ElfW(Sym) *symbols, const char *names, int size) {
    printf("\nSymbols:\n");
    for (int i = 0; i < size; i++) {
        printf("  [%2d] Name: %s\n", i, &names[symbols[i].st_name]);
        printf("       Value: 0x%lx\n", (unsigned long)symbols[i].st_value);
    }
}

// read_elf reads ELF
void read_elf(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // map the elf in memory
    void *map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // printing the ELF header
    ElfW(Ehdr) *ehdr = (ElfW(Ehdr) *)map;
    print_elf_header(ehdr);

    // printing the ELF program headers
    ElfW(Phdr) *phdr = (ElfW(Phdr) *)(map + ehdr->e_phoff);
    print_program_headers(phdr, ehdr->e_phnum);

    // printing the ELF section headers
    ElfW(Shdr) *shdr = (ElfW(Shdr) *)(map + ehdr->e_shoff);
    const char *shstrtab = (const char *)(map + shdr[ehdr->e_shstrndx].sh_offset);
    print_section_headers(shdr, shstrtab, ehdr->e_shnum);

    // searching the header of the section holding the symbol names
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            // printing the symbols of the ELF
            ElfW(Sym) *symtab = (ElfW(Sym) *)(map + shdr[i].sh_offset);
            const char *strtab = (const char *)(map + shdr[shdr[i].sh_link].sh_offset);
            int symnum = shdr[i].sh_size / sizeof(ElfW(Sym));
            print_symbols(symtab, strtab, symnum);
        }
    }

    // cleanup
    munmap(map, st.st_size);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ELF file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    read_elf(argv[1]);
    return EXIT_SUCCESS;
}
