
#include "glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
 
#include "linmath.h"
 
#include <stdlib.h>
#include <stdio.h>
 
float g_speedCoef=10.0;
 
#define K_TILE_RES 32
#define K_TILE_SIZE 1000
typedef struct
{
    float x, y,z;
    float r, g, b;
}T_vertexbuffer;

static T_vertexbuffer vertexBuffer[K_TILE_RES*K_TILE_RES];
 
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
vec3 gCamPos = {0.0,0.0,-10.0};
vec3 gCamDir = {0.0,0.0,1.0};
vec4 gMvDir = {0.0,0.0,0.0,0.0};
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if(key == GLFW_KEY_SPACE)
    {
        gCamPos[1]+=10;

    }       
    if(key == GLFW_KEY_KP_ADD)
    {
        g_speedCoef*=2;

    }         
    if(key == GLFW_KEY_KP_SUBTRACT)
    {
        g_speedCoef/=2;
    }   
    if(key == GLFW_KEY_LEFT)
    {
        vec3 mvDir3 = {gCamDir[0],0.0,gCamDir[2]};
        vec3_norm(mvDir3,mvDir3);
        vec4 mvDir = {mvDir3[0],0.0,mvDir3[2],1.0};
        mat4x4 R;
        mat4x4_identity(R);
        mat4x4_rotate_Y(R,R,1.57079632679);
        mat4x4_mul_vec4(gMvDir,R,mvDir);

    }
    else if(key == GLFW_KEY_RIGHT)
    {

        vec3 mvDir3 = {gCamDir[0],0.0,gCamDir[2]};
        vec3_norm(mvDir3,mvDir3);
        vec4 mvDir = {mvDir3[0],0.0,mvDir3[2],1.0};
        mat4x4 R;
        mat4x4_identity(R);
        mat4x4_rotate_Y(R,R,-1.57079632679);
        mat4x4_mul_vec4(gMvDir,R,mvDir);
    }
    else if(key == GLFW_KEY_UP)
    {
        gMvDir[0]= gCamDir[0];
        gMvDir[1]= gCamDir[1];
        gMvDir[2]= gCamDir[2];
    }
    else if(key == GLFW_KEY_DOWN)
    {
        gMvDir[0]= -gCamDir[0];
        gMvDir[1]= -gCamDir[1];
        gMvDir[2]= -gCamDir[2];
        
    }

    if (action == GLFW_RELEASE)
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

 
int main(void)
{
    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;
    int width, height;
    static double stc_lastTime;
    stc_lastTime = glfwGetTime();
 
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
 
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
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

    //produce terrain
    int i=0;
    float lsb = K_TILE_SIZE/K_TILE_RES;
    for (int x=0;x<K_TILE_RES;x++)
    {
        for (int z=0;z<K_TILE_RES;z++)
        {
            float y = groundGetY(x*lsb,z*lsb);
            vertexBuffer[i].x=x*lsb;
            vertexBuffer[i].y=y;
            vertexBuffer[i].z=z*lsb;
            vertexBuffer[i].r=1.0;
            vertexBuffer[i].g=1.0;
            vertexBuffer[i].b=1.0;
            i++;
        }
    }
 
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBuffer), vertexBuffer, GL_STATIC_DRAW);
 
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
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertexBuffer[0]), (void*) 0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertexBuffer[0]), (void*) (sizeof(float) * 3));

 
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

        gCamPos[0] += gMvDir[0]*lDeltaTime*g_speedCoef;
        gCamPos[1] += gMvDir[1]*lDeltaTime*g_speedCoef;
        gCamPos[2] += gMvDir[2]*lDeltaTime*g_speedCoef;

        printf("%f,%f,%f-%f,%f,%f\n",gCamDir[0],gCamDir[1],gCamDir[2],gCamPos[0],gCamPos[1],gCamPos[2]);

        ratio = width / (float) height;
 
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
 
        mat4x4_identity(m);
        vec3 eye = {gCamPos[0], gCamPos[1], gCamPos[2]};
        vec3 center = {gCamDir[0]+gCamPos[0], gCamDir[1]+gCamPos[1], gCamDir[2]+gCamPos[2]};
        vec3 up = {0, 1.0, 0.0};
        mat4x4_translate_in_place(m, gCamPos[0],gCamPos[1],gCamPos[2] );
        mat4x4_look_at(m,eye,center,up);
       // mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        //mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 10.f, -10.f);
        //mat4x4_frustum(p, -ratio, ratio, -1.f, 1.f, 10.f, 10000.f);
        mat4x4_perspective(p,0.785398,ratio,0.0f,10000.f);
        mat4x4_mul(mvp, p, m);
 
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_LINE_STRIP, 0, K_TILE_RES*K_TILE_RES);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}









