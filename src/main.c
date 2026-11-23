#include <stdio.h>
#include <stdlib.h>

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLSL_VERSION "#version 460 core\n"
#define SHADER_STR(...) GLSL_VERSION #__VA_ARGS__

typedef struct {
    int win_width;
    int win_height;
    GLuint shader_program;
    GLuint vbo;
    GLuint ebo;
    GLuint vao;
} Oglui_Ctx;

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


static GLuint _oglui_compile_shader(const char *shader, GLenum type)
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

static GLuint _oglui_link_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader);
    glAttachShader(program_id, fragment_shader);
    glLinkProgram(program_id);
    return program_id;
}

Oglui_Ctx oglui_ctx_create(void)
{
    Oglui_Ctx ctx = {0};

    /* setup shaders */
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

    GLuint vertex_shader_id = _oglui_compile_shader(vertex_shader, GL_VERTEX_SHADER);
    GLuint fragment_shader_id = _oglui_compile_shader(fragment_shader, GL_FRAGMENT_SHADER);
    ctx.shader_program = _oglui_link_program(vertex_shader_id, fragment_shader_id);
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    glUseProgram(ctx.shader_program);

    /* setup vbo and ebo */
    glCreateBuffers(1, &ctx.vbo);
    glCreateBuffers(1, &ctx.ebo);
    glCreateVertexArrays(1, &ctx.vao);

    glNamedBufferStorage(ctx.vbo, 8 * sizeof(GLfloat), 0, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(ctx.ebo, 6 * sizeof(GLuint), 0, GL_DYNAMIC_STORAGE_BIT);

    glEnableVertexArrayAttrib(ctx.vao, 0);
    glVertexArrayAttribBinding(ctx.vao, 0, 0);
    glVertexArrayAttribFormat(ctx.vao, 0, 2, GL_FLOAT, GL_FALSE, 0);

    glVertexArrayVertexBuffer(ctx.vao, 0, ctx.vbo, 0, 2 * sizeof(float));
    glVertexArrayElementBuffer(ctx.vao, ctx.ebo);

    return ctx;
}

static void oglui_clear_background(void)
{
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void oglui_draw_rect(Oglui_Ctx *ctx, int x, int y, int width, int height)
{
    float left = (2.0 * x) / ctx->win_width - 1.0;
    float right = (2.0 * (x + width)) / ctx->win_width - 1.0;
    float top = 1.0 - (2.0 * y) / ctx->win_height;
    float bottom = 1.0 - (2.0 * (y + height)) / ctx->win_height;

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

    glNamedBufferSubData(ctx->vbo, 0, sizeof(vertices), vertices);
    glNamedBufferSubData(ctx->ebo, 0, sizeof(indices), indices);

    glBindVertexArray(ctx->vao);
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
        return die("Failed to create window");
    }
    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) {
        return die("Failed to load glad");
    }

    glfwSwapInterval(1);

    Oglui_Ctx ctx = oglui_ctx_create();
    glfwGetWindowSize(window, &ctx.win_width, &ctx.win_height);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        oglui_clear_background();

        oglui_draw_rect(&ctx, 0, 0, 100, 100);
        oglui_draw_rect(&ctx, 100, 100, 100, 100);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
