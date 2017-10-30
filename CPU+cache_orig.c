#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "CPU.h" 

// to keep cache statistics
unsigned int I_accesses = 0;
unsigned int I_misses = 0;
unsigned int D_read_accesses = 0;
unsigned int D_read_misses = 0;
unsigned int D_write_accesses = 0; 
unsigned int D_write_misses = 0;

#include "cache.h" 

int main(int argc, char **argv)
{
  struct trace_item *tr_entry;
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

  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - any character>\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }
    
  trace_file_name = argv[1];
  if (argc == 3) trace_view_on = atoi(argv[2]) ;
  // here you should extract the cache parameters from the configuration file (cache size, associativity, latency)
  unsigned int I_size = 16; 
  unsigned int I_assoc = 4;
  unsigned int I_bsize = 8; 
  unsigned int D_size = 16;
  unsigned int D_assoc = 4;
  unsigned int D_bsize = 8;
  unsigned int mem_latency = 20;

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  trace_init();
  struct cache_t *I_cache, *D_cache;
  I_cache = cache_create(I_size, I_bsize, I_assoc, mem_latency); 
  D_cache = cache_create(D_size, D_bsize, D_assoc, mem_latency);

  while(1) {
    size = trace_get_item(&tr_entry);
   
    if (!size) {  /* no more instructions (trace_items) to simulate */
      printf("+ Simulation terminates at cycle : %u\n", cycle_number);
      printf("I-cache accesses %u and misses %u\n", I_accesses, I_misses);
      printf("D-cache Read accesses %u and misses %u\n", D_read_accesses, D_read_misses);
      printf("D-cache Write accesses %u and misses %u\n", D_write_accesses, D_write_misses);
      break ;
    }
    else{              /* parse the next instruction to simulate */
      cycle_number++;
      t_type = tr_entry->type;
      t_sReg_a = tr_entry->sReg_a;
      t_sReg_b = tr_entry->sReg_b;
      t_dReg = tr_entry->dReg;
      t_PC = tr_entry->PC;
      t_Addr = tr_entry->Addr;
    }  

// SIMULATION OF A SINGLE CYCLE cpu IS TRIVIAL - EACH INSTRUCTION IS EXECUTED
// IN ONE CYCLE, EXCEPT IF THERE IS A CACHE MISS.

	cycle_number = cycle_number + cache_access(I_cache, tr_entry->PC, 0); /* simulate instruction fetch */
	// update I_access and I_misses

      switch(tr_entry->type) {
        case ti_NOP:
		  if (trace_view_on) printf("[cycle %d] NOP:", cycle_number);
          break;
        case ti_RTYPE:
		  if (trace_view_on) {
			printf("[cycle %d] RTYPE:", cycle_number);
			printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->dReg);
		  };
          break;
        case ti_ITYPE:
		  if (trace_view_on){
			printf("[cycle %d] ITYPE:", cycle_number);
			printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
		  };
          break;
        case ti_LOAD:
		  if (trace_view_on){
			printf("[cycle %d] LOAD:", cycle_number);
			printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
		  };
		  cycle_number = cycle_number + cache_access(D_cache, tr_entry->Addr, 0);
		  // update D_read_access and D_read_misses
		  break;
        case ti_STORE:
		  if (trace_view_on){
			printf("[cycle %d] STORE:", cycle_number);
			printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
		  };
		  cycle_number = cycle_number + cache_access(D_cache, tr_entry->Addr, 1);
		  // update D_write_access and D_write_misses
		  break;
        case ti_BRANCH:
		  if (trace_view_on) {
			printf("[cycle %d] BRANCH:", cycle_number);
			printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
		  };
          break;
        case ti_JTYPE:
		  if (trace_view_on) {
			printf("[cycle %d] JTYPE:", cycle_number);
			printf(" (PC: %x)(addr: %x)\n", tr_entry->PC, tr_entry->Addr);
		  };
          break;
        case ti_SPECIAL:
		  if (trace_view_on) printf("[cycle %d] SPECIAL:", cycle_number);
          break;
        case ti_JRTYPE:
		  if (trace_view_on) {
			printf("[cycle %d] JRTYPE:", cycle_number);
			printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", tr_entry->PC, tr_entry->dReg, tr_entry->Addr);
		  };
          break;
      }
   
  }

  trace_uninit();

  exit(0);
}
