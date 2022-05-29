#ifndef COMPILER_ILIST_H
#define COMPILER_ILIST_H


template <typename T>
struct ast_ilist_node {
    T *Prev;
    T *Next;
    T *tail;
};


#endif //COMPILER_ILIST_H
