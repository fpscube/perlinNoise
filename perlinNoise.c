#include "stdlib.h"
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define K_GRID_WIDTH    100
#define K_GRID_HEIGHT   100

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


/* Create pseudorandom direction vector
 */
void randomGradient(int ix, int iy,float *pXout,float *pYout) {
    // No precomputed gradients mean this works for any number of grid coordinates
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2; // rotation width
    unsigned a = ix, b = iy;
    a *= 3284157443; b ^= (a << s) | (a >> (w-s));
    b *= 1911520717; a ^= (b << s) | (b >> (w-s));
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
    *pXout = cos(random); *pYout = sin(random);
}

void perlinGenGradiant(int pX,int pY,int pSize,int pGridSize)
 {
    int gridTabSize = pSize/pGridSize;
    for (int y=0;y<(gridTabSize+1);y++)
    {
        for (int x=0;x<(gridTabSize+1);x++)
        {
            float lRandY;
            float lRandX;
            randomGradient(pX + x*pGridSize,pY + y*pGridSize,&lRandX,&lRandY); 
            stc_gradient[x][y][0]= lRandX;
            stc_gradient[x][y][1]= lRandY;
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

void perlinGenTexture(uint32_t * pBuffer,int pPosX,int pPosY,int pSize,int pTextureSize,int pGridSize)
{
    perlinGenGradiant(pPosX,pPosY,pSize,pGridSize);
    float lPixelSize = (float)pSize/(float)pTextureSize;

    for (int x=0;x<pTextureSize;x++)
    {
        for (int y=0;y<pTextureSize;y++)
        {
            float xGrid = ((float)x)*lPixelSize/((float)pGridSize);
            float yGrid = ((float)y)*lPixelSize/((float)pGridSize);
            float pixelval = perlinGetPixel(xGrid,yGrid);
            pixelval = (pixelval +1.0) /2.0;
            int pixelIntVal = (pixelval*255);
            if(pixelIntVal>255) 
                printf("error\n");
            pixelIntVal = (pixelIntVal<<24) + (pixelIntVal<<16) + (pixelIntVal<<8) + 0xFF;
            //printf("%x\n",pixelIntVal);
            pBuffer[x+y*pTextureSize] =  pixelIntVal;
        }

    }
}


void perlinGenHeightMap(float * pBuffer,int pPosX,int pPosY,int pSize,int pTextureSize,int pGridSize)
{
    perlinGenGradiant(pPosX,pPosY,pSize,pGridSize);
    float lPixelSize = (float)pSize/(float)pTextureSize;

    for (int x=0;x<pTextureSize;x++)
    {
        for (int y=0;y<pTextureSize;y++)
        {
            float xGrid = ((float)x)*lPixelSize/((float)pGridSize);
            float yGrid = ((float)y)*lPixelSize/((float)pGridSize);
            pBuffer[x+y*pTextureSize] =  perlinGetPixel(xGrid,yGrid);;
        }

    }
}
