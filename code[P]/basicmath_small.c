#include <stdio.h>
//#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "snipmath.h"
#include <unistd.h>
#include <math.h>

/* The printf's may be removed to isolate just the math calculations */

int main(void)
{
 // system("date +%H:%M:%S.%3N >> 0220_hb_log_recovery.csv");
  int Data_value = 0;
  int True_value = 0;
//  int Reset_value = 0;

  while(1)
  {

  double  a1 = 1.0, b1 = -10.5, c1 = 32.0, d1 = -30.0;
  double  a2 = 1.0, b2 = -4.5, c2 = 17.0, d2 = -30.0;
  double  a3 = 1.0, b3 = -3.5, c3 = 22.0, d3 = -31.0;
  double  a4 = 1.0, b4 = -13.7, c4 = 1.0, d4 = -35.0;
  double  x[3];
  double X;
  int     solutions;
  int i;
  unsigned long l = 0x3fed0169L;
  struct int_sqrt q;
  long n = 0;

  Data_value++;
  if(Data_value < 90) True_value = Data_value;
  else if(Data_value >= 500){
		 Data_value = 0;
		// Reset_value++;
  }
  else if(Data_value == 100) system("date +%H:%M:%S.%3N >> 0220_100ms_hb_log_recovery.csv");
  else if(Data_value >= 90) True_value = 90;

// FILE *cf;
// cf=fopen("/home/pi/Desktop/heartbeat/basicmath/Check_value.txt","r");
// if( cf != NULL){
 if(access("/home/pi/Desktop/heartbeat/basicmath/Check_value.txt",F_OK) == 0){
 FILE *f;
 f=fopen("/home/pi/Desktop/heartbeat/basicmath/Data_value.txt","w");
 fprintf(f,"%d",True_value/10);
 fclose(f);
 //fclose(cf);
  system("sudo rm /home/pi/Desktop/heartbeat/basicmath/Check_value.txt");
 }

/*
  if(Reset_value >= 10) {
	Data_value = 0;
        Reset_value = 0;
  }*/
// else fclose(cf);
 /* solve soem cubic functions */
//  printf("********* CUBIC FUNCTIONS ***********\n");
  /* should get 3 solutions: 2, 6 & 2.5   */
  SolveCubic(a1, b1, c1, d1, &solutions, x);  
//  printf("Solutions:");
//  for(i=0;i<solutions;i++)
//    printf(" %f",x[i]);
//  printf("\n");
  /* should get 1 solution: 2.5           */
  SolveCubic(a2, b2, c2, d2, &solutions, x);  
//  printf("Solutions:");
//  for(i=0;i<solutions;i++)
//    printf(" %f",x[i]);
//  printf("\n");
  SolveCubic(a3, b3, c3, d3, &solutions, x);
//  printf("Solutions:");
//  for(i=0;i<solutions;i++)
//    printf(" %f",x[i]);
//  printf("\n");
  SolveCubic(a4, b4, c4, d4, &solutions, x);
//  printf("Solutions:");
//  for(i=0;i<solutions;i++)
//    printf(" %f",x[i]);
//  printf("\n");
  /* Now solve some random equations */
  for(a1=1;a1<10;a1++) {
    for(b1=10;b1>0;b1--) {
      for(c1=5;c1<15;c1+=0.5) {
	for(d1=-1;d1>-11;d1--) {
	  SolveCubic(a1, b1, c1, d1, &solutions, x);  
//	  printf("Solutions:");
//	  for(i=0;i<solutions;i++)
//	    printf(" %f",x[i]);
//	  printf("\n");
	}
      }
    }
  }
  
//  printf("********* INTEGER SQR ROOTS ***********\n");
  /* perform some integer square roots */
  for (i = 0; i < 1001; ++i)
    {
      usqrt(i, &q);
			// remainder differs on some machines
     // printf("sqrt(%3d) = %2d, remainder = %2d\n",
//     printf("sqrt(%3d) = %2d\n",
//	     i, q.sqrt);
       q.sqrt;
    }
  usqrt(l, &q);
  //printf("\nsqrt(%lX) = %X, remainder = %X\n", l, q.sqrt, q.frac);
//  printf("\nsqrt(%lX) = %X\n", l, q.sqrt);
    q.sqrt;


//  printf("********* ANGLE CONVERSION ***********\n");
  /* convert some rads to degrees */
  for (X = 0.0; X <= 360.0; X += 1.0)
	deg2rad(X);
//    printf("%3.0f degrees = %.12f radians\n", X, deg2rad(X));
//  puts("");
  for (X = 0.0; X <= (2 * PI + 1e-6); X += (PI / 180))
	rad2deg(X);
//    printf("%.12f radians = %3.0f degrees\n", X, rad2deg(X));

  }

 return 0;
}
