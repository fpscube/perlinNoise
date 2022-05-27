#include "stdlib.h"
#include <stdio.h>

#define K_GRID_WIDTH    10
#define K_GRID_HEIGHT   10

 static float stc_gradient[K_GRID_WIDTH+1][K_GRID_HEIGHT+1][2]={0};

 // Function to linearly interpolate between a0 and a1
 // Weight w should be in the range [0.0, 1.0]
 float lerp(float a0, float a1, float w) {
     return (1.0 - w)*a0 + w*a1;
 }
 
 // Computes the dot product of the distance and gradient vectors.
 float dotGridGradient(int ix, int iy, float x, float y) {
  
     // Compute the distance vector
     float dx = x - (float)ix;
     float dy = y - (float)iy;
 
     // Compute the dot-product
     return (dx*stc_gradient[ix][iy][0] + dy*stc_gradient[ix][iy][1]);
 }

void perlinGenGradiant()
 {
    for (int y=0;y<(K_GRID_HEIGHT+1);y++)
    {
        for (int x=0;x<(K_GRID_WIDTH+1);x++)
        {
         stc_gradient[x][y][0]= (float)(rand())/(float)RAND_MAX*2.0-1.0;
         stc_gradient[x][y][1]= (float)(rand())/(float)RAND_MAX*2.0-1.0;
        }
     }

 }

 static float fade(float t) {
                                                        // Fade function as defined by Ken Perlin.  This eases coordinate values
                                                        // so that they will ease towards integral values.  This ends up smoothing
                                                        // the final output.
    return t * t * t * (t * (t * 6 - 15) + 10);         // 6t^5 - 15t^4 + 10t^3
}
 
 // Compute Perlin noise at coordinates x, y
 float perlinGetPixel(float x, float y) {
 
     // Determine grid cell coordinates
     int x0 = x;
     int x1 = (x0 + 1.0);
     int y0 = y;
     int y1 = (y0 + 1.0);
 
     // Determine interpolation weights
     // Could also use higher order polynomial/s-curve here
     float sx = x - (float)x0;
     float sy = y - (float)y0;

     sx = fade(sx);
     sy = fade(sy);
 
     // Interpolate between grid point gradients
     float n0, n1, ix0, ix1, value;
     n0 = dotGridGradient(x0, y0, x, y);
     n1 = dotGridGradient(x1, y0, x, y);
     ix0 = lerp(n0, n1, sx);
     n0 = dotGridGradient(x0, y1, x, y);
     n1 = dotGridGradient(x1, y1, x, y);
     ix1 = lerp(n0, n1, sx);
     value = lerp(ix0, ix1,sy);
 
     return value;
 }

void perlinGenTexture(int * pBuffer,int pWidth,int pHeight)
{
    for (int x=0;x<pWidth;x++)
    {
        for (int y=0;y<pHeight;y++)
        {
            float xGrid = ((float)x)*K_GRID_WIDTH/((float)pWidth);
            float yGrid = ((float)y)*K_GRID_HEIGHT/((float)pHeight);
            float pixelval = perlinGetPixel(xGrid,yGrid);
            pixelval = (pixelval +1.0) /2.0;
            int pixelIntVal = (pixelval*255);
            if(pixelIntVal>255) 
                printf("error\n");
            pixelIntVal = (pixelIntVal<<24) + (pixelIntVal<<16) + (pixelIntVal<<8) + 0xFF;
            //printf("%x\n",pixelIntVal);
            pBuffer[x+y*pWidth] =  pixelIntVal;
        }

    }
}
