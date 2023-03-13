#include <common.h>
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

#define MAX_IRINGBUF 16


Decode* iringbuf[MAX_IRINGBUF];
int p_cur = 0;

void ringtrace_inst(Decode* s) {
  iringbuf[p_cur] = s;
  p_cur = (p_cur + 1) % MAX_IRINGBUF;
}


void rtrace_display(){
  int error = p_cur;
  while((error+1)%MAX_IRINGBUF != p_cur){
    char *p = iringbuf[error]->logbuf;
    if(error == p_cur)
      p += snprintf(p, sizeof(iringbuf[error]->logbuf), "->  " FMT_WORD ":", iringbuf[error]->pc);//snprintf将命令记录到开辟的p中
    else
      p += snprintf(p, sizeof(iringbuf[error]->logbuf), "    " FMT_WORD ":", iringbuf[error]->pc);
    int ilen = iringbuf[error]->dnpc - iringbuf[error]->pc;
    int i;
    uint8_t *inst = (uint8_t *)&iringbuf[error]->isa.inst.val;
    for (i = ilen - 1; i >= 0; i --) {
     p += snprintf(p, 4, " %02x", inst[i]);
    }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, iringbuf[error]->logbuf + sizeof(iringbuf[error]->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, iringbuf[error]->snpc, iringbuf[error]->pc), (uint8_t *)&iringbuf[error]->isa.inst.val, ilen);
  }
}


// /*log 的实现*/

// typedef struct {
// 	char name[32]; // func name, 32 should be enough
// 	paddr_t addr;
// 	unsigned char info;
// 	Elf64_Xword size;
// } SymEntry;

// SymEntry *symbol_tbl = NULL; // dynamic allocated
// int symbol_tbl_size = 0;
// int call_depth = 0;

// typedef struct tail_rec_node {
// 	paddr_t pc;
// 	paddr_t depend;
// 	struct tail_rec_node *next;
// } TailRecNode;
// TailRecNode *tail_rec_head = NULL; // linklist with head, dynamic allocated

// static void read_elf_header(int fd, Elf64_Ehdr *eh) {
// 	assert(lseek(fd, 0, SEEK_SET) == 0);
//   assert(read(fd, (void *)eh, sizeof(Elf64_Ehdr)) == sizeof(Elf64_Ehdr));

// 	  // check if is elf using fixed format of Magic: 7f 45 4c 46 ...
//   if(strncmp((char*)eh->e_ident, "\177ELF", 4)) {
// 		panic("malformed ELF file");
// 	}
// }



// static void read_section(int fd, Elf64_Shdr sh, void *dst) {
// 	assert(dst != NULL);
// 	assert(lseek(fd, (off_t)sh.sh_offset, SEEK_SET) == (off_t)sh.sh_offset);
// 	assert(read(fd, dst, sh.sh_size) == sh.sh_size);
// }

// static void read_section_headers(int fd, Elf64_Ehdr eh, Elf64_Shdr *sh_tbl) {
// 	assert(lseek(fd, eh.e_shoff, SEEK_SET) == eh.e_shoff);
// 	for(int i = 0; i < eh.e_shnum; i++) {
// 		assert(read(fd, (void *)&sh_tbl[i], eh.e_shentsize) == eh.e_shentsize);
// 	}
// }



// static void read_symbol_table(int fd, Elf64_Ehdr eh, Elf64_Shdr sh_tbl[], int sym_idx) {
//   Elf64_Sym sym_tbl[sh_tbl[sym_idx].sh_size];
//   read_section(fd, sh_tbl[sym_idx], sym_tbl);
  
//   int str_idx = sh_tbl[sym_idx].sh_link;
//   char str_tbl[sh_tbl[str_idx].sh_size];
//   read_section(fd, sh_tbl[str_idx], str_tbl);
  
//   int sym_count = (sh_tbl[sym_idx].sh_size / sizeof(Elf64_Sym));
// 	// log
//   log_write("Symbol count: %d\n", sym_count);
//   log_write("====================================================\n");
//   log_write(" num    value            type size       name\n");
//   log_write("====================================================\n");
//   for (int i = 0; i < sym_count; i++) {
//     log_write(" %-3d    %016lx %-4d %-10ld %s\n",
//       i,
//       sym_tbl[i].st_value, 
//       ELF64_ST_TYPE(sym_tbl[i].st_info),
// 			sym_tbl[i].st_size,
//       str_tbl + sym_tbl[i].st_name
//     );
//   }
//   log_write("====================================================\n\n");

// 	// read
// 	symbol_tbl_size = sym_count;
// 	symbol_tbl = malloc(sizeof(SymEntry) * sym_count);
//   for (int i = 0; i < sym_count; i++) {
//     symbol_tbl[i].addr = sym_tbl[i].st_value;
// 		symbol_tbl[i].info = sym_tbl[i].st_info;
// 		symbol_tbl[i].size = sym_tbl[i].st_size;
// 		memset(symbol_tbl[i].name, 0, 32);
// 		strncpy(symbol_tbl[i].name, str_tbl + sym_tbl[i].st_name, 31);
//   }
// }

// static void read_symbols(int fd, Elf64_Ehdr eh, Elf64_Shdr sh_tbl[]) {
//   for (int i = 0; i < eh.e_shnum; i++) {
// 		switch (sh_tbl[i].sh_type) {
// 		case SHT_SYMTAB: case SHT_DYNSYM:
// 			read_symbol_table(fd, eh, sh_tbl, i); break;
// 		}
//   }
// }

// static void init_tail_rec_list() {
// 	tail_rec_head = (TailRecNode *)malloc(sizeof(TailRecNode));
// 	tail_rec_head->pc = 0;
// 	tail_rec_head->next = NULL;
// }

// /* ELF64 as default */
// void init_elf(const char *elf_file) {
//   if (elf_file == NULL) return;
  
// 	Log("specified ELF file: %s", elf_file);
//   int fd = open(elf_file, O_RDONLY|O_SYNC);
//   Assert(fd >= 0, "Error %d: unable to open %s\n", fd, elf_file);
  
//   Elf64_Ehdr eh;
// 	read_elf_header(fd, &eh);

//   Elf64_Shdr sh_tbl[eh.e_shentsize * eh.e_shnum];
// 	read_section_headers(fd, eh, sh_tbl);

//   read_symbols(fd, eh, sh_tbl);

// 	init_tail_rec_list();

// 	close(fd);
// }

// static int find_symbol_func(paddr_t target, bool is_call) {
// 	int i;
// 	for (i = 0; i < symbol_tbl_size; i++) {
// 		if (ELF64_ST_TYPE(symbol_tbl[i].info) == STT_FUNC) {
// 			if (is_call) {
// 				if (symbol_tbl[i].addr == target) break;
// 			} else {
// 				if (symbol_tbl[i].addr <= target && target < symbol_tbl[i].addr + symbol_tbl[i].size) break;
// 			}
// 		}
// 	}
// 	return i<symbol_tbl_size?i:-1;
// }

// // static void insert_tail_rec(paddr_t pc, paddr_t depend) {
// // 	TailRecNode *node = (TailRecNode *)malloc(sizeof(TailRecNode));
// // 	node->pc = pc;
// // 	node->depend = depend;
// // 	node->next = tail_rec_head->next;
// // 	tail_rec_head->next = node;
// // }

// // static void remove_tail_rec() {
// // 	TailRecNode *node = tail_rec_head->next;
// // 	tail_rec_head->next = node->next;
// // 	free(node);
// // }

void trace_func_call(paddr_t pc, paddr_t target, bool is_tail) {
	// if (symbol_tbl == NULL) return;

	// ++call_depth;

	// if (call_depth <= 2) return; // ignore _trm_init & main

	// int i = find_symbol_func(target, true);
	// log_write(FMT_PADDR ": %*scall [%s@" FMT_PADDR "]\n",
	// 	pc,
	// 	(call_depth-3)*2, "",
	// 	i>=0?symbol_tbl[i].name:"???",
	// 	target
	// );

	// if (is_tail) {
	// 	insert_tail_rec(pc, target);
	// }
}

void trace_func_ret(paddr_t pc) {
	// if (symbol_tbl == NULL) return;
	
	// if (call_depth <= 2) return; // ignore _trm_init & main

	// int i = find_symbol_func(pc, false);
	// log_write(FMT_PADDR ": %*sret [%s]\n",
	// 	pc,
	// 	(call_depth-3)*2, "",
	// 	i>=0?symbol_tbl[i].name:"???"
	// );
	
	// --call_depth;

	// TailRecNode *node = tail_rec_head->next;
	// if (node != NULL) {
	// 	int depend_i = find_symbol_func(node->depend, true);
	// 	if (depend_i == i) {
	// 		paddr_t ret_target = node->pc;
	// 		// remove_tail_rec();
	// 		trace_func_ret(ret_target);
	// 	}
	// }
}
