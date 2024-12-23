/*
 * Mini-ELF interpreter
 *
 * Name: Griffin Moran
 */

#include "p4-interp.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

/*
Perform the decode and execute stages of the instruction cycle.
Depending on the instruction, the function may update the CPU status in case of error.
The cnd variable will be updated for conditional moves and jumps.
The valA will be updated as specified in the semantics, and the return value is the valE value as specified in the semantics.

@param cpu Y86 CPU structure
@param inst Y86 instruction structure for currently-executing instruction
@param cnd Pointer to boolean flag to be set for conditional jumps and moves
@param valA Pointer to Y86 register for passing valA to later stages
@returns Result of execute phase (valE)
*/
y86_reg_t decode_execute (y86_t *cpu, y86_inst_t *inst, bool *cnd, y86_reg_t *valA)
{
    y86_reg_t valE = 0;
    y86_reg_t valB;

    //check for invalid params
    if(!cpu) {
        return valE;
    }

    if(!inst || !cnd || !valA) {
        cpu -> stat = INS;
        return valE;
    }

    switch(inst -> icode) {
        case (HALT):
            cpu -> stat = HLT;
            break;

        case (NOP):
            break;

        case (CMOV):
            *valA = cpu -> reg[inst -> ra];
            valE = *valA;
            switch ((inst -> ifun).cmov) {
                case(RRMOVQ):
                    *cnd = true;
                    break;

                case(CMOVLE):
                    *cnd = (cpu -> sf ^ cpu -> of) || cpu -> zf;
                    break;

                case(CMOVL):
                    *cnd = (cpu -> sf ^ cpu -> of);
                    break;

                case(CMOVE):
                    *cnd = cpu -> zf;
                    break;

                case(CMOVNE):
                    *cnd = !(cpu -> zf);
                    break;

                case(CMOVGE):
                    *cnd = !(cpu -> sf ^ cpu -> of);
                    break;

                case(CMOVG):
                    *cnd = !(cpu -> sf ^ cpu -> of) && !(cpu -> zf);
                    break;

                case (BADCMOV):
                    cpu -> stat = INS;
                    break;
            }
            break;

        case (IRMOVQ):
            valE = (inst -> valC).v;
            break;

        case (RMMOVQ):
            *valA = cpu -> reg[inst -> ra];
            valB = cpu -> reg[inst -> rb];

            valE = valB + (inst -> valC).d;
            break;

        case (MRMOVQ):
            valB = cpu -> reg[inst -> rb];

            valE = valB + (inst -> valC).d;
            break;

        case (OPQ):
            switch ((inst -> ifun).op) {
                case(ADD):
                    *valA = cpu -> reg[inst -> ra];
                    valB = (cpu -> reg)[inst -> rb];
                    valE = valB + *valA;

                    //zero check set all flags to false if they fail the if
                    if((int64_t)valE == 0) {
                        cpu -> zf = true;
                    } else {
                        cpu -> zf = false;
                    }

                    //sign check
                    if ((int64_t)valE < 0) { //figure out alternative
                        cpu -> sf = true;
                    } else {
                        cpu -> sf = false;
                    }

                    //overflow check
                    if((((int64_t)*valA < 0) == ((int64_t)valB < 0)) && (((int64_t)valE < 0) != ((int64_t)valB < 0))) {
                        cpu -> of = true;
                    } else {
                        cpu -> of = false;
                    }
                    break;

                case(SUB):
                    *valA = (cpu -> reg)[inst -> ra];
                    valB = (cpu -> reg)[inst -> rb];
                    valE = valB - *valA;

                    //zero check
                    if((int64_t)valE == 0) {
                        cpu -> zf = true;
                    } else {
                        cpu -> zf = false;
                    }

                    //sign check
                    if ((int64_t)valE < 0) {
                        cpu -> sf = true;
                    } else {
                        cpu -> sf = false;
                    }

                    //overflow check
                    if((int64_t)*valA > 0 && (int64_t)valE > (int64_t)valB) {
                        cpu -> of = true;
                    } else if((int64_t)*valA < 0 && (int64_t)valE < (int64_t)valB) {
                        cpu -> of = true;
                    } else {
                        cpu -> of = false;
                    }

                    break;

                case(AND):
                    *valA = (cpu -> reg)[inst -> ra];
                    valB = (cpu -> reg)[inst -> rb];
                    valE = valB & *valA;

                    //zero check
                    if((int64_t)valE == 0) {
                        cpu -> zf = true;
                    } else {
                        cpu -> zf = false;
                    }

                    //sign check
                    if ((int64_t)valE < 0) { //figure out an alternative for this
                        cpu -> sf = true;
                    } else {
                        cpu -> sf = false;
                    }
                    cpu -> of = false;
                    break;

                case(XOR):
                    *valA = (cpu -> reg)[inst -> ra];
                    valB = (cpu -> reg)[inst -> rb];
                    valE = valB ^ *valA;

                    //zero check
                    if((int64_t)valE == 0) {
                        cpu -> zf = true;
                    } else {
                        cpu -> zf = false;
                    }

                    //sign check
                    if ((int64_t)valE < 0) {
                        cpu -> sf = true;
                    } else {
                        cpu -> sf = false;
                    }
                    cpu -> of = false;
                    break;

                case(BADOP):
                    cpu -> stat = INS;
                    break;
            }
            break;

        case (JUMP):
            switch ((inst -> ifun).jump) {
                case(JMP):
                    *cnd = true;
                    break;

                case(JLE):
                    *cnd = (cpu -> sf ^ cpu -> of) || cpu -> zf;
                    break;

                case(JL):
                    *cnd = (cpu -> sf ^ cpu -> of);
                    break;

                case(JE):
                    *cnd = cpu -> zf;
                    break;

                case(JNE):
                    *cnd = !(cpu -> zf);
                    break;

                case(JGE):
                    *cnd = !(cpu -> sf ^ cpu -> of);
                    break;

                case(JG):
                    *cnd = !(cpu -> sf ^ cpu -> of) && !(cpu -> zf);
                    break;

                case(BADJUMP):
                    cpu -> stat = INS;
                    break;
            }
            break;

        case (CALL):
            valB = cpu -> reg[RSP];
            valE = valB - 8;

            //check to make sure negative memory wont be written to in memory_wb_pc
            if((int64_t)valE < 0) {
                cpu -> stat = ADR;
            }
            break;

        case (RET):
            *valA = cpu -> reg[RSP];
            valB = cpu -> reg[RSP];

            valE = valB + 8;
            break;

        case (PUSHQ):
            *valA = cpu -> reg[inst -> ra];
            valB = cpu -> reg[RSP];

            valE = valB - 8;
            break;

        case (POPQ):
            *valA = cpu -> reg[RSP];
            valB = cpu -> reg[RSP];

            valE = valB + 8;
            break;

        case (IOTRAP):
            break;
        case(INVALID):
            cpu -> stat = INS;
            break;
    }
    return valE;
}

/*
Perform the memory, write-back, and update PC stages.
The CPU registers or memory could be modified depending on the instruction executed.
The CPU PC must be updated accordingly.

@param cpu Y86 CPU structure
@param inst Y86 instruction structure for currently-executing instruction
@param memory Pointer to beginning of the Y86 address space
@param cnd Flag that indicates whether a conditional jumps or move should happen
@param valA Register with valA from earlier stages
@param valE Register with valE from earlier stages
*/
void memory_wb_pc (y86_t *cpu, y86_inst_t *inst, byte_t *memory,
                   bool cnd, y86_reg_t valA, y86_reg_t valE)
{
    y86_reg_t valM;
    const int memsize = 4096;

    //check for invalid parameters
    if(!cpu) {
        return;
    }

    if(!inst) {
        cpu -> stat = INS;
    }

    if(!memory) {
        cpu -> stat = INS;
        return;
    }

    //static ensures values do not get overwritten, much like global variables
    //variables for the output buffer and a counter for variables in the buffer
    static char output[101] = {'\0'};
    static size_t bufLen = 0;
    y86_reg_t memVal = 0;

    switch(inst -> icode) {
        case (HALT):
            cpu -> pc = inst -> valP;
            break;

        case (NOP):
            cpu -> pc = inst -> valP;
            break;

        case (CMOV):
            if(cnd) {
                cpu -> reg[inst -> rb] = valE;
            }
            cpu -> pc = inst -> valP;
            break;

        case (IRMOVQ):
            cpu -> reg[inst -> rb] = valE;
            cpu -> pc = inst -> valP;
            break;

        case (RMMOVQ):
            if(valE + 8 <= memsize) {
                memcpy(memory + valE, &valA, sizeof(y86_reg_t));
            } else {
                cpu -> stat = ADR;
            }
            cpu -> pc = inst -> valP;
            break;

        case (MRMOVQ):
            if(valE + 8 <= memsize) {
                memcpy(&valM, memory + valE, sizeof(y86_reg_t));
                cpu -> reg[inst -> ra] = valM;
            } else {
                cpu -> stat = ADR;
            }
            cpu -> pc = inst -> valP;
            break;

        case (OPQ):
            cpu -> reg[inst -> rb] = valE;
            cpu -> pc = inst -> valP;
            break;

        case (JUMP):
            if(cnd) {
                cpu -> pc = (inst -> valC).dest;
            } else {
                cpu -> pc = inst -> valP;
            }
            break;

        case (CALL):
            //early check for invalid stack calls
            if(cpu -> stat != ADR) {
                memcpy(memory + valE, &(inst -> valP), sizeof(y86_reg_t));
                cpu -> reg[RSP] = valE;
            }
            cpu -> pc = (inst -> valC).dest;
            break;

        case (RET):
            memcpy(&valM, memory + valA, sizeof(y86_reg_t));
            cpu -> reg[RSP] = valE;
            cpu -> pc = valM;
            break;

        case (PUSHQ):
            memcpy(memory + valE, &valA, sizeof(y86_reg_t));
            cpu -> reg[RSP] = valE;
            cpu -> pc = inst -> valP;
            break;

        case (POPQ):
            memcpy(&valM, memory + valA, sizeof(y86_reg_t));
            cpu -> reg[RSP] = valE;
            cpu -> reg[inst -> ra] = valM;
            cpu -> pc = inst -> valP;
            break;

        case (IOTRAP):
            switch((inst -> ifun).trap) {
                case(CHAROUT):
                    if(!cpu -> reg[RSI] || bufLen >= sizeof(output)) {
                        cpu -> stat = HLT;
                        printf("I/O Error\n");
                    } else {
                        memVal = cpu -> reg[RSI];
                        snprintf(&output[bufLen], sizeof(char) * 2, "%c", memory[memVal]);
                        bufLen ++;
                    }
                    cpu -> pc = inst -> valP;
                    break;

                case(CHARIN):
                    memVal = cpu -> reg[RDI];
                    if(scanf("%c", &memory[memVal]) != 1) {
                        cpu -> stat = HLT;
                        printf("I/O Error\n");
                    }
                    cpu -> pc = inst -> valP;
                    break;

                case(DECOUT):
                    if(!cpu -> reg[RSI] || bufLen > sizeof(output)) {
                        cpu -> stat = HLT;
                        printf("I/O Error\n");
                    } else {
                        memVal = cpu -> reg[RSI];
                        //read a byte pointer from memory and typecast it to a 64 bit int pointer
                        int64_t* num = (int64_t*)&memory[memVal];
                        //write the value of the int pointer to output buffer
                        int numChars = snprintf(&output[bufLen],sizeof(output) - bufLen, "%lld", *num);
                        //update character count
                        bufLen += numChars;
                    }
                    cpu -> pc = inst -> valP;
                    break;

                case(DECIN):
                    memVal = cpu -> reg[RDI];
                    if(scanf("%lld", (int64_t*)(&memory[memVal])) != 1) {
                        cpu -> stat = HLT;
                        printf("I/O Error\n");
                    }
                    cpu -> pc = inst -> valP;
                    break;

                case(STROUT):
                    if(!cpu -> reg[RSI] || bufLen > sizeof(output)) {
                        cpu -> stat = HLT;
                        printf("I/O Error\n");
                    } else {
                        memVal = cpu -> reg[RSI];

                        //loop through chars adding them to output buffer until null pointer is reached
                        char cur = memory[memVal];
                        while(cur != '\0') {
                            snprintf(&output[bufLen], sizeof(output) - bufLen, "%c", cur);
                            cur = memory[++memVal];
                            bufLen++;
                        }
                    }
                    cpu -> pc = inst -> valP;
                    break;

                case(FLUSH):
                    output[bufLen] = '\0';
                    printf("%s", output);
                    memset(output, '\0', sizeof(output));
                    cpu -> pc = inst -> valP;
                    break;

                case(BADTRAP):
                    cpu -> stat = INS;
                    break;
            }
            break;
        case(INVALID):
            cpu -> stat = INS;
            break;
    }
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

/*
Print out the contents of the CPU according the format described below.
The address of the entry point.
The final state of the CPU, including all registers in unsigned hexadecimal, condition flags, and the status code.
The total number of instructions that were executed.
*/
void dump_cpu_state (y86_t *cpu)
{
    if(!cpu) {
        return;
    }

    char status[4];
    switch(cpu -> stat) {
        case AOK:
            strncpy(status, "AOK", sizeof(status));
            break;

        case HLT:
            strncpy(status, "HLT", sizeof(status));
            break;

        case ADR:
            strncpy(status, "ADR", sizeof(status));
            break;

        case INS:
            strncpy(status, "INS", sizeof(status));
            break;

        default:
            return;
    }

    printf("Y86 CPU state:\n");
    printf("    PC: %016llx   flags: Z%d S%d O%d     %s\n", cpu -> pc, cpu -> zf, cpu -> sf, cpu -> of,
           status);
    printf("  %%rax: %016llx    %%rcx: %016llx\n", cpu -> reg[0], cpu -> reg[1]);
    printf("  %%rdx: %016llx    %%rbx: %016llx\n", cpu -> reg[2], cpu -> reg[3]);
    printf("  %%rsp: %016llx    %%rbp: %016llx\n", cpu -> reg[4], cpu -> reg[5]);
    printf("  %%rsi: %016llx    %%rdi: %016llx\n", cpu -> reg[6], cpu -> reg[7]);
    printf("   %%r8: %016llx     %%r9: %016llx\n", cpu -> reg[8], cpu -> reg[9]);
    printf("  %%r10: %016llx    %%r11: %016llx\n", cpu -> reg[10], cpu -> reg[11]);
    printf("  %%r12: %016llx    %%r13: %016llx\n", cpu -> reg[12], cpu -> reg[13]);
    printf("  %%r14: %016llx\n", cpu -> reg[14]);
}
