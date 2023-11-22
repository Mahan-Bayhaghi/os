#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

int** matrix1;
int** matrix2;
int** matrix3;
int** result;

int thread_size;
int m,k,n;

typedef struct {
    int row;
    int col;
} Index;

int pipefd[2];

void* write_to_pipe(void* arg){
	for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            Index index;
            index.row = i;
            index.col = j;
            write(pipefd[1], &index, sizeof(Index));
        }
	}
}

void* worker_thread(void* arg) {
    Index index;
    read(pipefd[0], &index, sizeof(Index));
    //printf("now read <%d,%d> \n",index.row, index.col);
    int row = index.row;
    int col = index.col;
    int sum = 0;
    for (int i = 0; i < k; i++)
        sum += matrix1[row][i] * matrix2[i][col];

    result[row][col] = sum;
    return NULL;
}


void normal_matrix_multiplication(int** A, int** B, int** C, int m,int k,int n) {
    for (int i=0; i<m; i++){
        for (int j=0; j<n; j++){
            int value = 0;
            for (int l=0; l<k; l++)
                value += A[i][l] * B[l][j];
            C[i][j] = value;
        }
    }
}

int** allocate_matrix(int row, int col){
    int** A = (int**) malloc(sizeof(int*) * row);
    for (int i=0; i<row; i++)
        A[i] = (int*) malloc(sizeof(int) * col);
    return A;
}

void initialize_matrix (int** A, int row, int col){
    for (int i=0; i<row; i++){
        for (int j=0; j<col; j++)
            A[i][j] = rand() % RAND_MAX;
    }
}

int main() {
	srand(time(0));
     scanf("%d" , &thread_size);
     scanf("%d %d %d",&m,&k,&n);
    //m = rand() % 900;
    //k = rand() % 900;
    //n = rand() % 256;
   //printf("%d %d %d \n",m,k,n);
    
    matrix1 = allocate_matrix(m,k);
    matrix2 = allocate_matrix(k,n);
    matrix3 = allocate_matrix(m,n);
    result = allocate_matrix(m,n);
    
    initialize_matrix(matrix1,m,k);
    initialize_matrix(matrix2,k,n);
    initialize_matrix(matrix3,m,n);
            
    pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t) * (thread_size+1));
    // pipe initialization handling
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }


    struct timeval start, end;
    gettimeofday(&start, NULL);
	
    pthread_create(&threads[thread_size], NULL, write_to_pipe, NULL);
    for (int i=0; i<thread_size; i++){
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

    for (int i=0; i<thread_size; i++){
        pthread_join(threads[i], NULL);
    }    

    gettimeofday(&end, NULL);
    double threaded_execution_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000.0;

    printf("Multi-threaded Multiplication Execution Time:\t %.6f ms\n", threaded_execution_time);
    
    gettimeofday(&start, NULL);
    normal_matrix_multiplication(matrix1,matrix2,matrix3,m,k,n);
    gettimeofday(&end, NULL);
    double normal_execution_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000.0;
    if (normal_execution_time < 0)
	    normal_execution_time *= -1.0;
    printf("Normal Multiplication Execution Time: \t\t %.6f ms \n", normal_execution_time);

    close(pipefd[0]);
    close(pipefd[1]);


    return 0;
}

