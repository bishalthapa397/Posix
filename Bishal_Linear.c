#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>


/******************************************************************************
 * This program takes an initial estimate of m and c and finds the associated 
 * rms error. It is then as a base to generate and evaluate 8 new estimates, 
 * which are steps in different directions in m-c space. The best estimate is 
 * then used as the base for another iteration of "generate and evaluate". This 
 * continues until none of the new estimates are better than the base. This is
 * a gradient search for a minimum in mc-space.
 * 
 * To compile:
 *   mpicc -o Bishal_Linear Bishal_Linear.c -lm
 * 
 * To run:
 *   mpirun -n 9 ./Bishal_Linear
 * 
 * Dr Kevan Buckley, University of Wolverhampton, 2018
 *****************************************************************************/

typedef struct point_t {
  double x;
  double y;
} point_t;

int n_data = 1000;
point_t data[];

double residual_error(double x, double y, double m, double c) {
  double e = (m * x) + c - y;
  return e * e;
}

double rms_error(double m, double c) {
  int i;
  double mean;
  double error_sum = 0;
  
  for(i=0; i<n_data; i++) {
    error_sum += residual_error(data[i].x, data[i].y, m, c);  
  }
  
  mean = error_sum / n_data;
  
  return sqrt(mean);
}

int timedifference(struct timespec *start, struct timespec *finish, long long int *difference) {
   long long int dsec = finish->tv_sec - start->tv_sec;
   long long int dnsec = finish->tv_nsec - start->tv_nsec;

	if(dnsec < 0) {
		dsec--;
		dnsec += 1000000000; 
	}

   *difference = dsec * 1000000000 + dnsec;
   return !(*difference > 0);
}

int main() {

  struct timespec start, finish;
  long long int timeelapsed;
  clock_gettime(CLOCK_MONOTONIC, &start);

  int rank, size;
  int i;
  double bm = 1.3;
  double bc = 10;
  double be;
  double dm[8];
  double dc[8];
  double e[8];
  double step = 0.01;
  double best_error = 999999999;
  int best_error_i;
  int minimum_found = 0;
  double pError = 0;
  double baseMC[2];
  
  double om[] = {0,1,1, 1, 0,-1,-1,-1};
  double oc[] = {1,1,0,-1,-1,-1, 0, 1};

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  be = rms_error(bm, bc);

  if(size != 9) {
    if(rank == 0) {
      printf("This program needs to run on exactly 9 processes\n");
      return 0;
     }
   } 

  while(!minimum_found)
  {
    if (rank != 0)
	{
		i = rank -1;
		dm[i] = bm + (om[i] * step);
		dc[i] = bc + (oc[i] * step);
		pError = rms_error (dm[i], dc[i]);

		MPI_Send (&pError, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Send (&dm[i], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Send (&dc[i], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

		MPI_Recv (&bm, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv (&bc, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv (&minimum_found, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
    else
	{
		for(i = 1; i < size; i++)
		{
			MPI_Recv (&pError, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv (&dm[i-1] , 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv (&dc[i-1], 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			if(pError < best_error)
			{
				best_error = pError;
				best_error_i = i - 1;
			}
		}

		if(best_error < be)
		{
			be = best_error;
			bm = dm[best_error_i];
			bc = dc[best_error_i];
		}
		
		else
		{
			minimum_found = 1;
		}
		
		for(i = 1; i < size; i++)
		{
			MPI_Send (&bm, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
			MPI_Send (&bc, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
			MPI_Send (&minimum_found, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

		}
	}
    }

    if(rank == 0)
      {
	printf ("minimum m,c is %lf, %lf with error %lf\n", bm, bc, be);
	clock_gettime(CLOCK_MONOTONIC, &finish);
	timedifference(&start, &finish, &timeelapsed);
	printf("TIme elasped: %lldnsec or %0.9lfsec\n", timeelapsed, (timeelapsed/1.0e9));
      }

  MPI_Finalize();
  return 0;
}
point_t data[] = {
  {67.50,117.63},{65.33,126.07},{82.95,145.73},{76.19,113.32},
  {87.53,145.91},{73.30,132.50},{76.57,134.90},{68.72,115.55},
  {73.32,140.31},{78.84,143.44},{71.68,120.91},{92.42,138.04},
  {76.05,128.45},{34.36,75.04},{91.02,154.71},{98.33,178.86},
  {75.34,134.84},{84.94,143.54},{34.55,73.14},{22.78,36.84},
  {11.34,37.95},{77.02,121.28},{20.48,54.11},{85.81,158.12},
  {98.14,169.75},{61.93,126.98},{44.68,77.35},{38.15,83.43},
  {30.16,67.27},{76.68,133.30},{86.62,141.45},{60.09,116.95},
  {26.91,70.86},{ 9.10,37.19},{11.23,36.15},{26.50,59.09},
  {41.44,93.35},{67.47,131.65},{89.46,161.35},{19.81,38.73},
  {51.34,93.22},{97.28,174.58},{33.38,68.25},{19.98,40.87},
  {44.04,85.03},{70.68,125.28},{84.78,128.46},{63.73,98.19},
  {16.48,31.39},{91.67,169.18},{13.59,31.83},{69.96,133.01},
  {15.59,53.61},{53.87,116.25},{57.95,119.53},{88.77,156.53},
  { 4.54,19.19},{60.18,111.69},{76.51,143.61},{21.62,54.08},
  {53.82,107.72},{28.55,79.61},{51.32,89.52},{60.46,135.12},
  {68.14,124.70},{13.20,32.38},{94.61,160.20},{57.63,99.17},
  {80.81,143.84},{92.81,143.31},{91.60,164.46},{13.32,40.64},
  {93.33,174.11},{50.18,102.21},{ 0.11,28.39},{69.56,119.09},
  {55.02,110.04},{74.47,146.21},{91.14,163.71},{65.21,125.47},
  {58.83,100.80},{10.02,46.51},{94.36,158.71},{51.22,103.94},
  { 9.25,34.11},{12.64,44.32},{50.24,104.77},{15.73,37.63},
  {53.03,101.19},{30.42,72.53},{47.90,90.79},{25.89,68.64},
  {75.24,126.35},{80.87,133.87},{ 5.86,31.16},{62.88,112.89},
  {81.94,145.67},{51.81,88.24},{82.74,122.52},{80.97,139.14},
  { 0.28, 1.97},{62.20,111.18},{55.95,100.36},{29.17,61.74},
  {71.13,120.95},{11.42,42.50},{38.60,70.96},{47.24,88.20},
  { 4.25,26.16},{13.53,50.16},{23.30,64.38},{96.18,162.57},
  {97.45,167.05},{86.09,139.37},{19.61,40.67},{75.10,137.54},
  {61.73,124.08},{ 7.59,27.78},{ 5.53,13.80},{59.76,116.05},
  {19.08,55.39},{41.68,74.96},{16.33,42.42},{96.25,161.59},
  {69.83,121.89},{ 5.65,37.87},{42.46,86.94},{79.37,151.11},
  {48.34,97.43},{57.96,111.54},{22.31,63.95},{ 6.03,14.45},
  {38.59,72.82},{91.91,166.06},{77.34,149.68},{20.95,49.40},
  {18.24,44.16},{46.33,85.77},{87.69,162.06},{ 5.63,33.09},
  {25.64,62.39},{78.37,129.15},{90.63,162.90},{59.07,108.46},
  { 3.73, 9.62},{73.31,127.30},{85.44,148.06},{62.86,111.04},
  {27.61,66.72},{97.81,162.18},{76.86,141.77},{65.90,142.09},
  {89.34,144.60},{ 9.42,48.00},{51.54,104.84},{11.47,42.53},
  {42.31,79.72},{62.70,111.95},{15.81,44.71},{51.03,101.97},
  {50.54,98.74},{84.62,138.28},{95.25,169.94},{ 3.97,31.21},
  {10.63, 9.82},{ 1.47,32.91},{67.16,129.22},{10.14,26.05},
  {52.42,103.57},{41.93,90.91},{96.18,166.50},{ 0.25,16.19},
  {20.73,49.87},{34.86,70.58},{39.49,83.99},{93.26,153.09},
  {89.43,147.70},{46.72,90.16},{30.27,50.94},{ 7.73,40.77},
  {47.24,89.70},{60.71,110.70},{10.25,35.87},{87.93,176.16},
  {81.83,132.92},{47.52,95.67},{ 8.22,30.97},{ 0.16,19.43},
  { 7.67,39.19},{25.22,46.59},{37.39,94.24},{23.87,54.68},
  {53.00,94.78},{55.12,113.11},{ 0.39,17.41},{12.25,42.86},
  {24.12,67.60},{40.49,92.29},{52.77,87.06},{12.23,46.57},
  {67.85,125.89},{42.67,89.64},{34.42,61.02},{ 1.94,18.44},
  {53.40,111.16},{89.61,164.56},{ 3.82, 3.73},{96.24,158.21},
  {77.04,135.04},{97.05,148.24},{26.71,51.44},{95.02,163.28},
  {34.29,61.81},{ 1.62,21.43},{67.74,107.75},{98.19,159.80},
  {17.62,54.87},{85.72,146.11},{23.67,53.85},{49.02,101.01},
  {93.66,161.56},{44.72,86.99},{72.81,113.39},{60.91,112.51},
  {24.17,61.50},{49.89,89.80},{ 8.97,45.83},{26.67,59.28},
  {62.50,111.35},{11.07,25.58},{37.01,63.82},{18.94,46.54},
  {61.63,108.22},{28.93,50.01},{55.36,99.90},{92.64,173.42},
  {28.57,52.10},{ 9.61,30.76},{19.82,52.09},{47.92,90.78},
  {28.85,70.65},{33.80,38.50},{29.53,66.71},{42.50,89.01},
  {34.95,92.60},{83.24,150.06},{94.97,158.22},{63.79,123.66},
  {94.60,157.04},{79.72,136.29},{63.38,116.51},{16.22,41.53},
  {40.06,65.75},{54.36,89.16},{65.52,130.07},{19.95,52.07},
  {78.01,121.11},{32.30,71.77},{84.85,139.15},{50.25,98.61},
  {72.77,124.69},{59.41,100.91},{89.09,168.89},{76.82,142.52},
  {26.18,56.42},{10.95,52.42},{62.40,111.33},{62.71,102.03},
  { 2.35,13.42},{ 7.19,41.90},{62.53,123.58},{15.54,52.27},
  { 0.80,20.28},{ 5.30,26.03},{13.01,57.51},{19.16,35.04},
  {59.74,133.37},{33.93,83.73},{ 4.54,17.74},{18.61,48.23},
  {72.71,133.09},{51.18,90.42},{51.26,104.57},{58.02,83.55},
  {68.78,148.00},{29.56,70.05},{10.44,22.10},{90.96,154.32},
  {13.82,28.56},{88.00,150.89},{51.69,92.07},{54.30,90.90},
  {57.44,101.77},{80.12,149.10},{10.78,39.64},{11.95,64.56},
  {97.71,178.99},{37.08,69.88},{ 0.47,17.14},{65.62,109.62},
  {99.78,179.04},{81.77,158.12},{11.90,33.54},{85.24,151.30},
  {49.38,78.83},{63.79,114.06},{32.79,57.50},{31.96,88.52},
  {84.21,144.62},{49.77,102.25},{49.55,88.18},{ 8.26,38.38},
  {47.36,90.74},{97.88,170.49},{44.11,78.79},{38.23,88.99},
  {40.69,69.98},{ 7.36,36.33},{56.85,111.52},{13.64,36.87},
  {53.35,116.88},{72.47,124.76},{46.64,107.91},{94.73,151.85},
  {66.61,117.51},{12.35,61.26},{79.14,159.92},{79.02,140.10},
  {45.42,94.75},{16.54,29.85},{91.82,153.33},{28.86,55.11},
  {50.75,79.96},{10.11,36.95},{32.04,68.39},{71.95,131.62},
  {57.09,109.51},{10.91,27.38},{62.49,115.05},{20.30,46.80},
  {93.11,159.57},{71.85,130.18},{53.28,108.81},{55.38,111.22},
  {85.54,151.96},{22.64,52.50},{56.67,98.37},{ 0.97,21.47},
  {72.50,138.03},{26.98,45.21},{96.25,167.19},{16.31,40.83},
  {58.79,87.57},{47.38,89.11},{90.04,157.08},{32.23,62.24},
  {11.57,34.79},{23.99,51.20},{64.23,105.49},{72.15,107.38},
  {37.45,77.37},{73.55,128.18},{36.90,78.88},{45.26,95.74},
  {37.99,74.96},{63.67,123.99},{68.51,129.23},{13.85,35.04},
  {59.04,93.67},{54.42,102.56},{89.89,148.97},{76.40,139.33},
  {15.26,37.71},{61.79,114.18},{31.03,61.43},{96.81,157.36},
  {41.43,93.08},{59.88,107.00},{75.72,122.81},{47.51,113.65},
  {39.71,81.28},{73.15,145.81},{13.27,27.44},{73.94,130.58},
  {48.11,95.15},{91.97,147.09},{29.24,56.59},{88.10,143.34},
  {83.07,136.67},{ 1.60,25.57},{83.37,132.98},{32.81,81.72},
  {32.76,61.42},{26.69,62.44},{34.24,70.93},{75.68,125.27},
  {96.68,165.04},{95.66,168.80},{79.86,144.53},{74.34,121.30},
  {57.43,94.75},{56.67,79.08},{54.07,88.83},{99.94,171.14},
  {66.96,110.58},{77.27,141.31},{68.77,120.16},{27.42,77.06},
  { 3.47,33.83},{22.31,49.66},{56.78,101.75},{96.06,157.01},
  { 1.29,25.47},{ 2.97,42.15},{66.51,105.60},{37.81,72.23},
  { 3.07,33.29},{37.37,92.70},{ 7.52,32.65},{43.43,75.38},
  {63.53,120.10},{55.30,106.01},{65.04,118.04},{ 5.91,21.90},
  {65.28,121.06},{29.55,51.16},{41.39,88.10},{35.63,81.24},
  {86.27,136.99},{15.92,72.35},{75.93,120.09},{91.92,160.74},
  {97.55,169.39},{70.19,117.49},{16.28,38.79},{44.36,81.43},
  {87.91,149.02},{ 3.52,38.16},{59.12,120.72},{ 1.90, 0.73},
  {83.31,156.42},{44.25,74.81},{36.88,57.45},{80.37,157.35},
  {66.99,138.80},{79.54,145.55},{18.33,45.70},{64.15,122.52},
  {34.89,69.76},{46.89,93.34},{14.47,48.95},{ 4.47,11.21},
  {42.32,86.99},{31.84,63.03},{33.34,81.26},{ 4.88,25.36},
  {79.82,133.64},{40.63,100.56},{63.46,121.03},{96.80,151.04},
  {92.72,156.50},{90.13,156.67},{87.25,150.80},{63.02,122.96},
  {17.30,47.83},{24.10,53.74},{55.24,105.56},{49.54,106.29},
  {50.18,92.64},{28.50,73.07},{75.82,141.86},{43.76,88.26},
  {33.55,61.23},{66.59,98.81},{25.78,64.50},{ 5.19,31.93},
  {32.05,72.33},{61.50,119.08},{39.73,91.92},{80.39,146.69},
  {73.53,149.32},{40.57,62.81},{91.25,166.56},{63.33,112.85},
  { 1.32,13.80},{87.01,143.92},{84.90,132.20},{36.73,88.35},
  {81.82,127.95},{77.33,143.68},{ 4.44,17.14},{71.90,134.73},
  {59.09,106.07},{83.32,145.03},{56.43,87.15},{55.72,118.37},
  {35.02,93.87},{76.13,111.18},{43.98,75.47},{92.99,165.88},
  {31.66,59.37},{28.52,59.74},{82.09,144.05},{26.09,49.24},
  {70.97,117.20},{ 7.68,37.90},{70.42,123.06},{40.47,82.04},
  {73.52,133.29},{21.29,62.15},{74.56,121.04},{76.26,137.21},
  {10.29,56.09},{28.54,78.38},{21.19,63.67},{40.37,88.01},
  { 9.97,60.42},{59.83,106.32},{36.88,81.58},{64.00,122.44},
  {44.79,60.82},{25.61,52.42},{32.59,72.08},{65.16,118.02},
  {13.14,39.55},{75.40,123.94},{45.15,97.24},{53.90,113.09},
  {75.55,129.32},{ 0.43,21.46},{52.76,92.05},{90.01,148.61},
  {26.95,57.55},{30.46,68.83},{39.15,81.42},{58.32,98.73},
  {70.37,115.08},{ 5.94,21.53},{ 3.43,33.83},{32.38,68.35},
  {59.53,111.46},{37.94,108.20},{24.71,63.30},{96.93,166.78},
  {87.47,146.91},{33.94,100.63},{76.73,141.16},{31.78,71.95},
  {85.03,155.23},{ 2.52,39.44},{44.84,95.65},{77.68,131.95},
  {41.72,86.46},{18.32,57.93},{69.89,120.19},{54.70,86.01},
  {54.99,104.64},{48.59,95.15},{24.36,53.97},{51.98,96.80},
  {60.23,100.55},{59.09,85.63},{33.81,67.74},{12.22,41.13},
  {26.38,65.33},{ 7.09,30.43},{24.85,50.55},{99.52,170.23},
  {84.73,129.42},{39.71,92.69},{57.91,105.37},{33.52,75.23},
  {33.93,65.91},{27.34,52.79},{58.75,104.12},{60.52,110.72},
  { 2.81,12.48},{ 8.02,27.71},{64.73,120.96},{82.03,159.82},
  {22.60,38.52},{24.08,61.92},{66.05,102.86},{19.42,49.76},
  {48.04,97.54},{46.20,96.45},{ 1.17,17.39},{63.69,129.79},
  {29.84,75.40},{26.53,45.12},{95.19,149.02},{90.77,157.73},
  {41.81,86.87},{74.43,110.80},{49.39,97.73},{22.62,49.26},
  { 4.87,18.08},{19.41,58.94},{42.62,107.88},{77.24,159.90},
  {80.67,133.41},{44.37,89.30},{51.39,91.86},{25.27,57.14},
  {10.84,16.20},{99.73,182.30},{85.08,167.49},{16.49,38.24},
  {48.48,98.37},{30.56,50.30},{45.38,97.80},{33.13,73.18},
  {39.58,86.47},{56.27,115.05},{18.85,48.41},{51.63,99.71},
  { 7.00,29.08},{32.17,71.87},{44.00,94.70},{ 3.73,38.62},
  {72.17,111.87},{29.35,54.28},{50.13,94.46},{91.52,170.01},
  {40.05,72.34},{46.87,67.83},{76.24,138.98},{26.75,63.90},
  {63.87,105.49},{13.12,23.17},{12.58,53.66},{ 8.20,43.82},
  {14.36,32.76},{32.84,51.21},{11.45,24.07},{93.59,140.71},
  {58.09,85.90},{52.69,102.77},{38.38,85.50},{98.36,158.74},
  {74.87,125.72},{32.47,73.67},{55.48,122.80},{42.12,87.03},
  {75.24,144.54},{71.66,134.49},{34.01,66.08},{58.69,105.94},
  {35.47,72.45},{51.46,100.28},{87.79,150.58},{10.86,27.33},
  {68.38,133.79},{38.57,86.54},{64.01,109.90},{17.09,63.00},
  { 9.34,35.52},{66.20,127.61},{22.82,52.08},{79.23,148.39},
  {19.50,45.48},{ 4.76,14.25},{ 0.11,24.33},{55.86,91.16},
  {43.58,90.07},{14.59,50.39},{39.88,99.03},{41.04,85.30},
  {87.44,169.74},{55.54,98.60},{ 2.07, 1.75},{29.04,64.38},
  {41.45,92.95},{73.41,124.41},{78.49,152.32},{33.64,87.75},
  {67.48,139.43},{87.13,144.84},{59.65,100.97},{45.11,87.31},
  {76.40,139.82},{62.21,124.75},{78.60,163.67},{20.57,49.21},
  {80.06,138.88},{60.51,108.48},{ 2.05,29.92},{11.23,23.36},
  {10.61,39.17},{30.63,63.71},{ 5.13,41.33},{74.37,123.26},
  {14.03,38.39},{ 6.31,36.58},{ 9.16,36.90},{75.16,138.63},
  {88.12,149.50},{ 1.78,31.54},{28.88,64.20},{79.20,136.08},
  {27.98,48.89},{89.12,158.04},{ 9.51,11.76},{10.45,40.24},
  {22.73,61.87},{73.97,124.05},{ 7.09,10.69},{11.73,32.78},
  {90.67,166.68},{88.17,167.73},{97.82,164.53},{63.81,103.31},
  {74.11,137.22},{71.03,119.75},{43.78,85.30},{84.66,148.37},
  {12.33,30.33},{83.29,138.56},{21.34,71.07},{40.14,68.00},
  {73.05,119.85},{ 7.44,29.55},{89.02,151.86},{17.24,61.99},
  {41.66,73.47},{50.62,99.48},{60.53,111.85},{12.70,17.62},
  {66.84,110.12},{52.27,89.56},{98.72,178.46},{79.92,113.48},
  {23.55,43.25},{38.26,96.94},{56.52,118.31},{53.04,96.75},
  {35.73,72.29},{60.43,109.43},{77.67,137.73},{45.78,98.97},
  {32.36,67.11},{23.89,68.74},{24.53,45.00},{97.28,162.74},
  {27.73,50.67},{90.85,165.35},{93.94,153.83},{ 6.63,43.74},
  {93.38,150.59},{43.87,77.99},{49.91,86.07},{82.99,151.00},
  { 7.00,40.39},{46.17,89.39},{28.87,66.05},{72.85,141.73},
  {27.21,58.82},{42.02,79.42},{95.29,149.89},{ 7.03,21.47},
  {80.55,133.93},{75.29,147.77},{32.44,69.31},{29.14,61.10},
  {94.21,157.98},{48.51,115.01},{ 9.76,32.67},{ 6.69,20.71},
  {14.30,44.18},{98.57,173.85},{ 4.01,24.74},{34.46,60.56},
  {19.21,46.64},{89.60,166.71},{27.93,53.40},{22.10,65.12},
  {20.30,42.75},{95.02,166.30},{76.91,138.66},{ 0.28,32.32},
  {62.29,108.93},{18.53,44.52},{58.50,118.40},{79.87,133.47},
  { 1.06,31.67},{43.28,75.77},{34.13,84.84},{71.34,142.31},
  {94.14,172.56},{18.77,37.09},{ 3.58,15.15},{34.71,49.88},
  {15.87,25.31},{40.55,70.94},{63.57,116.94},{33.01,78.49},
  {12.21,36.69},{83.80,139.29},{15.41,38.32},{23.53,70.10},
  {19.25,53.57},{32.17,40.06},{80.00,133.35},{15.29,51.71},
  {43.63,81.51},{70.07,126.99},{44.69,85.99},{89.03,158.09},
  {36.23,60.18},{ 2.37, 1.33},{28.27,71.44},{37.81,80.29},
  {74.61,114.15},{32.45,63.47},{76.90,145.43},{45.78,89.56},
  {43.76,90.34},{72.40,121.11},{80.03,158.07},{89.76,159.97},
  { 0.79,30.07},{74.50,132.38},{46.19,76.00},{98.40,166.43},
  {83.71,152.87},{69.45,138.18},{20.09,57.62},{10.82,44.42},
  {94.90,161.52},{56.24,105.19},{25.80,45.99},{78.59,144.32},
  {41.90,95.14},{88.38,158.28},{72.22,136.40},{98.04,151.63},
  { 3.44,35.78},{18.58,59.71},{58.74,112.02},{43.90,84.81},
  {59.96,131.25},{55.08,113.52},{11.76,36.25},{75.05,134.27},
  {18.62,45.25},{49.76,101.82},{80.57,154.63},{93.50,167.65},
  {70.39,126.65},{53.57,107.27},{36.88,59.79},{10.52,25.86},
  {64.89,100.31},{35.21,90.41},{ 6.23,33.90},{93.30,143.70},
  {63.45,129.25},{10.07,36.79},{28.01,58.59},{59.22,100.12},
  {46.14,75.11},{51.65,78.56},{42.40,66.31},{99.08,164.34},
  { 8.14,35.13},{61.88,118.50},{39.24,88.28},{37.84,82.29},
  {77.53,154.65},{ 3.52,12.20},{94.10,150.41},{52.95,90.29},
  {33.45,63.79},{59.77,97.17},{37.34,66.25},{62.51,101.43},
  {58.38,123.37},{85.57,146.57},{59.50,110.36},{64.77,113.77},
  {52.31,86.72},{74.08,119.62},{20.13,55.40},{70.01,137.11},
  {73.03,141.72},{72.90,116.95},{ 9.77,18.64},{77.91,120.62},
  {35.13,81.81},{94.76,163.60},{84.97,153.65},{50.99,97.73},
  {76.95,139.73},{95.14,165.88},{53.85,91.54},{11.67,32.28},
  {74.95,128.36},{62.48,122.55},{52.39,104.02},{84.64,137.02},
  {60.79,90.69},{10.88,42.09},{89.36,155.24},{42.14,99.07},
  {10.47,24.63},{81.53,125.43},{83.23,156.18},{21.79,42.60},
  {22.12,42.96},{84.10,145.52},{ 7.28,19.37},{45.70,87.18},
  {68.93,116.49},{44.33,92.72},{83.48,164.04},{36.29,59.75},
  {56.87,105.36},{10.77,32.58},{37.26,72.49},{81.52,151.25},
  {20.22,51.77},{ 0.53,13.54},{70.22,141.70},{86.98,153.36},
  {86.88,155.08},{95.61,163.24},{10.92,46.94},{52.02,86.13},
  {79.54,145.77},{45.72,80.50},{23.64,54.82},{40.59,76.38},
  {10.51,24.25},{88.39,154.46},{96.15,153.89},{52.43,104.17},
  {56.14,93.00},{14.86,52.67},{17.22,45.09},{65.58,106.79},
  {37.27,49.60},{21.86,54.55},{30.77,65.57},{18.91,46.54},
  {99.20,188.44},{64.15,127.79},{53.69,114.35},{80.75,129.07},
  {20.46,42.99},{43.95,89.80},{11.86,34.56},{76.24,137.26},
  {60.32,123.89},{13.10,47.27},{ 3.21,27.37},{56.46,123.20},
  {28.08,60.38},{62.73,112.94},{56.62,118.19},{ 7.11,21.06},
  {35.00,74.47},{99.39,182.65},{31.10,63.43},{18.34,55.60},
  {63.21,119.43},{96.73,152.88},{85.87,131.41},{85.13,150.34},
  {58.50,106.92},{ 9.39,25.13},{32.07,64.76},{70.15,104.89},
  {85.64,126.01},{ 5.71,31.30},{10.14,34.51},{55.14,97.21},
  {40.93,71.15},{91.84,166.86},{11.77,33.90},{58.69,95.90},
  {32.25,88.75},{79.19,149.50},{38.70,81.86},{23.71,55.47},
  {58.19,95.57},{60.07,101.54},{20.08,56.31},{ 5.15,21.22},
  {63.36,118.68},{58.66,97.64},{99.72,167.67},{55.95,108.87},
  {83.51,155.14},{20.52,56.46},{62.20,126.56},{62.36,108.09},
  {25.79,51.49},{10.73,31.13},{40.02,89.61},{ 0.96,19.08}
};
