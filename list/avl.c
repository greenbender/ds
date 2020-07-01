/*
 * ANSI C Library for maintainance of AVL Balanced Trees
 *
 * ref.:
 *  G. M. Adelson-Velskij & E. M. Landis
 *  Doklady Akad. Nauk SSSR 146 (1962), 263-266
 *
 * see also:
 *  D. E. Knuth: The Art of Computer Programming Vol.3 (Sorting and Searching)
 *
 * (C) 2000 Daniel Nagy, Budapest University of Technology and Economics
 * Released under GNU General Public License (GPL) version 2
 *
 */

/* 
 * Adapted to suit AVL tree based indexed list
 */

#include "list.h"
#include <stdint.h>
#include <stdlib.h>


#define sizez(n) ((n) ? (n)->size : 0)
#define index(n) sizez((n)->left)
#define min(a, b) ((a) < (b) ? a : b)


typedef struct avl {
    struct avl *left;
    struct avl *right;
    int8_t balance;
    size_t size;
    void *data;
} avl;


typedef struct avllist_t {
    list_t list;
    avl *root;
} avllist_t;


static void rol(avl **root){
    avl *a = *root;
    avl *b = a->right;
    a->right = b->left;
    b->left = a;
    b->size = a->size;
    a->size -= sizez(b->right) + 1;
    *root = b;
}


static void ror(avl **root){
    avl *a = *root;
    avl *b = a->left;
    a->left = b->right;
    b->right = a;
    b->size = a->size;
    a->size -= sizez(b->left) + 1;
    *root = b;
}


static void nasty(avl* root){
   switch (root->balance) {
        case -1:
            root->left->balance = 0;
            root->right->balance = 1;
            break;
        case 1:
            root->left->balance = -1;
            root->right->balance = 0;
            break;
        case 0:
            root->left->balance = 0;
            root->right->balance = 0;
   }
   root->balance = 0;
}


/* minimize stack depth */
static int _insert(avl **root, avl *a) {
    (*root)->size++;
    if (a->size <= index(*root)) {
        if ((*root)->left) {
            if (_insert(&(*root)->left, a)) {
                switch ((*root)->balance--) {
                    case 1: return 0;
                    case 0: return 1;
                }
                if ((*root)->left->balance < 0){
                    ror(root);
                    (*root)->balance = 0;
                    (*root)->right->balance = 0;
                } else {
                    rol(&(*root)->left);
                    ror(root);
                    nasty(*root);
                }
            }
            return 0;
        } else {
            a->size = 1;
            (*root)->left = a;
            return !(*root)->balance--;
        }
    } else {
        if ((*root)->right) {
            a->size -= index(*root) + 1;
            if (_insert(&(*root)->right, a)) {
                switch ((*root)->balance++){
                    case -1: return 0;
                    case 0: return 1;
                }
                if ((*root)->right->balance > 0) {
                    rol(root);
                    (*root)->balance = 0;
                    (*root)->left->balance = 0;
                } else {
                    ror(&(*root)->right);
                    rol(root);
                    nasty(*root);
                }
            }
            return 0;
        } else {
            a->size = 1;
            (*root)->right = a;
            return !(*root)->balance++;
        }
    }
}


static void insert(list_t *list, size_t idx, void *data) {
    avllist_t *avllist = (avllist_t *)list;
    avl *node = calloc(1, sizeof(*node));
    node->data = data;
    if (sizez(avllist->root) == 0) {
        node->size = 1;
        avllist->root = node;
    } else {
        node->size = min(idx, sizez(avllist->root));
        _insert(&avllist->root, node);
    }
}


static avl *_get(avl *root, size_t idx) {
    if (idx < index(root)) {
        return _get(root->left, idx);
    }
    if (idx > index(root)) {
        return _get(root->right, idx - index(root) - 1);
    }
    return root;
}


static void *get(list_t *list, size_t idx) {
    avllist_t *avllist = (avllist_t *)list;
    if (idx >= sizez(avllist->root)) {
        return NULL;
    }
    return _get(avllist->root, idx)->data;
}


static size_t length(list_t *list) {
    avllist_t *avllist = (avllist_t *)list;
    return sizez(avllist->root);
}


static void append(list_t *list, void *data) {
    insert(list, length(list), data);
}


static void _destroy(avl *root, void (*handler)(void *)) {
    if (root->left) {
        _destroy(root->left, handler);
    }
    if (root->right) {
        _destroy(root->right, handler);
    }
    if (handler) {
        handler(root->data);
    }
    free(root);
}


static void destroy(list_t *list, void (*handler)(void *)) {
    avllist_t *avllist = (avllist_t *)list;
    if (sizez(avllist->root)) {
        _destroy(avllist->root, handler);
    }
    free(list);
}


#ifdef __DEBUG__
#include <stdio.h>
#include <inttypes.h>


static void _plot(avl *root, FILE *fd) {
    fprintf(fd, "    n%" PRIxPTR " [label=\"<f0>|<f1>%s(%d)|<f2>\"]\n",
        (uintptr_t)root, (char *)root->data, root->balance
    );
    if (root->left) {
        fprintf(fd, "    n%" PRIxPTR ":f0 -> n%" PRIxPTR ":f1\n",
            (uintptr_t)root, (uintptr_t)root->left
        );
        _plot(root->left, fd);
    }
    if (root->right) {
        fprintf(fd, "    n%" PRIxPTR ":f2 -> n%" PRIxPTR ":f1\n",
             (uintptr_t)root, (uintptr_t)root->right
        );
        _plot(root->right, fd);
    }
}


void plot(avl *root) {
    FILE *fd = fopen("avllist.gv", "wb");
    if (fd) {
        fprintf(fd,
            "digraph avllist {\n"
            "    node [height=.1 shape=record]\n"
        );
        if (root) {
            _plot(root, fd);
        }
        fprintf(fd, "}\n");
        fclose(fd);
    }
}


void debug(list_t *list) {
    avllist_t *avllist = (avllist_t *)list;
    plot(avllist->root);
}
#endif


list_t *list_create(void) {
    static avllist_t template = {
        .list = {
            .length = length,
            .get = get,
            .insert = insert,
            .append = append,
            .destroy = destroy,
#ifdef __DEBUG__
            .debug = debug,
#endif
        }
    };
    avllist_t *avllist = malloc(sizeof(*avllist));
    *avllist = template;
    return (list_t *)avllist;
}


#if 0    
    avl_tree *t, avl *a)
static void insert(avl_tree *t, avl *a)
{
     /* initialize */
     a->left=0;
     a->right=0;
     a->balance=0;
     /* insert into an empty tree */
     if(!t->root){
            t->root=a;
            return 1;
     }
     
     if(t->compar(t->root,a)>0){
            /* insert into the left subtree */
            if(t->root->left){
                 avl_tree left_subtree;
                 left_subtree.root=t->root->left;
                 left_subtree.compar=t->compar;
                 if(avl_insert(&left_subtree,a)){
                        switch(t->root->balance--){
                         case 1: return 0;
                         case 0:        return 1;
                        }
                        if(t->root->left->balance<0){
                             t->root = ror(t->root);
                        }else{
                             t->root->left = rol(t->root->left);
                             t->root = ror(t->root);
                        }
                 }else t->root->left=left_subtree.root;
                 return 0;
            }else{
                 t->root->left=a;
                 if(t->root->balance--) return 0;
                 return 1;
            }
     }else{
            /* insert into the right subtree */
            if(t->root->right){
                 avl_tree right_subtree;
                 right_subtree.root=t->root->right;
                 right_subtree.compar=t->compar;
                 if(avl_insert(&right_subtree,a)){
                        switch(t->root->balance++){
                         case -1: return 0;
                         case 0: return 1;
                        }
                        if(t->root->right->balance>0){
                             t->root = rol(t->root);
                        }else{
                             t->root->right = ror(t->root->right);
                             t->root = rol(t->root);
                        }
                 }else t->root->right=right_subtree.root;
                 return 0;
            }else{
                 t->root->right=a;
                 if(t->root->balance++) return 0;
                 return 1;
            }
     }
}

/* Remove an element a from the AVL tree t
 * returns -1 if the depth of the tree has shrunk
 * Warning: if the element is not present in the tree,
 *                  returns 0 as if it had been removed succesfully.
 */
int avl_remove(avl_tree* t, avl* a) {
    int b;
    if(t->root==a)
        return avl_removeroot(t);
    b=t->compar(t->root,a);
    if(b>=0){
        /* remove from the left subtree */
        int ch;
        avl_tree left_subtree;
        if( (left_subtree.root=t->root->left) ){
            left_subtree.compar=t->compar;
            ch=avl_remove(&left_subtree,a);
            t->root->left=left_subtree.root;
            if(ch){
                switch(t->root->balance++){
                    case -1: return -1;
                    case 0: return 0;
                }
                switch(t->root->right->balance){
                    case 0:
                        t->root = rol(t->root);
                        return 0;
                    case 1:
                        t->root = rol(t->root);
                        return -1;
                }
                t->root->right = ror(t->root->right);
                t->root = rol(t->root);
                return -1;
            }
        }
    }
    if(b<=0){
        /* remove from the right subtree */
        int ch;
        avl_tree right_subtree;
        if( (right_subtree.root=t->root->right) ){
            right_subtree.compar=t->compar;
            ch=avl_remove(&right_subtree,a);
            t->root->right=right_subtree.root;
            if(ch){
                switch(t->root->balance--){
                    case 1: return -1;
                    case 0: return 0;
                }
                switch(t->root->left->balance){
                    case 0:
                        t->root = ror(t->root);
                        return 0;
                    case -1:
                        t->root = ror(t->root);
                        return -1;
                }
                t->root->left = rol(t->root->left);
                t->root = ror(t->root);
                return -1;
            }
        }
    }
    return 0;
}

/* Remove the root of the AVL tree t
 * Warning: dumps core if t is empty
 */
int avl_removeroot(avl_tree* t)
{
     int ch;
     avl* a;
     if(!t->root->left){
            if(!t->root->right){
                 t->root=0;
                 return -1;
            }
            t->root=t->root->right;
            return -1;
     }
     if(!t->root->right){
            t->root=t->root->left;
            return -1;
     }
     if(t->root->balance<0){
            /* remove from the left subtree */
            a=t->root->left;
            while(a->right) a=a->right;
     }else{
            /* remove from the right subtree */
            a=t->root->right;
            while(a->left) a=a->left;
     }
     ch=avl_remove(t,a);
     a->left=t->root->left;
     a->right=t->root->right;
     a->balance=t->root->balance;
     t->root=a;
     if(a->balance==0) return ch;
     return 0;
}

/* Iterate through elements in t from a range between a and b (inclusive)
 * for each element calls iter(a) until it returns 0
 * returns the last value returned by iterator or 0 if there were no calls
 * Warning: a<=b must hold
 */
int avl_range(avl_tree* t,avl* a,avl* b,int(*iter)(avl* a))
{
     int x,c=0;
     if(!t->root) return 0;
     x=t->compar(t->root,a);
     if(a!=b){
            if(x<0){
                 x=t->compar(t->root,b);
                 if(x>0) x=0;
            }
     }
     if(x>=0){
            /* search in the left subtree */
            avl_tree left_subtree;
            if( (left_subtree.root=t->root->left) ){
                 left_subtree.compar=t->compar;
                 if(!(c=avl_range(&left_subtree,a,b,iter))) if(x>0) return 0;
            }
     }
     if(x==0){
            if(!(c=iter(t->root))) return 0;
     }
     if(x<=0){
            /* search in the right subtree */
            avl_tree right_subtree;
            if( (right_subtree.root=t->root->right) ){
                 right_subtree.compar=t->compar;
                 if(!(c=avl_range(&right_subtree,a,b,iter))) if(x<0) return 0;
            }
     }
     return c;
}
#endif
