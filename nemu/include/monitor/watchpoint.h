#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
    int NO;
    struct watchpoint *next;

    /* TODO: Add more members if necessary */
    /* PA1.3*/
    char expr[128];
    uint32_t value;
    int hit;

} WP;
bool check_wp();
void display_wp();
bool del_wp(int n);
void free_wp(WP *wp);
WP* new_wp(char *s);
#endif
