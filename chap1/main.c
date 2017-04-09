#include <stdio.h>
#include <string.h>

#include "util.h"
#include "slp.h"
#include "prog1.h"

int max(int, int);
int maxargs(A_stm);

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int maxargsExp(A_exp e)
{
    switch (e->kind)
    {
    case A_idExp:
        return 0;
    case A_numExp:
        return 0;
    case A_opExp:
        return max(maxargsExp(e->u.op.left), maxargsExp(e->u.op.right));
    case A_eseqExp:
        return max(maxargs(e->u.eseq.stm), maxargsExp(e->u.eseq.exp));
    default:
        assert(!"should not get here");
    }
}

int maxargsExplist(A_expList el)
{
    switch (el->kind)
    {
    case A_pairExpList:
        return max(maxargsExp(el->u.pair.head), maxargsExplist(el->u.pair.tail));
    case A_lastExpList:
        return maxargsExp(el->u.last);
    default:
        assert(!"should not get here");
    }
}

int countExp(A_expList el)
{
    int cnt = 1;
    A_expList el_ = el;
    for (; el_->kind == A_pairExpList; el_ = el_->u.pair.tail)
    {
        cnt++;
    }

    return cnt;
}

int maxargs(A_stm s)
{
    switch (s->kind)
    {
    case A_compoundStm:
        return max(maxargs(s->u.compound.stm1), maxargs(s->u.compound.stm2));
    case A_assignStm:
        return maxargsExp(s->u.assign.exp);
    case A_printStm:
        return max(countExp(s->u.print.exps), maxargsExplist(s->u.print.exps));
    default:
        assert(!"should not get here");
    }
}

//-------------------------------------------
typedef struct table *Table_;
struct table
{
    string id;
    int value;
    Table_ tail;
};
typedef struct intAndTable *IntAndTable_;
struct intAndTable
{
    int i;
    Table_ t;
};

void interp(A_stm);
Table_ Table(string, int, struct table*);
IntAndTable_ IntAndTable(int, Table_);
Table_ interpStm(A_stm, Table_);
Table_ update(Table_, string, int);
IntAndTable_ interpExp(A_exp, Table_);
IntAndTable_ interpExpList(A_expList, Table_);
int lookup(Table_, string);

Table_ Table(string id, int value, struct table *tail)
{
    Table_ t = checked_malloc(sizeof(*t));
    t->id = id;
    t->value = value;
    t->tail = tail;
    return t;
}

IntAndTable_ IntAndTable(int i, Table_ t)
{
    IntAndTable_ it = checked_malloc(sizeof(*it));
    it->i = i;
    it->t = t;
    return it;
}

Table_ interpStm(A_stm s, Table_ t)
{
    IntAndTable_ it;
    switch (s->kind)
    {
    case A_compoundStm:
        t = interpStm(s->u.compound.stm1, t);
        t = interpStm(s->u.compound.stm2, t);
        return t;
    case A_assignStm:
        it = interpExp(s->u.assign.exp, t);
        t = update(it->t, s->u.assign.id, it->i);
        return t;
    case A_printStm:
        it = interpExpList(s->u.print.exps, t);
        return it->t;
    default:
        assert(!"should not get here");
    }
}

IntAndTable_ interpExp(A_exp e, Table_ t)
{
    switch (e->kind)
    {
    case A_idExp:
        return IntAndTable(lookup(t, e->u.id), t);
    case A_numExp:
        return IntAndTable(e->u.num, t);
    case A_opExp:
    {
        IntAndTable_ left = interpExp(e->u.op.left, t);
        IntAndTable_ right = interpExp(e->u.op.right, t);
        switch (e->u.op.oper)
        {
        case A_plus:
            return IntAndTable(left->i + right->i, t);
        case A_minus:
            return IntAndTable(left->i - right->i, t);
        case A_times:
            return IntAndTable(left->i * right->i, t);
        case A_div:
            return IntAndTable(left->i / right->i, t);
        default:
            assert(!"should not get here");
        }
    }
    case A_eseqExp:
        t = interpStm(e->u.eseq.stm, t);
        return interpExp(e->u.eseq.exp, t);
    default:
        assert(!"should not get here");
    }
}

IntAndTable_ interpExpList(A_expList el, Table_ t)
{
    IntAndTable_ it;

    switch (el->kind)
    {
    case A_pairExpList:
        it = interpExp(el->u.pair.head, t);
        printf("%d ", it->i);
        return interpExpList(el->u.pair.tail, it->t);
    case A_lastExpList:
        it = interpExp(el->u.last, t);
        printf("%d\n", it->i);
        return it;
    default:
        assert(!"should not get here");
    }
}

Table_ update(Table_ t, string id, int value)
{
    return Table(id, value, t);
}

int lookup(Table_ t, string key)
{
    Table_ temp = t;
    while (temp != NULL)
    {
        if (temp->id == key)
        {
            return t->value;
        }
        temp = temp->tail;
    }
    assert(!"should not get here");
}

void interp(A_stm s)
{
    interpStm(s, NULL);
}

//---------------------------------------
typedef struct tree *T_tree;
struct tree
{
    T_tree left;
    string key;
    T_tree right;
};

T_tree Tree(T_tree l, string k, T_tree r)
{
    T_tree t = checked_malloc(sizeof(*t));
    t->left = l;
    t->key = k;
    t->right = r;
    return t;
}

T_tree insert(string key, T_tree t)
{
    if (t == NULL)
        return Tree(NULL, key, NULL);
    else if (strcmp(key, t->key) < 0)
        return Tree(insert(key, t->left), t->key, t->right);
    else if (strcmp(key, t->key) > 0)
        return Tree(t->left, t->key, insert(key, t->right));
    else
        return Tree(t->left, key, t->right);
}

bool member(string key, T_tree t)
{
    if (t == NULL)
        return FALSE;
    else if (strcmp(key, t->key) < 0)
        return member(key, t->left);
    else if (strcmp(key, t->key) > 0)
        return member(key, t->right);
    else
        return TRUE;
}

int main()
{
    A_stm s = prog();
    printf("result of maxargs is %d\n", maxargs(s));
    interp(s);
    T_tree tree = Tree(NULL, "a", NULL);
    tree = insert("b", tree);
    printf("is member %d\n", member("b", tree));
    return 0;
}
