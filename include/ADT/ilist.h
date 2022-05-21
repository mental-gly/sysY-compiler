#ifndef COMPILER_ILIST_H
#define COMPILER_ILIST_H


template <typename T>
struct ast_ilist_node {
    T *Prev;
    T *Next;
};


#endif //COMPILER_ILIST_H
