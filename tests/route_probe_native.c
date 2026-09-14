/* Exercise actual app callbacks with public SDK types. No physical routing
 * claims: the two mock buses are intentionally independent allocations. */
#define tapp_get_descriptor route_descriptor
#include "../tapp/route_probe/route_probe.c"
#undef tapp_get_descriptor
#define tapp_get_descriptor source_descriptor
#include "../tapp/route_source_probe/route_source_probe.c"
#undef tapp_get_descriptor
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

struct engine_s { void* ctx; };
/* SDK opaque spelling is validated by compiler via engine_t below. */
struct mixer_s { float* in; float* out; uint32_t n; };
struct gfx_t { int unused; };
static void* context;
static const engine_callbacks_t* installed;
static unsigned install_count, clears, exits, out_reads;
static int32_t delta;
static bool monitor;
static uint8_t input_source;
static char drawing[2048];
const uint8_t gfx_nunito_bold_18[] = {0};
const uint8_t gfx_nunito_semibold_14[] = {0};
void* engine_get_ctx(engine_t* e) { (void)e; return context; }
void* os_app_get_model(const os_app_t* a) { return a->data->model; }
void engine_set_callbacks(os_app_t* a, void* ctx) { installed = a->engine_cb; context = ctx; install_count++; }
void engine_set_active(bool value) { assert(installed); installed->active(NULL, value); }
void engine_clear_callbacks(const engine_callbacks_t* cb) { assert(cb == installed); clears++; installed = NULL; context = NULL; }
float* mixer_get_in(mixer_t* m) { return m->in; }
float* mixer_get_out(mixer_t* m) { out_reads++; return m->out; }
uint32_t mixer_get_fs(mixer_t* m) { return m->n; }
bool os_audio_get_monitor(void) { return monitor; }
uint8_t os_audio_get_input(void) { return input_source; }
int32_t os_controls_encoder_get_delta(void) { int32_t d = delta; delta = 0; return d; }
void os_app_exit(void) { exits++; }
void ui_statusbar_show(bool v) { assert(v); }
void ui_hints_show(bool v) { assert(v); }
void ui_hints_set_labels(const tapp_hint_pair_t* h) { assert(h); }
void gfx_set_color(gfx_t* g, uint8_t c) { (void)g; assert(c == 1); }
void gfx_set_font(gfx_t* g, const uint8_t* f) { (void)g; assert(f); }
gfx_uint_t gfx_draw_str(gfx_t* g, gfx_uint_t x, gfx_uint_t y, const char* s) {
    (void)g; assert(x == 10 && y >= 60 && y < 240);
    assert(strlen(drawing) + strlen(s) + 2 < sizeof(drawing));
    strcat(drawing, s); strcat(drawing, "\n"); return 0;
}
gfx_uint_t gfx_draw_strf(gfx_t* g, gfx_uint_t x, gfx_uint_t y, const char* fmt, ...) {
    va_list args; va_start(args, fmt); char s[256]; vsnprintf(s, sizeof(s), fmt, args); va_end(args);
    return gfx_draw_str(g,x,y,s);
}
static bool init(os_app_t* a, ...) { va_list args; va_start(args,a); bool ok=a->data->init(a,args); va_end(args); return ok; }
static route_model_t model;
static source_model_t src_model;
static os_app_t* app;
static void setup(bool src) {
    assert(!installed);
    memset(&model,0xa5,sizeof(model)); memset(&src_model,0xa5,sizeof(src_model));
    app = src ? source_descriptor() : route_descriptor();
    app->data->model = src ? (void*)&src_model : &model;
    delta = 0; monitor = false; input_source = 0; exits = 0;
    assert(init(app)); assert(installed && context == app->data->model);
    assert(installed->is_active(NULL));
}
static void finish(void) { assert(app->data->deinit(app)); assert(!installed && !context); }
static void select_mode(unsigned mode) {
    while (ROUTE_MODE(probe_load(&model.control)) != mode) app->on_input(app,0,KEY_STATE_PRESSED);
}
static void arm(void) { app->on_input(app,1,KEY_STATE_HOLD); }
static void render(const char* expected) {
    drawing[0]=0; app->redraw(NULL,app); assert(strstr(drawing,expected));
}
static void test_modes(void) {
    setup(false);
    assert(probe_load(&model.control) == ROUTE_DEFAULT);
    for (unsigned mode=0; mode<MODE_COUNT; mode++) {
        select_mode(mode); assert(!(probe_load(&model.control)&ROUTE_ARMED));
        app->on_input(app,1,KEY_STATE_PRESSED); app->on_input(app,1,KEY_STATE_RELEASED);
        assert(!(probe_load(&model.control)&ROUTE_ARMED));
        arm(); assert(!!(probe_load(&model.control)&ROUTE_ARMED) == (mode != METER_ZERO));
        if (mode != METER_ZERO) { arm(); assert(!(probe_load(&model.control)&ROUTE_ARMED)); arm(); }
        app->on_input(app,0,KEY_STATE_HOLD); app->on_input(app,0,KEY_STATE_RELEASED);
        assert(ROUTE_MODE(probe_load(&model.control))==mode);
        app->on_input(app,0,KEY_STATE_PRESSED); assert(!(probe_load(&model.control)&ROUTE_ARMED));
    }
    select_mode(TONE_L); arm(); render("WARNING: TONE ARMED");
    app->on_input(app,2,KEY_STATE_PRESSED); assert(ROUTE_MODE(probe_load(&model.control))==METER_ZERO);
    select_mode(ANTI_MONITOR); arm(); render("WARNING: ANTI ARMED");
    assert(app->on_pause(app)); assert(!(probe_load(&model.control)&ROUTE_ARMED));
    arm(); engine_set_active(false); assert(!(probe_load(&model.control)&ROUTE_ARMED));
    engine_set_active(true); assert(!(probe_load(&model.control)&ROUTE_ARMED));
    monitor=true; input_source=1; assert(app->tick(app)); render("Monitor Flag: ON   MIC");
    app->on_input(app,3,KEY_STATE_PRESSED); app->on_input(app,3,KEY_STATE_RELEASED); assert(!exits);
    arm(); app->on_input(app,3,KEY_STATE_HOLD); assert(exits==1 && !(probe_load(&model.control)&ROUTE_ARMED));
    finish();
    setup(false); assert(probe_load(&model.control)==ROUTE_DEFAULT); finish();
    puts("PASS route modes: explicit hold, disarm on changes/pause/exit, reinit, effective status");
}
static void test_buffers(void) {
    setup(false);
    const unsigned sizes[]={0,1,2,31,256};
    for (unsigned mode=0; mode<MODE_COUNT; mode++) for (unsigned enabled=0; enabled<2; enabled++) {
        select_mode(mode);
        if (!!(probe_load(&model.control)&ROUTE_ARMED) != !!enabled && mode!=METER_ZERO) arm();
        for (unsigned k=0;k<sizeof(sizes)/sizeof(sizes[0]);k++) {
            float in[258], out[258], saved[258];
            for(unsigned i=0;i<258;i++) in[i]=(i&1)?-0.25f:0.125f, out[i]=1234.f;
            memcpy(saved,in,sizeof(in)); struct mixer_s mix={in+1,out+1,sizes[k]};
            route_process(NULL,&mix);
            assert(out[0]==1234.f && out[sizes[k]+1]==1234.f);
            assert(memcmp(in,saved,sizeof(in))==0);
            for (unsigned i=1;i<=sizes[k];i++) {
                assert(out[i]!=1234.f);
                if (!enabled || mode==METER_ZERO) assert(out[i]==0.f);
                if (enabled && mode==COPY_INPUT) assert(out[i]==in[i]);
                if (enabled && mode==ANTI_MONITOR) assert(fabsf(out[i]+in[i]*0.1f)<0.000001f);
            }
        }
    }
    select_mode(COPY_INPUT); if (!(probe_load(&model.control)&ROUTE_ARMED)) arm();
    float x[]={-0.7f,0.8f,-0.3f,0.2f,0.1f}, saved[5]; memcpy(saved,x,sizeof(x));
    struct mixer_s mix={x,x,5}; route_process(NULL,&mix); assert(!memcmp(x,saved,sizeof(x)));
    assert((probe_load(&model.peaks)&65535u)==700 && (probe_load(&model.peaks)>>16)==800);
    mix.in=NULL; route_process(NULL,&mix); for(unsigned i=0;i<5;i++) assert(x[i]==0.f);
    mix.out=NULL; route_process(NULL,&mix); route_process(NULL,NULL);
    mix.out=x; context=NULL; route_process(NULL,&mix); context=&model;
    for(unsigned i=0;i<5;i++) assert(x[i]==0.f);
    finish(); puts("PASS route buffers: zero defaults, complete odd/even writes, unity copy, alias/null, L/R peaks");
}
static void test_tones(void) {
    setup(false);
    for (unsigned mode=TONE_L; mode<=TONE_R; mode++) {
        select_mode(mode); arm();
        float output[256]; struct mixer_s mix={NULL,output,256};
        double squares=0; unsigned crossings=0; float previous=0, peak=0;
        for(unsigned block=0;block<375;block++) { /* exactly one second at 48kHz */
            route_process(NULL,&mix);
            for(unsigned i=0;i<256;i+=2) {
                float s=output[i+(mode==TONE_R)];
                assert(output[i+(mode==TONE_L)]==0.f);
                assert(fabsf(s)<=0.01585f);
                if(fabsf(s)>peak) peak=fabsf(s);
                if(previous<=0 && s>0) crossings++;
                previous=s; squares+=(double)s*s;
            }
        }
        unsigned hz=mode==TONE_L?997:1499;
        assert(crossings>=hz-1 && crossings<=hz+1);
        assert(fabs(20*log10(peak)+36)<0.01);
        assert(fabs(sqrt(squares/48000)-TONE_AMPLITUDE/sqrt(2))<0.00001);
        assert(model.phase==0);
    }
    finish(); puts("PASS tones: 997/1499 Hz, -36dBFS peak, RMS, channel isolation, continuous blocks");
}
static void test_trim(void) {
    setup(false); select_mode(ANTI_MONITOR);
    assert(ROUTE_TRIM(probe_load(&model.control))==10);
    delta=2147483647; app->on_input(app,5,KEY_STATE_RELEASED); assert(ROUTE_TRIM(probe_load(&model.control))==25);
    arm(); delta=-25; app->on_input(app,5,KEY_STATE_RELEASED); assert(!delta && ROUTE_TRIM(probe_load(&model.control))==25);
    float in[]={1,-1}, out[2]; struct mixer_s mix={in,out,2}; route_process(NULL,&mix);
    assert(out[0]==-0.25f && out[1]==0.25f);
    arm(); delta=(-2147483647-1); app->on_input(app,5,KEY_STATE_RELEASED); assert(ROUTE_TRIM(probe_load(&model.control))==0);
    arm(); route_process(NULL,&mix); assert(out[0]==0 && out[1]==0);
    finish(); puts("PASS anti trim: default 10%, disabled, 0..25%, disarmed-only edit, no excess gain");
}
static void test_source(void) {
    setup(true); assert(app->on_pause==NULL);
    float input[]={.1f,-.7f,1.f,-1.f,.2f}, saved[5]; memcpy(saved,input,sizeof(input));
    float untouched[]={77,88}; struct mixer_s mix={input,untouched,5}; unsigned reads=out_reads;
    source_process(NULL,&mix); for(unsigned i=0;i<5;i++) assert(input[i]==0);
    for(unsigned state=0;state<3;state++) {
        app->on_input(app,0,state);
        assert(!!probe_load(&src_model.enabled)==(state==KEY_STATE_HOLD));
    }
    memcpy(input,saved,sizeof(input)); source_process(NULL,&mix); assert(!memcmp(input,saved,sizeof(input)));
    assert(untouched[0]==77 && untouched[1]==88 && out_reads==reads);
    for(unsigned btn=3;btn<=4;btn++) app->on_input(app,btn,KEY_STATE_HOLD);
    assert(probe_load(&src_model.enabled) && !exits); /* firmware owns navigation */
    app->on_input(app,1,KEY_STATE_PRESSED); source_process(NULL,&mix);
    for(unsigned i=0;i<5;i++) assert(input[i]==0);
    app->on_input(app,0,KEY_STATE_HOLD); engine_set_active(false); engine_set_active(true);
    assert(!probe_load(&src_model.enabled));
    mix.in=NULL; source_process(NULL,&mix); source_process(NULL,NULL);
    finish(); setup(true); assert(!probe_load(&src_model.enabled)); finish();
    puts("PASS source: silent start, held identity, full odd writes, no Output Bus access, system holds, lifecycle");
}
int main(void) {
    test_modes(); test_buffers(); test_tones(); test_trim(); test_source();
    assert(install_count==clears);
    puts("PASS routing probes: native behavior only; physical routing NOT TESTED");
}
