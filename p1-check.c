/*
 * Mini-ELF header verifier
 *
 * Name: Griffin Moran
 */

#include "p1-check.h"

bool read_header (FILE *file, elf_hdr_t *hdr) //needs the most work
{

    //valid pointer check
    if(!hdr) {
        return false;
    }

    //null value check
    if(hdr == NULL) {
        return false;
    }

    //file length check
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    if(size < 16) {
        printf("Failed to read file\n");
        return false;
    }

    //elf header field assignments
    fread(&(hdr -> e_version), sizeof(char) * 2, 1, file);

    //version check
    if(hdr -> e_version != 1) {
        printf("Failed to read file\n");
        return false;
    }

    //remember first argument requires a pointer and & adds a * to a variable
    fread(&(hdr -> e_entry), sizeof(char) * 2, 1, file);

    fread(&(hdr -> e_phdr_start), sizeof(char) * 2, 1, file);

    fread(&(hdr -> e_num_phdr), sizeof(char) * 2, 1, file);

    fread(&(hdr -> e_symtab), sizeof(char) * 2, 1, file);

    fread(&(hdr -> e_strtab), sizeof(char) * 2, 1, file);

    fread(&(hdr -> magic), sizeof(char) * 8, 1, file);

    //convert expected string to integer
    const int magic_expect = 4607045;

    //magic number check

    if(hdr -> magic != magic_expect) {
        printf("Failed to read file\n");
        return false;
    }

    return true;
}

void usage_p1 (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
}

bool parse_command_line_p1 (int argc, char **argv, bool *print_header, char **filename)
{
    int opt;
    //check command line args
    while((opt = getopt(argc, argv, "hH")) != -1) {
        switch(opt) {
            case 'H':
                *print_header = true;
                break;

            case 'h':
                usage_p1(argv);
                return true;

            case '?':
                usage_p1(argv);
                return false;

            default:
                break;
        }
    }
    //set filename iff only one name is present
    if(optind + 1 == argc) {
        *filename = argv[optind];
    } else {
        usage_p1(argv);
        return false;
    }
    return true;
}

//methods to convert to little endian
//unsigned to see actual byte values and prevent sign extension
//point to the individual bytes of num
void endian_helper_short(uint16_t num)
{
    unsigned char* bytes = (unsigned char*) &num;
    for(uint16_t i = 0; i < sizeof(num); i++) {
        printf("%02x ", bytes[i]);
    }
}

void endian_helper_int(uint32_t num)
{
    unsigned char* bytes = (unsigned char*) &num;
    for(uint32_t i = 0; i < sizeof(num); i++) {
        if(i != sizeof(num) - 1) {
            printf("%02x ", bytes[i]);
        } else {
            printf("%02x", bytes[i]);
        }
    }
}

void dump_header (elf_hdr_t *hdr)
{
    endian_helper_short(hdr -> e_version);
    endian_helper_short(hdr -> e_entry);
    endian_helper_short(hdr -> e_phdr_start);
    endian_helper_short(hdr -> e_num_phdr);
    printf(" ");
    endian_helper_short(hdr -> e_symtab);
    endian_helper_short(hdr -> e_strtab);
    endian_helper_int(hdr -> magic);
    printf("\n");

    printf("Mini-ELF version %d\n", hdr -> e_version);
    printf("Entry point 0x%02x\n", hdr -> e_entry);
    printf("There are %d program headers, starting at offset %d (0x%02x)\n", hdr -> e_num_phdr,
           hdr -> e_phdr_start, hdr -> e_phdr_start);

    if(!hdr -> e_symtab) {
        printf("There is no symbol table present\n");
    } else {
        printf("There is a symbol table starting at offset %d (0x%02x)\n", hdr -> e_symtab,
               hdr -> e_symtab);
    }

    if(!hdr -> e_strtab) {
        printf("There is no string table present\n");
    } else {
        printf("There is a string table starting at offset %d (0x%02x)\n", hdr -> e_strtab,
               hdr -> e_strtab);
    }
}

