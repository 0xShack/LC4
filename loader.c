/*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include "loader.h"

// memory array location
unsigned short memoryAddress;

/*
 * Read an object file and modify the machine state as described in the writeup
 */
int ReadObjectFile(char* filename, MachineState* CPU) {
    // check that filename is .obj
    int fn_len = strlen(filename);
    if (filename[fn_len - 4] != '.' || filename[fn_len - 3] != 'o' || \
        filename[fn_len - 2] != 'b' || filename[fn_len - 1] != 'j') {
        fprintf(stderr, "error: ReadObjectFile() failed...filename must be -.obj\n");
        return 1;
    }
    // open object file
    FILE *src_file = fopen(filename, "rb");
    printf("FILE \"%s\" opened...\n", filename);
    // check if null
    if (src_file == NULL) {
        fprintf(stderr, "error: ReadObjectFile() failed...file is null\n");
        return 1;
    }
    /* unsigned short int memory[65536]; */
    // initialize memory
    while (1) {
        int i;       
        unsigned short int instr;
        unsigned short int addr; // memory address
        unsigned short int n; // number of words
        unsigned short int byte1 = fgetc(src_file);
        if (feof(src_file)) {
            break;
        }
        unsigned short int byte2 = fgetc(src_file);
        /* Code and Data headers */
        if ( (byte1 == 0xCA && byte2 == 0xDE) ||
             (byte1 == 0xDA && byte2 == 0xDA) ) {
            printf("loading section: 0x%X%X ----------------\n", byte1, byte2);
            // find starting address
            byte1 = fgetc(src_file);
            byte2 = fgetc(src_file);
            // set address for first line of code
            addr = ( (byte1 << 8) | byte2 );
            printf("code address: 0x%05X\n", addr);
            // find number of words
            byte1 = fgetc(src_file);
            byte2 = fgetc(src_file);
            // set word count
            n = ( (byte1 << 8) | byte2 );
            printf("word count: 0x%05X\n", n);
            // load code into memory
            for (i = addr; i < (addr + n); i++) {
                byte1 = fgetc(src_file);
                byte2 = fgetc(src_file);
                if (byte1 == 0xFFFF || byte2 == 0xFFFF) {
                    fprintf(stderr, "error: ReadObjectFile() failed...bytes are invalid\n");
                    return 1;
                }
                instr = ( (byte1 << 8) | byte2 );
                CPU->memory[i] = instr;
                //printf("address: 0x%05d contents: 0x%X\n", i, instr);
            }
            printf("---------------- Code/Data processed!\n");
        } 
        /* Symbol header */
        else if (byte1 == 0xC3 && byte2 == 0xB7) { 
            printf("loading section: 0x%X%X ----------------\n", byte1, byte2);
            // find starting address
            byte1 = fgetc(src_file);
            byte2 = fgetc(src_file);
            // set address for first line of code
            addr = ( (byte1 << 8) | byte2 );
            printf("code address: 0x%05X\n", addr);
            // find number of words
            byte1 = fgetc(src_file);
            byte2 = fgetc(src_file);
            // set word count
            n = ( (byte1 << 8) | byte2 );
            printf("word count: 0x%05d\n", n);
            // check for errors in symbol
            for (i = 0; i < n; i++) {
                byte1 = fgetc(src_file);
                if (byte1 == 0xFFFF) {
                    fprintf(stderr, "error: ReadObjectFile() failed...byte is invalid\n");
                    return 1;
                }
                //CPU->memory[i] = byte1;
                //printf("address: 0x%X contents: 0x%X\n", i, instr);
            }
            printf("---------------- Symbol processed!\n");
        } 
        /* File name header */
        else if (byte1 == 0xF1 && byte2 == 0x7E) {
            printf("loading section: 0x%X%X ----------------\n", byte1, byte2);
            // find number of words
            byte1 = fgetc(src_file);
            byte2 = fgetc(src_file);
            // set word count
            n = ( (byte1 << 8) | byte2 );
            printf("word count: 0x%05d\n", n);
            // check for errors in file name
            for (i = 0; i < n; i++) {
                byte1 = fgetc(src_file);
                if (byte1 == 0xFFFF) {
                    fprintf(stderr, "error: ReadObjectFile() failed...byte is invalid\n");
                    fclose(src_file);
                    return 1;
                }
                //CPU->memory[i] = byte1;
                //printf("address: 0x%X contents: 0x%X\n", i, instr);
            }
            printf("---------------- File name processed!\n");
        } 
        /* TODO: Line number header */
        else if (byte1 == 0x71 && byte2 == 0x5E) {
            printf("loading section: 0x%X%X\n", byte1, byte2);
            byte1 = fgetc(src_file);
            byte2 = fgetc(src_file);
            if (byte1 == 0xFFFF || byte2 == 0xFFFF) {
                fprintf(stderr, "error: ReadObjectFile() failed...byte is invalid\n");
                fclose(src_file);
                return 1;
            }
            n = ( (byte1 << 8) | byte2 );
            for (i = 0; i < n; i++) {
                byte1 = fgetc(src_file);
                if (byte1 == 0xFFFF) {
                    fprintf(stderr, "error: ReadObjectFile() failed...byte is invalid\n");
                    fclose(src_file);
                    return 1;
                }
            }
            
        } 
        /* error */
        else {
            fprintf(stderr, "error: ReadObjectFile() failed...invalid header\n");
            return 1;
        }
    }
    
    printf("-?-!- closing FILE \"%s\" -!-?-\n", filename);
    
    fclose(src_file);
    
    return 0;
}
