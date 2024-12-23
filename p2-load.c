/*
 * Mini-ELF loader
 *
 * Name: Griffin Moran
 */

#include "p2-load.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr)
{
    if(phdr == NULL || file == NULL) {
        return false;
    }

    int expected = 0xDEADBEEF;

    fseek(file, offset, SEEK_SET);

    //initialize the program header
    fread(&(phdr -> p_offset), sizeof(char) * 4, 1, file);
    fread(&(phdr -> p_size), sizeof(char) * 4, 1, file);
    fread(&(phdr -> p_vaddr), sizeof(char) * 4, 1, file);
    fread(&(phdr -> p_type), sizeof(char) * 2, 1, file);
    fread(&(phdr -> p_flags), sizeof(char) * 2, 1, file);
    fread(&(phdr -> magic), sizeof(char) * 4, 1, file);

    //check for bad size
    if(phdr -> p_size < 0) {
        printf("Failed to read file\n");
        return false;
    }

    //check for bad virtual address
    if(phdr -> p_vaddr < 0 || phdr -> p_vaddr > 4096) {
        printf("Failed to read file\n");
        return false;
    }

    //check for bad magic
    if(phdr -> magic != expected) {
        printf("Failed to read file\n");
        return false;
    }

    return true;
}

//Read data from the file into an address space beginning at memory based on the program header phdr.
//Note that memory should be a pointer to the beginning of the address space,
//not the actual location where the segment should go (which can be accessed inside load_segment via phdr using offset).
//Note also that there could be zero-byte segments; for such segments there is no need to actually read anything from the file.
//This function should also reject unknown segment types and any segment that would write past the end of virtual memory.
bool load_segment (FILE *file, byte_t *memory, elf_phdr_t *phdr)
{
    if(memory == NULL || file == NULL || phdr == NULL) {
        return false;
    }

    //check for zero-byte segments
    if(phdr -> p_size == 0) {
        return true;
    }

    //check for large segments
    if(phdr -> p_size + phdr -> p_vaddr > 4096) {
        return false;
    }

    //check for unknown segment types
    if(phdr -> p_type != 0 && phdr -> p_type != 1 && phdr -> p_type != 2) {
        return false;
    }

    //check for bad read
    fseek(file, phdr -> p_offset, SEEK_SET);
    if(fread(&memory[phdr -> p_vaddr], phdr -> p_size, 1, file) != 1) {
        return false;
    }
    return true;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void dump_phdrs (uint16_t numphdrs, elf_phdr_t *phdrs)
{
    //formatting
    printf(" Segment   Offset    Size      VirtAddr  Type      Flags\n");
    for(int i = 0; i < numphdrs; i++) {
        printf("  %02x       0x%04x    0x%04x    0x%04x    ", i, phdrs[i].p_offset, phdrs[i].p_size,
               phdrs[i].p_vaddr);

        //print type
        if(phdrs[i].p_type == 1) {
            printf("CODE      ");
        } else if(phdrs[i].p_type == 0) {
            printf("DATA      ");
        } else {
            printf("STACK     ");
        } //maybe add heap

        //read flag
        if(phdrs[i].p_flags - 4 >= 0) {
            printf("R");
        } else {
            printf(" ");
        }

        //write flag
        if(phdrs[i].p_flags < 4) {
            if(phdrs[i].p_flags - 2 >= 0) {
                printf("W");
            } else {
                printf(" ");
            }
        } else {
            if(phdrs[i].p_flags > 5) {
                printf("W");
            } else {
                printf(" ");
            }
        }

        //execute flag
        if(phdrs[i].p_flags % 2 != 0) {
            printf("X\n");
        } else {
            printf(" \n");
        }
    }
}

//Print the contents of Y86 virtual memory starting at address start and ending just before address end.
//For instance, if start = 5 and end = 8, then you will print the bytes at addresses 5, 6, and 7.
//Each line of output should be 16-byte aligned, but you should only output hex for the actual bytes requested;
//any leading bytes should be printed as empty spaces. There should be no trailing spaces, however.
void dump_memory (byte_t *memory, uint16_t start, uint16_t end)
{
    printf("Contents of memory from %04x to %04x:", start, end);
    if(start <= end) {
        //16 bit alignment
        if(start % 16 != 0) {
            printf("\n  %04x  ", start - start % 16);
            for(int i = 0; i < start % 16; i++) {
                printf("   ");
                if(i == 8) {
                    printf(" ");
                }
            }
        }
    }
    //loop through bits
    while(start < end) {
        if(start % 16 == 0) {
            printf("\n  %04x  ", start);
        } else if(start % 8 == 0) {
            printf(" ");
        }
        printf("%02x", memory[start]);
        //check for last bit
        if(start != end - 1 && start % 16 != 15) {
            printf(" ");
        }
        start++;
    }
    printf("\n");
}

