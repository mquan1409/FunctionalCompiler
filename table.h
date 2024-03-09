#ifndef TABLE_H
#define TABLE_H

struct table_entry {
    char name[50];
    int type; // 0 for int, 1 for bool
    int node_id;
    char scope[50];
    int let_id;
    int is_func;
    struct table_entry *next;
};
struct sym_table {
    struct table_entry *start;
};

void st_append(char *name, int type, int node_id, char *scope, int let_id, int is_func);
int st_exists(char *name);
int st_exists_in(char *name, char *scope);
void st_print();

#endif
