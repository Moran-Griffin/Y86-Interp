/*
 * Mini-ELF disassembler
 *
 * Name: Griffin Moran
 */

#include "p3-disas.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

y86_inst_t fetch (y86_t *cpu, byte_t *memory)
{
    y86_inst_t ins;
    
    if(cpu == NULL){
        ins.icode = INVALID;
        return ins;
    }

    if(memory == NULL) {
        ins.icode = INVALID;
        cpu -> stat = INS;
        return ins;
    }
    ins.icode = (*(memory + cpu -> pc) & 0xF0) >> 4;

    bool b1 = false;
    bool rA = false;
    bool rB = false;
    bool jxx = false;
    bool cmovxx = false;
    bool irmov = false;
    bool opq = false;
    bool pushPop = false;
    bool trap = false;

    //go through and set rA and rB to NOREG if they arent already initilaized
    switch(ins.icode) {
        case (HALT):
            b1 = true;
            ins.ifun.b = *(memory + cpu -> pc) & 0x0F;
            cpu -> stat = HLT;
            if(cpu -> pc + 1 > MEMSIZE) {
                ins.ifun.b = ins.icode;
                ins.icode = INVALID;
                cpu -> stat = ADR;
                return ins;
            }
            ins.ra = NOREG;
            ins.rb = NOREG;
            ins.valP = cpu -> pc + 1;
            break;

        case (NOP):
            b1 = true;
            ins.ifun.b = *(memory + cpu -> pc) & 0x0F;
            if(cpu -> pc + 1 > MEMSIZE) {
                ins.ifun.b = ins.icode;
                ins.icode = INVALID;
                cpu -> stat = ADR;
                return ins;
            }
            ins.ra = NOREG;
            ins.rb = NOREG;
            ins.valP = cpu -> pc + 1;
            break;
        case (CMOV):
            cmovxx = true;
            rA = true;
            rB = true;
            ins.ifun.cmov = *(memory + cpu -> pc) & 0x0F;
            if(cpu -> pc + 2 > MEMSIZE) {
                ins.ifun.b = ins.icode;
                ins.icode = INVALID;
                cpu -> stat = ADR;
                return ins;
            }
            ins.ra = (*(memory + cpu -> pc + 1) & 0xF0) >> 4;
            ins.rb = *(memory + cpu -> pc + 1) & 0x0F;
            ins.valP = cpu -> pc + 2;
            break;

        case (IRMOVQ):
            b1 = true;
            irmov = true;
            rB = true;
            ins.ifun.b = *(memory + cpu -> pc) & 0x0F;
            if(cpu -> pc + 10 > MEMSIZE) {
                ins.ifun.b = ins.icode;
                ins.icode = INVALID;
                cpu -> stat = ADR;
                return ins;
            }
            ins.ra = (*(memory + cpu -> pc + 1) & 0xF0) >> 4;
            ins.rb = *(memory + cpu -> pc + 1) & 0x0F;
            memcpy(&(ins.valC.v), memory + cpu -> pc + 2, sizeof(int64_t));
            ins.valP = cpu -> pc + 10;
            break;

        case (RMMOVQ):
            b1 = true;
            rA = true;
            ins.ifun.b = *(memory + cpu -> pc) & 0x0F;
            if(cpu -> pc + 10 > MEMSIZE) {
                ins.ifun.b = ins.icode;
                ins.icode = INVALID;
                cpu -> stat = ADR;
                return ins;
            }
            ins.ra = (*(memory + cpu -> pc + 1) & 0xF0) >> 4;
            ins.rb = *(memory + cpu -> pc + 1) & 0x0F;
            memcpy(&(ins.valC.d), memory + cpu -> pc + 2, sizeof(int64_t));
            ins.valP = cpu -> pc + 10;
            break;

        case (MRMOVQ):
            b1 = true;
            ins.ifun.b = *(memory + cpu -> pc) & 0x0F;
            if(cpu -> pc + 10 > MEMSIZE) {
                ins.ifun.b = ins.icode;
                ins.icode = INVALID;
                cpu -> stat = ADR;
                return ins;
            }
            ins.ra = (*(memory + cpu -> pc + 1) & 0xF0) >> 4;
            ins.rb = *(memory + cpu -> pc + 1) & 0x0F;
            memcpy(&(ins.valC.d), memory + cpu -> pc + 2, sizeof(int64_t));
            ins.valP = cpu -> pc + 10;
            break;

        case (OPQ):
            opq = true;
            rA = true;
            rB = true;
            ins.ifun.op = *(memory + cpu -> pc) & 0x0F;
            if(cpu -> pc + 10 > MEMSIZE) {
                ins.ifun.b = ins.icode;
                ins.icode = INVALID;
                cpu -> stat = ADR;
                return ins;
            }
            ins.ra = (*(memory + cpu -> pc + 1) & 0xF0) >> 4;
            ins.rb = *(memory + cpu -> pc + 1) & 0x0F;
            ins.valP = cpu -> pc + 2;
            break;

        case (JUMP):
            jxx = true;
            ins.ifun.b = *(memory + cpu -> pc) & 0x0F;
            if(cpu -> pc + 9 > MEMSIZE) {
                ins.ifun.b = ins.icode;
                ins.icode = INVALID;
                cpu -> stat = ADR;
                return ins;
            }
            ins.ra = NOREG;
            ins.rb = NOREG;
            memcpy(&(ins.valC.dest), memory + cpu -> pc + 1, sizeof(int64_t));
            ins.valP = cpu -> pc + 9;
            break;

        case (CALL):
            b1 = true;
            ins.ifun.b = *(memory + cpu -> pc) & 0x0F;
            if(cpu -> pc + 9 > MEMSIZE) {
                ins.ifun.b = ins.icode;
                ins.icode = INVALID;
                cpu -> stat = ADR;
                return ins;
            }
            ins.ra = NOREG;
            ins.rb = NOREG;
            memcpy(&(ins.valC.dest), memory + cpu -> pc + 1, sizeof(int64_t));
            ins.valP = cpu -> pc + 9;
            break;

        case (RET):
            b1 = true;
            ins.ifun.b = *(memory + cpu -> pc) & 0x0F;
            if(cpu -> pc + 1 > MEMSIZE) {
                ins.ifun.b = ins.icode;
                ins.icode = INVALID;
                cpu -> stat = ADR;
                return ins;
            }
            ins.ra = NOREG;
            ins.rb = NOREG;
            ins.valP = cpu -> pc + 1;
            break;

        case (PUSHQ):
            b1 = true;
            rA = true;
            pushPop = true;
            ins.ifun.b = *(memory + cpu -> pc) & 0x0F;
            if(cpu -> pc + 1 > MEMSIZE) {
                ins.ifun.b = ins.icode;
                ins.icode = INVALID;
                cpu -> stat = ADR;
                return ins;
            }
            ins.ra = (*(memory + cpu -> pc + 1) & 0xF0) >> 4;
            ins.rb = *(memory + cpu -> pc + 1) & 0x0F;
            ins.valP = cpu -> pc + 2;
            break;

        case (POPQ):
            b1 = true;
            rA = true;
            pushPop = true;
            ins.ifun.b = *(memory + cpu -> pc) & 0x0F;
            if(cpu -> pc + 2 > MEMSIZE) {
                ins.ifun.b = ins.icode;
                ins.icode = INVALID;
                cpu -> stat = ADR;
                return ins;
            }
            ins.ra = (*(memory + cpu -> pc + 1) & 0xF0) >> 4;
            ins.rb = *(memory + cpu -> pc + 1) & 0x0F;
            ins.valP = cpu -> pc + 2;
            break;

        case (IOTRAP):
            trap = true;
            ins.ifun.trap = *(memory + cpu -> pc) & 0x0F;
            ins.ra = NOREG;
            ins.rb = NOREG;
            ins.valP = cpu -> pc + 1;
            break;

        default:
            ins.ifun.b = ins.icode;
            ins.ra = NOREG;
            ins.rb = NOREG;
            ins.icode = INVALID;
            cpu -> stat = INS;
            return ins;
    }

    //check validity of single byte instruction
    if(b1) {
        if(ins.ifun.b != 0) {
            ins.ifun.b = ins.icode;
            ins.icode = INVALID;
            cpu -> stat = INS;
            return ins;
        }
    }

    //check the rA field
    if(rA) {
        if(ins.ra > 14 || ins.ra < 0) {
            ins.ifun.b = ins.icode;
            ins.icode = INVALID;
            cpu -> stat = INS;
            return ins;
        }
    }

    //check the rB field
    if(rB) {
        if(ins.rb > 14 || ins.rb < 0) {
            ins.ifun.b = ins.icode;
            ins.icode = INVALID;
            cpu -> stat = INS;
            return ins;
        }
    }

    //check the cmov ifun
    if(cmovxx) {
        if(ins.ifun.cmov < 0 || ins.ifun.cmov > 6) {
            ins.ifun.b = ins.icode;
            ins.icode = INVALID;
            cpu -> stat = INS;
            return ins;
        }
    }

    //check opq ifun
    if(irmov) {
        if(ins.ra < 15) {
            ins.ifun.b = ins.icode;
            ins.icode = INVALID;
            cpu -> stat = INS;
            return ins;
        }
    }

    //check the opq ifun
    if(opq) {
        if(ins.ifun.op < 0 || ins.ifun.op > 3) {
            ins.ifun.b = ins.icode;
            ins.icode = INVALID;
            cpu -> stat = INS;
            return ins;
        }
    }

    //check the jump ifun
    if(jxx) {
        if(ins.ifun.op < 0 || ins.ifun.op > 6) {
            ins.ifun.b = ins.icode;
            ins.icode = INVALID;
            cpu -> stat = INS;
            return ins;
        }
    }

    //check the push/pop register
    if(pushPop) {
        if(ins.rb < 15) {
            ins.ifun.b = ins.icode;
            ins.icode = INVALID;
            cpu -> stat = INS;
            return ins;
        }
    }

    //check the trap ifun
    if(trap) {
        if(ins.ifun.trap < 0 || ins.ifun.trap > 5) {
            ins.ifun.b = ins.icode;
            ins.icode = INVALID;
            cpu -> stat = INS;
            return ins;
        }
    }
    return ins;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void disassemble (y86_inst_t *inst)
{
    if(inst == NULL) {
        return;
    }
    char rA[6] = {0};
    char rB[6] = {0};

    //set the string for the first register
    switch (inst -> ra) {
        case(RAX):
            strncpy(rA, "%rax", 6);
            break;

        case (RCX):
            strncpy(rA, "%rcx", 6);
            break;

        case (RDX):
            strncpy(rA, "%rdx", 6);
            break;

        case (RBX):
            strncpy(rA, "%rbx", 6);
            break;

        case (RSP):
            strncpy(rA, "%rsp", 6);
            break;

        case (RBP):
            strncpy(rA, "%rbp", 6);
            break;

        case (RSI):
            strncpy(rA, "%rsi", 6);
            break;

        case (RDI):
            strncpy(rA, "%rdi", 6);
            break;

        case (R8):
            strncpy(rA, "%r8", 6);
            break;

        case (R9):
            strncpy(rA, "%r9", 6);
            break;

        case (R10):
            strncpy(rA, "%r10", 6);
            break;

        case (R11):
            strncpy(rA, "%r11", 6);
            break;

        case (R12):
            strncpy(rA, "%r12", 6);
            break;

        case (R13):
            strncpy(rA, "%r13", 6);
            break;

        case (R14):
            strncpy(rA, "%r14", 6);
            break;

        case (NOREG):
            break;
    }

    //set the string for the second register
    switch (inst -> rb) {
        case(RAX):
            strncpy(rB, "%rax", 6);
            break;

        case (RCX):
            strncpy(rB, "%rcx", 6);
            break;

        case (RDX):
            strncpy(rB, "%rdx", 6);
            break;

        case (RBX):
            strncpy(rB, "%rbx", 6);
            break;

        case (RSP):
            strncpy(rB, "%rsp", 6);
            break;

        case (RBP):
            strncpy(rB, "%rbp", 6);
            break;

        case (RSI):
            strncpy(rB, "%rsi", 6);
            break;

        case (RDI):
            strncpy(rB, "%rdi", 6);
            break;

        case (R8):
            strncpy(rB, "%r8", 6);
            break;

        case (R9):
            strncpy(rB, "%r9", 6);
            break;

        case (R10):
            strncpy(rB, "%r10", 6);
            break;

        case (R11):
            strncpy(rB, "%r11", 6);
            break;

        case (R12):
            strncpy(rB, "%r12", 6);
            break;

        case (R13):
            strncpy(rB, "%r13", 6);
            break;

        case (R14):
            strncpy(rB, "%r14", 6);
            break;

        case (NOREG):
            break;
    }

    //print the disassembled instruction
    switch (inst -> icode) {
        case (HALT):
            printf("halt");
            break;

        case (NOP):
            printf("nop");
            break;

        case (CMOV):
            switch ((inst -> ifun).cmov) {
                case(RRMOVQ):
                    printf("rrmovq %s, %s", rA, rB);
                    break;

                case (CMOVLE):
                    printf("cmovle %s, %s", rA, rB);
                    break;

                case (CMOVL):
                    printf("cmovl %s, %s", rA, rB);
                    break;

                case (CMOVE):
                    printf("cmove %s, %s", rA, rB);
                    break;

                case (CMOVNE):
                    printf("cmovne %s, %s", rA, rB);
                    break;

                case (CMOVGE):
                    printf("cmovge %s, %s", rA, rB);
                    break;

                case (CMOVG):
                    printf("cmovg %s, %s", rA, rB);
                    break;

                case (BADCMOV):
                    break;
            }
            break;

        case (IRMOVQ):
            printf("irmovq 0x%lx, %s", (inst -> valC).v, rB);
            break;

        case (RMMOVQ):
            if(inst -> rb != NOREG) {
                printf("rmmovq %s, 0x%lx(%s)", rA, (inst -> valC).d, rB);
            } else {
                printf("rmmovq %s, 0x%lx", rA, (inst -> valC).d);
            }
            break;

        case (MRMOVQ):
            if(inst -> rb != NOREG) {
                printf("mrmovq 0x%lx(%s), %s", (inst -> valC).d, rB, rA);
            } else {
                printf("mrmovq 0x%lx, %s", (inst -> valC).d, rA);
            }
            break;

        case (OPQ):
            switch ((inst -> ifun).op) {
                case(ADD):
                    printf("addq %s, %s", rA, rB);
                    break;

                case(SUB):
                    printf("subq %s, %s", rA, rB);
                    break;

                case(AND):
                    printf("andq %s, %s", rA, rB);
                    break;

                case(XOR):
                    printf("xorq %s, %s", rA, rB);
                    break;

                case(BADOP):
                    break;
            }
            break;

        case (JUMP):
            switch ((inst -> ifun).jump) {
                case(JMP):
                    printf("jmp 0x%lx", (inst -> valC).dest);
                    break;

                case(JLE):
                    printf("jle 0x%lx", (inst -> valC).dest);
                    break;

                case(JL):
                    printf("jl 0x%lx", (inst -> valC).dest);
                    break;

                case(JE):
                    printf("je 0x%lx", (inst -> valC).dest);
                    break;

                case(JNE):
                    printf("jne 0x%lx", (inst -> valC).dest);
                    break;

                case(JGE):
                    printf("jge 0x%lx", (inst -> valC).dest);
                    break;

                case(JG):
                    printf("jg 0x%lx", (inst -> valC).dest);
                    break;

                case(BADJUMP):
                    break;
            }
            break;

        case (CALL):
            printf("call 0x%lx", (inst -> valC).dest);
            break;

        case (RET):
            printf("ret");
            break;

        case (PUSHQ):
            printf("pushq %s", rA);
            break;

        case (POPQ):
            printf("popq %s", rA);
            break;

        case (IOTRAP):
            switch((inst -> ifun).trap) {
                case(CHAROUT):
                    printf("iotrap 0");
                    break;
                case(CHARIN):
                    printf("iotrap 1");
                    break;
                case(DECOUT):
                    printf("iotrap 2");
                    break;
                case(DECIN):
                    printf("iotrap 3");
                    break;
                case(STROUT):
                    printf("iotrap 4");
                    break;
                case(FLUSH):
                    printf("iotrap 5");
                    break;
                case(BADTRAP):
                    break;
            }
            break;
        case(INVALID):
            break;

    }
}

void disassemble_code (byte_t *memory, elf_phdr_t *phdr, elf_hdr_t *hdr)
{
    if(memory == NULL || phdr == NULL || hdr == NULL) {
        return;
    }
    y86_t cpu;          // CPU struct to store "fake" PC
    y86_inst_t ins;     // struct to hold fetched instruction
    int currentAddr = phdr -> p_vaddr;

    // start at beginning of the segment
    cpu.pc = phdr->p_vaddr;

    printf("  0x%03x:                               | .pos 0x%03x code\n", currentAddr, currentAddr);

    // iterate through the segment one instruction at a time
    while (cpu.pc < phdr -> p_vaddr + phdr -> p_size) {
        int currentBits = 0;
        if(currentAddr == hdr -> e_entry) {
            printf("  0x%03x:                               | _start:\n", currentAddr);
        }
        ins = fetch (&cpu, memory);         // stage 1: fetch instruction
        if(ins.icode == INVALID) {
            printf("Invalid opcode: 0x%x%x\n\n", ins.ifun.b, INVALID);
            return;
        }


        //printing bytes for current instruction
        printf("  0x%03x: ", currentAddr);

        while(currentAddr < ins.valP) {
            printf("%02x ", memory[currentAddr]);
            currentBits ++;
            currentAddr ++;
        }

        //formatting for instructions != 10 bytes
        while(currentBits < 10) {
            printf("   ");
            currentBits ++;
        }

        //spacing for instrcution
        printf("|   ");

        //print instruction
        disassemble (&ins);                   // stage 2: print disassembly
        printf("\n");
        cpu.pc = ins.valP;                    // stage 3: update PC (go to next instruction)
    }
    printf("\n");
}

void disassemble_data (byte_t *memory, elf_phdr_t *phdr)
{
    if(memory == NULL || phdr == NULL) {
        return;
    }

    int currentAddr = phdr -> p_vaddr;
    int size = phdr -> p_vaddr + phdr -> p_size;

    printf("  0x%03x:                               | .pos 0x%03x data\n", currentAddr, currentAddr);

    //loop through the segment
    while(currentAddr < size) {
        printf("  0x%03x: ", currentAddr);

        //printing out little endian bits
        while(currentAddr < phdr -> p_vaddr + 8) {
            printf("%02x ", memory[currentAddr]);
            currentAddr ++;
        }

        //spacing for instrcution
        printf("      |   .quad 0x");

        //print the disassembled variables in hex
        for(int i = phdr -> p_vaddr + 7; i > phdr -> p_vaddr - 1; i--) {
            if(memory[i] != 0x0) {
                printf("%x", memory[i]);
            }
        }

        printf("\n");
        phdr -> p_vaddr += 8;

    }
    printf("\n");
}

void disassemble_rodata (byte_t *memory, elf_phdr_t *phdr)
{
    if(memory == NULL || phdr == NULL) {
        return;
    }

    int currentAddr = phdr -> p_vaddr;
    int size = phdr -> p_vaddr + phdr -> p_size;

    printf("  0x%03x:                               | .pos 0x%03x rodata\n", currentAddr, currentAddr);

    //loop through the segment
    while(currentAddr < size) {
        bool needLine = false;
        printf("  0x%03x: ", currentAddr);

        //find null terminator for current string
        int terminator = currentAddr;
        while(memory[terminator] != 0) {
            terminator++;
        }

        //printing out little endian bits
        while(currentAddr < phdr -> p_vaddr + 10) {
            //only print bytes until null terminator is reached
            if(currentAddr > terminator) {
                printf("   ");
            } else {
                printf("%02x ", memory[currentAddr]);
            }
            currentAddr++;

        }
        //if 10 bits exceeded
        if(currentAddr <= terminator) {
            needLine = true;
        }

        //update address to be byte after terminator
        if(currentAddr > terminator) {
            currentAddr = terminator + 1;
        }

        //spacing for instrcution
        printf("|   .string \"");

        //print the disassembled variables in hex
        for(int i = phdr -> p_vaddr; i < terminator; i++) {
            printf("%c", (char)memory[i]);
        }
        printf("\"\n");

        //fit bytes into 10 byte spaces
        if(needLine) {
            int numBytes = 0;
            printf("  0x%03x: ", currentAddr);
            while(currentAddr <= terminator) {
                if(numBytes == 10) {
                    printf("| \n  0x%03x: ", currentAddr);
                    numBytes = 0;
                }
                printf("%02x ", memory[currentAddr]);
                currentAddr++;
                numBytes ++;
            }
            //pad with spaces
            while(numBytes < 10) {
                printf("   ");
                numBytes++;
            }
            printf("| \n");
        }
        phdr -> p_vaddr = currentAddr;
    }
    printf("\n");
}

