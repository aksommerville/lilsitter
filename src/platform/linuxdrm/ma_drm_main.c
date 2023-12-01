#include "ma_drm_internal.h"
#include <signal.h>
#include <sys/poll.h>
#include <unistd.h>

/* Globals.
 */
 
struct ma_init_params ma_drm_init_params={0};
uint16_t ma_drm_input=0;
int ma_drm_use_signals=1;
int ma_drm_quit_requested=0;
int64_t ma_drm_next_frame_time=0;
int64_t ma_drm_start_time=0;
int ma_drm_frame_skipc=0;
int ma_drm_framec=0;
const char *ma_drm_file_sandbox=0;
struct drmgx *drmgx=0;
struct ma_alsa *ma_alsa=0;
int ma_drm_audio_locked=0;
struct ma_evdev *ma_evdev=0;
int ma_argc=0;
char **ma_argv=0;

static volatile int ma_drm_sigc=0;

/* Usage.
 */
 
static void ma_drm_print_usage(const char *exename) {
  if (!exename||!exename[0]) exename="multiarcade";
  fprintf(stderr,"\nUsage: %s [OPTIONS]\n",exename);
  fprintf(stderr,
    "OPTIONS:\n"
    "  --no-signals      Don't react to SIGINT.\n"
    "  --files=PATH      [] Let the app access files under the given directory.\n"
    "  --video-device=PATH\n"
    "  --video-rate=HZ\n"
    "  --video-filter=INT\n"
    "  --glsl-version=INT\n"
    "  --audio-device=NAME\n"
    "  --audio-rate=44100\n"
    "  --audio-chanc=1\n"
    "\n"
  );
}

int genioc_argv_get_boolean(int argc,char **argv,const char *k) {
  int p=1; for (;p<argc;p++) {
    if (!strcmp(argv[p],k)) return 1;
  }
  return 0;
}

int genioc_argv_get_int(int argc,char **argv,const char *k,int fallback) {
  int kc=0; while (k[kc]) kc++;
  int p=1; for (;p<argc;p++) {
    if (memcmp(argv[p],k,kc)) continue;
    if (argv[p][kc]!='=') continue;
    int v=0;
    const char *src=argv[p]+kc+1;
    for (;*src;src++) {
      if ((*src<'0')||(*src>'9')) return fallback;
      v*=10;
      v+=(*src)-'0';
    }
    return v;
  }
  return fallback;
}

const char *genioc_argv_get_string(int argc,char **argv,const char *k,const char *fallback) {
  int kc=0; while (k[kc]) kc++;
  int p=1; for (;p<argc;p++) {
    if (memcmp(argv[p],k,kc)) continue;
    if (argv[p][kc]!='=') continue;
    return argv[p]+kc+1;
  }
  return fallback;
}

/* Signal handler.
 */
 
static void ma_drm_rcvsig(int sigid) {
  switch (sigid) {
    case SIGINT: if (++ma_drm_sigc>=3) {
        fprintf(stderr," Too many unprocessed signals.\n");
        exit(1);
      } break;
  }
}

/* Cleanup.
 */
 
static void ma_drm_quit() {

  if (ma_drm_framec>0) {
    int64_t endtime=ma_drm_now();
    double elapsed=(endtime-ma_drm_start_time)/1000000.0;
    fprintf(stderr,
      "%d frames in %.03f s, average rate %.03f Hz\n",
      ma_drm_framec,elapsed,ma_drm_framec/elapsed
    );
  }
  
  ma_alsa_del(ma_alsa);
  ma_alsa=0;
  
  drmgx_del(drmgx);
  drmgx=0;
  
  ma_evdev_del(ma_evdev);
  ma_evdev=0;
}

/* Subsystem callbacks.
 */
 
static int ma_drm_cb_button(struct ma_evdev *evdev,uint16_t btnid,int value) {
  if (value) ma_drm_input|=btnid;
  else ma_drm_input&=~btnid;
  return 0;
}

/* Initialize.
 */
 
static int ma_drm_init(int argc,char **argv) {

  ma_argc=argc;
  ma_argv=argv;
  if (genioc_argv_get_boolean(argc,argv,"--help")) {
    ma_drm_print_usage(argv[0]);
    return -1;
  }
  if (genioc_argv_get_boolean(argc,argv,"--no-signals")) ma_drm_use_signals=0;
  ma_drm_file_sandbox=genioc_argv_get_string(argc,argv,"--files","/home/andy/proj/lilsitter/out/data");//TODO better to derive from argv[0] if possible?
  
  if (!ma_drm_file_sandbox) {
    ma_drm_file_sandbox="/home/andy/proj/lilsitter/out/data";//XXX get smarter
    fprintf(stderr,"%s: Using default data directory '%s'\n",argv[0],ma_drm_file_sandbox);
  }
  
  if (ma_drm_use_signals) {
    signal(SIGINT,ma_drm_rcvsig);
  }
  
  if (!(drmgx=drmgx_new(
    genioc_argv_get_string(argc,argv,"--video-device",0),
    genioc_argv_get_int(argc,argv,"--video-rate",60),
    96,64,DRMGX_FMT_TINY8,
    genioc_argv_get_int(argc,argv,"--video-filter",0),
    genioc_argv_get_int(argc,argv,"--glsl-version",0)
  ))) return -1;
  
  if (!(ma_evdev=ma_evdev_new((void*)ma_drm_cb_button,0))) {
    fprintf(stderr,"Failed to initialize evdev (joysticks). Proceeding without...\n");
  }
  
  // We do not initialize audio yet -- wait for ma_init()
  
  setup();
  ma_drm_start_time=ma_drm_now();
  return 0;
}

/* Update.
 * Returns >0 to proceed.
 */
 
static int ma_drm_update() {
  ma_drm_framec++;
  loop();
  if (ma_drm_audio_locked) {
    if (ma_alsa_unlock(ma_alsa)>=0) ma_drm_audio_locked=0;
  }
  
  struct pollfd pollfd={.fd=STDIN_FILENO,.events=POLLIN};
  if (poll(&pollfd,1,0)>0) {
    char dummy[256];
    if (read(STDIN_FILENO,dummy,sizeof(dummy))>0) {
      // something arrived on stdin (meaning, user hit ENTER).
      ma_drm_quit_requested=1;
    }
  }
  
  if (ma_drm_quit_requested) return 0;
  return 1;
}

/* Main.
 */
 
int main(int argc,char **argv) {
  if (ma_drm_init(argc,argv)<0) {
    ma_drm_quit();
    return 1;
  }
  while (!ma_drm_sigc) {
    int err=ma_drm_update();
    if (err<0) {
      ma_drm_quit();
      return 1;
    }
    if (!err) break;
  }
  ma_drm_quit();
  return 0;
}
