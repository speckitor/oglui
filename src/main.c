#include <stdio.h>
#include <stdlib.h>

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLSL_VERSION "#version 460 core\n"
#define SHADER_STR(...) GLSL_VERSION #__VA_ARGS__

static int die(const char *msg)
{
    fprintf(stderr, msg);
    return EXIT_FAILURE;
}

static void error_callback(int error, const char *desc)
{
    fprintf(stderr, "[ERROR]: %s\n", desc);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

GLuint compile_shader(const char *shader, GLenum type)
{
    GLuint shader_id = glCreateShader(type);
    glShaderSource(shader_id, 1, &shader, NULL);
    glCompileShader(shader_id);

    GLint compileStatus;

    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compileStatus);

    if (!compileStatus) {
        int length;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
        char buff[1024] = {0};
        glGetShaderInfoLog(shader_id, length, &length, buff);
        printf("%s\n", buff);
    }

    return shader_id;
}

GLuint link_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader);
    glAttachShader(program_id, fragment_shader);
    glLinkProgram(program_id);
    return program_id;
}


/* really slow because of creating buffers on every frame, but works
 * also leaks buffers
 */
static void draw_rect(GLFWwindow *win, int x, int y, int width, int height)
{
    int win_width, win_height;
    glfwGetWindowSize(win, &win_width, &win_height);

    float left = (2.0 * x) / win_width - 1.0;
    float right = (2.0 * (x + width)) / win_width - 1.0;
    float top = 1.0 - (2.0 * y) / win_height;
    float bottom = 1.0 - (2.0 * (y + height)) / win_height;

    const GLfloat vertices[] = {
        right, top,
        left, top,
        left, bottom,
        right, bottom,
    };

    const GLuint indices[] = {
        0, 1, 2,
        0, 2, 3,
    };

    GLuint vbo_id, ebo_id;
    glCreateBuffers(1, &vbo_id);
    glCreateBuffers(1, &ebo_id);

    glNamedBufferStorage(vbo_id, sizeof(vertices), vertices, 0);
    glNamedBufferStorage(ebo_id, sizeof(indices), indices, 0);

    GLuint vao_id;
    glCreateVertexArrays(1, &vao_id);

    glEnableVertexArrayAttrib(vao_id, 0);
    glVertexArrayAttribBinding(vao_id, 0, 0);
    glVertexArrayAttribFormat(vao_id, 0, 2, GL_FLOAT, GL_FALSE, 0);

    glVertexArrayVertexBuffer(vao_id, 0, vbo_id, 0, 2 * sizeof(float));
    glVertexArrayElementBuffer(vao_id, ebo_id);

    glBindVertexArray(vao_id);
    glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_INT, 0);
}

int main(void)
{
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        return die("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "My window", NULL, NULL);
    if (!window) {
        die("Failed to create window");
    }
    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) {
        return die("Failed to load glad");
    }

    glfwSwapInterval(1);

    const char *vertex_shader = SHADER_STR(
    layout (location = 0) in vec2 a_Pos;

    void main() {
        gl_Position = vec4(a_Pos, 1.0, 1.0);
    }
    );

    const char *fragment_shader = SHADER_STR(
    out vec4 fragColor;
    void main() {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
    );

    GLuint vShader_id = compile_shader(vertex_shader, GL_VERTEX_SHADER);
    GLuint fShader_id = compile_shader(fragment_shader, GL_FRAGMENT_SHADER);
    GLuint program_id = link_program(vShader_id, fShader_id);
    glUseProgram(program_id);
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw_rect(window, 0, 0, 100, 100);
        draw_rect(window, 100, 100, 100, 100);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
