/*
 * trace.c: location of main() to start the simulator
 */

#include "loader.h"

int main(int argc, char** argv)
{
    // this assignment does NOT require dynamic memory - do not use 'malloc() or free()'
    // create a local varible in main to represent the CPU and then pass it around
    // to subsequent functions using a pointer!
    
    /* notice: there are test cases for part 1 & part 2 here on codio
       they are located under folders called: p1_test_cases & p2_test_cases
    
       once you code main() to open up files, you can access the test cases by typing:
       ./trace p1_test_cases/divide.obj   - divide.obj is just an example
    
       please note part 1 test case files have an OBJ and a TXT file
       the .TXT shows you what's in the .OBJ files
    
       part 2 test case files have an OBJ, an ASM, and a TXT files
       the .OBJ is what you must read in
       the .ASM is the original assembly file that the OBJ was produced from
       the .TXT file is the expected output of your simulator for the given OBJ file
    */
    
    FILE* output_file;
    MachineState* CPU;
    int i;
    int update_error;
    
    // allocate memory to CPU
    CPU = malloc(sizeof(MachineState));
    
    // reset machine state
    Reset(CPU);
    ClearSignals(CPU);
    
    // check if correct number of args
    if (argc > 2) { 
        int i;
        for (i = 0; i < argc; i++) {
            int fn_len = strlen(argv[i]);
            if (i == 1) {
                // check if .txt file
                if (argv[i][fn_len - 4] != '.' || argv[i][fn_len - 3] != 't' || \
                    argv[i][fn_len - 2] != 'x' || argv[i][fn_len - 1] != 't') {
                        fprintf(stderr, "error: main() failed...first arg must be -.txt\n");
                        return -1;
                }
                // open file to write
                output_file = fopen(argv[i], "w");
            } else if (i == 0) {
                continue;
            } else {
                // check if .obj file
                if (argv[i][fn_len - 4] != '.' || argv[i][fn_len - 3] != 'o' || \
                    argv[i][fn_len - 2] != 'b' || argv[i][fn_len - 1] != 'j') {
                        fprintf(stderr, "error: main() failed...args 2+ must be -.obj\n");
                        return -1;
                }
            }
            printf("FILE: \"%s\" properly synced\n", argv[i]);
        }
    } else {
        fprintf(stderr, "error: usage: ./trace <output_filename.txt> <obj_file.obj> ...\n");
        return -1;
    }
    
    // read in all object files
    for (i = 2; i < argc; i++) {
        char* filename = argv[i];
        int read_obj_file_error = ReadObjectFile(filename, CPU);
        if (read_obj_file_error != 0) {
            return read_obj_file_error;
        }
    }
    
    // update output_file PT1
//     for (i = 0; i < 65536; i++) {
//         char buffer[50];
//         sprintf(buffer, "address: %05d contents: 0x%04X\n", i, CPU->memory[i]);
//         fputs(buffer, output_file);
//     }
    
    // update output_file PT2
    while (CPU->PC != 0x80FF) {
        update_error = UpdateMachineState(CPU, output_file);
        if (update_error != 0) {
            fclose(output_file);
            return update_error;
        }
    }
    
    printf("output_file \"%s\" updated!\n", argv[1]);
    
    /* close file */
    fclose(output_file);
    
    /* free dynamic memory */
    free(CPU);

    return 0;
}