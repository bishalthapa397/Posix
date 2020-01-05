#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <time.h>
#include <pthread.h>

/******************************************************************************
  Demonstrates how to crack an encrypted password using a simple
  "brute force" algorithm. Works on passwords that consist only of 2 uppercase
  letters and a 2 digit integer. Your personalised data set is included in the
  code. 

  Compile with:
    cc -o Threadcw Threadcw.c -lcrypt -pthread

  If you want to analyse the results then use the redirection operator to send
  output to a file that you can view using an editor or the less utility:

    ./Threadcw > Threadcw_results.txt

  Dr Kevan Buckley, University of Wolverhampton, 2018
******************************************************************************/
int n_passwords = 4;

char *encrypted_passwords[] = {
  "$6$KB$3MiAO5oLs/.coZCPQ2QYOy8Ozo3v7QzGdwBEv3N7E0pJen3CJ63DmYXIZz6KEsykHmGsu3Dh1KCNe0niN0wvx/",
  "$6$KB$7rLS8BU8lh76q9iZ3Ogb8w1G45hmJUMoHdmOyHuQFUBqyr7XnEMUEs2wF4xGJRgQob7nC/RD9e1AKQZr/CKI30",
  "$6$KB$L4mWcpv6rMAbZdxfSsuAL2UZhbJ4vSGAAxk.vEcRKvIuPpwcSRKHzi3BXzWQWaH1p1ubwaFl.06CRQv6bVo3M1",
  "$6$KB$jM4o2O3EJI9OCoHvf8Jo0YG4JcnwEPFqpJINXb4RGEahSL5JRIQt1s2djLbGHThVv9IGzrYsS18XICkn5074./"
};

/**
 Required by lack of standard function in C.   
*/

void substr(char *dest, char *src, int start, int length){
  memcpy(dest, src + start, length);
  *(dest + length) = '\0';
}

/**
 This function can crack the kind of password explained above. All combinations
 that are tried are displayed and when the password is found, #, is put at the 
 start of the line. Note that one of the most time consuming operations that 
 it performs is the output of intermediate results, so performance experiments 
 for this kind of program should not include this. i.e. comment out the printfs.
*/

void *function()
{
   int i;
   pthread_t thread1, thread2;
    void *kernel_function_1();
   void *kernel_function_2();

   for(i=0;i<n_passwords;i<i++) {
     pthread_create(&thread1, NULL, kernel_function_1, encrypted_passwords[i]);
     pthread_create(&thread2, NULL, kernel_function_2, encrypted_passwords[i]);

     pthread_join(thread1, NULL);
     pthread_join(thread2, NULL);

  }
}

void *kernel_function_1(char *salt_and_encrypted){
  int r, s, t;     // Loop counters
  char salt[7];    // String used in hashing the password. Need space for \0
  char plain[7];   // The combination of letters currently being checked
  char *enc;       // Pointer to the encrypted password
  int count = 0;   // The number of combinations explored so far

  substr(salt, salt_and_encrypted, 0, 6);

  for(r='A'; r<='M'; r++){
    for(s='A'; s<='Z'; s++){
      for(t=0; t<=99; t++){
        sprintf(plain, "%c%c%02d", r, s, t); 
        enc = (char *) crypt(plain, salt);
        count++;
        if(strcmp(salt_and_encrypted, enc) == 0){
          printf("#%-8d%s %s\n", count, plain, enc);
        }
// else {
         // printf(" %-8d%s %s\n", count, plain, enc);
        //}
      }
    }
  }
  printf("%d solutions explored\n", count);
}
void *kernel_function_2(char *salt_and_encrypted){
  int x, y, z;     // Loop counters
  char salt[7];    // String used in hashing the password. Need space for \0
  char plain[7];   // The combination of letters currently being checked
  char *enc;       // Pointer to the encrypted password
  int count = 0;   // The number of combinations explored so far

  substr(salt, salt_and_encrypted, 0, 6);

  for(x='N'; x<='Z'; x++){
    for(y='A'; y<='Z'; y++){
      for(z=0; z<=99; z++){
        sprintf(plain, "%c%c%02d", x, y, z); 
        enc = (char *) crypt(plain, salt);
        count++;
        if(strcmp(salt_and_encrypted, enc) == 0){
          printf("#%-8d%s %s\n", count, plain, enc);
        } //else {
         // printf(" %-8d%s %s\n", count, plain, enc);
       // }
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

int main(int argc, char *argv[]){
  
struct timespec start, finish;   
  long long int time_elapsed;

  clock_gettime(CLOCK_MONOTONIC, &start);
 
  
  
    function();
  

  clock_gettime(CLOCK_MONOTONIC, &finish);
  time_difference(&start, &finish, &time_elapsed);
  printf("Time elapsed was %lldns or %0.9lfs\n", time_elapsed, 
         (time_elapsed/1.0e9)); 

  return 0;
}
