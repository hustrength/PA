#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

bool check_parentheses(int p, int q, bool *valid);

int get_main_op(int p, int q);

uint32_t eval(int p, int q, bool *success);

enum {
    TK_NOTYPE = 256, TK_EQ, TK_UNEQ,

    /* TODO: Add more token types */
    /* PA1.2*/
    TK_DECI, TK_PLUS, TK_SUB, TK_MUL, TK_DIV,
    TK_LBR, TK_RBR, TK_NUM, TK_HEX, TK_REG, TK_AND, TK_OR, TK_DEREF, TK_NEG
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

    /* Judge whether the token is TK_NEG or TK_DEREF */
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
    return eval(0, nr_token - 1, success);
}

/*
 * int check_parentheses(int p, int q)
 * return 1  if the parentheses is a valid expression with left and right bracket.
 * return 0  if the parentheses is a valid expression but without left and right bracket.
 * return -1 if the parenteses isn't a valid expression.
 */
bool check_parentheses(int p, int q, bool *valid) {
    *valid = false;
    if (tokens[p].type != TK_LBR || tokens[q].type != TK_RBR)
        return false;  // without left or right bracket

    int nump = 0;
    /* flag means if the left and right brackets are the type of the expression like (exp)+(exp) */
    bool flag = false;
    for (int i = p + 1; i <= q - 1; ++i) {
        if (tokens[i].type == TK_LBR) {
            nump++;
        } else if (tokens[i].type == TK_RBR) {
            nump--;
            if (nump < 0)
                flag = true;
        }
    }
    *valid = (nump == 0);
    return *valid && !flag;
}

/* return -1 if there is no main operator */
int get_main_op(int p, int q) {
    int inBracket = 0, pos = -1, main_op_priority = 0;
    int type;
    for (int i = p; i <= q; i++) {
        type = tokens[i].type;
        if (!inBracket && tokens[i].priority > 0 && tokens[i].priority < 7) {
            if (tokens[i].priority >= main_op_priority) {
                pos = i;
                main_op_priority = tokens[i].priority;
            }
        } else if (type == TK_LBR) inBracket++;
        else if (type == TK_RBR) inBracket--;
    }
    return pos;
}

uint32_t eval(int p, int q, bool *success) {
    bool *valid;
    if (p > q) {
        /* Bad expression */
        *success = false;
        return -1;
    } else if (p == q) {
        /* Single token.
         * For now this token should be a number.
         * Return the value of the number.
         */
        uint32_t val = 0;
        int type = tokens[p].type;
        if (type == TK_NUM || type == TK_HEX) {
            // 当 base 的值为 0 时，默认采用 10 进制转换，但如果遇到 '0x' / '0X' 前置字符则会使用 16 进制转换，遇到 '0' 前置字符则会使用 8 进制转换。
            return strtoul(tokens[p].str, NULL, 0);
        } else if (type == TK_REG) {
            bool *success;
            val = isa_reg_str2val(tokens[p].str + 1, success);
            if (*success)
                return val;
            printf("Unknown register: %s\n", tokens[p].str);
            *success = false;
            return -1;
        }
        printf("eval error: %s\n", tokens[p].str);
        *success = false;
        return -1;
    } else if (check_parentheses(p, q, valid) == true) {
        /* The expression is surrounded by a matched pair of parentheses.
         * If that is the case, just throw away the parentheses.
         */
        return eval(p + 1, q - 1, success);
    } else {
        /* TODO: We should do more things here. */
        if (!valid) {
            printf("Error: the expression is invalid\n");
            *success = false;
            return -1;
        }
        int pos = get_main_op(p, q);
        if (pos == -1) {
            *success = false;
            return -1;
        }
        uint32_t val1 = 0, val2 = 0, val = 0;
        if (tokens[pos].type != TK_DEREF && tokens[pos].type != TK_NEG) {
            val1 = eval(p, pos - 1, success);
            if (*success == false)
                return -1;
        }

        val2 = eval(pos + 1, q, success);
        if (*success == false)
            return -1;

        switch (tokens[pos].type) {
            case TK_PLUS:
                val = val1 + val2;
                break;
            case TK_SUB:
                val = val1 - val2;
                break;
            case TK_MUL:
                val = val1 * val2;
                break;
            case TK_DIV:
                if (val2 == 0) {
                    printf("Error: Divide 0 at [%d, %d]", p, q);
                    *success = false;
                    return -1;
                }
                val = val1 / val2;
                break;
            case TK_AND:
                val = val1 && val2;
                break;
            case TK_OR:
                val = val1 || val2;
                break;
            case TK_EQ:
                val = val1 == val2;
                break;
            case TK_UNEQ:
                val = val1 != val2;
                break;
            case TK_DEREF:
                val = vaddr_read(val2, 4);
                break;
            case TK_NEG:
                val = -val2;
                break;
            default:
                printf("Error: Unknown token type %d: %d %d\n", pos, tokens[pos].type, TK_PLUS);
                *success = false;
                return -1;
        }
        *success = true;
        return val;
    }
}