#ifndef WINDOW_HPP
#define WINDOW_HPP

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>


typedef enum {
    ASPECT_RATIO_16_9,  // HD video
    ASPECT_RATIO_4_3,   // standard monitor
    ASPECT_RATIO_3_2,   // classic film
    ASPECT_RATIO_21_9,  // cinemascope
    ASPECT_RATIO_1_1    // quadratic window
} as_ratio;

typedef struct
{
    GLFWwindow* window;
    int width;
    int height;
    std::string title;
} window_info;

class FullscreenWindow
{
private:
    bool isMinimized;

public:
    GLFWwindow* window;
    std::string title;
    int width, height;

    FullscreenWindow(GLFWwindow* window)
    {
        this->window = window;
    }
    ~FullscreenWindow() {}

    GLFWwindow* Window()
    {
        return window;
    }
};


namespace window
{
    void init_GLFW(void)
    {
        if(glfwInit() == GL_FALSE)
        {
            std::cout << "Could not initialize GLFW!" << std::endl;
        }
    }

    void init_GLEW(void)
    {
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            std::cout << "Failed to initialize GLEW" << std::endl;
        }
    }

    void set_window_hints(void)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    }

    int get_aspect_ratio_height(int width, as_ratio aspect)
    {
        int height;

        switch(aspect) {
        case ASPECT_RATIO_16_9:
            height = width / 16 * 9;
            break;
        case ASPECT_RATIO_21_9:
            height = width / 21 * 9;
            break;
        case ASPECT_RATIO_3_2:
            height = width / 3 * 2;
            break;
        case ASPECT_RATIO_4_3:
            height = width / 4 * 3;
            break;
        case ASPECT_RATIO_1_1:
            height = width;
            break;
        default:
            std::cout << "Error - undefined aspect ratio!" << std::endl;
            height = 0;
        }

        return height;
    }

    window_info* create_window(std::string title, int width, as_ratio aspect)
    {
        init_GLFW();
        set_window_hints();

        int height = get_aspect_ratio_height(width, aspect);

        window_info* win = new window_info;
        win->window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        win->width = width;
        win->height = height;

        if (win->window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return NULL;
        }
        glfwMakeContextCurrent(win->window);

        // tell OpenGL the size of the rendering window
        // this information is retrieved from GLFW
        glfwGetFramebufferSize(win->window, &win->width, &win->height);

        // parameters: bottom-left corner and the dimensions of the rendering window
        glViewport(0, 0, win->width, win->height);
        win->title = title;

        init_GLEW();
        return win;
    }

    FullscreenWindow* create_window_fullscreen(std::string title)
    {
        init_GLFW();
        set_window_hints();

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // parameters: (height, width, title, monitor, share)
        GLFWwindow* window = glfwCreateWindow(mode->width, mode->height,
                                              title.c_str(), monitor, NULL);
        if(window == NULL) {
            std::cerr << "Failed to create full screen GLFW window" << std::endl;
            return NULL;
        }
        glfwMakeContextCurrent(window);

        // create the wrapper object
        FullscreenWindow* win = new FullscreenWindow(window);
        win->title = title;
        win->width = mode->width;
        win->height = mode->height;
        win->window = window;

        // tell OpenGL the size of the rendering window
        // this information is retrieved from GLFW
        glfwGetFramebufferSize(win->window, &win->width, &win->height);

        // parameters: bottom-left corner and the dimensions of the rendering window
        glViewport(0, 0, win->width, win->height);

        init_GLEW();
        return win;
    }

    // this function may only be called from the main thread
    void change_window_resolution(GLFWwindow* window, int width, as_ratio aspect)
    {
        int height = get_aspect_ratio_height(width, aspect);
        glfwSetWindowSize(window, width, height);
    }
}

#endif // WINDOW_HPP
