#include "../headers/matrices.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static bool is_matrices_valid(t_matrix *a, t_matrix *b)
{
    if (!a || !b)
        return (false);
    if (a->num_of_cols != b->num_of_rows)
        return (false);
    return (true);
}

t_matrix    *create_results_matrix(t_matrix *a, t_matrix *b)
{
    t_matrix    *new;

    new = malloc(sizeof(t_matrix));
    new->matrix = malloc((a->num_of_rows * b->num_of_cols) * sizeof(float));
    new->num_of_cols = b->num_of_cols;
    new->num_of_rows = a->num_of_rows;
    return (new);
}

float   calculate_dot_product(t_matrix *a, t_matrix *b, int row_idx, int col_idx)
{
    int     i;
    int     a_idx;
    int     b_idx;
    float   sum;

    i = 0;
    sum = 0;
    while (i < a->num_of_cols)
    {
        a_idx = row_idx * a->num_of_cols + i;
        b_idx = i * b->num_of_cols + col_idx;
        sum += a->matrix[a_idx] * b->matrix[b_idx];
        i++;
    }
    return (sum);
}

void    multiply_row(t_matrix *a, t_matrix *b, t_matrix *result, int row_idx)
{
    int i;
    int res_idx;
    int first_cell_idx;

    first_cell_idx = row_idx * result->num_of_cols;
    i = 0;
    while (i < b->num_of_cols)
    {
        res_idx = first_cell_idx + i;
        result->matrix[res_idx] = calculate_dot_product(a, b, row_idx, i);
        i++;
    }
}

t_matrix    *multiply_matrices(t_matrix *a, t_matrix *b)
{
    int         i;
    t_matrix    *res;

    if (!is_matrices_valid(a, b))
        return (NULL);
    res = create_results_matrix(a, b);
    i = 0;
    while (i < a->num_of_rows)
    {
        multiply_row(a, b, res, i);
        i++;
    }
    return (res);
}

int main(void)
{
    t_matrix    *a;
    t_matrix    *b;
    t_matrix    *res;

    a = malloc(sizeof(t_matrix));
    a->matrix = (float[]){
        1.0f, 2.0f, 3.0f, 4.0f,
        2.0f, 0.0f, 1.0f, 2.0f
    };
    a->num_of_rows = 2;
    a->num_of_cols = 4;

    b = malloc(sizeof(t_matrix));
    b->matrix = (float[]){
        2.0f,
        1.0f,
        0.0f,
        3.0f
    };
    b->num_of_rows = 4;
    b->num_of_cols = 1;

    res = multiply_matrices(a, b);
    if (!res)
    {
        printf("FAIL\n");
        return (0);
    }
    int i = 0;
    while (i < res->num_of_cols * res->num_of_rows)
    {
        printf("%f, ", res->matrix[i]);
        i++;
        if (i % res->num_of_cols == 0)
            printf("\n");
    }
    return (0);
}
