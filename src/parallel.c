#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/*#define DEBUG 1*/
#define ARRAY_SIZE 100000

void
bs (int n, int* vetor)
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

int*
interleaving (int vetor[], int tam)
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

int
print_vec (int* vec, int size)
{
  int i;

  printf("[ ");
  for (i = 0; i < size; i++)
    printf("%d ", vec[i] );
  printf("]\n");
  return 0;
}

int
parent (int my_rank)
{
  return (my_rank - 1) / 2;
}

int
left_child (int my_rank)
{
  return 2 * my_rank + 1;
}

int
right_child (int my_rank)
{
  return 2 * my_rank + 2;
}

int
root (void)
{
  double t1,t2;
  t1 = MPI_Wtime();

  int i;
  int j;
  int vec[ARRAY_SIZE];

  // Populate the vector
  for (i = 0, j = ARRAY_SIZE - 1; i < ARRAY_SIZE; i++, j--)
    vec[i] = j;

  // MPI stuff
  int proc_n;
  int my_rank;
  MPI_Status status;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

  // Set delta
  int delta = ARRAY_SIZE / ((proc_n + 1) / 2);

#ifdef DEBUG
  printf("Vector size: %d\n", ARRAY_SIZE);
  printf("Delta: %d\n", delta);
#endif

  if (ARRAY_SIZE <= delta)
    {
      bs(ARRAY_SIZE, vec);
#ifdef DEBUG
      print_vec(vec, ARRAY_SIZE);
#endif
    }
  else
    {
      int size = ARRAY_SIZE / 2;

      // Sending message to the children
      MPI_Send(&size,    1, MPI_INT, left_child(my_rank), 1, MPI_COMM_WORLD);
      MPI_Send(  vec, size, MPI_INT, left_child(my_rank), 1, MPI_COMM_WORLD);

      MPI_Send(     &size,    1, MPI_INT, right_child(my_rank), 1, MPI_COMM_WORLD);
      MPI_Send(vec + size, size, MPI_INT, right_child(my_rank), 1, MPI_COMM_WORLD);

      // Receiving message from the children
      MPI_Recv(       vec, size, MPI_INT,  left_child(my_rank), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Recv(vec + size, size, MPI_INT, right_child(my_rank), MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      int* ans = interleaving(vec, ARRAY_SIZE);

#ifdef DEBUG
      print_vec(ans, ARRAY_SIZE);
#endif

      free(ans);
    }

  t2 = MPI_Wtime();
  fprintf(stderr, "Time: %fs\n\n", t2-t1);

  return 0;
}

int
child (void)
{
  // MPI stuff
  int proc_n;
  int my_rank;
  MPI_Status status;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

  int size;
  MPI_Recv(&size, 1, MPI_INT, parent(my_rank), MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  int* vec = malloc(size * sizeof(int));

  if (!vec)
    return EXIT_FAILURE;

  MPI_Recv(vec, size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

#ifdef DEBUG
  printf("My rank is: %d and my vec size is: %d\n", my_rank, size);
#endif

  // Set delta
  int delta = ARRAY_SIZE / ((proc_n + 1) / 2);

  if (size <= delta)
    {
      bs(size, vec);
      MPI_Send(vec, size, MPI_INT, parent(my_rank), 1, MPI_COMM_WORLD);
    }
  else
    {
      int child_size = size / 2;

      // Sending message to the children
      MPI_Send(&child_size,          1, MPI_INT, left_child(my_rank), 1, MPI_COMM_WORLD);
      MPI_Send(        vec, child_size, MPI_INT, left_child(my_rank), 1, MPI_COMM_WORLD);

      MPI_Send(     &child_size,          1, MPI_INT, right_child(my_rank), 1, MPI_COMM_WORLD);
      MPI_Send(vec + child_size, child_size, MPI_INT, right_child(my_rank), 1, MPI_COMM_WORLD);

      // Receiving message from the children
      MPI_Recv(             vec, child_size, MPI_INT,  left_child(my_rank), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Recv(vec + child_size, child_size, MPI_INT, right_child(my_rank), MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      int* ans = interleaving(vec, size);

      MPI_Send(ans, size, MPI_INT, parent(my_rank), 1, MPI_COMM_WORLD);

      free(ans);
    }

  free(vec);

  return 0;
}

int
main (int argc, char** argv)
{
  int my_rank;
  int proc_n;

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

  if (my_rank == 0)
    root();
  else
    child();

  MPI_Finalize();

  return 0;
}

