/*
 * Copyright (c) 2008 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * - Redistribution of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 
 * - Redistribution in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of Sun Microsystems, Inc. or the names of
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN
 * MICROSYSTEMS, INC. ("SUN") AND ITS LICENSORS SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR
 * ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR
 * DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE
 * DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY,
 * ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF
 * SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "com_sun_javafx_newt_x11_X11Window.h"

#include "EventListener.h"
#include "MouseEvent.h"
#include "KeyEvent.h"

// #define VERBOSE_ON 1

#ifdef VERBOSE_ON

static void _dumpVisualInfo(const char * msg, XVisualInfo *pVisualQuery) {
    if(pVisualQuery!=NULL) {
        fprintf(stderr, "%s: screen %d, visual: %p, visual-id: 0x%X, depth: %d, class %d, cmap sz: %d, bpp: 3x%d, rgb 0x%X 0x%X 0x%X\n",
            msg,
            pVisualQuery->screen,
            pVisualQuery->visual,
            (int)pVisualQuery->visualid,
            pVisualQuery->depth,
            pVisualQuery->class,
            pVisualQuery->colormap_size,
            pVisualQuery->bits_per_rgb,
            (int)pVisualQuery->red_mask,
            (int)pVisualQuery->green_mask,
            (int)pVisualQuery->blue_mask
        );
    } else {
        fprintf(stderr, "%s: NULL XVisualInfo\n", msg);
    }
}

#define DUMP_VISUAL_INFO(a,b) _dumpVisualInfo((a),(b))

#else

#define DUMP_VISUAL_INFO(a,b)

#endif

/**
 * Display
 */

/*
 * Class:     com_sun_javafx_newt_x11_X11Display
 * Method:    CreateDisplay
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_javafx_newt_x11_X11Display_CreateDisplay
  (JNIEnv *env, jobject obj, jstring displayName)
{
    Display * dpy = NULL;
    const char * _displayName = NULL;
    if(displayName!=0) {
        _displayName = (*env)->GetStringUTFChars(env, displayName, 0);
    }
    dpy = XOpenDisplay(_displayName);
    if(dpy==NULL) {
        fprintf(stderr, "couldn't open display connection..\n");
    }
    return (jlong) (intptr_t) dpy;
}

/**
 * Screen
 */

/*
 * Class:     com_sun_javafx_newt_x11_X11Screen
 * Method:    GetScreen
 * Signature: (JI)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_javafx_newt_x11_X11Screen_GetScreen
  (JNIEnv *env, jobject obj, jlong display, jint screen_index)
{
    Display * dpy = (Display *)(intptr_t)display;
    Screen  * scrn= NULL;
    if(dpy==NULL) {
        fprintf(stderr, "[GetScreen] invalid display connection..\n");
        return 0;
    }
    scrn = ScreenOfDisplay(dpy,screen_index);
    if(scrn==NULL) {
        scrn=DefaultScreenOfDisplay(dpy);
    }
    if(scrn==NULL) {
        fprintf(stderr, "couldn't get screen ..\n");
    }
    return (jlong) (intptr_t) scrn;
}

/**
 * Window
 */

static jmethodID sizeChangedID = NULL;
static jmethodID positionChangedID = NULL;
static jmethodID windowClosedID = NULL;
static jmethodID windowDestroyedID = NULL;
static jmethodID windowCreatedID = NULL;
static jmethodID sendMouseEventID = NULL;
static jmethodID sendKeyEventID = NULL;

/*
 * Class:     com_sun_javafx_newt_x11_X11Window
 * Method:    initIDs
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_javafx_newt_x11_X11Window_initIDs
  (JNIEnv *env, jclass clazz)
{
    sizeChangedID = (*env)->GetMethodID(env, clazz, "sizeChanged", "(II)V");
    positionChangedID = (*env)->GetMethodID(env, clazz, "positionChanged", "(II)V");
    windowClosedID    = (*env)->GetMethodID(env, clazz, "windowClosed",    "()V");
    windowDestroyedID = (*env)->GetMethodID(env, clazz, "windowDestroyed", "()V");
    windowCreatedID = (*env)->GetMethodID(env, clazz, "windowCreated", "(IJ)V");
    sendMouseEventID = (*env)->GetMethodID(env, clazz, "sendMouseEvent", "(IIIII)V");
    sendKeyEventID = (*env)->GetMethodID(env, clazz, "sendKeyEvent", "(IIIC)V");
    if (sizeChangedID == NULL ||
        positionChangedID == NULL ||
        windowClosedID == NULL ||
        windowDestroyedID == NULL ||
        windowCreatedID == NULL ||
        sendMouseEventID == NULL ||
        sendKeyEventID == NULL) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

/*
 * Class:     com_sun_javafx_newt_x11_X11Window
 * Method:    CreateWindow
 * Signature: (JJIJIIII)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_javafx_newt_x11_X11Window_CreateWindow
  (JNIEnv *env, jobject obj, jlong display, jlong screen, jint screen_index, 
                             jlong visualID,
                             jint x, jint y, jint width, jint height)
{
    Display * dpy  = NULL;
    Screen  * scrn = NULL;
    int       scrn_idx = (int)screen_index;
    Window  windowParent = 0;
    Window  window = 0;

    XVisualInfo visualTemplate;
    XVisualInfo *pVisualQuery = NULL;
    Visual *visual = NULL;
    int depth;

    XSetWindowAttributes xswa;
    unsigned long attrMask;

    int n;

    dpy  = (Display *)(intptr_t)display;
    if(dpy==NULL) {
        fprintf(stderr, "[CreateWindow] invalid display connection..\n");
        return 0;
    }

    scrn = (Screen *)(intptr_t)screen;
    if(scrn==NULL) {
        fprintf(stderr, "invalid screen connection..\n");
        return 0;
    }

    if(visualID>0) {
        // try given VisualID on screen
        memset(&visualTemplate, 0, sizeof(XVisualInfo));
        visualTemplate.screen = scrn_idx;
        visualTemplate.visualid = (VisualID)visualID;
        pVisualQuery = XGetVisualInfo(dpy, VisualIDMask|VisualScreenMask, &visualTemplate,&n);
        DUMP_VISUAL_INFO("Given VisualID,ScreenIdx", pVisualQuery);
        if(pVisualQuery!=NULL) {
            visual   = pVisualQuery->visual;
            depth    = pVisualQuery->depth;
            visualID = pVisualQuery->visualid;
            XFree(pVisualQuery);
            pVisualQuery=NULL;
        }
#ifdef VERBOSE_ON
        fprintf(stderr, "trying given (screen %d, visualID: %d) found: %p\n", scrn_idx, (int)visualID, visual);
#endif
    }
    if (visual==NULL)
    { 
        // try default ..
        visual = XDefaultVisualOfScreen(scrn);
        if(visual!=NULL) {
            visualID = visual->visualid;
            // try given VisualID on screen
            memset(&visualTemplate, 0, sizeof(XVisualInfo));
            visualTemplate.screen = scrn_idx;
            visualTemplate.visualid = (VisualID)visualID;
            pVisualQuery = XGetVisualInfo(dpy, VisualIDMask|VisualScreenMask, &visualTemplate,&n);
            DUMP_VISUAL_INFO("Given ScreenIdx, Default VisualID", pVisualQuery);
            if(pVisualQuery!=NULL) {
                visual   = pVisualQuery->visual;
                depth    = pVisualQuery->depth;
                visualID = pVisualQuery->visualid;
                XFree(pVisualQuery);
                pVisualQuery=NULL;
            } else {
                visual = NULL;
                visualID = 0;
            }
#ifdef VERBOSE_ON
            fprintf(stderr, "default visual (screen %d, visualID: %d) found: %p\n", scrn_idx, (int)visualID, visual);
#endif
        }
    }
    if (visual==NULL)
    { 
        fprintf(stderr, "could not query any Visual, bail out!\n");
        return 0;
    } 

    if(pVisualQuery!=NULL) {
        XFree(pVisualQuery);
        pVisualQuery=NULL;
    }

    windowParent = XRootWindowOfScreen(scrn);

    attrMask = (CWBackPixel | CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect);
    memset(&xswa, 0, sizeof(xswa));
    xswa.override_redirect = True;
    xswa.border_pixel = 0;
    xswa.background_pixel = 0;
    xswa.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask;
    xswa.colormap = XCreateColormap(dpy,
                                    XRootWindow(dpy, scrn_idx),
                                    visual,
                                    AllocNone);

    window = XCreateWindow(dpy,
                           windowParent,
                           x, y,
                           width, height,
                           0, // border width
                           depth,
                           InputOutput,
                           visual,
                           attrMask,
                           &xswa);
    XClearWindow(dpy, window);

    (*env)->CallVoidMethod(env, obj, windowCreatedID, visualID, (jlong) window);

    return (jlong) window;
}

/*
 * Class:     com_sun_javafx_newt_x11_X11Window
 * Method:    setVisible0
 * Signature: (JJZ)V
 */
JNIEXPORT void JNICALL Java_com_sun_javafx_newt_x11_X11Window_CloseWindow
  (JNIEnv *env, jobject obj, jlong display, jlong window)
{
    Display * dpy = (Display *) (intptr_t) display;
    Window w = (Window)window;

    XUngrabPointer(dpy, CurrentTime);
    XUngrabKeyboard(dpy, CurrentTime);
    XSelectInput(dpy, w, 0);
    XUnmapWindow(dpy, w);
    XDestroyWindow(dpy, w);
}

/*
 * Class:     com_sun_javafx_newt_x11_X11Window
 * Method:    setVisible0
 * Signature: (JJZ)V
 */
JNIEXPORT void JNICALL Java_com_sun_javafx_newt_x11_X11Window_setVisible0
  (JNIEnv *env, jobject obj, jlong display, jlong window, jboolean visible)
{
    Display * dpy = (Display *) (intptr_t) display;
    Window w = (Window)window;
    if(visible==JNI_TRUE) {
        XMapRaised(dpy, w);

        XSetInputFocus(dpy, w, RevertToNone, CurrentTime);

        XSync(dpy, False);
    } else {
        XUngrabPointer(dpy, CurrentTime);
        XUngrabKeyboard(dpy, CurrentTime);
        XSelectInput(dpy, w, 0);
        XUnmapWindow(dpy, w);
    }
}

/*
 * Class:     com_sun_javafx_newt_x11_X11Window
 * Method:    DispatchMessages
 * Signature: (JJI)V
 */
JNIEXPORT void JNICALL Java_com_sun_javafx_newt_x11_X11Window_DispatchMessages
  (JNIEnv *env, jobject obj, jlong display, jlong window, jint eventMask)
{
    Display * dpy = (Display *) (intptr_t) display;
    Window w = (Window)window;

    if(eventMask<0) {
        long xevent_mask_key = 0;
        long xevent_mask_ptr = 0;
        long xevent_mask_win = 0;
        eventMask *= -1;
        if( 0 != ( eventMask & EVENT_MOUSE ) ) {
            xevent_mask_ptr |= ButtonPressMask|ButtonReleaseMask|PointerMotionMask;
        }
        if( 0 != ( eventMask & EVENT_KEY ) ) {
            xevent_mask_key |= KeyPressMask|KeyReleaseMask;
        }
        if( 0 != ( eventMask & EVENT_WINDOW ) ) {
            xevent_mask_win |= ExposureMask;
        }

        XSelectInput(dpy, w, xevent_mask_win|xevent_mask_key|xevent_mask_ptr);

        if(0!=xevent_mask_ptr) {
            XGrabPointer(dpy, w, True, xevent_mask_ptr,
                        GrabModeAsync, GrabModeAsync, w, None, CurrentTime);
        } 
        if(0!=xevent_mask_key) {
            XGrabKeyboard(dpy, w, True, GrabModeAsync, GrabModeAsync, CurrentTime);
        }

    }

    // Periodically take a break
    while( XPending(dpy)>0 ) {
        XEvent evt;
        KeySym keySym;
        char keyChar;
        char text[255];

        XNextEvent(dpy, &evt);

        switch(evt.type) {
            case ButtonPress:
            case ButtonRelease:
            case MotionNotify:
                if( ! ( eventMask & EVENT_MOUSE ) ) {
                    continue;
                }
                break;
            case KeyPress:
            case KeyRelease:
                if( ! ( eventMask & EVENT_KEY ) ) {
                    continue;
                }
                break;
            case FocusIn:
            case FocusOut:
            case DestroyNotify:
            case CreateNotify:
            case VisibilityNotify:
                if( ! ( eventMask & EVENT_WINDOW ) ) {
                    continue;
                }
                break;
        }
        
        switch(evt.type) {
            case ButtonPress:
                if(evt.xbutton.window==w) {
                    (*env)->CallVoidMethod(env, obj, sendMouseEventID, (jint) EVENT_MOUSE_PRESSED, 
                                          (jint) evt.xbutton.state, 
                                          (jint) evt.xbutton.x, (jint) evt.xbutton.y, (jint) evt.xbutton.button);
                }
                break;
            case ButtonRelease:
                if(evt.xbutton.window==w) {
                    (*env)->CallVoidMethod(env, obj, sendMouseEventID, (jint) EVENT_MOUSE_RELEASED, 
                                          (jint) evt.xbutton.state, 
                                          (jint) evt.xbutton.x, (jint) evt.xbutton.y, (jint) evt.xbutton.button);
                }
                break;
            case MotionNotify:
                if(evt.xmotion.window==w) {
                    (*env)->CallVoidMethod(env, obj, sendMouseEventID, (jint) EVENT_MOUSE_MOVED, 
                                          (jint) evt.xmotion.state, 
                                          (jint) evt.xmotion.x, (jint) evt.xmotion.y, (jint) 0);
                }
                break;
            case KeyPress:
                if(evt.xkey.window==w) {
                    if(XLookupString(&evt.xkey,text,255,&keySym,0)==1) {
                        keyChar=text[0];
                    } else {
                        keyChar=0;
                    }
                    (*env)->CallVoidMethod(env, obj, sendKeyEventID, (jint) EVENT_KEY_PRESSED, 
                                          (jint) evt.xkey.state, 
                                          (jint) keySym, (jchar) keyChar);
                }
                break;
            case KeyRelease:
                if(evt.xkey.window==w) {
                    if(XLookupString(&evt.xkey,text,255,&keySym,0)==1) {
                        keyChar=text[0];
                    } else {
                        keyChar=0;
                    }
                    (*env)->CallVoidMethod(env, obj, sendKeyEventID, (jint) EVENT_KEY_RELEASED, 
                                          (jint) evt.xkey.state, 
                                          (jint) keySym, (jchar) keyChar);
                }
                break;
            case FocusIn:
            case FocusOut:
                if(evt.xfocus.window==w) {
                }
                break;
            case DestroyNotify:
                if(evt.xdestroywindow.window==w) {
                    (*env)->CallVoidMethod(env, obj, windowDestroyedID);
                }
                break;
            case CreateNotify:
                if(evt.xcreatewindow.window==w) {
                    (*env)->CallVoidMethod(env, obj, windowCreatedID);
                }
                break;
            case VisibilityNotify:
                if(evt.xvisibility.window==w) {
                }
                break;
        }
    } 
}

/*
 * Class:     com_sun_javafx_newt_x11_X11Window
 * Method:    setSize0
 * Signature: (JJII)V
 */
JNIEXPORT void JNICALL Java_com_sun_javafx_newt_x11_X11Window_setSize0
  (JNIEnv *env, jobject obj, jlong display, jlong window, jint width, jint height)
{
    Display * dpy = (Display *) (intptr_t) display;
    Window w = (Window)window;
    XResizeWindow(dpy, w, (unsigned)width, (unsigned)height);
    (*env)->CallVoidMethod(env, obj, sizeChangedID, (jint) width, (jint) height);
}

/*
 * Class:     com_sun_javafx_newt_x11_X11Window
 * Method:    setPosition0
 * Signature: (JJII)V
 */
JNIEXPORT void JNICALL Java_com_sun_javafx_newt_x11_X11Window_setPosition0
  (JNIEnv *env, jobject obj, jlong display, jlong window, jint x, jint y)
{
    Display * dpy = (Display *) (intptr_t) display;
    Window w = (Window)window;
    XMoveWindow(dpy, w, (unsigned)x, (unsigned)y);
    // (*env)->CallVoidMethod(env, obj, positionChangedID, (jint) width, (jint) height);
}

/*
 * Class:     com_sun_javafx_newt_x11_X11Window
 * Method:    getDisplayWidth0
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_com_sun_javafx_newt_x11_X11Window_getDisplayWidth0
  (JNIEnv *env, jobject obj, jlong display, jint scrn_idx)
{
    Display * dpy = (Display *) (intptr_t) display;
    return (jint) XDisplayWidth( dpy, scrn_idx);
}

/*
 * Class:     com_sun_javafx_newt_x11_X11Window
 * Method:    getDisplayHeight0
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_com_sun_javafx_newt_x11_X11Window_getDisplayHeight0
  (JNIEnv *env, jobject obj, jlong display, jint scrn_idx)
{
    Display * dpy = (Display *) (intptr_t) display;
    return (jint) XDisplayHeight( dpy, scrn_idx);
}

