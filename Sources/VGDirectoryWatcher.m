#import "VGDirectoryWatcher.h"
#import <sys/attr.h>
#import <sys/event.h>
#import <sys/types.h>

@interface VGDirectoryWatcher ()
@property (nonatomic, copy) NSString *path;
@property (nonatomic, copy) void (^handler)(void);
@property (nonatomic) int dirFD;
@property (nonatomic, strong) dispatch_source_t source;
@property (nonatomic, strong) dispatch_queue_t watcherQueue;
@end

@implementation VGDirectoryWatcher

+ (instancetype)watchPath:(NSString *)path changeHandler:(void (^)(void))handler {
    if (!path.length || !handler) return nil;
    VGDirectoryWatcher *w = [VGDirectoryWatcher new];
    w.path = path;
    w.handler = handler;
    w.dirFD = -1;
    if (![w start]) return nil;
    return w;
}

- (BOOL)start {
    self.dirFD = open(self.path.fileSystemRepresentation, O_EVTONLY);
    if (self.dirFD < 0) return NO;

    self.watcherQueue = dispatch_queue_create("com.vidgrab.directorywatcher", DISPATCH_QUEUE_SERIAL);
    self.source = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE,
                                          (uintptr_t)self.dirFD,
                                          DISPATCH_VNODE_WRITE,
                                          self.watcherQueue);
    __weak typeof(self) weakSelf = self;
    dispatch_source_set_event_handler(self.source, ^{
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) return;
        // Coalesce bursts: kqueue can fire many times for one logical change
        // (e.g. a large file being written in chunks). Debounce 200 ms.
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.2 * NSEC_PER_SEC)),
                       self.watcherQueue, ^{
            if (self.handler) self.handler();
        });
    });
    dispatch_source_set_cancel_handler(self.source, ^{
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) return;
        if (self.dirFD >= 0) {
            close(self.dirFD);
            self.dirFD = -1;
        }
    });
    dispatch_resume(self.source);
    return YES;
}

- (void)invalidate {
    if (self.source) {
        dispatch_source_cancel(self.source);
        self.source = nil;
    }
    self.handler = nil;
}

- (void)dealloc {
    [self invalidate];
}

@end
