#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "dictionary.h"
#include "rexp.h"

// INTEGRITY STATEMENT (modify if necessary)
// I received help from the following sources:
// None. I am the sole author of this work

// sign this statement by removing the line below and entering your name
// ERROR_I_have_NOT_yet_completed_the_INTEGRITY_statement
// Name:
// login ID:

// implementation:

// ----------------Stack implementation ------------------
struct stack {
    int len;
    int maxlen;
    struct enode **data;
};

struct stack *create_stack(void) {
    struct stack *s = malloc(sizeof(struct stack));
    s->len = 0;
    s->maxlen = 100;
    s->data = malloc(s->maxlen * sizeof(int));
    return s;
}

void stack_push(struct enode *item, struct stack *s){
    assert(s);
    if (s->len == s->maxlen) {
        s->maxlen *= 2;
        s->data = realloc(s->data, s->maxlen * sizeof(int));
    }
    s->data[s->len] = item;
    s->len += 1;
}

void stack_destroy(struct stack *s) {
    free(s->data);
    free(s);
}

bool stack_is_empty(const struct stack *s) {
    assert(s);
    return s->len == 0;
}

struct enode *stack_top(const struct stack *s) {
    assert(s);
    assert(s->len);
    return s->data[s->len - 1];
}
struct enode *stack_pop(struct stack *s) {
    assert(s);
    assert(s->len);
    s->len -= 1;
    return s->data[s->len];
}

//--------------------- END OF Stack Implementation --------------------------
struct rexp *string_to_rexp(const char *s) {
    // Initializing Stack
    struct stack *my_stack = create_stack();
    
    int i = 0;
    // Tracking the poped enode
    struct enode *poped = NULL;
    int len=strlen(s);
    while (i < len){
        if (s[i] == '(') {
            struct enode *my_node = malloc(sizeof(struct enode));
            my_node->left = NULL;
            my_node->right = NULL;
            char type = s[i+1];
            i++;
            my_node->type = type;
            if (my_stack->len != 0){
                struct enode *parent = stack_top(my_stack);
                if (!parent->left) {
                    parent->left = my_node;
                }else{
                    parent->right = my_node;
                }
            }
            
            stack_push(my_node, my_stack);
            i++;
        }else if(s[i] == ')'){
            poped = stack_pop(my_stack);
            i++;
            // Reading operands
        }else if(s[i] >= '0' && s[i] <= '9'){
            int result = 0;
            int sign = 1;
            if (((i-1) >= 0) && s[i-1] == '-'){
                sign = -1;
            }
            while(s[i] != ' ' && s[i] != ')' && s[i] != '\0'){
                result = result * 10 + (s[i]- '0');
                i++;
            }
            result = result * sign;
            
            struct enode *my_operand = malloc(sizeof(struct enode));
            my_operand->left = NULL;
            my_operand->right = NULL;
            my_operand->number = result;
            my_operand->type = 'n';
            
            
            if (my_stack->len != 0){
                struct enode *parent = stack_top(my_stack);
                
                if (!parent->left) {
                    parent->left = my_operand;
                }else{
                    parent->right = my_operand;
                }
            }else{
                poped = my_operand;
            }
        }
        else if((s[i] >= 'a' && s[i] <= 'z') ||(s[i] >= 'A' && s[i] <= 'Z')){
            char *label = malloc(sizeof(char)*22);
            int index = 0;
            while(s[i] != ' ' && s[i] != ')' && s[i] != '\0'){
                label[index] = s[i];
                index++;
                i++;
            }
            label[index] = '\0';
            
            struct enode *my_operand = malloc(sizeof(struct enode));
            my_operand->left = NULL;
            my_operand->right = NULL;
            strcpy(my_operand->id, label);
            my_operand->type = 'v';
            free(label);
            
            if (my_stack->len != 0){
                struct enode *parent = stack_top(my_stack);
                
                if (parent->left == NULL) {
                    parent->left = my_operand;
                }else{
                    parent->right = my_operand;
                }
            }else{
                poped = my_operand;
            }
        }
        else{
            i++;
        }
    }
    
    struct rexp *result = malloc(sizeof(struct rexp));
    result->root = poped;
    
    stack_destroy(my_stack);
    return result;
}


// Helper function to evaluate enode
int enode_eval(const struct enode *node, const struct dictionary *constants){
    char type = node->type;
    int result = 0;
    
    
    if (type == 'n') {
        result = node->number;
    }else if (type == '+'){
        result = enode_eval(node->left, constants) + enode_eval(node->right, constants);
    }else if (type == '-'){
        result =  enode_eval(node->left, constants) - enode_eval(node->right, constants);
    }else if (type == '*'){
        result =  enode_eval(node->left, constants) * enode_eval(node->right, constants);
    }else if (type == '/'){
        result = enode_eval(node->left, constants) / enode_eval(node->right, constants);
    }else{ // type = 'v' in this case
        result =  dict_lookup(node->id, constants);
    }
    return result;
}

int rexp_eval(const struct rexp *r, const struct dictionary *constants) {
    return enode_eval(r->root, constants);
}


// Helper function of rexp_destroy
void free_enode(struct enode *n){
    if (n == NULL){
        return;
    }  
    if (n->type != 'n' && n->type != 'v'){
        free_enode(n->left);
        free_enode(n->right);
    }
    free(n);
}


void rexp_destroy(struct rexp *r) {
    if(r==NULL) return;
    free_enode(r->root);
    free(r);
}


void add_constant(const char *s, struct dictionary *constants) {
    // skip reading "(define "
    int i = 8; // start at index 8
    
    
    while ((s[i] >= 'a' && s[i] <= 'z')||(s[i] >= 'A' && s[i] <= 'Z')){ // a-z A-Z
        i++;
    }
    
    int cons_len = i-7;
    char *constant_name = malloc(sizeof(char)*cons_len);
    // printf("constant length: %d\n", cons_len);
    
    i = 8;
    while ((s[i] >= 'a' && s[i] <= 'z')||(s[i] >= 'A' && s[i] <= 'Z')){ // a-z A-Z
        // printf("%c\n",s[i]);
        constant_name[i-8] = s[i];
        i++;
    }
    constant_name[i-8] = '\0';
    
    
    
    int length = strlen(s)-i-1;
    // printf("length: %d\n", length);
    char *const_val_str = malloc(sizeof(char)*length);
    
    // get constant definition string
    int index = 0;
    i++;
    while(i < strlen(s)-1 ){
        // printf("%c\n",s[i]);
        const_val_str[index] = s[i];
        index++;
        i++;
    }
    const_val_str[index] = '\0';
    // getting value of constant
    
    // printf("const_val_str: %s\n", const_val_str);
    
    struct rexp *constant_val = string_to_rexp(const_val_str);
    // save key and value to dictionary
    int value = rexp_eval(constant_val, constants);
    // printf("key: %s\n",constant_name);
    // printf("value: %d\n", value);
    dict_add(constant_name, value, constants);
    
    rexp_destroy(constant_val);
    // free strings
    free(constant_name);
    free(const_val_str);
}


///////////////////////////////////////////////////////////////////////
// do NOT modify these functions: they are used for marmoset testing //
// (you may find them useful for your own testing)                   //
///////////////////////////////////////////////////////////////////////

void print_enode(const struct enode *e) {
    assert(e);
    if (e->type == 'n') {
        printf("%d", e->number);
    } else if (e->type == 'v') {
        printf("%s", e->id);
    } else {
        printf("(%c ", e->type);
        print_enode(e->left);
        printf(" ");
        print_enode(e->right);
        printf(")");
    }
}

void print_rexp(const struct rexp *r) {
    assert(r);
    print_enode(r->root);
    printf("\n");
}