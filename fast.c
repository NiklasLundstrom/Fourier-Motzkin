#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include "error.h"
#include <stdio.h>

struct rat
{
    int a;
    unsigned int b; // a/b
};

struct rat ratdiv(struct rat *rat1, struct rat *rat2)
{
    struct rat outrat;
        outrat.a = rat1->a * ( (rat2->a > 0) - (rat2->a < 0) ); // * sign(rat2->a)
        outrat.b = labs(rat2->a);
        return outrat;
}

bool ratGreaterThan(struct rat *rat1, struct rat *rat2)
{
    if ((rat1->a > 0) && (rat2->a <= 0))
    {
        return true;
    }else if ((rat1->a < 0) && (rat2->a >= 0))
    {
        return false;
    }
    // a/b > c/d  <=>  ad > cd, because b and d are unsigned
    return ((long) rat1->a)*((long) rat2->b) > ((long) rat2->a)*((long) rat1->b);
}


struct rat ratsub(struct rat *rat1, struct rat *rat2)
{
    struct rat outrat;
    outrat.a = rat1->a * rat2->b - rat1->b * rat2->a;
    outrat.b = rat1->b * rat2->b;
    return outrat;
}

bool fm(size_t rows, size_t cols, signed char a[rows][cols], signed char c[rows])
{
// 1) Initialise
size_t r = rows;
size_t s = cols;
struct rat *t = malloc(r*s*sizeof(struct rat));
struct rat *q = malloc(r * sizeof(struct rat));
for (size_t i=0; i<r; i++)
{
    for (size_t j=0; j<s; j++)
    {
        t[i*s + j].a = a[i][j];
        t[i*s + j].b = 1;
    }
    q[i].a = c[i];
    q[i].b = 1;
}

while(true)//-------------loop
{

// 2) rearrange and 3) devide
size_t pos = 0, neg = 0, zer = 0;
for (size_t i = 0; i<r; i++)
{
    if (t[i*s + s-1].a < 0)
    {
        neg++;
    }
    else if (t[i*s + s-1].a > 0)
    {
        pos++;
    }
    else
    {
        zer++;
    }
}

struct rat *t_pos = malloc(pos * (s-1) * sizeof(struct rat));
struct rat *q_pos = malloc(pos * sizeof(struct rat));
struct rat *t_neg = malloc(neg * (s-1) * sizeof(struct rat));
struct rat *q_neg = malloc(neg * sizeof(struct rat));
struct rat *t_zer = malloc(zer * (s-1) * sizeof(struct rat));
struct rat *q_zer = malloc(zer * sizeof(struct rat));
size_t poscount = 0, negcount = 0, zercount = 0;
for (size_t i = 0; i<r; i++)
{
    struct rat *t_last = &t[i*s + s-1];
    if (t_last->a < 0)
    {
        // add row to t_neg
        for (size_t j = 0; j<s-1; j++)
        {
            t_neg[negcount*(s-1) + j] = ratdiv(&t[i*s + j], t_last);
        }
        q_neg[negcount] = ratdiv(&q[i], t_last);
        negcount++;
    }
    else if (t_last->a > 0)
    {
        // add rowq to t_pos
        for (size_t j = 0; j<s-1; j++)
        {
            t_pos[poscount*(s-1) + j] = ratdiv(&t[i*s + j], t_last);
        }
        q_pos[poscount] = ratdiv(&q[i], t_last);
        poscount++;
    }
    else
    {
        // add row to t_zer
        for (size_t j = 0; j<(s-1); j++)
        {
            t_zer[zercount*(s-1) + j] = t[i*s + j];
        }
        q_zer[zercount] = q[i];
        zercount++;
    }
}               
// free t and q
free(t);
free(q);

// 4) if s==1 (base case)
if (s == 1)
{
    struct rat b = {INT_MIN, 1};
    for (size_t i = 0; i<neg; i++)
    {
        if (ratGreaterThan(&q_neg[i], &b))
        {
            b = q_neg[i];
        }
    }
    
    struct rat B = {INT_MAX, 1};
    for (size_t i = 0; i<pos; i++)
    {
        if (!ratGreaterThan(&q_pos[i], &B)) //q<=B
        {
            B = q_pos[i];
        }
    }

    // if b > B
    if (ratGreaterThan(&b, &B))
    {
        free(t_pos);
        free(q_pos);
        free(t_neg);
        free(q_neg);
        free(t_zer);
        free(q_zer);
        return false;
    }

    // if q_zer[i] < 0 for some i
    /*for (size_t i = 0; i<zer; i++)
    {
        if (q_zer[i].a < 0)
        {
            free(t_pos);
            free(q_pos);
            free(t_neg);
            free(q_neg);
            free(t_zer);
            free(q_zer);
            return false;
        }
    }*/

    // else
    free(t_pos);
    free(q_pos);
    free(t_neg);
    free(q_neg);
    free(t_zer);
    free(q_zer);
    return true;

}
// 5) and 6) add all new equations
s -= 1;
r = zer + pos*neg;
if (r==0)
{
    free(t_pos);
    free(q_pos);
    free(t_neg);
    free(q_neg);
    free(t_zer);
    free(q_zer);
    return true;
}

struct rat *t_new = malloc((pos*neg + zer) * s * sizeof(struct rat));
struct rat *q_new = malloc((pos*neg + zer)*sizeof(struct rat));
int row_count = 0;
for (size_t i = 0; i<pos; i++)
{
    for (size_t j = 0; j<neg; j++)
    {
       // fix t
        for (size_t c = 0; c<s; c++)
        {
            t_new[row_count*s + c] = ratsub(&t_pos[i*s + c], &t_neg[j*s + c]);
        }
       //fix q
       q_new[row_count] = ratsub(&q_pos[i], &q_neg[j]);
       // increase row_count
       row_count++;
    }

}
// free t/q pos/neg
free(t_pos);
free(q_pos);
free(t_neg);
free(q_neg);

// continue fixing t and q
for (size_t i = 0; i<zer; i++)
{
    for(size_t c = 0; c<s; c++)
    {
        t_new[row_count*s + c] = t_zer[i*s + c];
    }
    q_new[row_count] = q_zer[i];
    row_count++;
}
// free t/q zer
free(t_zer);
free(q_zer);

// set new t and q
t = t_new;
q = q_new;


}//---------------- loop

////////////////////////////////////////////////////
//	return 0;
}
