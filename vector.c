#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#define DATA_MAX 11

struct Data{
    char        data[DATA_MAX];
    uint32_t    start;
    uint32_t    length;
    bool        ready;
};

typedef struct Node{
    struct Node *prev;
    struct Node *next;
    struct Data *data;
} Node;

struct Vector
{
    struct Node *start;
    int size;
    int maxsize;
};


void push_back(struct Vector *v, struct Data *d){
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = d;
    if(v->start == NULL){
        v->size = 1;
        v->start = new_node;
    }else{
        Node* node = v->start;
        while(node->next != NULL){
            node = node->next;
        }
        v->size += 1;
        node->next = new_node;
    }
}