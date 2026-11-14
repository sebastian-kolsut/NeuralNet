#ifndef MATRICES_H
# define MATRICES_H

typedef struct s_matrix
{
    float   *data;
    int     num_of_cols;
    int     num_of_rows;
}           t_matrix;

t_matrix    *multiply_matrices(const t_matrix *a, const t_matrix *b);
t_matrix    *init_matrix(int row_c, int col_c);
void        print_matrix(t_matrix  *matrix);

#endif