#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define ARRAY_SIZE 40

int delta;

void bs(int n, int* vetor)
{
	int c = 0;
	int d;
	int troca;
	int trocou = 1;

	while ((c < (n-1)) & trocou )
	{
		trocou = 0;
		for (d = 0 ; d < n - c - 1; d++)
			if (vetor[d] > vetor[d+1])
			{
				troca      = vetor[d];
				vetor[d]   = vetor[d+1];
				vetor[d+1] = troca;
				trocou = 1;
			}
		c++;
	}
}

int* interleaving(int vetor[], int tam)
{
	int* vetor_auxiliar;
	int i1;
	int i2;
	int i_aux;

	vetor_auxiliar = malloc(tam * sizeof(int));

	i1 = 0;
	i2 = tam / 2;

	for (i_aux = 0; i_aux < tam; i_aux++) {
		if (((vetor[i1] <= vetor[i2]) && (i1 < (tam / 2))) || (i2 == tam))
			vetor_auxiliar[i_aux] = vetor[i1++];
		else
			vetor_auxiliar[i_aux] = vetor[i2++];
	}

	return vetor_auxiliar;
}

int print_vec(int* vec, int size)
{
	int i;

	printf("[ ");
	for (i = 0; i < size; i++)
		printf("%d ", vec[i] );
	printf("]\n");
	return 0;
}

int parent(int my_rank)
{
	return (my_rank - 1) / 2;
}

int left_child(int my_rank)
{
	return 2 * my_rank + 1;
}

int right_child(int my_rank)
{
	return 2 * my_rank + 2;
}

int a(void)
{
	int i;
	int j;
	int vec[ARRAY_SIZE];

	// Populate the vector
	for (i = 0, j = ARRAY_SIZE - 1; i < ARRAY_SIZE; i++, j--)
		vec[i] = j;

	int proc_n;
	int my_rank;

	delta = ARRAY_SIZE / (proc_n - 1);

	// MPI stuff
	/*MPI_Status status;*/
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

	// Set delta
	delta = ARRAY_SIZE / (proc_n - 1);

	if (ARRAY_SIZE <= delta)
	{
		bs(ARRAY_SIZE, vec);
	}
	else
	{
		int size = ARRAY_SIZE / 2;

		MPI_Send(&size,              1, MPI_INT, left_child(my_rank), 1, MPI_COMM_WORLD);
		MPI_Send(  vec, ARRAY_SIZE / 2, MPI_INT, left_child(my_rank), 1, MPI_COMM_WORLD);

		MPI_Send(               &size,              1, MPI_INT, right_child(my_rank), 1, MPI_COMM_WORLD);
		MPI_Send(vec + ARRAY_SIZE / 2, ARRAY_SIZE / 2, MPI_INT, right_child(my_rank), 1, MPI_COMM_WORLD);
	}

	return 0;
}

int b(void)
{
	int my_rank;
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	int size;
	MPI_Recv(&size, 1, MPI_INT, parent(my_rank), MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	int* vec = malloc(size * sizeof(int));

	if (!vec)
		return EXIT_FAILURE;

	MPI_Recv(vec, size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	printf("My rank is: %d and my vec is:\n", my_rank);
	print_vec(vec, size);

	free(vec);

	return 0;
}

int main(int argc, char** argv)
{
	int my_rank;
	int proc_n;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

	if (my_rank == 0)
		a();
	else
		b();

	MPI_Finalize();

	return 0;
}

