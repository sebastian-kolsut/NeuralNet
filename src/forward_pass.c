#include "../headers/neuron_layer.h"
#include <stdbool.h>
#include <stdlib.h>

static bool is_input_size_valid(int input_size, const t_layer *neural_network)
{
	int	input_layer_size;

	input_layer_size = neural_network->matrix->num_of_cols *
		neural_network->matrix->num_of_rows;
	return (input_layer_size == input_size);
}

static void	insert_input_to_neural_net(const float *input, t_layer *neural_net,
	int input_size)
{
	int	i;

	i = 0;
	while (i < input_size)
	{
		neural_net->matrix->data[i] = input[i];
		i++;
	}
}

static void	add_biases(t_layer *layer)
{
	int i;
	int	nodes;

	nodes = layer->matrix->num_of_rows;
	while (i < nodes)
	{
		layer->matrix->data[i] += layer->biases[i];
		i++;
	}
}

void	forward_pass(t_layer *neural_network, float *input, int input_size)
{
	t_layer		*curr_l;
	t_weights	*curr_w;

	if (!is_input_size_valid(input_size, neural_network))
		return (NULL);
	insert_input_to_neural_net(input, neural_network, input_size);
	// free(input);
	curr_l = neural_network;
	curr_w = curr_l->next;
	while (curr_w->next)
	{
		multiply_matrices(curr_w->matrix, curr_l->matrix, curr_w->next->matrix);
		curr_l = curr_w->next;
		add_biases(curr_l);
		curr_w = curr_l->next;
	}
}