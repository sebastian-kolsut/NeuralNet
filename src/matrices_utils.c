#include "../headers/matrices.h"
#include <stdlib.h>

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