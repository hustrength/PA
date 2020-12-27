#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

uint32_t eval(int p, int q, bool *success);

enum {
    TK_NOTYPE = 256, TK_EQ, TK_UNEQ

    /* TODO: Add more token types */
    /* PA1.2*/
    TK_DECI, TK_PLUS, TK_SUB, TK_MUL, TK_DIV,
    TK_LBR, TK_RBR, TK_HEX, TK_REG, TK_AND, TK_OR, TK_DEREF, TK_NEG
};

static struct rule {
    char *regex;
    int token_type;
    int priority; // add priority
} rules[] = {

        /* TODO: Add more rules.
         * Pay attention to the precedence level of different rules.
         */

        /* PA1.2 */
        /* rule, symbole, priority */
        /* The smaller value of priority means the greater priority */
        /* The priority of TK_NEG and TK_DEREF is 1 */
        {" +",                                                         TK_NOTYPE, 7},       // spaces
        {"\\+",                                                        TK_PLUS,   3},       // plus
        {"==",                                                         TK_EQ,     4},       // equal
        {"!=",                                                         TK_UNEQ,   4},       // unequal
        {"-",                                                          TK_SUB,    3},       // subtract
        {"\\*",                                                        TK_MUL,    2},       // multiply or derefrence
        {"/",                                                          TK_DIV,    2},       // divide
        {"0[Xx][0-9a-fA-F]+",                                          TK_HEX,    7},       // hex (must before the TK_DECI)
        {"[0-9]+",                                                     TK_DECI,   7},       // number(dec)
        {"\\(",                                                        TK_LBR,    0},       // left bracket
        {"\\)",                                                        TK_RBR,    0},       // right bracket
        {"\\$(E?[A-DS][XPI])|([A-D][HL])|(e?[a-ds][xpi])|([a-d][hl])", TK_REG,    7},       // register
        {"&&",                                                         TK_AND,    5},       // and
        {"\\|\\|",                                                     TK_OR,     6}        // or
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);    // comply the regex pattern
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
        }
    }
}

typedef struct token {
    int type;
    char str[32];
    int priority; //add priority
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
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

                /* PA1.2 */
                if (substr_len > 32) {
                    Log("!!!Error: The length of the substring is longer than 32 bits");
                    assert(0);
                }
                switch (rules[i].token_type) {
                    case TK_NOTYPE:
                        break;
                    case TK_DECI:
                    case TK_HEX:
                    case TK_REG:
                        strncpy(tokens[nr_token].str, substr_start, substr_len);
                        tokens[nr_token].str[substr_len] = '\0';
                    default:
                        tokens[nr_token].type = rules[i].token_type;
                        tokens[nr_token].priority = rules[i].priority;
                        nr_token++;
                }
                /* Match some regex successfully. Jump out of the FOR loop. */
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

uint32_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    /* TODO: Insert codes to evaluate the expression. */
    /* PA1.2 */

    /* Judge whether the token is NEG or DEREF */
    for (int i = 0; i < nr_token; i++) {
        if (tokens[i].type == TK_MUL &&
            (i == 0 || (tokens[i - 1].priority != 7 && tokens[i - 1].type != TK_RBR))) {
            tokens[i].type = TK_DEREF;
            tokens[i].priority = 1;
        } else if (tokens[i].type == TK_SUB &&
                   (i == 0 || (tokens[i - 1].priority != 7 && tokens[i - 1].type != TK_RBR))) {
            tokens[i].type = TK_NEG;
            tokens[i].priority = 1;
        }
    }
    *success = true;
    return eval(0, nr_token - 1);
}

uint32_t eval(int p, int q) {

}