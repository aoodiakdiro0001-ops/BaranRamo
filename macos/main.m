/*
 * BaranRamo — native macOS wrapper (WKWebView, offline single-file game)
 * Universal binary: arm64 + x86_64.
 * © 2026 Baran Avisat — All Rights Reserved.
 */
#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (strong) NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)note {
    NSRect r = NSMakeRect(0, 0, 1280, 720);
    self.window = [[NSWindow alloc]
        initWithContentRect:r
                  styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                             NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable)
                    backing:NSBackingStoreBuffered
                      defer:NO];
    self.window.title = @"BaranRamo";
    self.window.minSize = NSMakeSize(640, 400);
    [self.window center];
    [self.window setFrameAutosaveName:@"BaranRamoWindow"];

    WKWebView *wv = [[WKWebView alloc] initWithFrame:r];
    NSURL *url = [[NSBundle mainBundle] URLForResource:@"BaranRamo" withExtension:@"html"];
    if (url) {
        [wv loadRequest:[NSURLRequest requestWithURL:url]];
    } else {
        [wv loadHTMLString:@"<h1 style='font-family:sans-serif;color:#ffd76e;background:#0d1b33;padding:2em'>BaranRamo — game file missing</h1>"
                   baseURL:nil];
    }
    self.window.contentView = wv;
    [self.window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end

int main(int argc, const char *argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        AppDelegate *d = [AppDelegate new];
        app.delegate = d;
        [app run];
    }
    return 0;
}
