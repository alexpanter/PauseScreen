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
#include <glm/gtc/type_ptr.hpp>

// SOIL
#include <SOIL/SOIL.h>

// FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H

// STANDARD
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <map>

// CUSTOM
#include "error.hpp"
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
GLuint shader_fonts;
Texture* tex0;
Texture* tex1;

glm::mat4 projection;

// buffer objects
GLuint VAO, VBO, EBO;
GLuint FT_VAO, FT_VBO;

std::string text_text = "TEXT";

// STATE MACHINE (could / should probably encapsulate this in a struct)
bool wrongPasswordStatus;
bool shouldExitStatus;

// Character
typedef struct
{
    GLuint     TextureID;  // ID handle of the glyph texture
    glm::ivec2 Size;       // size of glyph
    glm::ivec2 Bearing;    // offset from baseline to left/top of glyph
    long int   Advance;    // offset to advance to next glyph
} _character_t;



// FUNCTIONS
void resetInputMethods()
{
    readIndex = 0;
    memset(passwordBuffer, '\0', MAX_PASSWORD_BUFFER_SIZE);
}
void printBuffer()
{
    std::string msg = "entry: '";
    msg += passwordBuffer;
    msg += "'";

    PRINT_MSG_COUT(msg);
}

void insertChar(int ch)
{
    if(!(readIndex >= MAX_PASSWORD_BUFFER_SIZE))
    {
        passwordBuffer[readIndex++] = ch;
    }

    printBuffer();
}

void deleteChar()
{
    // good for deletion!
    if(readIndex > 0)
    {
        passwordBuffer[--readIndex] = '\0';
    }

    printBuffer();
}

bool testPassword()
{
    if(strncmp(password.c_str(), passwordBuffer, MAX_PASSWORD_BUFFER_SIZE) == 0)
    {
        PRINT_MSG_COUT("correct password!");
        shouldExitStatus = true;
        return true;
    }

    PRINT_MSG_CERR("invalid password!");
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


void RenderText(GLuint shader, std::map<GLchar, _character_t>& characters,
                std::string& text, glm::vec2 coords, GLfloat scale, glm::vec3 color)
{
    // activate the shader
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));
    glBindVertexArray(FT_VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        _character_t ch = characters[*c];

        GLfloat xpos = coords.x + ch.Bearing.x * scale;
        GLfloat ypos = coords.y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // update VBO for each character
        GLfloat vertices[6][4] {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // render glyph texture over quad
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glUniform1i(glGetUniformLocation(shader, "text"), ch.TextureID);

        // update contents of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, FT_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // advance cursors for next glyph (note: advance is number of 1/64 pixels)
        // bitshift by 6 to get value in pixels (2^6 = 64)
        coords.x += (ch.Advance >> 6) * scale;
    }
    // unbind
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}



void RenderLoop_Incorrect()
{
    PRINT_MSG_COUT("Entering RenderLoop_Incorrect");

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

    PRINT_MSG_COUT("Exiting RenderLoop_Incorrect");
    wrongPasswordStatus = false;
}


int main()
{
    // WINDOW
    win = window::create_window("PauseScreen", 800, ASPECT_RATIO_4_3); // 1380

    // KEY EVENTS
    glfwSetKeyCallback(win->window, key_callback);
    glfwSetCharCallback(win->window, character_callback);
    resetInputMethods();
    shouldExitStatus = false;

    // SHADERS
    shader_program = shaders::loadShadersVGF("shaders|container");
    shader_fonts = shaders::loadShadersVF("shaders|font");
    glUseProgram(shader_program);

    // TEXTURES
    unsigned long tex_options = TEX_GENERATE_MIPMAP | TEX_NEAREST_FILTER;
    tex0 = new Texture("assets|images|container.jpg", tex_options);
    tex1 = new Texture("assets|images|skull-hd.jpg", tex_options);

    // CHARACTERS
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        PRINT_MSG_CERR("FreeType: Could not init FreeType Library");
    }
    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/gentium/Gentium-I.ttf", 0, &face))
    {
        PRINT_MSG_CERR("FreeType: Failed to load font");
    }

    // width and height parameters, 0 means auto
    FT_Set_Pixel_Sizes(face, 0, 48);

    // list of characters
    std::map<GLchar, _character_t> Characters;

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // create textures for each glyph and store them in the container for later use
    for (GLubyte c = 0; c < 128; c++)
    {
        // load character glyph, set c as the active glyph
        //
        // using FT_LOAD_RENDER as flag tells FreeType to create an 8-bit greyscale
        // bitmap image that can be acessed as face->glyph->bitmap
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::string msg = "FreeType: Failed to load glyph '";
            msg += c;
            msg += "'";
            PRINT_MSG_CERR(msg);
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
    glBindTexture(GL_TEXTURE_2D, 0);

    // free memory when done processing the glyphs
    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    // vertex attributes (for fonts)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    projection = glm::ortho(0.0f, (GLfloat)win->width,
                            0.0f, (GLfloat)win->height);

    glGenVertexArrays(1, &FT_VAO);
    glGenBuffers(1, &FT_VBO);
    glBindVertexArray(FT_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, FT_VBO);

    // the 2D quad requires 6 vertices of 4 floats each, so reserve 6 * 4 floats
    // of memory. Because VBO memory will be updated quite often, GL_DYNAMIC_DRAW
    // shall suffice.
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



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
        // glUseProgram(shader_program);

        // if(wrongPasswordStatus)
        // {
        //     RenderLoop_Incorrect();
        //     continue;
        // }
        // if(shouldExitStatus)
        // {
        //     break;
        // }

        

        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // void RenderText(GLuint shader, std::map<GLchar, _character_t>& characters,
        //         std::string& text, glm::vec2 coords, GLfloat scale, glm::vec3 color)
        std::string str = "(C) LearnOpenGL.com";
        RenderText(shader_fonts, Characters, text_text, glm::vec2(25.0f, 25.0f),
                   1.0f, glm::vec3(0.5f, 0.8f, 0.2f));
        RenderText(shader_fonts, Characters, str, glm::vec2(540.0f, 570.0f),
                   0.5f, glm::vec3(0.3, 0.7f, 0.9f));

        glfwSwapBuffers(win->window);

        glfwPollEvents();
        // glfwWaitEvents(); // puts the main thread to sleep

    }

    // do proper cleanup of any allocated resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();

    // exiting the application
    return 0;
}

