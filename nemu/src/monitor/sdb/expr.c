/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */


#include <regex.h>

enum
{
  /* TODO: Add more token types */
  TK_NOTYPE = 256,TK_EQ = 255,TK_NEQ = 254,TK_AND = 253,TK_OR = 252,
  TK_HNUM = 251,TK_NUM = 250,TK_VAR = 257,TK_REG = 258,TK_DEREF = 259
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"\\*", '*'},		// *
  {"-", '-'},		// -
  {"/", '/'},		// /
  {"\\(", '('},
  {"\\)", ')'},

  {"!=", TK_NEQ},
  {"[0-9]+", TK_NUM},
  {"[a-z]\\w+",TK_VAR},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};



/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        
        tokens[nr_token].type  = rules[i].token_type;
        switch (rules[i].token_type) {

          case TK_NUM:
          case TK_REG:
          case TK_VAR:
            strncpy(tokens[nr_token].str,substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
        }
        nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}
bool check_parentheses(int p, int q) { //处理括号是否合法 是否是最外层括号
  bool flag = true;//设置标志位判断括号是否匹配
  int par = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type=='(') par++;
    else if (tokens[i].type==')') par--;
     printf("%d",par);
    if(par < 0) assert(0);
    else if (par == 0&&q != i) flag = false; // the leftest parenthese is matched
  }
  if(par != 0)
    assert(0);
  return flag;
}

int find_major(int p, int q) {
  int ret = -1, par = 0, op_type = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == TK_NUM || tokens[i].type == TK_REG || tokens[i].type == TK_DEREF ) {
      continue;
    }
    if (tokens[i].type == '(') { //括号内的符号不可能为主符号
      par++;
    } else if (tokens[i].type == ')') {
      par--;
    } else if (par == 0){
      int tmp_type = 0;
      printf("输出tokenstype的数值%d\n",tokens[i].type);
      switch (tokens[i].type) {
      case TK_DEREF: tmp_type = 0; break;
      case '*': case '/': tmp_type = 1; break;
      case '+': case '-': tmp_type = 2; break;
      default: assert(0);
      }
      if (tmp_type >= op_type) {
        op_type = tmp_type;
        ret = i;
      }
    }
  }
  if (par != 0) return -1;
  printf("输出主符号的字符串%c位置%d\n",tokens[ret].type,ret);
  return ret;
}



word_t eval(int p, int q, bool *ok) {
  printf("式子的起始点%d  式子的结束点%d \n",p,q);
  *ok = true;
  if (p > q) {
    *ok = false;
    return 0;
  } else if (p == q) {//根据不同类型返回值
    if (tokens[p].type != TK_NUM && tokens[p].type != TK_REG) {
      *ok = false;
      return 0;
    }else if(tokens[p].type == TK_NUM){
      printf("返回数字的值\n");
      word_t ret = strtol(tokens[p].str, NULL, 10);
      return ret;
    }else 
    {
      printf("返回寄存器的值\n");
      return isa_reg_str2val(tokens[p].str,ok);
    }
  } 
  else if (check_parentheses(p, q)) {
    printf("括号合法\n");
    return eval(p+1, q-1, ok);
  } else if (tokens[p].type == TK_DEREF){//特殊符号时进行处理
      return eval(p+1,q,ok); 
  }else {    
    printf("去掉括号后进入寻找主符号\n");
    int major = find_major(p, q);
    if (major < 0) {
      *ok = false;
      return 0;
    }

    word_t val1 = eval(p, major-1, ok);
    if (!*ok) return 0;
    word_t val2 = eval(major+1, q, ok);
    if (!*ok) return 0;
    
    switch(tokens[major].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': if (val2 == 0) {
        *ok = false;
        return 0;
      } 
      return (sword_t)val1 / (sword_t)val2; // e.g. -1/2, may not pass the expr test
      default: assert(0);
    }
  }
}



word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for (int i = 0; i < nr_token; i ++) {
    if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_REG) )) {
      printf("将*号转变为解引用%d\n",i);//如果有多个*-+例如 开头写成 if(tokens[i].type == '-')
      tokens[i].type = TK_DEREF ;
      printf("将deref的位置后面%d位转变为寄存器\n",i+1);     
      tokens[i + 1].type = TK_REG;
    }
  }
   return eval(0, nr_token-1, success);
}
