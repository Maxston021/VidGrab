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

NS_ASSUME_NONNULL_END
