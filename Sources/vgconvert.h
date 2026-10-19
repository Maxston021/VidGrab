// VidGrab converter core (plain C on top of FFmpeg).
// Repackages H.264/HEVC videos into MP4 and converts audio to AAC (M4A) or MP3.
#ifndef VGCONVERT_H
#define VGCONVERT_H

#include <stddef.h>

typedef int (*vg_cancel_cb)(void *ctx);
typedef void (*vg_progress_cb)(void *ctx, double fraction);

typedef struct {
    char video[32];
    char audio[32];
    char container[64];
    double duration;
    int width;
    int height;
} VGProbe;

typedef struct {
    char strategy[128];
    char audioIn[32];
    double expectedDuration;
    double gapSeconds;
    int discontinuities;
    int leadingDropped;
    int hadAudio;
    int width;
    int height;
    long videoIn;
    long videoOut;
    long videoLost;
} VGMuxInfo;

enum { VG_OK = 0, VG_ERROR = -1, VG_CANCELLED = -2 };

int vg_probe(const char *path, VGProbe *out);
int vg_describe(const char *path, char *buf, size_t n);
int vg_ts_scan(const char *path, char *buf, size_t n, long *scrambled, int *isTS);
int vg_ts_clean(const char *in, const char *out, long long *junkBytes, long *pieces,
                int *needed, vg_progress_cb progress, vg_cancel_cb cancel, void *ctx,
                char *err, size_t errlen);
int vg_remux_mp4(const char *in, const char *out,
                 vg_progress_cb progress, vg_cancel_cb cancel, void *ctx,
                 char *err, size_t errlen);
int vg_mux_mp4(const char *videoIn, const char *audioIn, const char *out,
               vg_progress_cb progress, vg_cancel_cb cancel, void *ctx,
               char *err, size_t errlen);
int vg_mux_mp4_ex(const char *videoIn, const char *audioIn, const char *out,
                  VGMuxInfo *info, vg_progress_cb progress, vg_cancel_cb cancel,
                  void *ctx, char *err, size_t errlen);
int vg_validate_mp4(const char *path, double expectDur, int wantAudio,
                    char *summary, size_t sumlen, char *err, size_t errlen);
int vg_audio(const char *in, const char *out, int mp3, int bitrate,
             const char *title, const char *artist,
             vg_progress_cb progress, vg_cancel_cb cancel, void *ctx,
             char *err, size_t errlen);
int vg_audio_wav(const char *in, const char *out,
                 vg_progress_cb progress, vg_cancel_cb cancel, void *ctx,
                 char *err, size_t errlen);

#endif