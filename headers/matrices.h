#ifndef MATRICES_H
# define MATRICES_H

typedef struct s_matrix
{
    float   *matrix;
    int     num_of_cols;
    int     num_of_rows;
}           t_matrix;

t_matrix    *multiply_matrices(t_matrix *a, t_matrix *b);

#endif