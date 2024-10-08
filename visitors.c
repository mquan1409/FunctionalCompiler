#include "visitors.h"
#include "helpers.h"
#include "table.h"
#include "map.h"

int fill_table(struct ast *node){
  struct ast_child *child;
  char name[50];
  char scope[50];
  struct arg args[10];
  int num_arg = 0;
  int type, let_id, is_func;
  if (!strcmp(node->token, "funcdef")) {
    strcpy(name, node->child->id->token);
    strcpy(scope, "prog");
    int n = get_child_num(node) - 2;
    for (child = node->child; child && n != 0; child = child->next){ n--; }
    char *tp = strchr(child->id->token, ' ') + 1;
    if (!strcmp(tp, "bool")) type = 0;
    else if (!strcmp(tp, "int")) type = 1;
    else type = -1;
    let_id = 0;
    is_func = 1;
    int end_arg = get_child_num(node) - 2;
    int start_arg = 1;
    child = node->child->next;
    for (; start_arg < end_arg; start_arg += 2) {
       if (strcmp(child->id->token,"bool") == 0) {
          args[num_arg].type = 0;
       }
       if (strcmp(child->id->token,"int") == 0){
          args[num_arg].type = 1;
       }
       child = child->next;
       args[num_arg].id = child->id->id;
       child = child->next;
       num_arg ++;
    }
  } else if (!strcmp(node->token, "PEP")){
    strcpy(name, "PEP");
    strcpy(scope, "prog");
    type = getType(node->child->id);
    let_id = 0;
    is_func = 1;
  } else if (!strcmp(node->token, "let")) {
    struct ast *definition, *parent;
    int var_type = 3;
    definition = node->child->next->next->id;
    if (!strcmp(definition->token, "getbool")) type = 0;
    else if (!strcmp(definition->token, "getint")) type = 1; 
    else type = getType(node->child->next->next->id);
    strcpy(name, node->child->id->token);

    for (parent = node->parent; parent->parent; parent = parent->parent){}
    parent = node->parent;
    if (!strcmp(parent->token, "funcdef")){
      strcpy(scope, parent->child->id->token);
    } else if (!strcmp(parent->token, "PEP")){
      strcpy(scope, "PEP");
    }
    let_id = 0;
    is_func = false;
    st_append(name, type, node->child->id->id, scope, let_id, args, num_arg, is_func);
    strcpy(name, node->token);
    if (!strcmp(node->child->next->id->token, "getbool")) var_type = 0;
    else if (!strcmp(node->child->next->id->token, "getint")) var_type = 1; 
    else var_type = getType(node->child->next->id);
    args[0].id = node->child->id->id;
    args[0].type = var_type;
    num_arg = 1;
    findNestedLetVars(node->child->next->id, node->child->id->token, node->child->id->id);
    findNestedLetVars(node->child->next->next->id, node->child->id->token, node->child->id->id);
  } else return 0;
  st_append(name, type, node->id, scope, let_id, args, num_arg, is_func);
  for (int i = 0; i < num_arg; i++){
    args[i].id = 0;
    args[i].type = -1;
  }
  return 0;
}

int declare_var_before_use(struct ast *node) {
   if (node->ntoken == 1) {
      struct ast *tmp = node->parent;
      struct ast *old_tmp = node;
      while (tmp != NULL) {
         if (strcmp(tmp->token,"funcdef") == 0) {
            struct table_entry *en = st_find_entry(tmp->child->id->token,"prog");
            if (en == NULL) {
               printf("Error: Variable %s not declared\n",node->token);
               return 1;
            }
            int num_arg = en->num_arg;
            int i;
            for (i = 0; i < num_arg; i ++) {
               if (strcmp(find_ast_node(en->args[i].id)->token,node->token) == 0) {
                  return 0;
               }
            }
            printf("Error: Variable %s not declared\n",node->token);
            return 1;
         }
         if (strcmp(tmp->token,"let") == 0) {
            if (tmp->child->next->id != old_tmp) {
               struct table_entry *en = st_get_entry(tmp->token,tmp->id);
               int num_arg = en->num_arg;
               int i;

               struct ast *let_define_node = find_ast_node(en->node_id)->child->next->id;

               if (find_parent(node,let_define_node) != NULL) {
                  tmp = tmp->parent;
                  continue;
               }
               if (strcmp(find_ast_node(en->args[0].id)->token,node->token) == 0) {
                  return 0;
               }
            }
         }
         old_tmp = tmp;
         tmp = tmp->parent;
      }
      printf("Error: Variable %s not declared\n",node->token);
      return 1;
   }

   return 0;
}

int duplicate_arg_func(struct ast *node) {
   if (strcmp(node->token,"funcdef") == 0) {
      char *func_name = node->child->id->token;
      struct table_entry *en = st_get_entry(func_name,node->id);
      int num_arg = en->num_arg;
      int i;
      int j;
      for (i = 0; i < num_arg; i ++) {
         int count = 0;
         for (j = 0; j < num_arg; j ++) {
            if (strcmp(find_ast_node(en->args[i].id)->token
                       ,find_ast_node(en->args[j].id)->token) == 0) {
               count ++;
            }
         }
         if (count >= 2) {
            printf("Error: Function argument %s defined twice\n",find_ast_node(en->args[i].id)->token);
            return 1;
         }
      }
      return 0;
   }
   return 0;
}

int duplicate_var_declare(struct ast *node) {
   if (node->ntoken == 3) {
      struct ast *tmp = node->parent;
      struct table_entry *let_entry = st_get_entry("let",node->id);
      char *var_decl = find_ast_node(let_entry->args[0].id)->token;
      while (tmp != NULL) {
         if (strcmp(tmp->token,"funcdef") == 0) {
            struct table_entry *en = st_find_entry(tmp->child->id->token,"prog");
            if (en == NULL) {
               printf("Error: Variable %s declared twice\n",var_decl);
               return 1;
            }
            int num_arg = en->num_arg;
            int i;
            for (i = 0; i < num_arg; i ++) {
               if (strcmp(find_ast_node(en->args[i].id)->token,var_decl) == 0) {
                  printf("Error: Variable %s declared twice\n",var_decl);
                  return 1;
               }
            }
            return 0;
         }
         if (strcmp(tmp->token,"let") == 0) {
            struct table_entry *en = st_get_entry(tmp->token,tmp->id);
            int num_arg = en->num_arg;
            int i;
	    if (strcmp(find_ast_node(en->args[0].id)->token,var_decl) == 0) {
		   printf("Error: Variable %s declared twice\n",var_decl);
		   return 1;
	    }
         }
         tmp = tmp->parent;
      }
      return 0;
   }

   return 0;
}

int declare_func_before_use(struct ast *node) {
   if (node->ntoken == 2) {
      struct ast *tmp = node->parent;
      struct table_entry *en = st_find_entry(node->token,"prog");
      if (en == NULL) {
         printf("Error: Function %s not defined\n",node->token);
         return 1;
      }
      struct ast *n = find_ast_node(en->node_id);
      int use_id = node->id;
      int decl_id = en->node_id;
      if (decl_id < use_id) {
         printf("Error: Function %s not defined\n",node->token);
         return 1;
      }
      else {
         return 0;
      }
   }

   return 0;
}

int unique_func_names(struct ast *node) {
  struct table_entry *f = st_get_func(node->token);
  if (!f) return 0;
  if (st_is_func_unique(f->name)) {
      printf("Error: Function %s name defined twice\n",node->token);
      return 1;
  }
  return 0;
}

int vars_with_func_names(struct ast *node) {
   if (node->ntoken == 1 && st_get_func(node->token)){
      struct table_entry *f = st_get_func(node->token);
      printf("Error: Variable shares a name of a defined function %s\n",f->name);
      return 1;
   }
  return 0;
}

int match_num_args_func(struct ast *node) {
   if (node->ntoken == 2) {
      struct table_entry *en = st_find_entry(node->token,"prog");
      if (en == NULL) {
         printf("Function %s not declared\n",node->token);
         return 1;
      }
      int use_num_child = get_child_num(node);
      if (use_num_child != en->num_arg) {
         printf("Error: Number of arguments in function %s does not match definition\n",node->token);
         return 1;
      }
   }
   return 0;
}

int init_map(struct ast *node) {
   int type = 2;

   // if this node is defined in symbol table, use that type
   // if this is a funcdef, search by nodeid
   struct table_entry *st_en;
   struct table_entry *st_tmp;
   
   // if this is the name of a func, search by name and scope="prog"
   st_en = st_find_entry(node->token, "prog");
   if (st_en != NULL)
      type = st_en->type;
   else if (!strcmp(node->token, "int") || !strcmp(node->token, "ret int")) {
      type = 1;
   }
   else if (!strcmp(node->token, "bool") || !strcmp(node->token, "ret bool")) {
      type = 0;
   }
   // if this node is a const/explicit, use that type
   else if (isArithematic(node->token) || isArithematicConst(node->token) \
      || !strcmp(node->token, "int") || !strcmp(node->token, "ret int")) 
      type = 1;
   else if (isBoolean(node->token) || isBooleanConst(node->token) \
      || !strcmp(node->token, "bool") || !strcmp(node->token, "ret bool"))
      type = 0; 
   else if (!strcmp(node->token, "funcdef")) {
      st_tmp = st_get_func(node->child->id->token);
      type = st_tmp->type;
   }
   else if (node->parent != NULL && !strcmp(node->parent->token, "funcdef")) {
      struct table_entry *st_tmp = st_find_by_id(node->parent->id);
      // find matching in func declare
         // find matching in args
      int i;
      for(i=0; i<st_tmp->num_arg; i++) {
         if (!strcmp(node->token, find_ast_node(st_tmp->args[i].id)->token)) {
            type = st_tmp->args[i].type;
            break;
         }
      }
   }
   

   tm_append(node, type);
   return 0;
}

int fill_map(struct ast *node) {
   struct map_entry *tm_cur, *tm_tmp;
   struct table_entry *st_tmp;
   struct ast *a_tmp;
   int i;
   // skip if already set
   tm_cur = tm_find(node);
   if (tm_cur->type == 2) {
      if (!strcmp(node->token, "if")) {
         // if second child has type, take it
         tm_tmp = tm_find(node->child->next->id);
         if (tm_tmp->type != 2)
            tm_cur->type = tm_tmp->type;
         // if third child has type, take it
         tm_tmp = tm_find(node->child->next->next->id);
         if (tm_tmp->type != 2)
            tm_cur->type = tm_tmp->type;
      }
      else if (!strcmp(node->token, "let")) {
         tm_tmp = tm_find(node->child->next->next->id);
         tm_cur->type = tm_tmp->type;
      }
      // var declaration
      // check function call
      else if (st_get_func(node->token) != NULL) {
         st_tmp = st_get_func(node->token);
         tm_cur->type = st_tmp->type;
      }
      // var use
      else {
         a_tmp = node->parent;
         bool match = false;
         // var declare in let
         if (!strcmp(a_tmp->token, "let")) {
            st_tmp = st_find_by_id(a_tmp->id);
            if (node->id == a_tmp->child->id->id) {
               tm_tmp = tm_find(a_tmp->child->next->id);
               tm_cur->type = tm_tmp->type;
               match = true;
            }
         }
         // walk up until let def (if exists)
         if (!match) {
            while (a_tmp != NULL) {
               if (!strcmp(a_tmp->token, "let")) {
                  // found let. compare args            
                  st_tmp = st_find_by_id(a_tmp->id);
                  if (!strcmp(node->token,a_tmp->child->id->token)) {
                     tm_tmp = tm_find(a_tmp->child->id);
                     tm_cur->type = tm_tmp->type;
                     break;
                  }
               }
               if (!strcmp(a_tmp->token, "funcdef")) {
                  st_tmp = st_find_by_id(get_root(node)->id);
                  for(i=0; i<st_tmp->num_arg; i++) {
                     if (!strcmp(node->token, find_ast_node(st_tmp->args[i].id)->token)) {
                        tm_cur->type = st_tmp->args[i].type;
                        match = true;
                        break;
                     }
                  }
                  if (match)
                     break;
               }
               a_tmp = a_tmp->parent;
            }
         }
      }
   }
   return 0;
}

int well_formed_aop(struct ast *node) {
   if (node->ntoken == 4) {
      int n = get_child_num(node);
      int i = 0;
      struct ast_child *ptr = node->child;
      for (i = 0; i < n; i ++) {
         struct map_entry *en = tm_find(ptr->id);
         if (en == NULL) {
            return 1; // entry not in the map
         }
         else {
            if (en->type != 1) {
               printf("Error: arguments of %s do not type check\n",node->token);
               return 1;
            }
         }
         ptr = ptr->next;
      }
   }
   return 0;
}

int well_formed_bop(struct ast *node) {
   if (node->ntoken == 5) {
      int n = get_child_num(node);
      int i = 0;
      struct ast_child *ptr = node->child;
      for (i = 0; i < n; i ++) {
         struct map_entry *en = tm_find(ptr->id);
         if (en == NULL) {
            return 1; // entry not in the map
         }
         else {
            if (en->type != 0) {
               printf("Error: arguments of %s do not type check\n",node->token);
               return 1;
            }
         }
         ptr = ptr->next;
      }
   }
   return 0;
}

int well_formed_not(struct ast *node) {
   if (node->ntoken == 7) {
      int n = 1;
      int i = 0;
      struct ast_child *ptr = node->child;
      for (i = 0; i < n; i ++) {
         struct map_entry *en = tm_find(ptr->id);
         if (en == NULL) {
            return 1; // entry not in the map
         }
         else {
            if (en->type != 0) {
               printf("Error: arguments of %s do not type check\n",node->token);
               return 1;
            }
         }
         ptr = ptr->next;
      }
   }
   return 0;
}

int func_call_args_type(struct ast *node) {
   if (node->ntoken == 2) {
      struct ast_child *ptr = node->child;
      int n = get_child_num(node);
      int i = 0;
      struct table_entry *table_en = st_get_func(node->token);
      for (i = 0; i < n; i ++) {
         struct map_entry *en = tm_find(ptr->id);
         if (en == NULL) {
            return 1; // entry not in the map
         }
         else {
            if (en->type != table_en->args[i].type) {
               printf("Error: Argument #%d of %s does not type check with type of %s\n",(i+1),node->token,find_ast_node(table_en->args[i].id)->token);
               return 1;
            }
         }
         ptr = ptr->next;
      }
   }
   return 0;
}

int if_first_arg(struct ast *node) {
   if (node->ntoken == 6) {
      struct ast_child *ptr = node->child;
      struct map_entry *en = tm_find(ptr->id);
      if (en == NULL) {
         return 1; // not found entry in map
      }
      else {
         if (en->type != 0) {
            printf("Error: Argument #1 of if do not type check\n");
            return 1;
         }
      }
   }
   return 0;
}

int check_ifs(struct ast *node){
   if (strcmp(node->token, "if")) return 0;
   if (tm_find(node->child->next->id)->type != tm_find(node->child->next->next->id)->type){
      printf("Error: Arguments of if do not type check\n");
      return 1;
   }
   if (tm_find(node)->type != tm_find(node->child->next->id)->type){
      printf("Error: If-expression does not type check with arguments\n");
      return 2;
   }
   return 0; 
}

int check_lets(struct ast *node){
   if (strcmp(node->token, "let")) return 0;
   struct ast_child *child = node->child;
   if (tm_find(child->id)->type != tm_find(child->next->next->id)->type){
      printf("Error: Let-variable %s does not type check with expression\n",
             child->id->token);
      return 1;
   }
   return 0;
}

char *convert_type(int type){
   return type ? "an int" : "a bool";
}

int check_function_returns(struct ast *node){
   struct table_entry *function = st_get_func(node->token);
   if (node->ntoken != 0 || !function || !strcmp(node->token, "PEP")) return 0;
   struct ast_child *child;
   for (child = node->parent->child; child->next; child = child->next);
   if (tm_find(node)->type != tm_find(child->id)->type){
      printf("Error: Function %s returns %s, but %s is expected by the definition\n", node->token,
             convert_type(tm_find(child->id)->type),
             convert_type(tm_find(node)->type));
      return 1;
   }
   return 0;
}
