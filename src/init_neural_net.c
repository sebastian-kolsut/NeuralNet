#include "../headers/neuron_layer.h"
#include <stdlib.h>
#include <stdio.h>

static void	populate_array(float *array, float value, int lenght)
{
	int	i;

	i = 0;
	while (i < lenght)
	{
		array[i] = value;
		i++;
	}
}

static t_layer	*init_input_layer(int count)
{
	t_layer	*input_layer;

	input_layer = malloc(sizeof(t_layer));
	if (!input_layer)
		return (NULL);
	input_layer->matrix = init_matrix(count, 1);
	populate_array(input_layer->matrix->data, 0.0f, count);
	input_layer->biases = NULL;
	input_layer->next = NULL;
	input_layer->prev = NULL;
	return (input_layer);
}

static t_layer	*init_layer(int nodes)
{
	t_layer	*layer;

	layer = malloc(sizeof(t_layer));
	layer->matrix = init_matrix(nodes, 1);
	populate_array(layer->matrix->data, 0.0f, nodes);
	layer->biases = malloc(nodes * sizeof(float));
	populate_array(layer->biases, 0.1f, nodes);
	layer->next = NULL;
	layer->prev = NULL;
	return (layer);
}

static t_weights	*init_weights(int row_c, int col_c)
{
	t_weights	*new;

	new = malloc(sizeof(t_weights));
	new->matrix = init_matrix(row_c, col_c);
	populate_array(new->matrix->data, 0.1f, row_c * col_c);
	new->prev = NULL;
	new->next = NULL;
	return (new);
}

static void	connect_layers(t_layer *prev, t_layer *curr, t_weights *weights)
{
	prev->next = weights;
	weights->prev = prev;
	weights->next = curr;
	curr->prev = weights;
}

static void	create_hidden_layers(t_layer *neural_net, int hidden_layers,
	int nodes_count)
{
	t_layer		*temp_l;
	t_layer		*curr;
	t_weights	*temp_w;
	int			i;

	i = 0;
	curr = neural_net;
	while (i < hidden_layers)
	{
		temp_l = init_layer(nodes_count);
		temp_w = init_weights(temp_l->matrix->num_of_rows,
			curr->matrix->num_of_rows);
		connect_layers(curr, temp_l, temp_w);
		curr = temp_l;
		i++;
	}
}

static t_layer	*get_last_layer(t_layer *neural_net)
{
	t_layer		*curr_l;
	t_weights	*curr_w;

	curr_l = neural_net;
	while (curr_l->next)
	{
		curr_w = curr_l->next;
		curr_l = curr_w->next;
	}
	return (curr_l);
}

static void	create_and_connect_output_layer(int count, t_layer *neural_net)
{
	t_layer		*output_layer;
	t_layer		*last_layer;
	t_weights	*weights;

	output_layer = init_layer(count);
	last_layer = get_last_layer(neural_net);
	weights = init_weights(output_layer->matrix->num_of_rows,
		last_layer->matrix->num_of_rows);
	connect_layers(last_layer, output_layer, weights);
}

void	print_neural_network(t_layer *neural_network)
{
	t_layer		*curr_l;
	t_weights	*curr_w;

	curr_l = neural_network;
	while (curr_w)
	{
		printf("\nLayer:\n");
		print_matrix(curr_l->matrix);
		curr_w = curr_l->next;
		if (!curr_w)
			break;
		printf("\nWeights:\n");
		print_matrix(curr_w->matrix);
		curr_l = curr_w->next;
	}
}

t_layer	*init_neural_net(int input, int output, int hidden_layers,
    int nodes_count)
{
	t_layer	*neural_net;

	neural_net = init_input_layer(input);
	create_hidden_layers(neural_net, hidden_layers, nodes_count);
	create_and_connect_output_layer(output, neural_net);
	return (neural_net);
}
