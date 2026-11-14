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

static t_matrix    *create_results_matrix(const t_matrix *a, const t_matrix *b)
{
    t_matrix    *new;

    new = init_matrix(a->num_of_rows, b->num_of_cols);
    return (new);
}

static float   calculate_dot_product(const t_matrix *a, const t_matrix *b,
    int row_idx, int col_idx)
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
        sum += a->data[a_idx] * b->data[b_idx];
        i++;
    }
    return (sum);
}

static void    multiply_row(const t_matrix *a, const t_matrix *b,
    t_matrix *result, int row_idx)
{
    int i;
    int res_idx;
    int first_cell_idx;

    first_cell_idx = row_idx * result->num_of_cols;
    i = 0;
    while (i < b->num_of_cols)
    {
        res_idx = first_cell_idx + i;
        result->data[res_idx] = calculate_dot_product(a, b, row_idx, i);
        i++;
    }
}

t_matrix    *multiply_matrices(const t_matrix *a, const t_matrix *b)
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
