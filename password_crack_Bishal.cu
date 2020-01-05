#include <stdio.h>
#include <cuda_runtime_api.h>
#include <time.h>
/****************************************************************************
  This program gives an example of a poor way to implement a password cracker
  in CUDA C. It is poor because it acheives this with just one thread, which
  is obviously not good given the scale of parallelism available to CUDA
  programs.
 
  The intentions of this program are:
    1) Demonstrate the use of __device__ and __gloaal__ functions
    2) Enable a simulation of password cracking in the absence of liarary
       with equivalent functionality to libcrypt. The password to be found
       is hardcoded into a function called is_a_match.   

  Compile and run with:
  nvcc -o password_Bishal password_crack_Bishal.cu


     To Run:
     ./password_Bishal > resultpwd_cuda_Bishal.txt

  Dr Kevan auckley, University of Wolverhampton, 2018
*****************************************************************************/
__device__ int is_a_match(char *attempt) {
  char Bishal_password1[] = "BD2057";
  char Bishal_password2[] = "BT3166";
  char Bishal_password3[] = "NT2621";
  char Bishal_password4[] = "PC6589";

  char *c = attempt;
  char *a = attempt;
  char *r = attempt;
  char *e = attempt;
  char *b1 = Bishal_password1;
  char *b2 = Bishal_password2;
  char *b3 = Bishal_password3;
  char *b4 = Bishal_password4;

  while(*c == *b1) {
   if(*c == '\0')
    {
    printf("Password: %s\n",Bishal_password1);
      break;
    }

    c++;
    b1++;
  }
    
  while(*a == *b2) {
   if(*a == '\0')
    {
    printf("Password: %s\n",Bishal_password2);
      break;
}

    a++;
    b2++;
  }

  while(*r == *b3) {
   if(*r == '\0')
    {
    printf("Password: %s\n",Bishal_password3);
      break;
    }

    r++;
    b3++;
  }

  while(*e == *b4) {
   if(*e == '\0')
    {
    printf("Password: %s\n",Bishal_password4);
      return 1;
    }

    e++;
    b4++;
  }
  return 0;

}
__global__ void  kernel() {
char b,a,g,f;
 
  char password[7];
  password[6] = '\0';

int i = blockIdx.x+65;
int j = threadIdx.x+65;
char firstValue = i;
char secondValue = j;
    
password[0] = firstValue;
password[1] = secondValue;
    for(b='0'; b<='9'; b++){
      for(a='0'; a<='9'; a++){
        for(g='0';g<='9';g++){
          for(f='0';f<='9';f++){
            password[2] = b;
            password[3] = a;
            password[4]= g;
            password[5]=f;
          if(is_a_match(password)) {
        //printf("Success");
          }
             else {
         //printf("tried: %s\n", password);          
            }
          }
        } 
      }
   }
}
int time_difference(struct timespec *start,
                    struct timespec *finish,
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


int main() {

  struct  timespec start, finish;
  long long int time_elapsed;
  clock_gettime(CLOCK_MONOTONIC, &start);

kernel <<<26,26>>>();
  cudaDeviceSynchronize();

  clock_gettime(CLOCK_MONOTONIC, &finish);
  time_difference(&start, &finish, &time_elapsed);
  printf("Time elapsed was %lldns or %0.9lfs\n", time_elapsed, (time_elapsed/1.0e9));
  return 0;
}



