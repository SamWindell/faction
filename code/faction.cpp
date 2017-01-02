
#include "gl_lite.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "faction_platform.h"
#include "faction.h"

#define GLSL(src) "#version 150 core\n" #src

void FactionMain() {

    // Create Vertex Array Object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create a Vertex Buffer Object and copy the vertex data to it
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float vertices[] = {
    //  Position        Color             Texcoords
        -0.25f,  0.25f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // Top-left
         0.25f,  0.25f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // Top-right
         0.25f, -0.25f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
        -0.25f, -0.25f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create an Element Buffer Object and copy the element data to it
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    GLuint elements[] = {
         0, 1, 2,
         2, 3, 0
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements,
        GL_STATIC_DRAW);

    // Create and compile the vertex shader
    const char* vertexSource = GLSL(
        in vec2 position;
        in vec3 color;
        in vec2 texcoord;

        out vec3 Color;
        out vec2 Texcoord;
        
        void main() {
            Color = color;
            Texcoord = texcoord;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    );
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    const char* fragmentSource = GLSL(
        uniform sampler2D tex0;

        in vec3 Color;
        in vec2 Texcoord;

        out vec4 outColor;
        
        void main() {
            outColor = texture(tex0, Texcoord);
        }
    );   
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Create 2 textures and load images to them
    GLuint texture;
    glGenTextures(1, &texture);

    int width, height, comp = 0;
    unsigned char* img = 0;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    img = stbi_load("../resources/terrain.png", &width, &height, &comp, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(img);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

}

void RenderFrame(Vec2 windowSize, UserInput *userInput) {
	// Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, (GLint)windowSize.x, (GLint)windowSize.y);

	// Draw a quad from the 2 triangles using 6 elements
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
