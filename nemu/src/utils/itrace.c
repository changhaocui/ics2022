#include <common.h>
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

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