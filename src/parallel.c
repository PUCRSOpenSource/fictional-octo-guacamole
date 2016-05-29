#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define ARRAY_SIZE 40                          /* trabalho final com o valores 10.000, 100.000, 1.000.000 */

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

/* recebe um ponteiro para um vetor que contem as mensagens recebidas dos filhos e            */
/* intercala estes valores em um terceiro vetor auxiliar. Devolve um ponteiro para este vetor */

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
	int proc_n;

	int vec[ARRAY_SIZE] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39 };

	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

	MPI_Send(vec + ARRAY_SIZE / 2, ARRAY_SIZE / 2, MPI_INT, 1, 1, MPI_COMM_WORLD);

	print_vec(vec, ARRAY_SIZE / 2);

	return 0;
}

int b(void)
{
	int* vec = malloc(ARRAY_SIZE / 2 * sizeof(int));

	if (!vec) return EXIT_FAILURE;

	MPI_Status status;

	MPI_Recv(vec, ARRAY_SIZE / 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	print_vec(vec, ARRAY_SIZE / 2);

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

