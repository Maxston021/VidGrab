// VidGrab converter C API.
#ifndef VGCONVERT_C_H
#define VGCONVERT_C_H

#include <stddef.h>
#include <stdint.h>





#ifdef __cplusplus
extern "C" {
#endif

#define VG_OK        0
#define VG_ERROR     1
#define VG_CANCELLED 2

typedef struct {
    char video[64];
    char audio[64];
    char container[64];
    int width;
    int height;
    double duration;
} VGProbe;

typedef struct {
    char strategy[64];
    double expectedDuration;
    int hadAudio;
    int discontinuities;
    int leadingDropped;
    int64_t videoIn;
    int64_t videoOut;
    int64_t videoLost;
    char audioIn[64];
    double gapSeconds;
    int width;
    int height;
} VGMuxInfo;

typedef void (*vg_progress_cb)(void *ctx, double progress);
typedef int  (*vg_cancel_cb)(void *ctx);

int vg_probe(const char *path, VGProbe *out);
int vg_describe(const char *path, char *buf, size_t n);
int vg_ts_scan(const char *path, char *buf, size_t n, long *scrambled, int *isTS);
int vg_ts_clean(const char *in, const char *out, long long *junkBytes, long *pieces, int *needed,
                vg_progress_cb progress, vg_cancel_cb cancel, void *ctx, char *err, size_t errlen);
int vg_audio(const char *in, const char *out, int mp3, int bitrate, const char *title, const char *artist,
             vg_progress_cb progress, vg_cancel_cb cancel, void *ctx, char *err, size_t errlen);
int vg_audio_wav(const char *in, const char *out, vg_progress_cb progress, vg_cancel_cb cancel, void *ctx,
                 char *err, size_t errlen);
int vg_mux_mp4_ex(const char *videoIn, const char *audioIn, const char *out, VGMuxInfo *info,
                  vg_progress_cb progress, vg_cancel_cb cancel, void *ctx, char *err, size_t errlen);
int vg_mux_mp4(const char *videoIn, const char *audioIn, const char *out,
               vg_progress_cb progress, vg_cancel_cb cancel, void *ctx, char *err, size_t errlen);
int vg_remux_mp4(const char *in, const char *out,
                 vg_progress_cb progress, vg_cancel_cb cancel, void *ctx, char *err, size_t errlen);
int vg_validate_mp4(const char *path, double expectDur, int wantAudio,
                    char *summary, size_t sumlen, char *err, size_t errlen);

#ifdef __cplusplus
}
#endif



#endif /* VGCONVERT_C_H */#if
def __OBJC__

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface VGConvert : NSObject

/// Probe a media file and return codec/container/dimension info.
+ (NSDictionary<NSString *, NSString *> *)probe:(NSString *)path;

/// Human-readable description of all streams.
+ (NSString *)describe:(NSString *)path;

/// Scan an MPEG-TS file for scrambling and structure.
+ (NSDictionary<NSString *, id> *)tsScan:(NSString *)path;

/// Clean junk from an MPEG-TS file. Returns nil if no cleaning needed.
+ (nullable NSDictionary<NSString *, id> *)tsClean:(NSString *)path to:(NSString *)output;

/// Cancel the current operation.
- (void)cancel;

/// Run a block on a background queue and call completion on the main thread.
- (void)run:(int (^)(char *err, size_t n))work completion:(void (^)(NSString * _Nullable))completion;

/// Remux a file to MP4 (video copied, audio converted to AAC if needed).
- (void)remux:(NSString *)input to:(NSString *)output
      progress:(nullable void (^)(double))progress
    completion:(void (^)(NSString * _Nullable))completion;

/// Mux separate video and audio into MP4 with validation.
- (void)mux:(NSString *)video audio:(nullable NSString *)audio to:(NSString *)output
    progress:(nullable void (^)(double))progress
  completion:(void (^)(NSString * _Nullable, NSDictionary<NSString *, id> *))completion;

/// Merge video and audio (simpler, no validation).
- (void)merge:(NSString *)video audio:(NSString *)audio to:(NSString *)output
     progress:(nullable void (^)(double))progress
   completion:(void (^)(NSString * _Nullable))completion;

/// Convert audio to M4A (mp3=NO) or MP3 (mp3=YES).
- (void)audio:(NSString *)input to:(NSString *)output mp3:(BOOL)mp3
        title:(nullable NSString *)title artist:(nullable NSString *)artist
     progress:(nullable void (^)(double))progress
   completion:(void (^)(NSString * _Nullable))completion;

/// Extract audio as WAV.
- (void)wavFrom:(NSString *)input to:(NSString *)output
       progress:(nullable void (^)(double))progress
     completion:(void (^)(NSString * _Nullable))completion;

@end

NS_ASSUME_NONNULL_END#endif /* __OBJC__ */
