#ifndef NEURON_LAYER_H
# define NEURON_LAYER_H
# include "matrices.h"

typedef struct s_layer t_layer;

typedef struct s_weights
{
    t_layer     *prev;
    t_layer     *next;
    t_matrix    *matrix;
}               t_weights;

typedef struct s_layer
{
    t_weights   *prev;
    t_weights   *next;
    t_matrix    *matrix;
    float       *biases;
}               t_layer;

t_layer *init_neural_net(int input, int output, int hidden_layers,
    int nodes_count);
void	print_neural_network(t_layer *neural_network);

#endif
