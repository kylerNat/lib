#ifndef WIN23_GL_WINDOW
#define WIN23_GL_WINDOW

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include <GL/GL.h>
#include "gl/glext.h"
#include "gl/wglext.h"

#include <maths/maths.h>
#include <utils/misc.h>
#include <utils/logging.h>

#define platform_big_alloc(memory_size) VirtualAlloc(0, memory_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
#include <utils/memory.h>
// #include <maths/maths.h>

#define gl_load_operation(rval, ext, args)  typedef rval (APIENTRY * ext##_Func)args; ext##_Func ext = 0;
#include "gl_functions_list.h"

void load_gl_functions()
{
    HMODULE gl_module_handle = GetModuleHandle("opengl32");

    // this is pointer to function which returns pointer to string with list of all wgl extensions
    PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT = 0;
    wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) wglGetProcAddress("wglGetExtensionsStringEXT");

    assert(wglGetExtensionsStringEXT, "could not load wglGetExtensionsStringEXT");
    const char* extensions_list = wglGetExtensionsStringEXT();

    #define gl_check(extension) if(strstr(extensions_list, extension))
    #define gl_load_operation(ret, func, args) func = (CONCAT(func, _Func)) wglGetProcAddress(STR(func)); assert(func, STR(func) " could not be loaded");
    #include "gl_functions_list.h"
}

__inline uint get_cd(char* output)
{
    return GetCurrentDirectory(GetCurrentDirectory(0,0), output);
}

__inline void set_cd(char* directory_name)
{
    SetCurrentDirectory(directory_name);
}

__inline int load_file(char* filename, char* output)
{
    HANDLE file = CreateFile(filename,
                             GENERIC_READ,
                             FILE_SHARE_READ, 0,
                             OPEN_EXISTING,
                             FILE_FLAG_SEQUENTIAL_SCAN, 0);

    if(file == INVALID_HANDLE_VALUE)
    {
        log_error("windows code ", GetLastError(), ", file ", filename, " could not be found\n");
        exit(EXIT_SUCCESS);
    }

    LARGE_INTEGER file_size;
    auto error = GetFileSizeEx(file, &file_size);

    if(!error)
    {
        log_error(GetLastError(), " opening file\n");
        exit(EXIT_SUCCESS);
    }

    DWORD bytes_read;
    error = ReadFile(file,
                     output,
                     file_size.LowPart,
                     &bytes_read,
                     0);
    if(!error)
    {
        log_error("error reading file\n");
        exit(EXIT_SUCCESS);
    }
    CloseHandle(file);

    return bytes_read;
}

char* load_file_0_terminated(char* filename)
{
    char* output = (char*) free_memory;
    size_t output_size = load_file(filename, output);
    output[output_size] = 0;
    permalloc(output_size+1);
    return output;
}

#include "gl_graphics.h"

WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

struct window_t
{
    HWND hwnd;
    HGLRC hglrc;
};

void fullscreen(window_t wnd)
{
    DWORD dwStyle = GetWindowLong(wnd.hwnd, GWL_STYLE);
    if (dwStyle & WS_OVERLAPPEDWINDOW) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(wnd.hwnd, &g_wpPrev) &&
            GetMonitorInfo(MonitorFromWindow(wnd.hwnd,
                                             MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong(wnd.hwnd, GWL_STYLE,
                          dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(wnd.hwnd, HWND_TOP,
                         mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(wnd.hwnd, GWL_STYLE,
                      dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(wnd.hwnd, &g_wpPrev);
        SetWindowPos(wnd.hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

static HINSTANCE h_instance;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SIZE:
        {
            // //TODO: set correct context
            auto window_width = LOWORD(lParam);
            auto window_height = HIWORD(lParam);
            glViewport(0.5*(window_width-window_height), 0, window_height, window_height);
            break;
        }
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

window_t create_window(char* window_title, char* class_name, int width, int height, int x, int y)
{
    int error;
    WNDCLASSEX wc;
    {//init the window class
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = h_instance;
        wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName = 0;
        wc.lpszClassName = class_name;
        wc.hIconSm = LoadIcon(0, IDI_APPLICATION);

        error = RegisterClassEx(&wc);
        assert(error, "window registration failed");
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_APPWINDOW, //extended window style
        class_name, //the class name
        window_title, //The window title
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, //window style //TODO: allow user to specify these
        x, y, //window_position
        width, height, //window dimensions
        0, //handle to the parent window, this has no parents
        0, //menu handle
        h_instance, //duh
        0 //lparam
        );
    assert(hwnd, "window creation failed");

    HGLRC hglrc;
    {//init gl context
        HDC dc = GetDC(hwnd);
        assert(dc, "could not get dc");

        // const int attribList[] = {
        //     WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        //     WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        //     WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        //     WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        //     WGL_COLOR_BITS_ARB, 32,
        //     WGL_DEPTH_BITS_ARB, 32, //TODO: allow user to set these parameters
        //     WGL_STENCIL_BITS_ARB, 0,
        //     0,        //End
        // };

        // int pixFormat;
        // uint numFormats;

        // wglChoosePixelFormatARB(dc, attribList, 0, 1, &pixFormat, &numFormats);

        // PIXELFORMATDESCRIPTOR pfd;
        // error = SetPixelFormat(dc, pixFormat, &pfd);
        // assert(error, "could not set pixel format")

        {
            PIXELFORMATDESCRIPTOR pfd =
                {
                    .nSize = sizeof(PIXELFORMATDESCRIPTOR),
                    .nVersion = 1,
                    .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
                    .iPixelType = PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
                    .cColorBits = 32,                        //Colordepth of the framebuffer.
                    .cAlphaBits = 8,
                    .cDepthBits = 32,                        //Number of bits for the depthbuffer
                    .cStencilBits = 0,                        //Number of bits for the stencilbuffer
                    .iLayerType = PFD_MAIN_PLANE,
                };

            int pf = ChoosePixelFormat(dc, &pfd);
            assert(pf, "could not choose pixel format");
            error = SetPixelFormat(dc, pf, &pfd);
            assert(error, "could not set pixel format");
        }

        int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 4,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            0        //End
        };

        hglrc = wglCreateContextAttribsARB(dc, 0, attribs);
        assert(hglrc, "could not create gl context")

        error = wglMakeCurrent(dc, hglrc);
        assert(error, "could not make gl context current")

        log_output("OPENGL VERSION ", (char*)glGetString(GL_VERSION), "\n");
    }


    {
        glEnable(GL_FRAMEBUFFER_SRGB);

        wglSwapIntervalEXT(1);
    }

    //TODO: figure this out
    glDebugMessageCallbackARB(gl_error_callback, 0);
    glEnable(GL_DEBUG_OUTPUT);

    return {hwnd, hglrc};
}

window_t create_window(char* window_title, char* class_name)
{
    return create_window(window_title, class_name, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT);
}

void show_window(window_t wnd)
{
    ShowWindow(wnd.hwnd, SW_NORMAL); //TODO: support other showing modes
    UpdateWindow(wnd.hwnd);
}

int update_window(window_t wnd)
{
    MSG msg;
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);

        switch(msg.message)
        {
            case WM_QUIT:
            {
                return 0;
            }
            default:
                DispatchMessage(&msg);
        }
    }

    HDC dc = GetDC(wnd.hwnd);
    // SwapBuffers(dc);

    // RECT wnd_rect;
    // GetWindowRect(wnd.hwnd, &wnd_rect);

    // auto window_width = wnd_rect.right-wnd_rect.left;
    // auto window_height = wnd_rect.bottom-wnd_rect.top;
    // glViewport(0.5*(window_width-window_height), 0, window_height, window_height);

    real gamma = 2.2;
    // glClearColor(pow(0.2, gamma), pow(0.0, gamma), pow(0.3, gamma), 1.0);
    glClearColor(0.25, 0.0, 0.35, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return 1;
}

// int main(int n_args, char** args);
int mymain();
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    h_instance = hInstance;

    //TODO; check if there is somewhere better to put this
    {//Load GL functions
        int error;

        const char* dummy_class_name = "dummyglinit";
        {//init the window class
            WNDCLASSA window_class = {
                .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
                .lpfnWndProc = DefWindowProc,
                .hInstance = h_instance,
                .lpszClassName = dummy_class_name,
            };

            error = RegisterClassA(&window_class);
            assert(error, "failed to register dummy OpenGL window.");
        }

        HWND dummy_hwnd = CreateWindowEx(
            0, //extended window style
            dummy_class_name, //the class name
            "", //The window title
            0, //window style
            CW_USEDEFAULT, CW_USEDEFAULT, //window_position
            CW_USEDEFAULT, CW_USEDEFAULT, //window dimensions
            0, //handle to the parent window, this has no parents
            0, //menu handle
            h_instance, //duh
            0 //lparam
            );
        assert(dummy_hwnd, "dummy window creation failed");

        HDC dummy_dc;
        {
            dummy_dc = GetDC(dummy_hwnd);
            assert(dummy_dc, "could not get dummy dc");

            PIXELFORMATDESCRIPTOR pfd =
                {
                    .nSize = sizeof(PIXELFORMATDESCRIPTOR),
                    .nVersion = 1,
                    .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
                    .iPixelType = PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
                    .cColorBits = 32,                        //Colordepth of the framebuffer.
                    .cAlphaBits = 8,
                    .cDepthBits = 32,                        //Number of bits for the depthbuffer
                    .cStencilBits = 0,                        //Number of bits for the stencilbuffer
                    .iLayerType = PFD_MAIN_PLANE,
                };

            int pf = ChoosePixelFormat(dummy_dc, &pfd);
            assert(pf, "could not choose pixel format");
            error = SetPixelFormat(dummy_dc, pf, &pfd);
            assert(error, "could not set pixel format");
        }
        HGLRC dummy_glrc = wglCreateContext(dummy_dc);
        assert(dummy_glrc, "could not create dummy gl context");

        error = wglMakeCurrent(dummy_dc, dummy_glrc);
        assert(error, "could not activate dummy gl context");

        load_gl_functions();

        log_output("OPENGL VERSION ", (char*)glGetString(GL_VERSION), "\n");

        wglMakeCurrent(dummy_dc, 0);
        wglDeleteContext(dummy_glrc);
        ReleaseDC(dummy_hwnd, dummy_dc);
        DestroyWindow(dummy_hwnd);
    }

    //TODO: figure out how to pass arguments
    int n_args;
    char** args = (char**) 0;//CommandLineToArgvW(GetCommandLineW(), &n_args);
    // return main(n_args, args);
    return mymain();

/////////////////////////////////////////////////////
    // GLuint fbo;
    // GLuint colorbuffer;
    // GLuint depthbuffer;
    // {//TODO: use direct state access?
    //     uint n_samples = 8;
    //     printf("%dX Multisampling\n", n_samples);

    //     //fbo
    //     glGenFramebuffers(1, &fbo);
    //     glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    //     //color buffer
    //     glGenRenderbuffers(1, &colorbuffer);
    //     glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
    //     glRenderbufferStorageMultisample(GL_RENDERBUFFER, n_samples, GL_RGBA8, window_width, window_height);
    //     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuffer);

    //     // //depth buffer
    //     glGenRenderbuffers(1, &depthbuffer);
    //     glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
    //     glRenderbufferStorageMultisample(GL_RENDERBUFFER, n_samples, GL_DEPTH_COMPONENT32, window_width, window_height);
    //     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);

    //     //errors
    //     int error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    //     assert(error == GL_FRAMEBUFFER_COMPLETE);
    // }

    // {
    //     glEnable(GL_DEPTH_TEST);
    //     glDepthMask(GL_TRUE);
    //     glDepthFunc(GL_LEQUAL);
    //     glDepthRange(0.0f, 1.0f);
    //     glEnable(GL_DEPTH_CLAMP);

    //     // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //     glFrontFace(GL_CCW);
    //     // glCullFace(GL_BACK);
    //     // glEnable(GL_CULL_FACE);

    //     glLineWidth(1.0);

    //     glEnable(GL_FRAMEBUFFER_SRGB);
    //     #define gamma 2.2
    //     glClearColor(pow(0.2, gamma), pow(0.0, gamma), pow(0.3, gamma), 1.0);
    //     glClearDepth(1.0);

    //     wglSwapIntervalEXT(1);
    // }

    // LARGE_INTEGER timer_frequency;
    // LARGE_INTEGER last_time = {0};
    // LARGE_INTEGER this_time = {0};

    // QueryPerformanceFrequency(&timer_frequency);

    // // enum button_code
    // // {
    // //     button_lmb,
    // //     button_rmb,
    // //     button_count,
    // // };
    // // byte buttons[(button_count-1)/8+1];
    // // #define set_button_off(code) (buttons[code/8] |= 1<<code%8)
    // // #define set_button_on(code) (buttons[code/8] &= ~(1<<code%8))
    // // #define get_button(code) ((buttons[code/8]>>(code%8))&1)

    //     glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //     glViewport(0, 0, window_width, window_height);
    //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //     glUseProgram(program);

    //     glBindTexture(GL_TEXTURE_2D, data_texture[0]);
    //     glUniform1i(data_uniform, 0);
    //     glUniformMatrix4fv(transform_uniform, 1, false, (float *) &camera);

    //     // bind_vertex_and_index_buffers(vi_buffers[vi_id_cube].vb, vi_buffers[vi_id_cube].ib);
    //     // glDrawElements(GL_TRIANGLES, vi_buffers[vi_id_cube].n, GL_UNSIGNED_SHORT, 0);

    //     glPatchParameteri(GL_PATCH_VERTICES, 4);
    //     glBegin(GL_PATCHES);
    //     glVertexAttrib2f(attrib_tex_coord, 0.0, 0.0); glVertexAttrib3f(attrib_normal, 0.0, 0.0, 1.0); glVertex3f(-1, -1, 0);
    //     glVertexAttrib2f(attrib_tex_coord, 1.0, 0.0); glVertexAttrib3f(attrib_normal, 0.0, 0.0, 1.0); glVertex3f( 1, -1, 0);
    //     glVertexAttrib2f(attrib_tex_coord, 1.0, 1.0); glVertexAttrib3f(attrib_normal, 0.0, 0.0, 1.0); glVertex3f( 1,  1, 0);
    //     glVertexAttrib2f(attrib_tex_coord, 0.0, 1.0); glVertexAttrib3f(attrib_normal, 0.0, 0.0, 1.0); glVertex3f(-1,  1, 0);
    //     glEnd();

    //     //render fbo
    //     #if 1
    //     // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    //     // glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    //     // glDrawBuffer(GL_BACK);                       // Set the back buffer as the draw buffer

    //     // glBlitFramebuffer(0, 0, window_width, window_height,
    //     //                   0, 0, window_width, window_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    //     #else
    //     glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //     glViewport(0, 0, window_width, window_height);
    //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //     glUseProgram(post_program);

    //     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //     glActiveTexture(GL_TEXTURE0);
    //     glBindTexture(GL_TEXTURE_2D, colorbuffer);
    //     glUniform1i(post_tex_uniform, 0);

    //     glUniform1i(width_uniform, window_width);
    //     glUniform1i(height_uniform, window_height);

    //     glBegin(GL_TRIANGLE_FAN);

    //     glVertex2f( 1.0,  1.0);
    //     glVertex2f(-1.0,  1.0);
    //     glVertex2f(-1.0, -1.0);
    //     glVertex2f( 1.0, -1.0);

    //     glEnd();
    //     #endif
    //     SwapBuffers(dc);
    // }
    // while(msg.message != WM_QUIT);

    // return 0;
}

void draw_circle(window_t wnd, real x, real y, real r, real red, real green, real blue)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(pinfo_2d_flat.program);

    real transform[] = {
        r, 0, x,
        0, r, y,
        0, 0, 1,
    };
    glUniformMatrix3fv(pinfo_2d_flat.uniforms[0], 1, true, transform);

    glUniform3f(pinfo_2d_flat.uniforms[1], red, green, blue);

    // glBegin(GL_TRIANGLE_FAN);

    // glVertex3f( 1.0,  1.0, -1.0);
    // glVertex3f(-1.0,  1.0, -1.0);
    // glVertex3f(-1.0, -1.0, -1.0);
    // glVertex3f( 1.0, -1.0, -1.0);

    // glEnd();
    glBegin(GL_POLYGON);
    int n_lines = 10;
    for(int i = 0; i < n_lines; i++)
    {
        glVertex3f(cos(2*pi*i/n_lines), sin(2*pi*i/n_lines), 1);
    }
    // glVertex3f(-1, -1, 0);
    // glVertex3f( 1, -1, 0);
    // glVertex3f( 1,  1, 0);
    // glVertex3f(-1,  1, 0);
    glEnd();
}

#endif //WIN23_GL_WINDOW
