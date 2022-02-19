#ifndef MA_DRM_INTERNAL_H
#define MA_DRM_INTERNAL_H

#include "multiarcade.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

struct ma_drm;
struct ma_alsa;
struct ma_evdev;

extern struct ma_init_params ma_drm_init_params;
extern uint16_t ma_drm_input;
extern int ma_drm_use_signals;
extern int ma_drm_quit_requested;
extern int64_t ma_drm_next_frame_time;
extern int64_t ma_drm_start_time;
extern int ma_drm_frame_skipc;
extern int ma_drm_framec;
extern const char *ma_drm_file_sandbox;
extern struct ma_drm *ma_drm;
extern struct ma_alsa *ma_alsa;
extern int ma_drm_audio_locked;
extern struct ma_evdev *ma_evdev;

int64_t ma_drm_now();

/* DRM.
 ***********************************************************/
 
struct ma_drm *ma_drm_new(int fbw,int fbh);
void ma_drm_del(struct ma_drm *drm);
int ma_drm_swap(struct ma_drm *drm,const void *fb);

/* ALSA.
 *************************************************************/

struct ma_alsa *ma_alsa_new(
  int rate,int chanc,
  void (*cb)(int16_t *dst,int dstc,struct ma_alsa *alsa),
  void *userdata
);

void ma_alsa_del(struct ma_alsa *alsa);

int ma_alsa_get_rate(const struct ma_alsa *alsa);
int ma_alsa_get_chanc(const struct ma_alsa *alsa);
void *ma_alsa_get_userdata(const struct ma_alsa *alsa);
int ma_alsa_get_status(const struct ma_alsa *alsa); // 0,-1

int ma_alsa_lock(struct ma_alsa *alsa);
int ma_alsa_unlock(struct ma_alsa *alsa);

/* Evdev.
 **********************************************************/
 
struct ma_evdev *ma_evdev_new(
  int (*cb_button)(struct ma_evdev *evdev,uint16_t btnid,int value),
  void *userdata
);

void ma_evdev_del(struct ma_evdev *evdev);

void *ma_evdev_get_userdata(const struct ma_evdev *evdev);

int ma_evdev_update(struct ma_evdev *evdev);

#endif
