#import <OpenGL/OpenGL.h>
#import <OpenGL/GL.h>
#import "pez.h"
#import "stack.h"
#import "bstrlib.h"
#import <Cocoa/Cocoa.h>
#import <mach/mach_time.h>
#import <wchar.h>
#import <stdio.h>

extern int __pez__Argc;
extern const char** __pez__Argv;

static int _PezDesktopWidth;
static int _PezDesktopHeight;

@class View;

@interface View : NSOpenGLView <NSWindowDelegate> {
    NSRect m_frameRect;
    BOOL m_didInit;
    uint64_t m_previousTime;
    NSTimer* m_timer;
}

- (void) animate;

@end

@implementation View

-(void)windowWillClose:(NSNotification *)note {
    [[NSApplication sharedApplication] terminate:self];
}

- (void) timerFired:(NSTimer*) timer
{
    [self animate];     
}

- (id) initWithFrame: (NSRect) frame
{
    m_didInit = FALSE;
    
    int attribs[] = {
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFAColorSize, 32,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFANoRecovery,
        kCGLPFASampleBuffers, 1,
        kCGLPFASamples, 8,
        0
    };

    if (!PezGetConfig().Multisampling) {
        attribs[11] = 0;
    }

    NSOpenGLPixelFormat *fmt = [[NSOpenGLPixelFormat alloc]
                            initWithAttributes:(NSOpenGLPixelFormatAttribute*) attribs];

    self = [self initWithFrame:frame pixelFormat:fmt];

    [fmt release];

    m_frameRect = frame;
    m_previousTime = mach_absolute_time();

    m_timer = [[NSTimer
                       scheduledTimerWithTimeInterval:1.0f/120.0f
                       target:self 
                       selector:@selector(timerFired:)
                       userInfo:nil
                       repeats:YES] retain];

    return self;
}

- (void) drawRect:(NSRect) theRect
{
    if (!m_didInit) {
            
        int TransparentWindow = 0;
        if (TransparentWindow) {
            int opaque = NO;
            [[self openGLContext]
                setValues:&opaque
                forParameter:NSOpenGLCPSurfaceOpacity];
    
            [[self window] setOpaque:NO];
            [[self window] setAlphaValue:0.99];
        }

        // Reset OpenGL error state:
        glGetError();
    
        // Lop off the trailing .c
        bstring name = bfromcstr(PezGetConfig().Title);
        pezSwInit(0);

        // Set up the Shader Wrangler
        pezSwAddPath("./shaders/", ".glsl");
        pezSwAddDirective("*", "#version 330");

        // Perform user-specified intialization
        pezPrintString("OpenGL Version: %s\n", glGetString(GL_VERSION));
        PezInitialize();

        m_didInit = YES;
        
        [[self window] setLevel: NSFloatingWindowLevel];
        [[self window] makeKeyAndOrderFront: self];
        [[self window] setTitle: [NSString stringWithUTF8String: bdata(name)]];
    }

    PezRender();
    
    if (glGetError() != GL_NO_ERROR) {
        pezFatal("OpenGL Error\n");
    }
    
    [[self openGLContext] flushBuffer]; 
}

- (void) mouseDragged: (NSEvent*) theEvent
{
    NSPoint curPoint = [theEvent locationInWindow];
    PezHandleMouse(curPoint.x, m_frameRect.size.height - curPoint.y, PEZ_MOVE);
}

- (void) mouseUp: (NSEvent*) theEvent
{
    NSPoint curPoint = [theEvent locationInWindow];
    PezHandleMouse(curPoint.x, m_frameRect.size.height - curPoint.y, PEZ_UP);
}

- (void) mouseDown: (NSEvent*) theEvent
{
    NSPoint curPoint = [theEvent locationInWindow];
    PezHandleMouse(curPoint.x, m_frameRect.size.height - curPoint.y, PEZ_DOWN);
}

- (void) animate
{
    uint64_t currentTime = mach_absolute_time();
    uint64_t elapsedTime = currentTime - m_previousTime;
    m_previousTime = currentTime;
    
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    
    elapsedTime *= info.numer;
    elapsedTime /= info.denom;
    
    float timeStep = elapsedTime / 1000000.0f;

    PezUpdate(timeStep / 1000.0f);
    
    [self display];
}

- (void) onKey: (unichar) character downEvent: (BOOL) flag
{
    switch(character) {
        case 27:
        case 'q':
            [[NSApplication sharedApplication] terminate:self];
            break;
    }
}

- (void) keyDown:(NSEvent *)theEvent
{
    NSString *characters;
    unsigned int characterIndex, characterCount;
    
    characters = [theEvent charactersIgnoringModifiers];
    characterCount = [characters length];

    for (characterIndex = 0; characterIndex < characterCount; characterIndex++)
        [self onKey:[characters characterAtIndex:characterIndex] downEvent:YES];
}

@end

int main(int argc, const char *argv[])
{
    __pez__Argc = argc;
    __pez__Argv = argv;
    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    NSApplication *NSApp = [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    
    id menubar = [[NSMenu new] autorelease];
    id appMenuItem = [[NSMenuItem new] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];
    
    id appMenu = [[NSMenu new] autorelease];
    id appName = [[NSProcessInfo processInfo] processName];
    id quitTitle = [@"Quit " stringByAppendingString:appName];
    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
    action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];

    NSRect screenBounds = [[NSScreen mainScreen] frame];
    _PezDesktopWidth = screenBounds.size.width;
    _PezDesktopHeight = screenBounds.size.height;

    NSRect viewBounds = NSMakeRect(0, 0, PezGetConfig().Width, PezGetConfig().Height);
    
    View* view = [[View alloc] initWithFrame:viewBounds];
    
    NSRect centered = NSMakeRect(NSMidX(screenBounds) - NSMidX(viewBounds),
                                 NSMidY(screenBounds) - NSMidY(viewBounds),
                                 viewBounds.size.width, viewBounds.size.height);
    
    NSWindow *window = [[NSWindow alloc]
        initWithContentRect:centered
        styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask
        backing:NSBackingStoreBuffered
        defer:NO];

    [window setContentView:view];
    [window setDelegate:view];
    [window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];

    [view release];
    
    [NSApp run];


    [pool release];
    return EXIT_SUCCESS;
}

void pezPrintStringW(const wchar_t* pStr, ...)
{
    va_list a;
    va_start(a, pStr);

    wchar_t msg[1024] = {0};
    vswprintf(msg, countof(msg), pStr, a);
    fputws(msg, stderr);
}

void pezPrintString(const char* pStr, ...)
{
    va_list a;
    va_start(a, pStr);

    char msg[1024] = {0};
    vsnprintf(msg, countof(msg), pStr, a);
    fputs(msg, stderr);
}

void _pezFatalW(const wchar_t* pStr, va_list a)
{
    wchar_t msg[1024] = {0};
    vswprintf(msg, countof(msg), pStr, a);
    fputws(msg, stderr);
    exit(1);
}

void _pezFatal(const char* pStr, va_list a)
{
    char msg[1024] = {0};
    vsnprintf(msg, countof(msg), pStr, a);
    puts(msg);
    puts("\n");
    exit(1);
}

void pezFatal(const char* pStr, ...)
{
    va_list a;
    va_start(a, pStr);
    _pezFatal(pStr, a);
}

void pezFatalW(const wchar_t* pStr, ...)
{
    va_list a;
    va_start(a, pStr);
    _pezFatalW(pStr, a);
}

void pezCheck(int condition, ...)
{
    va_list a;
    const char* pStr;
    
    if (condition)
        return;
    
    pezPrintStackTrace();

    va_start(a, condition);
    pStr = va_arg(a, const char*);
    _pezFatal(pStr, a);
}

void pezCheckGL(const char *call)
{
    char enums[][20] =
    {
        "invalid enumeration", // GL_INVALID_ENUM
        "invalid value",       // GL_INVALID_VALUE
        "invalid operation",   // GL_INVALID_OPERATION
        "stack overflow",      // GL_STACK_OVERFLOW
        "stack underflow",     // GL_STACK_UNDERFLOW
        "out of memory"        // GL_OUT_OF_MEMORY
    };

    GLenum errcode = glGetError();
    if (errcode == GL_NO_ERROR)
        return;

    errcode -= GL_INVALID_ENUM;
    pezCheck(0, "OpenGL %s in '%s'", enums[errcode], call);
}

void pezCheckFBO()
{
    char enums[][20] =
    {
        "attachment",         // GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT........... All framebuffer attachment points are 'framebuffer attachment complete'.
        "missing attachment", // GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT....There is at least one image attached to the framebuffer.
        "",                   //
        "dimensions",         // GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT............All attached images have the same width and height.
        "formats",            // GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT...............All images attached to the attachment points COLOR_ATTACHMENT0_EXT through COLOR_ATTACHMENTn_EXT must have the same internal format.
        "draw buffer",        // GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT...........The value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT must not be NONE for any color attachment point(s) named by DRAW_BUFFERi.
        "read buffer",        // GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT...........If READ_BUFFER is not NONE, then the value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT must not be NONE for the color attachment point named by READ_BUFFER.
        "unsupported format"  // GL_FRAMEBUFFER_UNSUPPORTED_EXT......................The combination of internal formats of the attached images does not violate an implementation-dependent set of restrictions.
    };

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status == GL_FRAMEBUFFER_COMPLETE_EXT)
        return;

    status -= GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT;
    pezCheck(0, "incomplete framebuffer object due to %s", enums[status]);
}

void pezCheckW(int condition, ...)
{
    va_list a;
    const wchar_t* pStr;
    
    if (condition)
        return;
    
    va_start(a, condition);
    pStr = va_arg(a, const wchar_t*);
    _pezFatalW(pStr, a);
}

int pezGetDesktopWidth()
{
    return _PezDesktopWidth;
}

int pezGetDesktopHeight()
{
    return _PezDesktopHeight;
}
