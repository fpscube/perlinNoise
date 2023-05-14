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


/**
 *  Create pseudorandom direction vector
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


/**
 * @brief perlinGenGradiant will generation a static table stc_gradient
 * with random vector according the size of the grid
 * 
 * @param pX_WC  position of the current zone in world coord
 * @param pY_WC  position of the current zone in world coord
 * @param pSizeWC size of the current zone in world coord
 * @param pGridSizeWC grid size in world coord (must a multiple of the size and lower)
 */
void perlinGenGradiant(int pX_WC,int pY_WC,int pSizeWC,int pGridSizeWC)
 {
    int lGridVectorCount = (pSizeWC/pGridSizeWC +1);
    for (int y=0;y<(lGridVectorCount);y++)
    {
        for (int x=0;x<(lGridVectorCount);x++)
        {
            float lRandY;
            float lRandX;
            randomGradient(pX_WC + x*pGridSizeWC,pY_WC + y*pGridSizeWC,&lRandX,&lRandY); 
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
 
 /**
  * @brief Compute Perlin noise value at coordinates x, y 
  * 
  * @param x pos in grid normalised coord 1.0 = 1 grid step
  * @param y pos in grid normalised coord 1.0 = 1 grid step
  * @return float noise value range (-1,1)
  */
 float perlinGetPixel(float x, float y) {
 
     // Find grid cell coordinates for the current point
     // in order to demtermine the 4 around gradient vector sqyare(x0,y0,x1,y1)
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
 
     // Interpolate between 4 grid point gradients
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

void perlinGenTexture(uint32_t * pBuffer,int PosX_WC,int pPosY_WC,int pSizeWC,int pTextureSizePx,int pGridSizeWC)
{
    int lGridCoef = pSizeWC/pGridSizeWC;
    if(lGridCoef > K_GRID_SIZE) 
    {
        printf("error perlin grid must be < %d",K_GRID_SIZE);
        exit(1);
    }
    perlinGenGradiant(PosX_WC,pPosY_WC,pSizeWC,pGridSizeWC);

    // pixel=0 and pixel=(pTextureSizePx-1) shall fit the grid
    for (int xPx=0;xPx<pTextureSizePx;xPx++)
    {
        for (int yPx=0;yPx<pTextureSizePx;yPx++)
        {
            // convert position in grid normalised coord
            float xGrid = ((float)xPx)*lGridCoef/((float)(pTextureSizePx-1));
            float yGrid = ((float)yPx)*lGridCoef/((float)(pTextureSizePx-1));
            float pixelval = perlinGetPixel(xGrid,yGrid);
            pixelval = (pixelval +1.0) /2.0;
            int pixelIntVal = (pixelval*255);
            if(pixelIntVal>255) 
                printf("error\n");
            pixelIntVal = (pixelIntVal<<24) + (pixelIntVal<<16) + (pixelIntVal<<8) + 0xFF;
            pBuffer[xPx+yPx*pTextureSizePx] =  pixelIntVal;
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
        printf("error perlin grid size  must multiple of size %d\n",pSizeWC);
        exit(1);
    }

    perlinGenGradiant(PosX_WC,pPosY_WC,pSizeWC,pGridSizeWC);

    int lGridCoef = pSizeWC/pGridSizeWC;
    for (int yPx=0;yPx<(pTextureSizePX);yPx++)
    {
        for (int xPx=0;xPx<(pTextureSizePX);xPx++)
        {
            float xGrid = ((float)xPx)*lGridCoef/((float)(pTextureSizePX-1));
            float yGrid = ((float)yPx)*lGridCoef/((float)(pTextureSizePX-1));
            float val = perlinGetPixel(xGrid,yGrid);
            pBuffer[xPx+yPx*pTextureSizePX] =  val;

        }

    }
}
