#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <link.h>

// Fonction pour afficher l'en-tête ELF
void print_elf_header(ElfW(Ehdr) *ehdr) {
    printf("ELF Header:\n");
    printf("  Magic:   %02x %02x %02x %02x\n", ehdr->e_ident[EI_MAG0], ehdr->e_ident[EI_MAG1], ehdr->e_ident[EI_MAG2], ehdr->e_ident[EI_MAG3]);
    printf("  Class:                             %s\n", ehdr->e_ident[EI_CLASS] == ELFCLASS32 ? "ELF32" : "ELF64");
    printf("  Entry point address:               0x%lx\n", (unsigned long)ehdr->e_entry);
    printf("  Program header offset:             %ld (bytes into file)\n", (long)ehdr->e_phoff);
    printf("  Section header offset:             %ld (bytes into file)\n", (long)ehdr->e_shoff);
    printf("  Number of program headers:         %d\n", ehdr->e_phnum);
    printf("  Number of section headers:         %d\n", ehdr->e_shnum);
}

// Fonction pour afficher les en-têtes de programme
void print_program_headers(ElfW(Phdr) *phdr, int phnum) {
    printf("\nProgram Headers:\n");
    for (int i = 0; i < phnum; i++) {
        printf("  Type:   %d\n", phdr[i].p_type);
        printf("  Offset: 0x%lx\n", (unsigned long)phdr[i].p_offset);
        printf("  Virtual address: 0x%lx\n", (unsigned long)phdr[i].p_vaddr);
    }
}

// Fonction pour afficher les en-têtes de section
void print_section_headers(ElfW(Shdr) *shdr, const char *strtab, int shnum) {
    printf("\nSection Headers:\n");
    for (int i = 0; i < shnum; i++) {
        printf("  [%2d] Name: %s\n", i, &strtab[shdr[i].sh_name]);
        printf("       Type: %d\n", shdr[i].sh_type);
        printf("       Offset: 0x%lx\n", (unsigned long)shdr[i].sh_offset);
        printf("       Address: 0x%lx\n", (unsigned long)shdr[i].sh_addr);
    }
}

// Fonction pour afficher la table des symboles
void print_symbols(ElfW(Sym) *symtab, const char *strtab, int symnum) {
    printf("\nSymbols:\n");
    for (int i = 0; i < symnum; i++) {
        printf("  [%2d] Name: %s\n", i, &strtab[symtab[i].st_name]);
        printf("       Value: 0x%lx\n", (unsigned long)symtab[i].st_value);
    }
}

// Fonction principale pour lire un fichier ELF et afficher ses structures
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

    // Mappe le fichier en mémoire
    void *map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    ElfW(Ehdr) *ehdr = (ElfW(Ehdr) *)map;
    print_elf_header(ehdr);

    // Affiche les en-têtes de programme
    ElfW(Phdr) *phdr = (ElfW(Phdr) *)(map + ehdr->e_phoff);
    print_program_headers(phdr, ehdr->e_phnum);

    // Affiche les en-têtes de section
    ElfW(Shdr) *shdr = (ElfW(Shdr) *)(map + ehdr->e_shoff);
    const char *shstrtab = (const char *)(map + shdr[ehdr->e_shstrndx].sh_offset);
    print_section_headers(shdr, shstrtab, ehdr->e_shnum);

    // Recherche la table des symboles
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            ElfW(Sym) *symtab = (ElfW(Sym) *)(map + shdr[i].sh_offset);
            const char *strtab = (const char *)(map + shdr[shdr[i].sh_link].sh_offset);
            int symnum = shdr[i].sh_size / sizeof(ElfW(Sym));
            print_symbols(symtab, strtab, symnum);
        }
    }

    // Nettoyage
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
