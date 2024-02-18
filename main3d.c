
#include "glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
 
#include "map.h"
#include "perlinNoise.h"
#include "linmath.h"
 
#include <stdlib.h>
#include <stdio.h>
#define K_TILE_RES_PX 100
#define K_TILE_SIZE 1000
typedef struct
{
    float x, y,z;
    float r, g, b;
}T_vertexbuffer;

static T_vertexbuffer vertexBuffer[K_TILE_RES_PX*K_TILE_RES_PX*6];
static int indexBuffer[K_TILE_RES_PX*K_TILE_RES_PX*6];
GLint mvp_location, vpos_location, vcol_location;
 
static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec3 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 1.0);\n"
"    color = vCol;\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";
 
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
vec3 gCamPos = {0.0,10519.624023,0.0};
vec3 gCamDir = {0.0 , 0.0, 1.0};
vec4 gMvDir = {0.0,0.0,0.0,0.0};
float gCamSpeed=1000.0;
 
GLenum  gRenderType = GL_TRIANGLES;

 
typedef struct 
{
    int left;
    int right;
    int up;
    int down;
}T_media;

T_media gMedia;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if(key == GLFW_KEY_SPACE)
    {
        gCamPos[1]+=100;

    }        
    if(key == GLFW_KEY_L)
    {
        gRenderType = GL_LINES;
    }     
    if(key == GLFW_KEY_T)
    {
        gRenderType = GL_TRIANGLES;
    }
    if(key == GLFW_KEY_R)
    {

        gCamPos[0] = 0;
        gCamPos[1] = 0;
        gCamPos[2] = 0;
        gCamSpeed=1000.0;

    }      
    if(key == GLFW_KEY_KP_ADD)
    {
        gCamSpeed*=2;

    }         
    if(key == GLFW_KEY_KP_SUBTRACT)
    {
        gCamSpeed/=2;
    }           

    if(key == GLFW_KEY_LEFT || key == GLFW_KEY_A)
    {
        gMedia.left = (action != GLFW_RELEASE);
        vec3 mvDir3 = {gCamDir[0],0.0,gCamDir[2]};
        vec3_norm(mvDir3,mvDir3);
        vec4 mvDir = {mvDir3[0],0.0,mvDir3[2],1.0};
        mat4x4 R;
        mat4x4_identity(R);
        mat4x4_rotate_Y(R,R,1.57079632679);
        mat4x4_mul_vec4(gMvDir,R,mvDir);

    }
    else if(key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)
    {
        gMedia.right = (action != GLFW_RELEASE);

        vec3 mvDir3 = {gCamDir[0],0.0,gCamDir[2]};
        vec3_norm(mvDir3,mvDir3);
        vec4 mvDir = {mvDir3[0],0.0,mvDir3[2],1.0};
        mat4x4 R;
        mat4x4_identity(R);
        mat4x4_rotate_Y(R,R,-1.57079632679);
        mat4x4_mul_vec4(gMvDir,R,mvDir);
    }
    else if(key == GLFW_KEY_UP || key == GLFW_KEY_W)
    {
        gMedia.up = (action != GLFW_RELEASE);
        gMvDir[0]= gCamDir[0];
        gMvDir[1]= gCamDir[1];
        gMvDir[2]= gCamDir[2];
    }
    else if(key == GLFW_KEY_DOWN || key == GLFW_KEY_S)
    {
        gMedia.down = (action != GLFW_RELEASE);
        gMvDir[0]= -gCamDir[0];
        gMvDir[1]= -gCamDir[1];
        gMvDir[2]= -gCamDir[2];
        
    }

    if (action == GLFW_RELEASE && !gMedia.up && !gMedia.down && !gMedia.left  && !gMedia.right   )
    {
        gMvDir[0]=0.0;
        gMvDir[1]=0.0;
        gMvDir[2]=0.0;
    }

}

float groundGetY(float x,float z)
{
  float y = sin(x/50)*5+sin(z/50)*5 + sin(x/90)*9 + sin(z/90)*9 + sin(x/150)*15 + sin(z/150)*15 -(x/200)*(x/200) -(z/200)*(z/200);

  return(y);
}

void map_computeTex(T_map_texture * pTexture)
{
}
 
GLuint vertex_buffer;
GLuint elementbuffer;


typedef struct
{
    float  tex[K_TILE_RES_PX*K_TILE_RES_PX];
}T_MapTileTex;

/**
 * @brief generate and draw map tile
 * 
 * @param pCenterX_WC center X of the tile in world coord
 * @param pCenterY_WC center Y of the tile in world coord
 * @param pSizeWC  size of the tile in world coord
 * @param pGridSizeWC grid size in world coord
 * @param pRingId ring id 
 * @param pTileId position of the tile in the ring in order to manage resolution limit
 * tile 0/2/4/6 draw limit with inner tile
 *  x:(left<-right) z:(down->up)
 *    1 0 7 
 *    2 x 6
 *    3 4 5 
 * @param pTextureTab List of texture (pRingId*8+pTileId) (0=internal 8-15=ring1 16-23=ring2 24-31=ring3 ...) 
 */
void drawMapTile(int pCenterX_WC,int pCenterY_WC,int pSizeWC,int pGridSizeWC,int pRingId,int pTileId,T_MapTileTex *pTextureTab)
{
    // get left bottom corner coord in order to generate texture at center
    int pOriginX = pCenterX_WC - pSizeWC/2;
    int pOriginY = pCenterY_WC - pSizeWC/2;       
    
    int vcount=0;
    int icount=0;
    float lsb =  ((float)pSizeWC)/((float)(K_TILE_RES_PX-1));
    float hrLsb =  lsb/3.0;

    // debug color to comment
    float colorR = ((float)(pOriginX%124))/255.0;
    float colorG = ((float)(pOriginY%168))/255.0;
    //  colorR=0.0;
    //  colorG=0.0;


    // get current Texture
    float *lTexture = pTextureTab[pRingId*8+pTileId].tex;
    float *lHRTexture[8];

    const int cstHrMap[8][3]=
    {
        [0] = {7,0,1},
        [2] = {3,2,1},
        [6] = {5,6,7},
        [4] = {5,4,3}
    };
    
    // get High resolution texture using RingId for all 8 tileId
    for(int i=0;i<8;i++)
    {
        lHRTexture[i] = pTextureTab[(pRingId-1)*8+i].tex;
    }

    // prepare triangle for current tile id except for limit low res/high res tile
    for (int zPx=0;zPx<K_TILE_RES_PX;zPx++)
    {
        for (int xPx=0;xPx<K_TILE_RES_PX;xPx++)
        {                
            /* tile 0/2/4/6 draw limit with inner tile
                *  x (left<-right) z(down->up)
                *    1 0 7 
                *    2 x 6
                *    3 4 5 
                */  
            int highReslimit= ((pTileId==0 && zPx==0) ||
                               (pTileId==2 && xPx==0) ||
                               (pTileId==4 && zPx==(K_TILE_RES_PX-2)) ||
                               (pTileId==6 && xPx==(K_TILE_RES_PX-2)));

                     
            
            // compute vertex color using perlin texture
            float y = lTexture[xPx+zPx*K_TILE_RES_PX];
           

            if(y<0)
            {
                vertexBuffer[vcount].r=(y + 1.0)/2.0 + colorR;
                vertexBuffer[vcount].g=(y + 1.0)/2.0 + colorG;
                vertexBuffer[vcount].b=(y + 1.0)/2.0 + 0.5;
            }
            else
            {
                vertexBuffer[vcount].r=(y + 1.0)/2.0 + colorR;
                vertexBuffer[vcount].g=(y + 1.0)/2.0 + 0.2 + colorG;
                vertexBuffer[vcount].b=(y + 1.0)/2.0 ;
            }

            //Add Vertex
            
            vertexBuffer[vcount].x=xPx*lsb + pOriginX;
            vertexBuffer[vcount].y=y*1000;
            vertexBuffer[vcount].z=zPx*lsb + pOriginY;
            vcount++;



            // //Add Triangles
            if (((xPx+1)<K_TILE_RES_PX) && ((zPx+1)<K_TILE_RES_PX))
            {
                // Do not add triangle for transition limit between high res and low res
                if(highReslimit && pRingId>1) continue;

               // todo add high res draw for tile 0

                //triangle 1 
                // x--x
                // | /
                // |/
                // x
                indexBuffer[icount++] = vcount-1; 
                indexBuffer[icount++] = vcount;
                indexBuffer[icount++] = vcount-1 + K_TILE_RES_PX;  // todo add K_TILE_RES_PX*3 offset for high res

                //triangle 2
                //    x
                //   /| 
                //  / |
                // x--x
                indexBuffer[icount++] = vcount;
                indexBuffer[icount++] = vcount + K_TILE_RES_PX;
                indexBuffer[icount++] = vcount-1 + K_TILE_RES_PX;
               
            
            }
        }
    }

    // send triangle to gpu except for limit high res/low res

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBuffer), vertexBuffer, GL_STATIC_DRAW);
 
   
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertexBuffer[0]), (void*) 0);
                          
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertexBuffer[0]), (void*) (sizeof(float) * 3));

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, icount* sizeof(unsigned int), &indexBuffer[0], GL_STATIC_DRAW);
          
    glDrawElements(gRenderType, K_TILE_SIZE*K_TILE_SIZE,GL_UNSIGNED_INT,(void*)0 );




    if(pTileId!=0 && pTileId!=2 && pTileId!=4 && pTileId!=6) return;
    if(pRingId<=1) return;


    // TILE 0 --- add triangle for pTileId limit high res low rest
    if(pTileId==0)
    {
        // Add low high resolution and low resolution vertex
        vcount=0;
        icount=0;
        for (int zPx=0;zPx<2;zPx++)
        {
            float lCoef=1.0;
            if(zPx==0)lCoef=3.0;
            for (int xPx=0;xPx<(K_TILE_RES_PX*lCoef);xPx++)
            {              

                // compute vertex color using perlin texture
                float y = lTexture[xPx+zPx*K_TILE_RES_PX];
                if(zPx==0)
                {  
                    const int *lHrId = &cstHrMap[pTileId][0];
                    if(xPx < K_TILE_RES_PX) 
                        y = lHRTexture[lHrId[0]][xPx+(K_TILE_RES_PX)*(K_TILE_RES_PX-1)];
                    else if(xPx < (K_TILE_RES_PX*2-1)) 
                        y = lHRTexture[lHrId[1]][(xPx-K_TILE_RES_PX+1) + K_TILE_RES_PX*(K_TILE_RES_PX-1)] ;
                    else 
                        y = lHRTexture[lHrId[2]][(xPx-K_TILE_RES_PX*2 + 2) + K_TILE_RES_PX*(K_TILE_RES_PX-1)] ;
                }

                if(y<0)
                {
                    vertexBuffer[vcount].r=(y + 1.0)/2.0 + colorR;
                    vertexBuffer[vcount].g=(y + 1.0)/2.0 + colorG;
                    vertexBuffer[vcount].b=(y + 1.0)/2.0 + 0.5;
                }
                else
                {
                    vertexBuffer[vcount].r=(y + 1.0)/2.0 + colorR;
                    vertexBuffer[vcount].g=(y + 1.0)/2.0 + 0.2 + colorG;
                    vertexBuffer[vcount].b=(y + 1.0)/2.0 ;
                }

                //Add Vertex            
                vertexBuffer[vcount].x=xPx*(lsb/lCoef) + pOriginX;
                vertexBuffer[vcount].y=y*1000;
                vertexBuffer[vcount].z=zPx*lsb + pOriginY;

            
                vcount++;                                        
                
            }
        }
        // For all position of the limit line draw the 4 limits triangles
        // between high and low resolution
        int xHrPx = 0;
        for (int xPx=0;xPx<(K_TILE_RES_PX-1);xPx++)
        {       
            
            //Triangle 1-3  
            // 1  <= Lres (zPx=1)
            // |\    //
            // | \   //
            // 3--2 <= HRes (zPx=0) (repeat while Hres point reach the next low res point)   
            int nextXLr = (xPx+1)*lsb;
            int xHr =  (xHrPx+1)*hrLsb;

            while(xHr<=nextXLr)
            {
                indexBuffer[icount++] = K_TILE_RES_PX*3 + xPx; 
                indexBuffer[icount++] = xHrPx;
                indexBuffer[icount++] = xHrPx+1;
                xHrPx++;
                xHr =  (xHrPx+1)*hrLsb;
            }

            //Triangle 4
            // 1--2  <= Lres (zPx=1)
            //  \ | 
            //   \|
            //    3  <= HRes (zPx=0)
            indexBuffer[icount++] = K_TILE_RES_PX*3 + xPx;
            indexBuffer[icount++] = K_TILE_RES_PX*3 + xPx+1;
            indexBuffer[icount++] = xHrPx;          
        
        }
    }
    // TILE 2 --- add triangle for pTileId limit high res low rest
    if(pTileId==2 )
    {
        // Add low high resolution and low resolution vertex
        vcount=0;
        icount=0;


        // for i=0 low res i=1 high res
        for (int i=0;i<2;i++)
        {
            int xPx,zPx;
            float lCoef=1.0;
            if(i==0)lCoef=3.0;

            if(pTileId==0)  zPx = (i==0)?0:1;
            if(pTileId==2)  xPx = (i==0)?0:1;
            if(pTileId==4)  zPx = (i==0)?K_TILE_RES_PX-1:K_TILE_RES_PX-2;
            if(pTileId==6)  xPx = (i==0)?K_TILE_RES_PX-1:K_TILE_RES_PX-2;


            // for all pixel in line
            for (int linePx=0;linePx<(K_TILE_RES_PX*lCoef);linePx++)
            {  
                if(pTileId==0) xPx = linePx;
                if(pTileId==2) zPx = linePx;
                if(pTileId==4) xPx = linePx;
                if(pTileId==6) zPx = linePx;

                // compute vertex color using perlin texture
                float y = lTexture[xPx+zPx*K_TILE_RES_PX];
                // High resolution get y in one of the 3 HR textures
                if(i==0)
                {  
                    int xHrPx,zHrPx;
                    // todo à generaliser 
                    xHrPx = K_TILE_RES_PX-1;
                    const int *lHrId = &cstHrMap[pTileId][0];
                    // choose the good texture (get good part of the high res texture)
                    if(linePx < K_TILE_RES_PX) 
                    {
                        // todo à generaliser 
                        zHrPx = zPx;
                        y = lHRTexture[lHrId[0]][xHrPx+zHrPx*K_TILE_RES_PX];
                    }
                    else if(linePx < (K_TILE_RES_PX*2-1))
                    { 
                        // todo à generaliser 
                        zHrPx = zPx-K_TILE_RES_PX+1;
                        y = lHRTexture[lHrId[1]][xHrPx+zHrPx*K_TILE_RES_PX] ;
                    }
                    else 
                    {
                        // todo à generaliser 
                        zHrPx = zPx-K_TILE_RES_PX*2+2;
                        y = lHRTexture[lHrId[2]][xHrPx+zHrPx*K_TILE_RES_PX] ;
                    }
                }

                if(y<0)
                {
                    vertexBuffer[vcount].r=(y + 1.0)/2.0 + colorR;
                    vertexBuffer[vcount].g=(y + 1.0)/2.0 + colorG;
                    vertexBuffer[vcount].b=(y + 1.0)/2.0 + 0.5;
                }
                else
                {
                    vertexBuffer[vcount].r=(y + 1.0)/2.0 + colorR;
                    vertexBuffer[vcount].g=(y + 1.0)/2.0 + 0.2 + colorG;
                    vertexBuffer[vcount].b=(y + 1.0)/2.0 ;
                }

                //Add Vertex
                
                // todo à generaliser             
                vertexBuffer[vcount].x=xPx*lsb + pOriginX;
                vertexBuffer[vcount].y=y*1000;
                vertexBuffer[vcount].z=zPx*(lsb/lCoef) + pOriginY;

            
                vcount++;                                        
                
            }
        }
        // For all position of the limit line draw the 4 limits triangles
        // between high and low resolution
        int iHrPx = 0;

            // todo à generaliser 
        for (int iPx=0;iPx<(K_TILE_RES_PX-1);iPx++)
        {       
            
            //Triangle 1-3  
            // 1  <= Lres line (offset K_TILE_RES_PX*3 )
            // |\    //
            // | \   //
            // 3--2 <= HRes line (repeat while Hres point reach the next low res point)   
            int nextXLr = (iPx+1)*lsb;
            int xHr =  (iHrPx+1)*hrLsb;


                        // todo à generaliser 
            while(xHr<=nextXLr)
            {
                indexBuffer[icount++] = K_TILE_RES_PX*3 + iPx; 
                indexBuffer[icount++] = iHrPx;
                indexBuffer[icount++] = iHrPx+1;
                iHrPx++;
                xHr =  (iHrPx+1)*hrLsb;
            }

            //Triangle 4
            // 1--2  <= Lres line (offset K_TILE_RES_PX*3 )
            //  \ | 
            //   \|
            //    3  <= HRes line
            indexBuffer[icount++] = K_TILE_RES_PX*3 + iPx;
            indexBuffer[icount++] = K_TILE_RES_PX*3 + iPx+1;
            indexBuffer[icount++] = iHrPx;          
        
        }
    }
    
    // TILE 4 --- add triangle for pTileId limit high res low rest
    if(pTileId==4 )
    {
    }
    // TILE 6 --- add triangle for pTileId limit high res low rest
    if(pTileId==6 )
    {
    }


    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBuffer), vertexBuffer, GL_STATIC_DRAW);
 
   
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertexBuffer[0]), (void*) 0);
                          
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertexBuffer[0]), (void*) (sizeof(float) * 3));

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, icount* sizeof(unsigned int), &indexBuffer[0], GL_STATIC_DRAW);
          
   glDrawElements(gRenderType, K_TILE_SIZE*K_TILE_SIZE,GL_UNSIGNED_INT,(void*)0 );
  // glDrawElements(GL_LINES, K_TILE_SIZE*K_TILE_SIZE,GL_UNSIGNED_INT,(void*)0 );


    
}

void drawMapTilesRing(int pPosX_WC,int pPosY_WC,int pSizeWC,int pGridSizeWC)
{

    static T_MapTileTex perlinHeightMapTab[4*8];

    const int cstXOffset[]= {0.0, 1.0, 1.0, 1.0, 0.0,-1.0,-1.0,-1.0};
    const int cstYOffset[]= {1.0, 1.0, 0.0,-1.0,-1.0,-1.0, 0.0, 1.0};

    // 7 0 1
    // 6 x 2
    // 5 4 3

    perlinGenHeightMap(perlinHeightMapTab[0].tex,pPosX_WC-pSizeWC/2,pPosY_WC-pSizeWC/2,pSizeWC,K_TILE_RES_PX,pGridSizeWC);
    for(int ringId=1;ringId<4;ringId++)
    {
        int lRingSize_WC;
        lRingSize_WC= pow(3,ringId-1)*pSizeWC;
        for(int i=0;i<8;i++)
        {
            int xOffset = cstXOffset[i]*lRingSize_WC + pPosX_WC - lRingSize_WC/2;
            int yOffset = cstYOffset[i]*lRingSize_WC + pPosY_WC - lRingSize_WC/2;
            perlinGenHeightMap(perlinHeightMapTab[ringId*8 + i].tex,xOffset,yOffset,lRingSize_WC,K_TILE_RES_PX,pGridSizeWC);
        }
    }


    drawMapTile(pPosX_WC,pPosY_WC,pSizeWC,pGridSizeWC,0,0,&perlinHeightMapTab[0]);


    for(int ringId=1;ringId<4;ringId++)
    {
        int lRingSize_WC;
        lRingSize_WC= pow(3,ringId-1)*pSizeWC;
        for(int i=0;i<8;i++)
        {
            int xOffset = cstXOffset[i]*lRingSize_WC + pPosX_WC;
            int yOffset = cstYOffset[i]*lRingSize_WC + pPosY_WC;
            drawMapTile(xOffset,yOffset,lRingSize_WC,pGridSizeWC,ringId,i,&perlinHeightMapTab[0]) ;
        }
    }


}


int main(void)
{
    GLFWwindow* window;
    GLuint  vertex_shader, fragment_shader, program;
    int width, height;
    static double stc_lastTime;
    stc_lastTime = glfwGetTime();

 
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
 
    window = glfwCreateWindow(800, 800, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwGetFramebufferSize(window, &width, &height);
    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_HIDDEN);
    glfwSetCursorPos(window,width/2.0,height/2.0);
    glfwSetKeyCallback(window, key_callback);
 
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);
 
    // NOTE: OpenGL error checks have been omitted for brevity
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
 
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
 
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
 
    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");
     
 
    glEnableVertexAttribArray(vpos_location);
    glEnableVertexAttribArray(vcol_location);
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST); 
    //glEnable(GL_CULL_FACE);    

    while (!glfwWindowShouldClose(window))
    {
        double lCurrentTime = glfwGetTime();
        double lDeltaTime =  lCurrentTime-stc_lastTime;
        stc_lastTime =  lCurrentTime;

        float ratio;
        glfwGetFramebufferSize(window, &width, &height);
        mat4x4 m, p, mvp;
        double xcursor;
        double ycursor;
        glfwGetCursorPos(window,&xcursor,&ycursor);
        glfwSetCursorPos(window,width/2.0,height/2.0);
        double xDelta = width/2.0 - xcursor;
        double yDelta = height/2.0 - ycursor;

        {
            vec3 up = {0,1,0};
            vec3 hVector;
            vec3_mul_cross(hVector, gCamDir, up);
            vec3_norm(hVector,hVector);
            gCamDir[0] -= hVector[0]* xDelta/1000.0;
            gCamDir[1] +=  yDelta/1000.0;
            gCamDir[2] -= hVector[2]* xDelta/1000.0;
            vec3_norm(gCamDir,gCamDir);
        }

        gCamPos[0] += gMvDir[0]*lDeltaTime*gCamSpeed;
        gCamPos[1] += gMvDir[1]*lDeltaTime*gCamSpeed;
        gCamPos[2] += gMvDir[2]*lDeltaTime*gCamSpeed;

        // printf("dir x:%f,y:%f,z:%f-pos x:%f,y:%f,z:%f\n",gCamDir[0],gCamDir[1],gCamDir[2],gCamPos[0],gCamPos[1],gCamPos[2]);

        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4x4_identity(m);
        vec3 eye = {gCamPos[0], gCamPos[1], gCamPos[2]};
        vec3 center = {gCamDir[0]+gCamPos[0], gCamDir[1]+gCamPos[1], gCamDir[2]+gCamPos[2]};
        vec3 up = {0, 1.0, 0.0};
        mat4x4_translate_in_place(m, gCamPos[0],gCamPos[1],gCamPos[2] );
        mat4x4_look_at(m,eye,center,up);
    // mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        //mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 10.f, -10.f);
        //mat4x4_frustum(p, -ratio, ratio, -1.f, 1.f, 10.f, 10000.f);

        /*
        The X axis extends from left to right, with increasing values to the right.
        The Y axis extends from bottom to top, with increasing values upwards.
        The Z axis extends from the screen towards you (depth-wise), with increasing values going towards the back of the screen.
        */
        mat4x4_perspective(p,0.785398,ratio,1.0f,100000.f);
        mat4x4_mul(mvp, p, m);


        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);



        // draw center at 0,0
        drawMapTilesRing(0,0,10000,1000); 

        //todo for ring 

        // implementation 
        // for(int iRing=0;iRing<K_MAP_NB_RING;iRing++)
        // {        
            
        //     for(int texId=0;texId<K_MAP_NB_TEXTURE_BY_RING;texId++)
        //     {
        //     //drawMapTile(&lMap->ring[iRing].tex[texId]);
        // }


        //void perlinGenHeightMap(float * pBuffer,int PosX_WC,int pPosY_WC,int pSizeWC,int pTextureSizePX,int pGridSizeWC)


        // T_map * lMap = map_update(K_TILE_SIZE*10,gCamPos[0],gCamPos[2]);



        // // Todo afficher de la couleur sur les texture en fonction du ring
        // for(int iRing=0;iRing<K_MAP_NB_RING;iRing++)
        // {
        //     for(int texId=0;texId<K_MAP_NB_TEXTURE_BY_RING;texId++)
        //     {
        //         drawMapTile(&lMap->ring[iRing].tex[texId]);
        //     }
        // }



        glfwSwapBuffers(window);
        glfwPollEvents();
        // printf(" - PollEvents %d\n",(counter++)%50);

    }
        
    
    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}









