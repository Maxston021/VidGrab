#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// A kqueue/GCD-vnode based directory watcher that fires a block whenever the
/// contents of a directory change (file added, removed, renamed, or modified).
///
/// This is the same mechanism as the classic MHWDirectoryWatcher: open the
/// directory, create a DISPATCH_SOURCE_TYPE_VNODE source with DISPATCH_VNODE_WRITE,
/// and handle events on a dedicated serial queue. Vnode events are not recursive —
/// only direct children of the watched path trigger events — so callers should
/// also do a full rescan when the app returns to the foreground.
@interface VGDirectoryWatcher : NSObject

/// The path being watched.
@property (nonatomic, readonly, copy) NSString *path;

/// Creates a watcher for `path` and begins watching immediately. The block is
/// called on a private serial queue; dispatch to the main queue before touching UI.
+ (nullable instancetype)watchPath:(NSString *)path changeHandler:(void (^)(void))handler;

/// Stops watching. After this call the handler will never fire again.
- (void)invalidate;

@end

NS_ASSUME_NONNULL_END
