#include "list.h"
#include <stdio.h>


void cleanup(void *data) {
    /* printf("cleanup %s\n", (char *)data); */
}


int main(int argc, char **argv) {
    int i;
    list_t *list = list_create();
    
    for (i = 1; i < argc; i++) {
        if (i < 6) {
            list->append(list, argv[i]);
        } else if (i < 11) {
            list->insert(list, 0, argv[i]);
        } else {
            list->insert(list, 5, argv[i]);
        }
    }

    printf("length = %zu\n", list->length(list));
    for (i = 0; i < list->length(list); i++) {
        printf("%d: %s\n", i, (char *)list->get(list, i));
    }
    
    list->debug(list);
    list->destroy(list, cleanup);

    return 0; 
}
