#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h> 
#include "CPU.h"
#include "cache.h"

int main(int argc, char **argv)
{
	
	unsigned int I_accesses = 0;
	unsigned int I_misses = 0;
	unsigned int D_read_accesses = 0;
	unsigned int D_read_misses = 0;
	unsigned int D_write_accesses = 0; 
	unsigned int D_write_misses = 0;
	int counter = 0;
	int penalty = 0;
	int dontUpdate = 0;
	int test = 0;
	struct trace_item *tr_entry;
	struct trace_item IF;
	struct trace_item ID;
	struct trace_item EX;
	struct trace_item MEM;
	struct trace_item WB;
	struct trace_item SQUASHED;
	SQUASHED.type = 7;
	int endCounter = 0;
	int noOp;
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
  if (argc == 4) trace_view_on = atoi(argv[3]);
  
  //read in from cache_config.txt
  unsigned int I_size = 16; 
  unsigned int I_assoc = 4;
  unsigned int I_bsize = 8; 
  unsigned int D_size = 16;
  unsigned int D_assoc = 4;
  unsigned int D_bsize = 8;
  unsigned int mem_latency = 20;
  
  	FILE* file = fopen ("cache_config.txt", "r");
  	if (file != NULL) {
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
		if((EX.type == 3) && ((EX.dReg == ID.sReg_a) || (EX.dReg == ID.sReg_b)))//correct
		{
			//stall
			//bubble IF and ID 
			IF = ID;
			noOp = 1;
		}
		
		// Branch Prediction
		else if(IF.type == 5)
		{
			// Branch Was Taken
			if(IF.PC - tr_entry->PC != 4)//det branch 
			{
				//branch, set IF = SQUASHED for two cycles, continue to propogate
				counter = 1;				
			}
			else
			{
				size = trace_get_item(&tr_entry);
				penalty = cache_access(I_cache, tr_entry->Addr, 0);
				cycle_number = cycle_number + penalty;		
				I_accesses++;
					if(penalty != 0){
						I_misses++;
					}	
				}	
		}
		else
		{
			if(counter == 0){		
				size = trace_get_item(&tr_entry);
				penalty = cache_access(I_cache, tr_entry->Addr, 0);
				cycle_number = cycle_number + penalty;
				I_accesses++;
					if(penalty != 0){
						I_misses++;
					}	
			}
			test = 0;
		}
		// Cascade States
		WB = MEM;
		MEM = EX;
		EX = ID; 
		ID = IF;
		
		if(noOp ==1 ){
			EX.type = 0;
			noOp = 0;
		}
		if(counter > 0){//if counter = 1
			IF = SQUASHED;
			counter++;
			if(counter == 3){//after IF has been set to SQUASHED 2 times, will equal 3, reset counter.
				counter = 0;
			}
		}
		else{
			IF = *tr_entry;
			}
		cycle_number++;//good
		

	// Print Executed Instructions (trace_view_on=1)
		if((!size) && endCounter == 0){
			endCounter = 1;
		}
		if(endCounter > 0){
			endCounter++;
		}
		if(endCounter == 5){
			printf("+ Simulation terminates at cycle : %u\n", cycle_number);
			printf("I-cache accesses %u and misses %u\n", I_accesses, I_misses);
			printf("D-cache Read accesses %u and misses %u\n", D_read_accesses, D_read_misses);
			printf("D-cache Write accesses %u and misses %u\n", D_write_accesses, D_write_misses);
			break;
		}
		
		switch(WB.type)
		{
		   case ti_NOP:
			  if (trace_view_on) printf("[cycle %d] NOP:\n", cycle_number);
			  break;
			case ti_RTYPE:
			  if (trace_view_on) {
				printf("[cycle %d] RTYPE:", cycle_number);
				printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", WB.PC, WB.sReg_a, WB.sReg_b, WB.dReg);
			  }
			  break;
			case ti_ITYPE:
			  if (trace_view_on){
				printf("[cycle %d] ITYPE:", cycle_number);
				printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", WB.PC, WB.sReg_a, WB.dReg, WB.Addr);
			  }
			  break;
			case ti_LOAD:
			  if (trace_view_on){
				printf("[cycle %d] LOAD:", cycle_number);
				printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", WB.PC, WB.sReg_a, WB.dReg, WB.Addr);
			  }
			  penalty = cache_access(D_cache, WB.Addr, 0);
			  cycle_number = cycle_number + penalty;
			  D_read_accesses++;
			  if(penalty != 0){
			  	D_read_misses++;
			  }
			  	
			  // update D_read_access and D_read_misses
			  break;
			case ti_STORE:
			  if (trace_view_on){
				printf("[cycle %d] STORE:", cycle_number);
				printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", WB.PC, WB.sReg_a, WB.sReg_b, WB.Addr);
			  }
			  D_write_accesses++;
			  penalty = cache_access(D_cache, WB.Addr, 1);
			  cycle_number = cycle_number + penalty;
			  
			  if(penalty != 0){
			  	D_write_misses++;
			  }
			  // update D_write_access and D_write_misses
			  break;
			case ti_BRANCH:
			  if (trace_view_on) {
				printf("[cycle %d] BRANCH:", cycle_number);
				printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", WB.PC, WB.sReg_a, WB.sReg_b, WB.Addr);
			  }
			  break;
			case ti_JTYPE:
			  if (trace_view_on) {
				printf("[cycle %d] JTYPE:", cycle_number);
				printf(" (PC: %x)(addr: %x)\n", WB.PC, WB.Addr);
			  }
			  break;
			case ti_SPECIAL:
			  if (trace_view_on) printf("[cycle %d] SQUASHED!\n", cycle_number);
			  break;
			case ti_JRTYPE:
			  if (trace_view_on) {
				printf("[cycle %d] JRTYPE:", cycle_number);
				printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", WB.PC, WB.dReg, WB.Addr);
			  }
			  break;
		}
	
}
	trace_uninit();
	exit(0);
	
}