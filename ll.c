#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct Node {
    struct Node *next;
    int data;
} Node;

typedef struct {
    Node *head;
    Node *tail;
} LinkedList;

void ll_create(LinkedList *ll) {
    assert(ll != NULL);
    ll->head = NULL;
    ll->tail = NULL;
}

void ll_push(LinkedList *ll, int data) {
    assert(ll != NULL);

    Node *new = malloc(sizeof(Node));
    assert(new != NULL);

    new->data = data;
    new->next = ll->head;
    ll->head = new;

    if(ll->tail == NULL) ll->tail = new;
}

void ll_append(LinkedList *ll, int data) {
    assert(ll != NULL);

    Node *new = malloc(sizeof(Node));
    assert(new != NULL);

    new->data = data;
    new->next = NULL;

    if(ll->tail == NULL) {
        ll->head = new;
        ll->tail = new;
    } else {
        ll->tail->next = new;
        ll->tail = new;
    }
}

void ll_delete(LinkedList *ll, int value) {
    assert(ll != NULL && ll->head != NULL);

    Node *prev = NULL;
    Node *current = ll->head;

    while(current != NULL) {
        if(current->data != value) {
            prev = current;
            current = current->next;
            continue;
        }

        if(prev == NULL) {
            ll->head = current->next;
            if(ll->tail == current) ll->tail = NULL;
        } else {
            prev->next = current->next;
            if(ll->tail == current) {
                ll->tail = prev;
            }
        }
        free(current);
        break;
    }
}

void ll_print(const LinkedList *ll) {
    Node *current = ll->head;
    while(current != NULL) {
        printf("%d->", current->data);
        current = current->next;
    }

    printf("\n");
}

void ll_destroy(const LinkedList *ll) {
    assert(ll != NULL);

    Node *current = ll->head;
    while(current != NULL) {
        Node *next = current->next;
        free(current);
        current = next;
    }
}

int main() {
    LinkedList ll = {0};
    ll_create(&ll);

    for(size_t i = 1; i <= 5; ++i) {
        ll_append(&ll, i);
    }
    ll_print(&ll);

    for(size_t i = 6; i <= 10; ++i) {
        ll_push(&ll, i);
    }
    ll_print(&ll);

    for(size_t i = 6; i <= 10; ++i) {
        ll_delete(&ll, i);
    }
    ll_print(&ll);

    ll_destroy(&ll);
    return 0;
}
