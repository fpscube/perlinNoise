#include "stdlib.h"
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define K_GRID_SIZE    100

 static float stc_gradient[K_GRID_SIZE+1][K_GRID_SIZE+1][2]={0};

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

void perlinGenGradiant(int pX,int pY,int pSizeWC,int pGridSizeWC)
 {
    int gridTabSize = pSizeWC/pGridSizeWC;
    for (int y=0;y<(gridTabSize+1);y++)
    {
        for (int x=0;x<(gridTabSize+1);x++)
        {
            float lRandY;
            float lRandX;
            randomGradient(pX + x*pGridSizeWC,pY + y*pGridSizeWC,&lRandX,&lRandY); 
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
 
 // Compute Perlin noise at coordinates x, y (wc world coordinate)
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

void perlinGenTexture(uint32_t * pBuffer,int PosX_WC,int pPosY_WC,int pSizeWC,int pTextureSize,int pGridSizeWC)
{
    if((pSizeWC/pGridSizeWC) > K_GRID_SIZE) 
    {
        printf("error perlin grid must be < %d",K_GRID_SIZE);
        exit(1);
    }
    perlinGenGradiant(PosX_WC,pPosY_WC,pSizeWC,pGridSizeWC);
    float lPixelSize = (float)pSizeWC/(float)pTextureSize;

    for (int x=0;x<pTextureSize;x++)
    {
        for (int y=0;y<pTextureSize;y++)
        {
            float xGrid = ((float)x)*lPixelSize/((float)pGridSizeWC);
            float yGrid = ((float)y)*lPixelSize/((float)pGridSizeWC);
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

/**
 * @brief Generate a height map texture using perlin noise algorithm
 *  at x y position, this call the same position will always generate the same texture
 * 
 * @param pBuffer output texture buffer
 * @param PosX_WC position X of the texture in world Coord 
 * @param pPosY_WC position Y of the texture in world Coord 
 * @param pSizeWC size of the texture in world Coord
 * @param pTextureSize texture size in pixel
 * @param pGridSizeWC grid size in world coord
 */

void perlinGenHeightMap(float * pBuffer,int PosX_WC,int pPosY_WC,int pSizeWC,int pTextureSizePX,int pGridSizeWC)
{   
    
    if((pSizeWC/pGridSizeWC) > K_GRID_SIZE) 
    {
        printf("error perlin grid must be < %d\n",K_GRID_SIZE);
        exit(1);
    }

    if(pSizeWC<pGridSizeWC)
    {
        printf("error perlin grid size %d  must lower than size %d\n",pGridSizeWC,pSizeWC);
        exit(1);
    }

    if((pSizeWC%pGridSizeWC)!=0) 
    {
        printf("error perlin grid size  must multiple of size %d\n",pGridSizeWC,pSizeWC);
        exit(1);
    }

    perlinGenGradiant(PosX_WC,pPosY_WC,pSizeWC,pGridSizeWC);
    float lWcPxRatio= (float)pSizeWC/(float)pTextureSizePX;

    for (int x=0;x<pTextureSizePX;x++)
    {
        for (int y=0;y<pTextureSizePX;y++)
        {
            float xGrid = ((float)x)*lWcPxRatio/((float)pGridSizeWC);
            float yGrid = ((float)y)*lWcPxRatio/((float)pGridSizeWC);
            pBuffer[x+y*pTextureSizePX] =  perlinGetPixel(xGrid,yGrid);
        }

    }
}
