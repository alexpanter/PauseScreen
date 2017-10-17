// GLEW
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// SOIL
#include <SOIL/SOIL.h>

// FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H

// STANDARD
#include <iostream>
#include <string.h>
#include <string>
#include <map>

// CUSTOM
#include "shaders.hpp"
#include "window.hpp"
#include "texture.hpp"



// VERTEX DATA
GLfloat vertices[] = {
    // Positions          // Colors           // Texture Coords
    -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,   // Top Left
     1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // Top Right
     1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // Bottom Right
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f    // Bottom Left
};
GLuint indices[] = {  // Note that we start from 0!
        0, 1, 2, // First Triangle
        0, 2, 3  // Second Triangle
};

// GLOBALS
window_info* win;
const std::string password("hej");

#define MAX_PASSWORD_BUFFER_SIZE 50
char passwordBuffer[MAX_PASSWORD_BUFFER_SIZE];
unsigned short int readIndex;

GLuint shader_program;
Texture* tex0;
Texture* tex1;

// buffer objects
GLuint VAO, VBO, EBO;
GLuint FT_VAO, FT_VBO;

// STATE MACHINE (could / should probably encapsulate this in a struct)
bool wrongPasswordStatus;
bool shouldExitStatus;

// Character
typedef struct
{
    GLuint     TextureID;  // ID handle of the glyph texture
    glm::ivec2 Size;       // size of glyph
    glm::ivec2 Bearing;    // offset from baseline to left/top of glyph
    GLuint     Advance;    // offset to advance to next glyph
} _character_t;



// FUNCTIONS
void resetInputMethods()
{
    readIndex = 0;
    memset(passwordBuffer, '\0', MAX_PASSWORD_BUFFER_SIZE);
}

void insertChar(int ch)
{
    if(!(readIndex >= MAX_PASSWORD_BUFFER_SIZE))
    {
        passwordBuffer[readIndex++] = ch;
    }

    std::cout << "entry: '" << passwordBuffer << "'" << std::endl;
}

void deleteChar()
{
    // good for deletion!
    if(readIndex > 0)
    {
        passwordBuffer[--readIndex] = '\0';
    }

    std::cout << "entry: '" << passwordBuffer << "'" << std::endl;
}

bool testPassword()
{
    if(strncmp(password.c_str(), passwordBuffer, MAX_PASSWORD_BUFFER_SIZE) == 0)
    {
        std::cout << "correct password!" << std::endl;
        shouldExitStatus = true;
        return true;
    }

    std::cout << "invalid password!" << std::endl;
    resetInputMethods();
    wrongPasswordStatus = true;
    return false;
}

// handle user input on the keyboard (last arg is 'modifier_bits')
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        switch(key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_BACKSPACE:
            deleteChar();
            break;
        case GLFW_KEY_ENTER:
            testPassword();
        default:
            break;
        }
    }
}

// this callback function receives key presses as unicode points
void character_callback(GLFWwindow* window, unsigned int codepoint)
{
    insertChar(codepoint);
}



void RenderLoop_Incorrect()
{
    std::cout << "Entering RenderLoop_Incorrect" << std::endl;

    // some timer structure ...
    const double nowTime = glfwGetTime();
    double passedTime = nowTime;

    // this loop should run for 5 seconds
    while(passedTime - nowTime < 5.0)
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);

        // Activate texture unit 0 first before binding texture
        // can bind multiple textures to the same shader program by
        // activating the, e.g. `GL_TEXTURE1`, `GL_TEXTURE2`, etc.
        // Can allow up to `GL_MAX_TEXTURE_UNITS` (probably 16)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0->GetTexture());
        glUniform1i(glGetUniformLocation(shader_program, "U_texture0"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex1->GetTexture());
        glUniform1i(glGetUniformLocation(shader_program, "U_texture1"), 1);

        GLfloat timeValue = sin(glfwGetTime() / 3.0);
        float mixer = (timeValue < 0.0) ? -timeValue : timeValue;
        glUniform1f(glGetUniformLocation(shader_program, "mixer"), mixer);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(win->window);
        passedTime = glfwGetTime();
    }

    std::cout << "Exiting RenderLoop_Incorrect" << std::endl;
    wrongPasswordStatus = false;
}


int main()
{
    // WINDOW
    win = window::create_window("PauseScreen", 400, ASPECT_RATIO_16_9); // 1380

    // KEY EVENTS
    glfwSetKeyCallback(win->window, key_callback);
    glfwSetCharCallback(win->window, character_callback);
    resetInputMethods();
    shouldExitStatus = false;

    // SHADERS
    shader_program = shaders::loadShaders("shaders|container");
    glUseProgram(shader_program);

    // TEXTURES
    unsigned long tex_options = TEX_GENERATE_MIPMAP | TEX_NEAREST_FILTER;
    tex0 = new Texture("assets|images|container.jpg", tex_options);
    tex1 = new Texture("assets|images|skull-hd.jpg", tex_options);

    // CHARACTERS
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }
    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    // width and height parameters, 0 means auto
    FT_Set_Pixel_Sizes(face, 0, 48);

    // set the glyph 'X' as the active glyph
    //
    // using FT_LOAD_RENDER as flag tells FreeType to create an 8-bit greyscale
    // bitmap image that can be acessed as face->glyph->bitmap
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
    }

    // list of characters
    std::map<GLchar, _character_t> Characters;

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // create textures for each glyph and store them in the container for later use
    for (GLubyte c = 0; c < 128; c++)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYPE: Failed to load glyph '"
                      << c << "'" << std::endl;
            continue;
        }

        // generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
                     face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);

        // set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // store the character for later use
        _character_t character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, _character_t>(c, character));
    }

    // free memory when done processing the glyphs
    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    // vertex attributes (for fonts)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);




    // vertex attributes (for background image)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // color attributes
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (GLfloat*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // texture coordinate attribute (texCoords)
    glVertexAttribPointer(2, 2, GL_FLOAT,GL_FALSE, 8 * sizeof(GLfloat),
                          (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);


    while(!glfwWindowShouldClose(win->window))
    {
        if(wrongPasswordStatus)
        {
            RenderLoop_Incorrect();
            continue;
        }
        if(shouldExitStatus)
        {
            break;
        }

        //glfwPollEvents();
        glfwWaitEvents(); // puts the main thread to sleep

        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(win->window);
    }

    // do proper cleanup of any allocated resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();

    // exiting the application
    return 0;
}

