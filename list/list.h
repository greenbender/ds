#ifndef __LIST_H__
#define __LIST_H__


#include <stddef.h>


typedef struct list_t list_t;


/* define a list interface */
struct list_t {
    size_t (*length)(list_t *self);
    void *(*get)(list_t *self, size_t idx);
    void *(*set)(list_t *self, size_t idx, void *data);
    void *(*del)(list_t *self, size_t idx);
    void (*insert)(list_t *self, size_t idx, void *data);
    void *(*pop)(list_t *self, size_t idx);
    void (*append)(list_t *self, void *data);
    void (*remove)(list_t *self, void *data);
    void (*extend)(list_t *self, list_t *other);
    void *(*next)(list_t *self);
    size_t (*index)(list_t *self, void *data, size_t start, size_t end);
    size_t (*count)(list_t *self, void *data);
    void (*sort)(list_t *self, int (*cmp)(void *), int reverse);
    void (*reverse)(list_t *self);
    void (*setSlice)(list_t *self, size_t start, size_t end, list_t *other);
    void (*destroy)(list_t *self, void (*handler)(void *));
#ifdef __DEBUG__
    void (*debug)(list_t *self);
#endif
};


list_t *list_create(void);


#endif
