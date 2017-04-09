#include <stdio.h>

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
    switch (e->kind) {
    case A_idExp:
        return 0;
    case A_numExp:
        return 0;
    case A_opExp:
        return max(maxargsExp(e->u.op.left), maxargsExp(e->u.op.right));
    case A_eseqExp:
        return max(maxargs(e->u.eseq.stm), maxargsExp(e->u.eseq.exp));
    default:
        assert(!"A_exp.kind is not defined");
    }
}

int maxargsExplist(A_expList el)
{
    switch (el->kind) {
    case A_pairExpList:
        return max(maxargsExp(el->u.pair.head), maxargsExplist(el->u.pair.tail));
    case A_lastExpList:
        return maxargsExp(el->u.last);
    default:
        assert("A_expList.kind is not defined");
    }
}

int countExp(A_expList el)
{
    int cnt = 1;
    A_expList el_ = el;
    for (; el_->kind == A_pairExpList; el_ = el_->u.pair.tail) {
        cnt++;
    }

    return cnt;
}

int maxargs(A_stm s)
{
    switch (s->kind) {
    case A_compoundStm:
        return max(maxargs(s->u.compound.stm1), maxargs(s->u.compound.stm2));
    case A_assignStm:
        return maxargsExp(s->u.assign.exp);
    case A_printStm:
        return max(countExp(s->u.print.exps), maxargsExplist(s->u.print.exps));
    default:
        assert(!"A_stm.kind is not defined");
    }
}

int main()
{
    A_stm s = prog();
    printf("result of maxargs is %d\n", maxargs(s));
    return 0;
}
