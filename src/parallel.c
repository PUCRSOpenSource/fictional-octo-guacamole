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

	while (c < (n-1) & trocou )
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

/*
int* vetor_auxiliar;         // ponteiro para o vetor resultantes que sera alocado dentro da rotina

vetor_aux = interleaving(vetor, tam);
*/

int main(int argc, char** argv)
{
	int my_rank;
	int proc_n;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

	//Do what we need to do
	printf("My rank is %d\n", my_rank);

	MPI_Finalize();

	return 0;
}

