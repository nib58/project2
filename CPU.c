#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h> 
#include "CPU.h"
#include "cache.h" 

void trace_view(struct trace_item stage, int cycle_number)
{
	switch(stage.type)
	{
       case ti_NOP:
		  if (trace_view_on) printf("[cycle %d] NOP:", cycle_number);
          break;
        case ti_RTYPE:
		  if (trace_view_on) {
			printf("[cycle %d] RTYPE:", cycle_number);
			printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", stage.type->PC, stage.type->sReg_a, stage.type->sReg_b, stage.type->dReg);
		  };
          break;
        case ti_ITYPE:
		  if (trace_view_on){
			printf("[cycle %d] ITYPE:", cycle_number);
			printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", stage.type->PC, stage.type->sReg_a, stage.type->dReg, stage.type->Addr);
		  };
          break;
        case ti_LOAD:
		  if (trace_view_on){
			printf("[cycle %d] LOAD:", cycle_number);
			printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", stage.type->PC, stage.type->sReg_a, stage.type->dReg, stage.type->Addr);
		  };
		  cycle_number = cycle_number + cache_access(D_cache, stage.type->Addr, 0);
		  // update D_read_access and D_read_misses
		  break;
        case ti_STORE:
		  if (trace_view_on){
			printf("[cycle %d] STORE:", cycle_number);
			printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", stage.type->PC, stage.type->sReg_a, stage.type->sReg_b, stage.type->Addr);
		  };
		  cycle_number = cycle_number + cache_access(D_cache, stage.type->Addr, 1);
		  // update D_write_access and D_write_misses
		  break;
        case ti_BRANCH:
		  if (trace_view_on) {
			printf("[cycle %d] BRANCH:", cycle_number);
			printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", stage.type->PC, stage.type->sReg_a, stage.type->sReg_b, stage.type->Addr);
		  };
          break;
        case ti_JTYPE:
		  if (trace_view_on) {
			printf("[cycle %d] JTYPE:", cycle_number);
			printf(" (PC: %x)(addr: %x)\n", stage.type->PC, stage.type->Addr);
		  };
          break;
        case ti_SPECIAL:
		  if (trace_view_on) printf("[cycle %d] SQUASHED:", cycle_number);
          break;
        case ti_JRTYPE:
		  if (trace_view_on) {
			printf("[cycle %d] JRTYPE:", cycle_number);
			printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", stage.type->PC, stage.type->dReg, stage.type->Addr);
		  };
          break;
    }
}

int main(int argc, char **argv)
{
	struct trace_item *tr_entry;
	struct trace_item IF;
	struct trace_item ID;
	struct trace_item EX;
	struct trace_item MEM;
	struct trace_item WB;
	
	size_t size;
	char *trace_file_name;
	int trace_view_on = 0;

	unsigned char t_type = 0;
	unsigned char t_sReg_a= 0;
	unsigned char t_sReg_b= 0;
	unsigned char t_dReg= 0;
	unsigned int t_PC = 0;
	unsigned int t_Addr = 0;
	unsigned int cycle_number = 0;


// Handling Inputs
  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - any character>\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }
    
  trace_file_name = argv[1];
  if (argc == 3) trace_view_on = atoi(argv[2]);
  
  //read in from cache_config.txt
  unsigned int I_size = 16;
  unsigned int I_assoc = 4;
  unsigned int I_bsize = 8; 
  unsigned int D_size = 16;
  unsigned int D_assoc = 4;
  unsigned int D_bsize = 8;
  unsigned int mem_latency = 20;
  
  	FILE* file = fopen ("cache_config.txt", "r");
  	if (f != NULL) {
		fscanf(file,"%u %u %u %u %u %u %u",&I_size,&I_assoc, &I_bsize, &D_size, &D_assoc, &D_bsize, &mem_latency);  
		fclose (file);  
	}
	else{
		printf("cannot open cache_config.txt. Using default values for cache /n");
	}
	
	// Checking File
	fprintf(stdout, "\n ** opening file %s\n", trace_file_name);
	trace_fd = fopen(trace_file_name, "rb");
	if (!trace_fd)
	{
		fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
		exit(0);
	}

	trace_init();
	struct cache_t *I_cache, *D_cache;
	I_cache = cache_create(I_size, I_bsize, I_assoc, mem_latency); 
	D_cache = cache_create(D_size, D_bsize, D_assoc, mem_latency);

	
	// Start Processes
	while(1)
	{
		// EX Processing
		// Data Hazard
		if((EX.type == 3) && (EX.dReg == ID.sReg_a || EX.dReg == ID.sReg_b))
		{
			//stall
			
			EX.type = 0;
			//bubble IF and ID 
			*tr_entry = IF;
			IF = ID;			
			
		}
		
		// Branch Prediction
		else if(EX.type == 5)
		{
			// Branch Was Taken
			if(ID.PC - EX.PC != 4)
			{
					IF.type = 7;
					ID.type = 7;
					size = trace_get_item(&tr_entry);
					if(!size){
						printf("+ Simulation terminates at cycle : %u\n", cycle_number);
     					printf("I-cache accesses %u and misses %u\n", I_accesses, I_misses);
					  	printf("D-cache Read accesses %u and misses %u\n", D_read_accesses, D_read_misses);
					  	printf("D-cache Write accesses %u and misses %u\n", D_write_accesses, D_write_misses);
					 	break;
					}						
			}
			else
			{
				size = trace_get_item(&tr_entry);
				if(!size){
					printf("+ Simulation terminates at cycle : %u\n", cycle_number);
					printf("I-cache accesses %u and misses %u\n", I_accesses, I_misses);
					printf("D-cache Read accesses %u and misses %u\n", D_read_accesses, D_read_misses);
					printf("D-cache Write accesses %u and misses %u\n", D_write_accesses, D_write_misses);
					break;
				}
			}	
		}
		else
		{
			size = trace_get_item(&tr_entry);
			if(!size){
				printf("+ Simulation terminates at cycle : %u\n", cycle_number);
				printf("I-cache accesses %u and misses %u\n", I_accesses, I_misses);
				printf("D-cache Read accesses %u and misses %u\n", D_read_accesses, D_read_misses);
				printf("D-cache Write accesses %u and misses %u\n", D_write_accesses, D_write_misses);
				break;
				break;
			}
		}
		// Cascade States

		WB = MEM;
		MEM = EX;
		EX = ID; 
		ID = IF;
		IF = *tr_entry;
		cycle_number++;

		// Print Executed Instructions (trace_view_on=1)
		if (trace_view_on)
		{
			trace_view(WB, cycle_number);
		}
	}

	trace_uninit();
	exit(0);
}