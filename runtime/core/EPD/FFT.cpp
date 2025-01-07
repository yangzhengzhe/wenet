#include "FFT.h"
#define PI 3.1415926
#include "math.h"

void FFT::DoFFT(float *s, int n,const bool inverted) {
   int ii,jj,nn,limit,m,j,inc,i;
   double wx,wr,wpr,wpi,wi,theta;
   double xre,xri,x;

   nn=n / 2; j = 1;
   for (ii=1;ii<=nn;ii++) {
      i = 2 * ii - 1;
      if (j>i) {
         xre = s[j-1]; xri = s[j + 1-1];
         s[j-1] = s[i-1];  s[j + 1-1] = s[i + 1-1];
         s[i-1] = xre; s[i + 1-1] = xri;
      }
      m = n / 2;
      while (m >= 2  && j > m) {
         j -= m; m /= 2;
      }
      j += m;
   };
   limit = 2;
   while (limit < n) {
      inc = 2 * limit; theta = 2.0*PI / (double)limit;
      if (inverted) theta= -theta;
      x = sin(0.5 * theta);
      wpr = -2.0 * x * x; wpi = sin(theta);
      wr = 1.0; wi = 0.0;
      for (ii=1; ii<=limit/2; ii++) {
         m = 2 * ii - 1;
         for (jj = 0; jj<=(n - m) / inc;jj++) {
            i = m + jj * inc;
            j = i + limit;
            xre = wr * s[j-1] - wi * s[j + 1-1];
            xri = wr * s[j + 1-1] + wi * s[j-1];
            s[j-1] = s[i-1] - xre; s[j + 1-1] = s[i + 1-1] - xri;
            s[i-1] = s[i-1] + xre; s[i + 1-1] = s[i + 1-1] + xri;
         }
         wx = wr;
         wr = wr * wpr - wi * wpi + wr;
         wi = wi * wpr + wx * wpi + wi;
      }
      limit = inc;
   }
   if (inverted) {
      for (i = 1;i<=n;i++)
         s[i-1] = s[i-1] / nn;
   }
}

void FFT::RealFFT(float *s,int n,const bool inverted) {
   int n2, i, i1, i2, i3, i4;
   double xr1, xi1, xr2, xi2, wrs, wis;
   double yr, yi, yr2, yi2, yr0, theta, x;

   DoFFT(s, n);	//计算FFT
   n=n / 2; n2 = n/2;
   theta = PI / n;
   x = sin(0.5 * theta);
   yr2 = -2.0 * x * x;
   yi2 = sin(theta); yr = 1.0 + yr2; yi = yi2;
   for (i=2; i<=n2; i++) {
      i1 = i + i - 1;      i2 = i1 + 1;
      i3 = n + n + 3 - i2; i4 = i3 + 1;
      wrs = yr; wis = yi;
      xr1 = (s[i1-1] + s[i3-1])/2.0; xi1 = (s[i2-1] - s[i4-1])/2.0;
      xr2 = (s[i2-1] + s[i4-1])/2.0; xi2 = (s[i3-1] - s[i1-1])/2.0;
      s[i1-1] = xr1 + wrs * xr2 - wis * xi2;
      s[i2-1] = xi1 + wrs * xi2 + wis * xr2;
      s[i3-1] = xr1 - wrs * xr2 + wis * xi2;
      s[i4-1] = -xi1 + wrs * xi2 + wis * xr2;
      yr0 = yr;
      yr = yr * yr2 - yi  * yi2 + yr;
      yi = yi * yr2 + yr0 * yi2 + yi;
   }
   xr1 = s[1-1];
   s[1-1] = xr1 + s[2-1];
   s[2-1] = 0.0;
}
