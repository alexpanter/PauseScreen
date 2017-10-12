// GLEW
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>

// SOIL
#include <SOIL/SOIL.h>

#include <iostream>
#include <string.h>
#include <string>

// local includes (static linking or not)
#include "shaders.hpp"
#include "window.hpp"
#include "texture.hpp"


// FUNCTION PROTOTYPES:
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);


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


int main()
{
    // WINDOW
    window_info* win = window::create_window("Textures 2", 1380, ASPECT_RATIO_16_9);

    // KEY EVENTS
    glfwSetKeyCallback(win->window, key_callback);

    // SHADERS
    GLuint shader_program = shaders::loadShaders("shaders|container");
    glUseProgram(shader_program);

    // TEXTURES
    unsigned long tex_options = TEX_GENERATE_MIPMAP | TEX_NEAREST_FILTER;
    Texture tex0 = Texture("assets|images|container.jpg", tex_options);
    Texture tex1 = Texture("assets|images|skull-hd.jpg", tex_options);


    // buffer objects
    GLuint VAO, VBO, EBO;

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
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);

        // Activate texture unit 0 first before binding texture
        // can bind multiple textures to the same shader program by
        // activating the, e.g. `GL_TEXTURE1`, `GL_TEXTURE2`, etc.
        // Can allow up to `GL_MAX_TEXTURE_UNITS` (probably 16)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0.GetTexture());
        glUniform1i(glGetUniformLocation(shader_program, "U_texture0"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex1.GetTexture());
        glUniform1i(glGetUniformLocation(shader_program, "U_texture1"), 1);

        GLfloat timeValue = sin(glfwGetTime() / 3.0);
        float mixer = (timeValue < 0.0) ? -timeValue : timeValue;
        glUniform1f(glGetUniformLocation(shader_program, "mixer"), mixer);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

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

// handle user input on the keyboard
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}
