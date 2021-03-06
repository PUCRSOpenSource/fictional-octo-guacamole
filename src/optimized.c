#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/*#define DEBUG 1*/
#define ARRAY_SIZE 100000
#define DELTA(proc_n) (ARRAY_SIZE / proc_n)
#define VALID(i, lim_i) (i < lim_i)
#define HOI(array, i, lim_i, next_i) (array[i] > array[next_i] || !VALID(i, lim_i))

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
	    trocou     = 1;
	  }
      c++;
    }
}

int*
interleaving (int vetor[], int tam, int delta)
{
  int* vetor_auxiliar;
  int i1;
  int lim_i1;
  int i2;
  int lim_i2;
  int i3;
  int i_aux;

  vetor_auxiliar = malloc(tam * sizeof(int));

  int child_size = (tam - delta) / 2;

  i1     = 0;
  lim_i1 = child_size;
  i2     = child_size;
  lim_i2 = child_size * 2;
  i3     = child_size * 2;

  for (i_aux = 0; i_aux < tam; i_aux++) {
    if ((VALID(i1, lim_i1)) && HOI(vetor, i2, lim_i2, i1) &&  HOI(vetor, i3, tam, i1))
      {
	vetor_auxiliar[i_aux] = vetor[i1++];
      }
    else
      {
	if (VALID(i2, lim_i2) &&  HOI(vetor, i3, tam, i2))
	  {
	    vetor_auxiliar[i_aux] = vetor[i2++];
	  }
	else
	  {
	    vetor_auxiliar[i_aux] = vetor[i3++];
	  }
      }
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
  int delta = DELTA(proc_n);

#ifdef DEBUG
  printf("Vector size: %d\n", ARRAY_SIZE);
  printf("Delta: %d\n", delta);
  printf("My rank is: %d and my vec size is: %d\n", my_rank, ARRAY_SIZE);
#endif

  if (ARRAY_SIZE < 2 * delta)
    {
      bs(ARRAY_SIZE, vec);
#ifdef DEBUG
      print_vec(vec, ARRAY_SIZE);
#endif
    }
  else
    {
      int child_size = (ARRAY_SIZE - delta) / 2;

      // Sending message to the children
      MPI_Send(&child_size,    1, MPI_INT, left_child(my_rank), 1, MPI_COMM_WORLD);
      MPI_Send(  vec, child_size, MPI_INT, left_child(my_rank), 1, MPI_COMM_WORLD);

      MPI_Send(     &child_size,          1, MPI_INT, right_child(my_rank), 1, MPI_COMM_WORLD);
      MPI_Send(vec + child_size, child_size, MPI_INT, right_child(my_rank), 1, MPI_COMM_WORLD);

      bs(ARRAY_SIZE - 2 * child_size, vec + 2 * child_size);

      // Receiving message from the children
      MPI_Recv(             vec, child_size, MPI_INT,  left_child(my_rank), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Recv(vec + child_size, child_size, MPI_INT, right_child(my_rank), MPI_ANY_TAG, MPI_COMM_WORLD, &status);

#ifdef DEBUG
      printf("Rank %d: Before interleaving\n", my_rank);
      print_vec(vec, ARRAY_SIZE);
#endif

      int* ans = interleaving(vec, ARRAY_SIZE, delta);

#ifdef DEBUG
      printf("Rank %d: After interleaving\n", my_rank);
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
  int delta = DELTA(proc_n);

  if (size <= 2 * delta)
    {
      bs(size, vec);
      MPI_Send(vec, size, MPI_INT, parent(my_rank), 1, MPI_COMM_WORLD);
    }
  else
    {
      int child_size = (size - delta) / 2;

      // Sending message to the children
      MPI_Send(&child_size,          1, MPI_INT, left_child(my_rank), 1, MPI_COMM_WORLD);
      MPI_Send(        vec, child_size, MPI_INT, left_child(my_rank), 1, MPI_COMM_WORLD);

      MPI_Send(     &child_size,          1, MPI_INT, right_child(my_rank), 1, MPI_COMM_WORLD);
      MPI_Send(vec + child_size, child_size, MPI_INT, right_child(my_rank), 1, MPI_COMM_WORLD);

      bs(size - 2 * child_size, vec + 2 * child_size);

      // Receiving message from the children
      MPI_Recv(             vec, child_size, MPI_INT,  left_child(my_rank), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Recv(vec + child_size, child_size, MPI_INT, right_child(my_rank), MPI_ANY_TAG, MPI_COMM_WORLD, &status);

#ifdef DEBUG
      printf("Rank %d: Before interleaving\n", my_rank);
      print_vec(vec, size);
#endif

      int* ans = interleaving(vec, size, delta);

#ifdef DEBUG
      printf("Rank %d: After interleaving\n", my_rank);
      print_vec(vec, size);
#endif

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

