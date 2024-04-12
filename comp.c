#include <string.h>
#include "y.tab.h"
#include "ast.h"
#include "stack.h"
#include "table.h"
#include "visitors.h"
#include "map.h"
int yyparse();

int cleanup(int error){
  free_ast();
  st_free();
  tm_free();
  return error;
}

int main (int argc, char **argv) {
  extern struct stack st;
  st.top = NULL;
  extern struct sym_table table;
  table.start = NULL; 
  extern struct type_map map;
  map.start = NULL;
  if (!yyparse()) {
    visit_ast(fill_table);
    if (visit_ast(declare_var_before_use)) return cleanup(1);
    if (visit_ast(declare_func_before_use)) return cleanup(2);
    if (visit_ast(match_num_args_func)) return cleanup(3);
    if (visit_ast(unique_func_names)) return cleanup(4);
    if (visit_ast(vars_with_func_names)) return cleanup(5);
    if (visit_ast(duplicate_var_declare)) return cleanup(6);
    if (visit_ast(duplicate_arg_func)) return cleanup(7);
    visit_ast(init_map);
    
    //tm_print();
    // pass over as many times as needed until zero unknowns remain
    st_print();  // should not print if any failures occured
    
    while (true) {
      visit_ast(fill_map);
      printf("passed\n");
      if (!tm_contains_unknowns()) {
         break;
      }
    }
    print_ast();
    st_print();
    tm_print();

    if (visit_ast(check_ifs)) return cleanup(14);
    if (visit_ast(check_function_returns)) return cleanup(8);
    if (visit_ast(well_formed_aop)) return cleanup(9);
    if (visit_ast(check_function_returns)) return cleanup(10);
    if (visit_ast(well_formed_bop)) return cleanup(11);
    if (visit_ast(well_formed_not)) return cleanup(14);
    if (visit_ast(if_first_arg)) return cleanup(12);
    if (visit_ast(func_call_args_type)) return cleanup(13);
    //if (visit_ast(check_lets)) return cleanup(15);
  }
  return cleanup(0);
}
