#include <stdio.h>
#include <stdlib.h>
#include "linmath.h"

#include "glad.h"
#include <GLFW/glfw3.h>

#define UNUSED(x) (void)(sizeof(x))

static const float VERTICES[] = {
     0.f,  -0.5f, 0.0,
    -0.5f, 0.5f, 1.0,
     0.5f, 0.5f, -1.0,
    -0.5f, -0.5f, 0.0,
     0.5f, -0.5f, 1.0,
     0.f,   0.5f, -1.0
};
 
static const char *VERTEX_SHADER_SOURCE =
    "#version 330 core\n"
    "in vec3 aPos;\n"
    "uniform mat4 MVP;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = MVP *  vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\n"
    "\0";
 
static const char *FRAGMENT_SHADER_SOURCE =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec4 gl_FragCoord ;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0f);\n"
    "}\n"
    "\0";


float gScreenRatio=1.0;
vec3 gCamPos = {0,0,0};
vec3 gCamDir = {0.336166,-0.354075,-0.872710};
vec4 gMvDir = {0.0,0.0,0.0,0.0};
float gCamSpeed=1.0;


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
        gCamSpeed=1.0;

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

static void error_callback(int err, const char *msg) {
    fprintf(stderr, "GLFW callback: %s (error code %d)\n", msg, err);
}

static void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    UNUSED(window);
    gScreenRatio = width / (float) height;
    glViewport(0, 0, width, height);
}

static unsigned compile_shader(GLenum shader_type, const char *shader_src) {
    int success;
    unsigned shader;

    shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_src, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "error: failed to compile vertex shader, %s\n", infoLog);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    return shader;
}

int main(void) {
    GLFWwindow *window;
    int width, height;
    static double stc_lastTime;
    stc_lastTime = glfwGetTime();
    const char *msg;
    unsigned vertex_buffer, vertex_shader;
    int success;
    unsigned fragment_shader, program, VAO, VBO;

    fputs("Hello, Triangle!\n", stderr);

    glfwSetErrorCallback(error_callback);

    msg = glfwGetVersionString();
    if (!msg) {
        fputs("error: unable to determine GLFW version\n", stderr);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Compiled against GLFW %s\n", msg);

    if (!glfwInit()) {
        fputs("error: unable to initialize GLFW\n", stderr);
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(800, 800, "Hello, Triangle!", NULL, NULL);
    if (!window) {
        fputs("error: unable to create window\n", stderr);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fputs("error: unable to initialize GLAD\n", stderr);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }


    glfwGetFramebufferSize(window, &width, &height);    
    gScreenRatio = width / (float) height;
    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_HIDDEN);
    glfwSetCursorPos(window,width/2.0,height/2.0);
    glfwSetKeyCallback(window, key_callback);

    msg = (const char *)glGetString(GL_VERSION);
    if (!msg) {
        fputs("error: unable to determine OpenGL version\n", stderr);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    printf("OpenGL %s\n", msg);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

    vertex_shader   = compile_shader(GL_VERTEX_SHADER,   VERTEX_SHADER_SOURCE);
    fragment_shader = compile_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);


    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "error: failed to link shaders, %s\n", infoLog);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    
    	// Get a handle for our "MVP" uniform
	GLuint MVP = glGetUniformLocation(program, "MVP");
 
	// Get a handle for our buffers
	GLuint aPos = glGetAttribLocation(program, "aPos");


    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void *)0);
    glEnableVertexAttribArray(aPos);
      
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1);

    		
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST); 
    //glEnable(GL_CULL_FACE);    


    while (!glfwWindowShouldClose(window)) {
        double lCurrentTime = glfwGetTime();
        double lDeltaTime =  lCurrentTime-stc_lastTime;
        stc_lastTime =  lCurrentTime;

        glfwGetFramebufferSize(window, &width, &height);
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

        printf("%f,%f,%f-%f,%f,%f\n",gCamDir[0],gCamDir[1],gCamDir[2],gCamPos[0],gCamPos[1],gCamPos[2]);


        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        {
            vec3 eye = {gCamPos[0], gCamPos[1], gCamPos[2]};
            vec3 center = {gCamDir[0]+gCamPos[0], gCamDir[1]+gCamPos[1], gCamDir[2]+gCamPos[2]};
            vec3 up = {0, 1.0, 0.0};

            mat4x4 m, p, mvp;
            mat4x4_identity(m);
            mat4x4_translate_in_place(m, gCamPos[0],gCamPos[1],gCamPos[2] );
            mat4x4_look_at(m,eye,center,up);
            //mat4x4_ortho(p, -gScreenRatio, gScreenRatio, -1.f, 1.f, 1.f, -1.f);
            mat4x4_perspective(p,0.785398,gScreenRatio,-1.0f,1.f);
            mat4x4_mul(mvp, p, m);
            glUniformMatrix4fv(MVP, 1, GL_FALSE, (const GLfloat*) mvp);

        }

        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
