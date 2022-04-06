/*
 * LC4.c: Defines simulator functions for executing instructions
 */

#include "LC4.h"
#include <stdio.h>

#define INSN_OP(I) ( (I) >> 12 ) // [15:12]
#define INSN_11_9(I) ( ( (I) >> 9 ) & 0x7 ) // [11:9]
#define INSN_8_7(I) ( ( (I) >> 7 ) & 0x3 ) // [8:7]
#define INSN_8_6(I) ( ( (I) >> 6 ) & 0x7 ) // [8:6]
#define INSN_5_4(I) ( ( (I) >> 4 ) & 0x3 ) // [5:4]
#define INSN_5_3(I) ( ( (I) >> 3 ) & 0x7 ) // [5:3]
#define INSN_2_0(I) ( (I) & 0x7 ) // [2:0]
#define INSN_10_0(I) ( (I) & 0x7FF ) // IMM11
#define INSN_8_0(I) ( (I) & 0x1FF ) // IMM9
#define INSN_7_0(I) ( (I) & 0xFF ) // IMM8
#define INSN_6_0(I) ( (I) & 0x7F ) // IMM7
#define INSN_5_0(I) ( (I) & 0x3F ) // IMM6
#define INSN_4_0(I) ( (I) & 0x1F ) // IMM5
#define INSN_3_0(I) ( (I) & 0xF ) // IMM4
#define INSN_15(I) ( (I) >> 15 ) // [15]
#define INSN_11(I) ( ( (I) >> 11 ) & 0x1 ) // [11]

/*
 * Helper function that turns unsigned int into binary string
 * TODO may not work
 */
void to_binary(unsigned short int input, char* bin_str) {
    int i = 15;
    while (input != 0) {
        if (input % 2 == 0) {
            bin_str[i] = '0';
        } else {
            bin_str[i] = '1';
        }
        input = input >> 1;
        --i;
    }
    while (i > -1) {
        bin_str[i] = '0';
        --i;
    }
}

/*
 * Reset the machine state as Pennsim would do
 */
void Reset(MachineState* CPU)
{
    CPU->PC = 0x8200;
    CPU->PSR = 0x8002; // 1000000000000010
    int i;
    for (i = 0; i < 8; i++) {
        CPU->R[i] = 0;
    }
    for (i = 0; i < 65536; i++) {
        CPU->memory[i] = 0;
    }
}


/*
 * Clear all of the control signals (set to 0)
 */
void ClearSignals(MachineState* CPU)
{
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    CPU->regFile_WE = 0;
    CPU->NZP_WE = 0;
    CPU->DATA_WE = 0;
    CPU->regInputVal = 0;
    CPU->NZPVal = 0;
    CPU->dmemAddr = 0;
    CPU->dmemValue = 0;
}


/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState* CPU, FILE* output)
{
    
    // check if null
    if (output == NULL) {
        fprintf(stderr, "error: WriteOut() failed...FILE output is NULL\n");
        return;
    }
    
    if (CPU->PC == 0x80FF) {
        return;
    }
    
    // eg 8200 1001111000000000 1 7 0002 1 1 0 0000 0000
    char buffer[50]; // to write ouput
    
    unsigned short int pc = CPU->PC; // program counter
    unsigned short int curr_mem = CPU->memory[pc]; // instruction at current memory
    char* instr_bin = NULL; // instruction as a binary string
    instr_bin = malloc(3 * sizeof(*instr_bin));
    to_binary(curr_mem, instr_bin);
    printf("curr_mem: 0x%X ...PC 0x%X %s\n", curr_mem, pc, instr_bin);
    
    unsigned short int opcode = INSN_OP(curr_mem); // opcode of instructions
    
    switch (opcode) {
        case 0: // branch
            sprintf(buffer, "%04X %s %X 0 0000 %X 0 %X 0000 0000\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, CPU->NZP_WE, CPU->DATA_WE);
            break;
        case 1: // arithmetic
            //unsigned short int rd = INSN_11_9(curr_mem);
            sprintf(buffer, "%04X %s %X %X %04X %X %X %X 0000 0000\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, INSN_11_9(curr_mem), CPU->regInputVal,\
                    CPU->NZP_WE, CPU->NZPVal, CPU->DATA_WE);
            break;
        case 2: // comparison
            sprintf(buffer, "%04X %s %X 0 0000 %X %X %X 0000 0000\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, CPU->NZP_WE, CPU->NZPVal, CPU->DATA_WE);
            break;
        case 4: // jsr
            sprintf(buffer, "%04X %s %X 7 %04X %X %X %X 0000 0000\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, CPU->regInputVal, CPU->NZP_WE, CPU->NZPVal, CPU->DATA_WE);
            break;
        case 5: // logical
            //unsigned short int rd = INSN_11_9(curr_mem);
            sprintf(buffer, "%04X %s %X %X %04X %X %X %X 0000 0000\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, INSN_11_9(curr_mem), CPU->regInputVal,\
                    CPU->NZP_WE, CPU->NZPVal, CPU->DATA_WE);
            break;
        case 6: // ldr
            //unsigned short int rd = INSN_11_9(curr_mem);
            sprintf(buffer, "%04X %s %X %X %04X %X %X %X %04X %04X\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, INSN_11_9(curr_mem), CPU->regInputVal,\
                    CPU->NZP_WE, CPU->NZPVal, CPU->DATA_WE, CPU->dmemAddr, CPU->dmemValue);
            break;
        case 7: // str
            sprintf(buffer, "%04X %s %X 0 0000 %X 0 %X %04X %04X\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, CPU->NZP_WE, CPU->DATA_WE, CPU->dmemAddr, CPU->dmemValue);
            break;
        case 8: // RTI
            sprintf(buffer, "%04X %s %X 0 0000 %X 0 %X 0000 0000\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, CPU->NZP_WE, CPU->DATA_WE);
            break;
        case 9: // const
            //unsigned short int rd = INSN_11_9(curr_mem);
            sprintf(buffer, "%04X %s %X %X %04X %X %X %X 0000 0000\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, INSN_11_9(curr_mem), CPU->regInputVal,\
                    CPU->NZP_WE, CPU->NZPVal, CPU->DATA_WE);
            break;
        case 10: // shifts and mod
            //unsigned short int rd = INSN_11_9(curr_mem);
            sprintf(buffer, "%04X %s %X %X %04X %X %X %X 0000 0000\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, INSN_11_9(curr_mem), CPU->regInputVal,\
                    CPU->NZP_WE, CPU->NZPVal, CPU->DATA_WE);
            break;
        case 12: // jumps
            sprintf(buffer, "%04X %s %X 0 0000 %X 0 %X 0000 0000\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, CPU->NZP_WE, CPU->DATA_WE);
            break;
        case 13: // hiconst
            //unsigned short int rd = INSN_11_9(curr_mem);
            sprintf(buffer, "%04X %s %X %X %04X %X %X %X 0000 0000\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, INSN_11_9(curr_mem), CPU->regInputVal,\
                    CPU->NZP_WE, CPU->NZPVal, CPU->DATA_WE);
            break;
        case 15: // trap
            //unsigned short int rd = INSN_11_9(curr_mem);
            sprintf(buffer, "%04X %s %X 7 %04X %X %X %X 0000 0000\n",\
                    pc, instr_bin,\
                    CPU->regFile_WE, CPU->regInputVal,\
                    CPU->NZP_WE, CPU->NZPVal, CPU->DATA_WE);
            break;
        default:
            fprintf(stderr, "error: WriteOut() failed...invlaid instruction at memory 0x%X\n", pc);
            return;
    }
    ClearSignals(CPU);
    free(instr_bin);
    fputs(buffer, output);
}


/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState* CPU, FILE* output)
{
    unsigned short int pc = CPU->PC;
    unsigned short int privilege = INSN_15(CPU->PSR);
    // Except Error 1:
    if ( (pc > 0x1FFF && pc < 0x8000) || pc > 0x9FFF ) {
        fprintf(stderr, "error1: attempting to execute a data section address as code\n");
        return -1;
    }
    // Except Error 3:
    if (pc > 0x7FFF && privilege == 0) {
        fprintf(stderr, "error3: attempting to accesss OS while in user mode\n");
        return -1;
    }
    unsigned short int instr = CPU->memory[CPU->PC]; // instruction
    unsigned short int opcode = INSN_OP(instr); // opcode
    unsigned short int rd;
    unsigned short int rs;
    unsigned short int rt;
    short int sign;
    signed short int imm6 = INSN_5_0(instr);
    sign = ( (imm6) >> 5);
    if (sign == 1) {
        imm6 = ( 0xFFCE | imm6 );
    }
    signed short int imm9 = INSN_8_0(instr);
    sign = ( (imm9) >> 8);
    if (sign == 1) {
        imm9 = ( 0xFE00 | imm9 );
    }
    unsigned short int uimm8 = INSN_7_0(instr);
    
    switch (opcode) {
        case 0: // branch
            BranchOp(CPU, output);
            break;
        case 1: // arithmetic
            ArithmeticOp(CPU, output);
            break;
        case 2: // comparison
            ComparativeOp(CPU, output);
            break;
        case 4: // jsr
            JSROp(CPU, output);
            break;
        case 5: // logical
            LogicalOp(CPU, output);
            break;
        case 6: // ldr
            rd = INSN_11_9(instr);
            rs = INSN_8_6(instr);
            //imm6 = INSN_5_0(instr);
            CPU->R[rd] = CPU->memory[CPU->R[rs] + imm6];
            // Except Error 2
            if ( (CPU->R[rs] + imm6) < 0x2000 ||\
                ((CPU->R[rs] + imm6) > 0x7FFF && (CPU->R[rs] + imm6) < 0xA000) ) {
                fprintf(stderr, "error2: attempting to read data from code section\n");
                return -1;
            }
            // Except Error 3
            if ( (CPU->R[rs] + imm6) > 0x7FFF && privilege == 0) {
                fprintf(stderr, "error3: attempting to accesss OS while in user mode\n");
                return -1;
            }
            CPU->rsMux_CTL = 0;
            CPU->rtMux_CTL = 0;
            CPU->rdMux_CTL = 0;
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 1;
            CPU->DATA_WE = 0;
            CPU->regInputVal = CPU->R[rd];
            SetNZP(CPU, CPU->regInputVal);
            CPU->dmemAddr = CPU->R[rs] + imm6;
            CPU->dmemValue = CPU->R[rd];
            WriteOut(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
        case 7: // str
            rt = INSN_11_9(instr);
            rs = INSN_8_6(instr);
            //imm6 = INSN_5_0(instr);
            printf("Accessing memory address 0x%X\n", (CPU->R[rs]+imm6));
            CPU->memory[CPU->R[rs] + imm6] = CPU->R[rt];
            // Except Error 2
            if ( (CPU->R[rs] + imm6) < 0x2000 ||\
                ((CPU->R[rs] + imm6) > 0x7FFF && (CPU->R[rs] + imm6) < 0xA000) ) {
                fprintf(stderr, "error2: attempting to write data to code section\n");
                return -1;
            }
            // Except Error 3
            if ( (CPU->R[rs] + imm6) > 0x7FFF && privilege == 0) {
                fprintf(stderr, "error3: attempting to accesss OS while in user mode\n");
                return -1;
            }
            CPU->rsMux_CTL = 0;
            CPU->rtMux_CTL = 1;
            CPU->rdMux_CTL = 0;
            CPU->regFile_WE = 0;
            CPU->NZP_WE = 0;
            CPU->DATA_WE = 1;
            CPU->dmemAddr = CPU->R[rs] + imm6;
            CPU->dmemValue = CPU->R[rt];
            WriteOut(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
        case 8: // RTI
            CPU->rsMux_CTL = 1;
            CPU->rtMux_CTL = 0;
            CPU->rdMux_CTL = 0;
            CPU->regFile_WE = 0;
            CPU->NZP_WE = 0;
            CPU->DATA_WE = 0;
            WriteOut(CPU, output);
            CPU->PC = CPU->R[7];
            CPU->PSR = ( (CPU->PSR) & 0x7FFF ); // PSR[15] = 0
            break;
        case 9: // const
            rd = INSN_11_9(instr);
            //imm9 = INSN_8_0(instr);
            CPU->R[rd] = imm9;
            CPU->rsMux_CTL = 0;
            CPU->rtMux_CTL = 0;
            CPU->rdMux_CTL = 0;
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 1;
            CPU->DATA_WE = 0;
            CPU->regInputVal = CPU->R[rd];
            SetNZP(CPU, CPU->regInputVal);
            // writout
            WriteOut(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
        case 10: // shifts and mod
            ShiftModOp(CPU, output);
            break;
        case 12: // jumps
            JumpOp(CPU, output);
            break;
        case 13: // hiconst
            rd = INSN_11_9(instr);
            //uimm8 = INSN_7_0(instr);
            CPU->R[rd] = ( (CPU->R[rd] & 0xFF) | (uimm8 << 8) );
            CPU->rsMux_CTL = 2;
            CPU->rtMux_CTL = 0;
            CPU->rdMux_CTL = 0;
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 1;
            CPU->DATA_WE = 0;
            CPU->regInputVal = CPU->R[rd];
            SetNZP(CPU, CPU->regInputVal);
            // writeout
            WriteOut(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
        case 15: // trap
            //uimm8 = INSN_7_0(instr);
            CPU->R[7] = CPU->PC + 1;
            CPU->rsMux_CTL = 0;
            CPU->rtMux_CTL = 0;
            CPU->rdMux_CTL = 1;
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 1;
            CPU->DATA_WE = 0;
            CPU->regInputVal = CPU->R[7];
            SetNZP(CPU, CPU->regInputVal);
            // put writeout here instead?
            WriteOut(CPU, output);
            CPU->PC = (0x8000 | uimm8);
            CPU->PSR = ( (CPU->PSR) | 0x8000 ); // PSR[15] = 1
            break;
        default:
            fprintf(stderr, "error: UpdateMachineState() failed...invlaid instruction at memory 0x%X\n", CPU->PC);
            return -1; 
    }
    
    /*----------------------*/
    return 0;
}



//////////////// PARSING HELPER FUNCTIONS ///////////////////////////



/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState* CPU, FILE* output)
{
    unsigned short int instr = CPU->memory[CPU->PC];
    unsigned short int subop = INSN_11_9(instr);
    signed short int imm9 = INSN_8_0(instr);
    short int sign = ( (imm9) >> 8);
    if (sign == 1) {
        imm9 = ( 0xFE00 | imm9 );
    }
    unsigned short int nzp = INSN_2_0(CPU->PSR);
    
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    
    CPU->regFile_WE = 0;
    CPU->NZP_WE = 0;
    CPU->DATA_WE = 0;
    
//     if (subop == 0) {
//         CPU->PC = CPU->PC + 1;
//         return;
//     }
    
    WriteOut(CPU, output);
    
    switch (subop) {
        case 0: // NOP
            CPU->PC = CPU->PC + 1;
            break;
        case 1: // BRp
            if (nzp == 1) {
                CPU->PC = CPU->PC + 1 + imm9;
            } else {
                CPU->PC = CPU->PC + 1;
            }
            break;
        case 2: // BRz
            if (nzp == 2) {
                CPU->PC = CPU->PC + 1 + imm9;
            } else {
                CPU->PC = CPU->PC + 1;
            }
            break;
        case 3: // BRzp
            if (nzp == 2 || nzp == 1) {
                CPU->PC = CPU->PC + 1 + imm9;
            } else {
                CPU->PC = CPU->PC + 1;
            }
            break;
        case 4: // BRn
            if (nzp == 4) {
                CPU->PC = CPU->PC + 1 + imm9;
            } else {
                CPU->PC = CPU->PC + 1;
            }
            break;
        case 5: // BRnp
            if (nzp == 1 || nzp == 4) {
                CPU->PC = CPU->PC + 1 + imm9;
            } else {
                CPU->PC = CPU->PC + 1;
            }
            break;
        case 6: // BRnz
            if (nzp == 4 || nzp == 2) {
                CPU->PC = CPU->PC + 1 + imm9;
            } else {
                CPU->PC = CPU->PC + 1;
            }
            break;
        case 7: // BRnzp
            CPU->PC = CPU->PC + 1 + imm9;
            break;
        default:
            fprintf(stderr, "error: BranchOp() failed...invalid nzp value\n");
            return;
    }
    
}

/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState* CPU, FILE* output)
{
    unsigned short int instr = CPU->memory[CPU->PC];
    unsigned short int rd = INSN_11_9(instr);
    unsigned short int rs = INSN_8_6(instr);
    unsigned short int rt = INSN_2_0(instr);
    unsigned short int subop = INSN_5_3(instr);
    signed short int imm5 = INSN_4_0(instr);
    short int sign = ( (imm5) >> 4);
    if (sign == 1) {
        imm5 = ( 0xFFE0 | imm5 );
    }
    
    switch (subop) {
        case 0: // ADD
            CPU->R[rd] = CPU->R[rs] + CPU->R[rt];
            break;
        case 1: // MUL
            CPU->R[rd] = CPU->R[rs] * CPU->R[rt];
            break;
        case 2: // SUB
            CPU->R[rd] = CPU->R[rs] - CPU->R[rt];
            break;
        case 3: // DIV
            CPU->R[rd] = CPU->R[rs] / CPU->R[rt];
            break;
        default: // ADD IMM5
            CPU->R[rd] = (CPU->R[rs]) + imm5;
    }
    
    CPU->regInputVal = CPU->R[rd];
    
    SetNZP(CPU, (short) CPU->regInputVal);
    
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    CPU->DATA_WE = 0;
    
    WriteOut(CPU, output);
    CPU->PC = CPU->PC + 1;
    
}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState* CPU, FILE* output)
{
    unsigned short int instr = CPU->memory[CPU->PC];
    unsigned short int rs = INSN_11_9(instr);
    unsigned short int rt = INSN_2_0(instr);
    unsigned short int subop = INSN_8_7(instr);
    signed short int imm7 = INSN_6_0(instr);
    unsigned short int uimm7 = INSN_6_0(instr);
    signed short int result;
    unsigned short int uresult;
    short int sign = ( (imm7) >> 6);
    if (sign == 1) {
        imm7 = ( 0xFF80 | imm7 );
    }
    
    switch (subop) {
        case 0:
            result = CPU->R[rs] - CPU->R[rt];
            SetNZP(CPU, result);
            break;
        case 1:
            uresult = CPU->R[rs] - CPU->R[rt];
            SetNZP(CPU, uresult);
            break;
        case 2:
            result = CPU->R[rs] - imm7;
            SetNZP(CPU, result);
            break;
        case 3:
            uresult = CPU->R[rs] - uimm7;
            SetNZP(CPU, uresult);
            break;
        default:
            fprintf(stderr, "error: ComparativeOp() failed...invalid subop code 0x%X\n", subop);
            return;
    }
        
    CPU->rsMux_CTL = 2;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    
    CPU->regFile_WE = 0;
    CPU->NZP_WE = 1;
    CPU->DATA_WE = 0;
    
    WriteOut(CPU, output);
    CPU->PC = CPU->PC + 1;
    
}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState* CPU, FILE* output)
{
    unsigned short int instr = CPU->memory[CPU->PC];
    unsigned short int rd = INSN_11_9(instr);
    unsigned short int rs = INSN_8_6(instr);
    unsigned short int rt = INSN_2_0(instr);
    unsigned short int subop = INSN_5_3(instr);
    signed short int imm5 = INSN_4_0(instr);
    short int sign = ( (imm5) >> 4);
    if (sign == 1) {
        imm5 = ( 0xFFE0 | imm5 );
    }
    
    switch (subop) {
        case 0: // AND
            CPU->R[rd] = CPU->R[rs] & CPU->R[rt];
            break;
        case 1: // NOT
            CPU->R[rd] = -(CPU->R[rs]);
            break;
        case 2: // OR
            CPU->R[rd] = CPU->R[rs] | CPU->R[rt];
            break;
        case 3: // XOR
            CPU->R[rd] = CPU->R[rs] ^ CPU->R[rt];
            break;
        default: // AND IMM5
             CPU->R[rd] = CPU->R[rs] & imm5;
    }
        
    CPU->regInputVal = CPU->R[rd];
    
    SetNZP(CPU, (short) CPU->regInputVal);
    
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    CPU->DATA_WE = 0;
    
    WriteOut(CPU, output);
    CPU->PC = CPU->PC + 1;
}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState* CPU, FILE* output)
{
    unsigned short int instr = CPU->memory[CPU->PC];
    unsigned short int rs = INSN_8_6(instr);
    unsigned short int subop = INSN_11(instr);
    signed short int imm11 = INSN_10_0(instr);
    short int sign = ( (imm11) >> 10);
    if (sign == 1) {
        imm11 = ( 0xF800 | imm11 );
    }
    
    switch (subop) {
        case 0:
            CPU->PC = rs;
            break;
        case 1:
            CPU->PC = CPU->PC + 1 + imm11;
            break;
    }
        
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    
    CPU->regFile_WE = 0;
    CPU->NZP_WE = 0;
    CPU->DATA_WE = 0;
    
    WriteOut(CPU, output);
    CPU->PC = CPU->PC + 1;
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState* CPU, FILE* output)
{
    unsigned short int instr = CPU->memory[CPU->PC];
    unsigned short int rs = INSN_8_6(instr);
    unsigned short int subop = INSN_11(instr);
    signed short int imm11 = INSN_10_0(instr);
    short int sign = ( (imm11) >> 10);
    if (sign == 1) {
        imm11 = ( 0xF800 | imm11 );
    }
    
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 1;
    
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    CPU->DATA_WE = 0;
    
    switch (subop) {
        case 0:
            CPU->R[7] = CPU->PC + 1;
            CPU->regInputVal = CPU->R[7];
            SetNZP(CPU, CPU->regInputVal);
            WriteOut(CPU, output);
            // writeout then set new pc
            CPU->PC = rs;
            break;
        case 1:
            CPU->R[7] = CPU->PC + 1;
            CPU->regInputVal = CPU->R[7];
            SetNZP(CPU, CPU->regInputVal);
            WriteOut(CPU, output);
            // writeout then set new pc
            CPU->PC = ( (CPU->PC & 0x8000) | (imm11 << 4) );
            break;
    }  
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState* CPU, FILE* output)
{
    unsigned short int instr = CPU->memory[CPU->PC];
    unsigned short int rd = INSN_11_9(instr);
    unsigned short int rs = INSN_8_6(instr);
    unsigned short int rt = INSN_2_0(instr);
    unsigned short int subop = INSN_5_4(instr);
    unsigned short int uimm4 = INSN_3_0(instr);
    
    switch (subop) {
        case 0: // SLL
            CPU->R[rd] = CPU->R[rs] << uimm4;
            break;
        case 1: // SRA
            CPU->R[rd] = ((short) CPU->R[rs]) >> uimm4;
            break;
        case 2: // SRL
            CPU->R[rd] = CPU->R[rs] >> uimm4;
            break;
        case 3: // MOD
            CPU->R[rd] = CPU->R[rs] % CPU->R[rt];
            break;
    }
    
    CPU->regInputVal = CPU->R[rd];
    
    SetNZP(CPU, (short) CPU->regInputVal);
    
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    CPU->DATA_WE = 0;
    
    WriteOut(CPU, output);
    CPU->PC = CPU->PC + 1;
}

/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result)
{
    CPU->PSR = (CPU->PSR & 0xFFF8); // clear last three bits
    if (result > 0) {
        CPU->PSR = ( (CPU->PSR) | 0x0001);
        CPU->NZPVal = 1;
    } else if (result == 0) {
        CPU->PSR = ( (CPU->PSR) | 0x0002);
        CPU->NZPVal = 2;
    } else { // result < 0
        CPU->PSR = ( (CPU->PSR) | 0x0004);
        CPU->NZPVal = 4;
    }
}
