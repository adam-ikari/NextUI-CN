#include "../rt64es.h"
#include <GLES3/gl32.h>
#include <stdio.h>
static struct {
    GLuint shader_program;
    GLuint vertex_array;
    GLuint vertex_buffer;
    bool initialized;
    RT64ES_DrawBatch current_batch;
    RT64ES_DirtyRect dirty_rects[16];
    int dirty_rect_count;
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
    const char* vert_src = "#version 320 es\nprecision highp float;\nlayout(location=0) in vec3 a_pos;\nlayout(location=1) in vec2 a_uv;\nlayout(location=2) in vec4 a_col;\nout vec2 v_uv;out vec4 v_col;void main(){gl_Position=vec4(a_pos,1.0);v_uv=a_uv;v_col=a_col;}";
    const char* frag_src = "#version 320 es\nprecision highp float;\nin vec2 v_uv;in vec4 v_col;out vec4 fragColor;uniform sampler2D u_tex;uniform bool u_use_tex;void main(){fragColor=u_use_tex?texture(u_tex,v_uv)*v_col:v_col;}";
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
    glGenVertexArrays(1, &g_state.vertex_array);
    glGenBuffers(1, &g_state.vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, g_state.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, RT64ES_MAX_VERTEX_BUFFER * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    g_state.current_batch.count = 0;
    g_state.dirty_rect_count = 0;
    g_state.initialized = true;
    return RT64ES_SUCCESS;
}
RT64ES_Error rt64es_deinit(void) {
    if (g_state.shader_program) glDeleteProgram(g_state.shader_program);
    if (g_state.vertex_array) glDeleteVertexArrays(1, &g_state.vertex_array);
    if (g_state.vertex_buffer) glDeleteBuffers(1, &g_state.vertex_buffer);
    g_state.initialized = false;
    return RT64ES_SUCCESS;
}
RT64ES_Error rt64es_begin_frame(void) { return g_state.initialized ? RT64ES_SUCCESS : RT64ES_ERROR_NO_GLES; }
RT64ES_Error rt64es_end_frame(void) { return g_state.initialized ? RT64ES_SUCCESS : RT64ES_ERROR_NO_GLES; }
RT64ES_Error rt64es_clear(uint32_t color, float depth) {
    if (!g_state.initialized) return RT64ES_ERROR_NO_GLES;
    glClearColor(((color>>24)&0xFF)/255.0f, ((color>>16)&0xFF)/255.0f, ((color>>8)&0xFF)/255.0f, (color&0xFF)/255.0f);
    glClearDepthf(depth);
    if (g_state.dirty_rect_count > 0) {
        glEnable(GL_SCISSOR_TEST);
        for (int i = 0; i < g_state.dirty_rect_count; i++) {
            if (g_state.dirty_rects[i].dirty) {
                glScissor(g_state.dirty_rects[i].x, g_state.dirty_rects[i].y, 
                         g_state.dirty_rects[i].width, g_state.dirty_rects[i].height);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            }
        }
        glDisable(GL_SCISSOR_TEST);
        g_state.dirty_rect_count = 0;
    } else {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    }
    return RT64ES_SUCCESS;
}
RT64ES_Error rt64es_set_viewport(int x, int y, int w, int h) { glViewport(x,y,w,h); return RT64ES_SUCCESS; }
RT64ES_Error rt64es_present(void) { return RT64ES_SUCCESS; }
RT64ES_Error rt64es_submit_draw_call(const RT64ES_DrawCall* call) {
    if (!g_state.initialized || !call) return RT64ES_ERROR_INVALID_PARAM;
    glUseProgram(g_state.shader_program);
    glBindVertexArray(g_state.vertex_array);
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

RT64ES_Error rt64es_begin_batch(uint32_t texture_id, bool depth_test, bool alpha_blend) {
    if (!g_state.initialized) return RT64ES_ERROR_NO_GLES;
    g_state.current_batch.count = 0;
    g_state.current_batch.texture_id = texture_id;
    g_state.current_batch.depth_test = depth_test;
    g_state.current_batch.alpha_blend = alpha_blend;
    return RT64ES_SUCCESS;
}

RT64ES_Error rt64es_add_to_batch(const RT64ES_DrawCall* call) {
    if (!g_state.initialized || !call) return RT64ES_ERROR_INVALID_PARAM;
    if (g_state.current_batch.count >= RT64ES_MAX_BATCH_SIZE) return RT64ES_ERROR_OUT_OF_MEMORY;
    if (call->texture_id != g_state.current_batch.texture_id ||
        call->depth_test != g_state.current_batch.depth_test ||
        call->alpha_blend != g_state.current_batch.alpha_blend) {
        return RT64ES_ERROR_INVALID_PARAM;
    }
    g_state.current_batch.calls[g_state.current_batch.count++] = *call;
    return RT64ES_SUCCESS;
}

RT64ES_Error rt64es_end_batch(void) {
    if (!g_state.initialized) return RT64ES_ERROR_NO_GLES;
    glUseProgram(g_state.shader_program);
    glBindVertexArray(g_state.vertex_array);
    if (g_state.current_batch.depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (g_state.current_batch.alpha_blend) {
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else glDisable(GL_BLEND);
    for (uint32_t i = 0; i < g_state.current_batch.count; i++) {
        glDrawArrays(g_state.current_batch.calls[i].primitive_type,
                    g_state.current_batch.calls[i].vertex_offset,
                    g_state.current_batch.calls[i].vertex_count);
    }
    g_state.current_batch.count = 0;
    return RT64ES_SUCCESS;
}

RT64ES_Error rt64es_set_dirty_rect(int x, int y, int width, int height) {
    if (g_state.dirty_rect_count >= 16) return RT64ES_ERROR_OUT_OF_MEMORY;
    g_state.dirty_rects[g_state.dirty_rect_count++] = {x, y, width, height, true};
    return RT64ES_SUCCESS;
}

RT64ES_Error rt64es_clear_dirty_rects(void) {
    g_state.dirty_rect_count = 0;
    return RT64ES_SUCCESS;
}

bool rt64es_has_dirty_rects(void) {
    return g_state.dirty_rect_count > 0;
}
