#include "../rt64es.h"
#include <GLES3/gl32.h>
#include <stdio.h>
static struct {
    GLuint shader_program;
    bool initialized;
} g_state = {0};
const char* rt64es_get_error_string(RT64ES_Error error) {
    switch (error) {
        case RT64ES_SUCCESS: return "Success";
        case RT64ES_ERROR_NO_GLES: return "OpenGL ES not available";
        case RT64ES_ERROR_NO_ES3_2: return "OpenGL ES 3.2 not supported";
        default: return "Unknown error";
    }
}
static GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) return 0;
    return shader;
}
RT64ES_Error rt64es_init(void) {
    if (g_state.initialized) return RT64ES_SUCCESS;
    const char* version = (const char*)glGetString(GL_VERSION);
    if (!version || strstr(version, "OpenGL ES 3.2") == NULL) return RT64ES_ERROR_NO_ES3_2;
    const char* vert_src = "#version 320 es\nprecision highp float;\nlayout(location=0) in vec3 a_pos;\nout vec2 v_uv;\nvoid main(){gl_Position=vec4(a_pos,1.0);v_uv=a_pos.xy*0.5+0.5;}";
    const char* frag_src = "#version 320 es\nprecision highp float;\nin vec2 v_uv;\nout vec4 fragColor;\nuniform vec4 u_color;\nvoid main(){fragColor=u_color;}";
    GLuint v = compile_shader(GL_VERTEX_SHADER, vert_src);
    GLuint f = compile_shader(GL_FRAGMENT_SHADER, frag_src);
    if (!v || !f) return RT64ES_ERROR_SHADER_COMPILE;
    g_state.shader_program = glCreateProgram();
    glAttachShader(g_state.shader_program, v);
    glAttachShader(g_state.shader_program, f);
    glLinkProgram(g_state.shader_program);
    glDeleteShader(v); glDeleteShader(f);
    GLint ok; glGetProgramiv(g_state.shader_program, GL_LINK_STATUS, &ok);
    if (!ok) return RT64ES_ERROR_SHADER_COMPILE;
    g_state.initialized = true;
    return RT64ES_SUCCESS;
}
RT64ES_Error rt64es_deinit(void) {
    if (g_state.shader_program) glDeleteProgram(g_state.shader_program);
    g_state.initialized = false;
    return RT64ES_SUCCESS;
}
RT64ES_Error rt64es_begin_frame(void) { return g_state.initialized ? RT64ES_SUCCESS : RT64ES_ERROR_NO_GLES; }
RT64ES_Error rt64es_end_frame(void) { return g_state.initialized ? RT64ES_SUCCESS : RT64ES_ERROR_NO_GLES; }
RT64ES_Error rt64es_clear(uint32_t color, float depth) {
    if (!g_state.initialized) return RT64ES_ERROR_NO_GLES;
    glClearColor(((color>>24)&0xFF)/255.0f, ((color>>16)&0xFF)/255.0f, ((color>>8)&0xFF)/255.0f, (color&0xFF)/255.0f);
    glClearDepthf(depth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    return RT64ES_SUCCESS;
}
RT64ES_Error rt64es_set_viewport(int x, int y, int w, int h) { glViewport(x,y,w,h); return RT64ES_SUCCESS; }
RT64ES_Error rt64es_present(void) { return RT64ES_SUCCESS; }
RT64ES_Error rt64es_submit_draw_call(const RT64ES_DrawCall* call) {
    if (!g_state.initialized || !call) return RT64ES_ERROR_INVALID_PARAM;
    glUseProgram(g_state.shader_program);
    if (call->depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    glDrawArrays(call->primitive_type, call->vertex_offset, call->vertex_count);
    return RT64ES_SUCCESS;
}
RT64ES_Error rt64es_create_texture(const RT64ES_TextureConfig* cfg, const void* data, uint32_t* id) {
    if (!cfg || !id) return RT64ES_ERROR_INVALID_PARAM;
    GLuint tex; glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, cfg->format, cfg->width, cfg->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, cfg->min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, cfg->mag_filter);
    *id = tex; return RT64ES_SUCCESS;
}
RT64ES_Error rt64es_destroy_texture(uint32_t id) { glDeleteTextures(1, &id); return RT64ES_SUCCESS; }
