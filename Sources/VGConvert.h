#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface VGConvert : NSObject

+ (NSDictionary<NSString *, NSString *> *)probe:(NSString *)path;
+ (NSString *)describe:(NSString *)path;
+ (NSDictionary<NSString *, id> *)tsScan:(NSString *)path;
+ (nullable NSDictionary<NSString *, id> *)tsClean:(NSString *)path to:(NSString *)output;

- (void)cancel;
- (void)run:(int (^)(char *err, size_t n))work completion:(void (^)(NSString *_Nullable error))completion;
- (void)remux:(NSString *)input to:(NSString *)output
     progress:(nullable void (^)(double fraction))progress
   completion:(void (^)(NSString *_Nullable error))completion;
- (void)mux:(NSString *)video audio:(nullable NSString *)audio to:(NSString *)output
   progress:(nullable void (^)(double fraction))progress
 completion:(void (^)(NSString *_Nullable error, NSDictionary<NSString *, id> *info))completion;
- (void)merge:(NSString *)video audio:(NSString *)audio to:(NSString *)output
     progress:(nullable void (^)(double fraction))progress
   completion:(void (^)(NSString *_Nullable error))completion;
- (void)audio:(NSString *)input to:(NSString *)output mp3:(BOOL)mp3
         title:(nullable NSString *)title artist:(nullable NSString *)artist
      progress:(nullable void (^)(double fraction))progress
    completion:(void (^)(NSString *_Nullable error))completion;
- (void)wavFrom:(NSString *)input to:(NSString *)output
       progress:(nullable void (^)(double fraction))progress
     completion:(void (^)(NSString *_Nullable error))completion;

@end

NS_ASSUME_NONNULL_END