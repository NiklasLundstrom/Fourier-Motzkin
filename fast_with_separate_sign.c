#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include "error.h"
#include <stdio.h>

void bp(){};

void end()
{
//    printf("had to reduce in ratdiv %d times\n", reduce_count);
};
struct rat
{
    unsigned long int a, b; // a/b
    bool sign; // 1: neg, 0: pos
};

struct rat ratdiv(struct rat *rat1, struct rat *rat2)
{
    // TODO use pointer to output instead of actually returning it
    struct rat outrat;
        outrat.a = rat1->a;// * ( (rat2->a > 0) - (rat2->a < 0) ); // * sign(rat2->a)
        outrat.b = rat2->a;//labs(rat2->a);
        outrat.sign = rat1->sign ^ rat2->sign;
        return outrat;
}

bool ratGreaterThan(struct rat *rat1, struct rat *rat2)
{
    // TODO division is slow!
    //return ((float) rat1->a)/((float) rat1->b) > ((float) rat2->a)/((float) rat2->b);
    if (rat1->sign ^ rat2->sign)
    {
        return rat2->sign; //1 is positive
    }
    if ( (rat2->a >= rat2->b) & (rat1->a <= rat1->b) )
    {
        return rat2->sign;
    }
    if ( (rat2->a <= rat2->b) & (rat1->a >= rat1->b) )
    {
        return !rat1->sign; //if both are negative it's false
    }
    if ( (rat1->b < rat2->b) & (rat1->a > rat2->a) )
    {
        return !rat1->sign;
    }
   // TODO Not recursive!
    struct rat rat_new = {rat1->a * rat2->b, rat1->b * rat2->a, rat1->sign};
    struct rat one = {1, 1, rat2->sign};
    return ratGreaterThan(&rat_new, &one);

    //bool G =  (((float) rat1->a)/((float) rat1->b) > ((float) rat2->a)/((float) rat2->b) );
    //return (G & (!rat1->sign)) | ( (!G) & rat2->sign);
}


struct rat ratsub(struct rat *rat1, struct rat *rat2)
{
    struct rat outrat;
    // TODO is it possible to speed this up?
    unsigned int x = rat1->a * rat2->b;
    unsigned int y = rat1->b * rat2->a;
    if (rat1->sign ^ rat2->sign)
    {
        outrat.a = x + y;
    }
    else
    {
        // TODO max/min istället
        outrat.a = (x>y) ? x-y : y-x;
    }
    outrat.b = rat1->b * rat2->b;
    outrat.sign = (x!=y) & ( (x>y) ? rat1->sign : ~rat2->sign );
    return outrat;
}

bool fm(size_t rows, size_t cols, signed char a[rows][cols], signed char c[rows])
{
// 1) Initialise
        size_t r = rows;
        size_t s = cols;
    if((r==6) && (s==1))
    {
        bp();
    }
        struct rat *t = (struct rat *) calloc(r*s, sizeof(struct rat));//malloc(r*s*sizeof(struct rat));
        struct rat *q = (struct rat *) calloc(r, sizeof(struct rat));//malloc(r * sizeof(struct rat));
        for (size_t i=0; i<r; i++)
        {
            for (size_t j=0; j<s; j++)
            {
                //TODO memcpy eller nåt?
                t[i*s + j].a = abs(a[i][j]);
                t[i*s + j].b = 1;
                t[i*s + j].sign = (a[i][j] < 0);
            }
            q[i].a = abs(c[i]);
            q[i].b = 1;
            q[i].sign = (c[i] < 0);
        }
while(true)//-------------loop
{

// 2) rearrange and 3) devide
size_t pos = 0, neg = 0, zer = 0;
for (size_t i = 0; i<r; i++)
{
    if (t[i*s + s-1].sign)
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

struct rat *t_pos = (struct rat *) calloc(pos * (s-1), sizeof(struct rat));
struct rat *q_pos = (struct rat *) calloc(pos , sizeof(struct rat));
struct rat *t_neg = (struct rat *) calloc(neg * (s-1) , sizeof(struct rat));
struct rat *q_neg = (struct rat *) calloc(neg , sizeof(struct rat));
struct rat *t_zer = (struct rat *) calloc(zer * (s-1) , sizeof(struct rat));
struct rat *q_zer = (struct rat *) calloc(zer , sizeof(struct rat));
size_t poscount = 0, negcount = 0, zercount = 0;
for (size_t i = 0; i<r; i++)
{
    struct rat *t_last = &t[i*s + s-1];
    //TODO slow
    if (t_last->sign)
    {
        // add row to t_neg
        for (size_t j = 0; j<s-1; j++)
        {
            t_neg[negcount*(s-1) + j] = ratdiv(&t[i*s + j], t_last);
            //reduce(&t_pos[poscount*(s-1) + j]);
        }
        // TODO varför tar det här lång tid??
        q_neg[negcount] = ratdiv(&q[i], t_last);
        //reduce(&q_pos[poscount]);
        negcount++;
    }
    else if (t_last->a > 0)
    {
        // add rowq to t_pos
        for (size_t j = 0; j<s-1; j++)
        {
            t_pos[poscount*(s-1) + j] = ratdiv(&t[i*s + j], t_last);
            //reduce(&t_neg[negcount*(s-1) + j]);
        }
        q_pos[poscount] = ratdiv(&q[i], t_last);
        //reduce(&q_neg[negcount]);
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
    struct rat b = {INT_MAX, 1, true};
    for (size_t i = 0; i<neg; i++)
    {
        if (ratGreaterThan(&q_neg[i], &b))
        {
            b = q_neg[i];
	        /*if (b.b < 0)
	        {
	            error("b is negative, in 4) b");
	        }*/
        }
    }
    
    struct rat B = {INT_MAX, 1, false};
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
        end();
        return false;
    }

    // q_zer[i] < 0 for some i
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
            end();
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
    end();
    return true;

}
// 5) and 6) add all new equations
s = s-1;
r = zer + pos*neg;
if (r==0)
{
    free(t_pos);
    free(q_pos);
    free(t_neg);
    free(q_neg);
    free(t_zer);
    free(q_zer);
    end();
    return true;
}

struct rat *t_new = (struct rat *) calloc((pos*neg + zer) * s , sizeof(struct rat));
struct rat *q_new = (struct rat *) calloc((pos*neg + zer),sizeof(struct rat));
int row_count = 0;
for (size_t i = 0; i<pos; i++)
{
    for (size_t j = 0; j<neg; j++)
    {
       // fix t
        for (size_t c = 0; c<s; c++)
        {
            //TODO slow
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
	return 0;
}
// TODO:It works :) optimise by checking operf ./fm, opreport -rl. 
// is it the function reduce itself that is slow, or is it called to often?
