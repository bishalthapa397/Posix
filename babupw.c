
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <crypt.h>
#include <mpi.h>

/*****************************************************************************
The variable names and the function names of this program is same as provided by the university.
The added variable and function are the only changes made to this program.
  

To compile:
     mpicc -o babupw babupw.c -lrt -lcrypt
     
  To run 3 processes on this computer:
    mpirun -n 3 ./babupw >result.txt
*****************************************************************************/


int n_passwords = 4;
pthread_t t1, t2;
char *encrypted_passwords[] = {
"$6$KB$/qYEV93N/mW7XZ14YzQf07RSkZmhs3S6rwb06oKRQ9eNk6cg1x3NcnFp7uIbi30hTX6ZNb04xsyyVTrCWIrTW.",
  "$6$KB$pWbhk/RfwwSRZIfvVB16Wxu3e8UWxGwQSwWpLAw4qOo.2a.oiPowz2ZHby52Noh08WPUqTJO5Nny.phpFiq011",
  "$6$KB$C9g/Ym0aQ6rOdFNoa4gtUST0onsUWY70W/u/aA3x1ny5url8EzAA.sI8pYI9le9mYBk5cDTg1K2qmNXtZ7Kd/0",
  "$6$KB$8VSDL//Ale6pVSBrwWoRfTpuluOK1xf.uW6Ay6ba0jmQM4pZpFsPTI6DO7z/yLdZKGZvkQjLT6jHzJ2SiDzZi1"
};

void substr(char *dest, char *src, int start, int length){
  memcpy(dest, src + start, length);
  *(dest + length) = '\0';
}
  
void kernel_function1(char *salt_and_encrypted){
  int i, o, u;     
  char salt[7];    

  char plain[7];   
  char *enc;       
  int count = 0;   

  substr(salt, salt_and_encrypted, 0, 6);
  
  for(i='A'; i<='M'; i++){
    for(o='A'; o<='Z'; o++){
      for(u=0; u<=9999; u++){
	//printf("Instance 1:");
	sprintf(plain, "%c%c%02d",i, o, u);
	enc = (char *) crypt(plain, salt);
	count++;
	if(strcmp(salt_and_encrypted, enc) == 0){
	  printf("#%-8d%s %s\n", count, plain, enc);
	} else {
      }
}
    }
  }
  printf("%d solutions explored\n", count);
}
void kernel_function2(char *salt_and_encrypted){
  int p, q, r;     
  char salt[7];    

  char plain[7];   
  char *enc;       
  int count = 0;  

  substr(salt, salt_and_encrypted, 0, 6);
  
  for(p='N'; p<='Z'; p++){
    for(q='A'; q<='Z'; q++){
      for(r=0; r<=9999; r++){
	//printf("Instance 2:");
	sprintf(plain, "%c%c%02d",p, q, r);
	enc = (char *) crypt(plain, salt);
	count++;
	if(strcmp(salt_and_encrypted, enc) == 0){
	  printf("#%-8d%s %s\n", count, plain, enc);
	} else {
      }
}
    }
  }
  printf("%d solutions explored\n", count);
}


int time_difference(struct timespec *start, struct timespec *finish,
                    long long int *difference) {
  long long int ds =  finish->tv_sec - start->tv_sec; 
  long long int dn =  finish->tv_nsec - start->tv_nsec; 

  if(dn < 0 ) {
    ds--;
    dn += 1000000000; 
  } 
  *difference = ds * 1000000000 + dn;
  return !(*difference > 0);
}

int main(int argc, char** argv) {
 struct timespec start, finish;   
  long long int time_elapsed;

  clock_gettime(CLOCK_MONOTONIC, &start);

 
  int size, rank;
int i;

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if(size != 3) {
    if(rank == 0) {
      printf("This program needs to run on exactly 3 processes\n");
    }
  } else {
    if(rank ==0){

      int x;
    
      MPI_Send(&x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
      MPI_Send(&x, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);


 
    } else if(rank==1){
      int number;
      MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, 
                         MPI_STATUS_IGNORE);
        for(i=0;i<n_passwords;i<i++) {
          kernel_function1(encrypted_passwords[i]);
        }
      }
      else{
      int number;
      MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, 
                         MPI_STATUS_IGNORE);
        for(i=0;i<n_passwords;i<i++) {
          kernel_function2(encrypted_passwords[i]);
        }
      }
    }
    MPI_Finalize(); 
 clock_gettime(CLOCK_MONOTONIC, &finish);
  time_difference(&start, &finish, &time_elapsed);
  printf("Time elapsed was %lldns or %0.9lfs\n", time_elapsed,
         (time_elapsed/1.0e9)); 

  return 0;
}
