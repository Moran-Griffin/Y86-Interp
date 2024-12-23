/*
 * Execute the program using y86 without flags to see uses.
 *
 * Name: Griffin Moran
 */

#include "p1-check.h"
#include "p2-load.h"
#include "p3-disas.h"
#include "p4-interp.h"

/*
 * helper function for printing help text
 */
void usage (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
    printf("  -d      Disassemble code contents\n");
    printf("  -D      Disassemble data contents\n");
    printf("  -e      Execute program\n");
    printf("  -E      Execute program (trace mode)\n");
}

int main (int argc, char **argv)
{
    //flag conditionals
    bool h = false;
    bool H = false;
    bool s = false;
    bool m = false;
    bool M = false;
    bool d = false;
    bool D = false;
    bool e = false;
    bool E = false;

    const int memsize = 4096;

    //setup memory, filename, and header
    byte_t* memory = (byte_t*)calloc(memsize, 1);
    char* filename = NULL;
    elf_hdr_t header;

    int opt;
    //check command line args
    while((opt = getopt(argc, argv, "hHafsmMdDeE")) != -1) {
        switch(opt) {
            case 'h':
                h = true;
                break;

            case 'H':
                H = true;
                break;

            case 'a':
                H = true;
                s = true;
                m = true;
                break;

            case 'f':
                H = true;
                s = true;
                M = true;
                break;

            case 's':
                s = true;
                break;

            case 'm':
                m = true;
                break;

            case 'M':
                M = true;
                break;

            case 'd':
                d = true;
                break;

            case 'D':
                D = true;
                break;

            case 'e':
                e = true;
                break;

            case 'E':
                E = true;
                break;

            default:
                usage(argv);
                break;
        }
    }
    //set filename iff only one name is present
    if(optind + 1 == argc) {
        filename = argv[optind];
    } else {
        usage(argv);
        free(memory);
        return EXIT_FAILURE;
    }

    //open and check the file
    FILE* file = fopen(filename, "r");
    if(h) {
        free(memory);
        return EXIT_SUCCESS;
    }

    if(file == NULL) {
        printf("Failed to read file\n");
        free(memory);
        return EXIT_FAILURE;
    }

    //check for header validity
    if(!read_header(file, &header)) {
        fclose(file);
        free(memory);
        printf("Failed to read file\n");
        return EXIT_FAILURE;
    }

    //populate the p_headers array
    elf_phdr_t p_headers[header.e_num_phdr];
    memset(p_headers, 0, header.e_num_phdr* sizeof(elf_phdr_t));
    int offset = header.e_phdr_start;
    for(int i = 0; i < header.e_num_phdr; i++) {
        if(!read_phdr(file, offset, &p_headers[i])) {
            free(memory);
            printf("Failed to read file\n");
            return EXIT_FAILURE;
        }
        offset += 20;
    }

    for(int i = 0; i < header.e_num_phdr; i++) {
        if(!load_segment(file, memory, &p_headers[i])) {
            free(memory);
            printf("Failed to read file\n");
            return EXIT_FAILURE;
        }
    }

    //conditional handling for flags
    if(M && m) {
        free(memory);
        usage(argv);
        return EXIT_FAILURE;
    }

    if(H) {
        dump_header(&header);
    }
    //may need to add flags for a and f, reassess after testing

    if(s) {
        dump_phdrs(header.e_num_phdr, p_headers);
    }

    if(m) {
        for(int i = 0; i < header.e_num_phdr; i++) {
            dump_memory(memory, p_headers[i].p_vaddr, p_headers[i].p_vaddr + p_headers[i].p_size);
        }
    }

    if(M) {
        dump_memory(memory, 0, memsize);
    }

    if(d) {
        printf("Disassembly of executable contents:\n");
        for(int i = 0; i < header.e_num_phdr; i++) {
            if(p_headers[i].p_type == CODE) {
                disassemble_code(memory, &p_headers[i], &header);
            }
        }
    }

    if(D) {
        printf("Disassembly of data contents:\n");
        for(int i = 0; i < header.e_num_phdr; i++) {
            if(p_headers[i].p_type == DATA) {
                if(p_headers[i].p_flags == 4) {
                    disassemble_rodata(memory, &p_headers[i]);
                } else if(p_headers[i].p_flags == 6) {
                    disassemble_data(memory, &p_headers[i]);
                }
            }
        }
    }

    if(e && E) {
        free(memory);
        usage(argv);
        return EXIT_FAILURE;
    }

    //setup variables
    y86_t cpu;
    memset(&cpu, 0, sizeof(y86_t));
    cpu.stat = AOK;
    cpu.pc = header.e_entry;
    cpu.zf = false;
    cpu.of = false;
    cpu.sf = false;

    //populate registers with 0 by default
    for(int i = 0; i < 15; i++) {
        cpu.reg[i] = 0x0;
    }

    //more setup use memset for inst
    y86_inst_t inst;
    memset(&inst, 0, sizeof(y86_inst_t));
    bool cond = false;
    y86_reg_t valA = 0;
    y86_reg_t valE = 0;

    if(e) {//Execute mode
        printf("Beginning execution at 0x%04x\n", header.e_entry);
        int numIns = 0;
        while(cpu.stat == AOK) {
            inst = fetch(&cpu, memory);

            //invalid instruction
            if(cpu.stat == ADR || cpu.stat == INS) {
                break;
            }

            //remaining von-neumann
            valE = decode_execute (&cpu, &inst, &cond, &valA);
            memory_wb_pc (&cpu, &inst, memory, cond, valA, valE);
            numIns++;
        }
        dump_cpu_state(&cpu);
        printf("Total execution count: %d\n", numIns);
    }

    if(E) {//Trace mode
        printf("Beginning execution at 0x%04x\n", header.e_entry);
        dump_cpu_state(&cpu);
        printf("\n");
        int numIns = 0;
        while(cpu.stat == AOK) {
            inst = fetch(&cpu, memory);

            //invalid instruction
            if(cpu.stat == ADR || cpu.stat == INS) {
                printf("Invalid instruction at 0x%04lx\n", cpu.pc);
                dump_cpu_state(&cpu);
                break;
            }

            //print out instruction
            printf("Executing: ");
            disassemble(&inst);
            printf("\n");

            //remining von-neumann
            valE = decode_execute (&cpu, &inst, &cond, &valA);
            memory_wb_pc (&cpu, &inst, memory, cond, valA, valE);
            dump_cpu_state(&cpu);

            if(cpu.stat == AOK) {
                printf("\n");
            }
            numIns++;
        }
        printf("Total execution count: %d\n\n", numIns);
        dump_memory(memory, 0, memsize);
    }

    free(memory);
    return EXIT_SUCCESS;
}

