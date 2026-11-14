#include "../headers/matrices.h"
#include <stdlib.h>
#include <stdio.h>

t_matrix    *init_matrix(int row_c, int col_c)
{
    t_matrix    *new;

    new = malloc(sizeof(t_matrix));
    if (!new)
        return (NULL);
    new->data = malloc((row_c * col_c) * sizeof(float));
    if (!new)
        return (NULL);
    new->num_of_cols = col_c;
    new->num_of_rows = row_c;
    return (new);
}

void    print_matrix(t_matrix  *matrix)
{
    int i;

    i = 0;
    while (i < matrix->num_of_cols * matrix->num_of_rows)
    {
        printf("%f ", matrix->data[i]);
        i++;
        if (i % matrix->num_of_cols == 0)
            printf("\n");
    }
}