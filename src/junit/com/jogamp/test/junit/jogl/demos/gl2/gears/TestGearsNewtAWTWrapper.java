/*
 * Copyright (c) 2010 Sven Gothel. All Rights Reserved.
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
 * Neither the name Sven Gothel or the names of
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
 * SVEN GOTHEL HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 */

package com.jogamp.test.junit.jogl.demos.gl2.gears;

import javax.media.nativewindow.*;
import javax.media.opengl.*;
import com.jogamp.opengl.util.Animator;

import com.jogamp.test.junit.jogl.demos.gl2.gears.Gears;
import com.jogamp.newt.*;
import com.jogamp.newt.event.*;
import com.jogamp.newt.opengl.*;

import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.AfterClass;
import org.junit.After;
import org.junit.Test;

public class TestGearsNewtAWTWrapper {
    static GLProfile glp;
    static int width, height;

    @BeforeClass
    public static void initClass() {
        glp = GLProfile.getDefault();
        Assert.assertNotNull(glp);
        width  = 512;
        height = 512;
    }

    @AfterClass
    public static void releaseClass() {
    }

    protected void runTestGL(GLCapabilities caps) throws InterruptedException {
        Display nDisplay = NewtFactory.createDisplay(NativeWindowFactory.TYPE_AWT, null); // local display
        Screen nScreen  = NewtFactory.createScreen(NativeWindowFactory.TYPE_AWT, nDisplay, 0); // screen 0
        Window nWindow = NewtFactory.createWindow(NativeWindowFactory.TYPE_AWT, nScreen, caps, false);

        GLWindow glWindow = GLWindow.create(nWindow);
        Assert.assertNotNull(glWindow);
        glWindow.setTitle("Gears NewtAWTWrapper Test");

        glWindow.addGLEventListener(new Gears());

        Animator animator = new Animator(glWindow);
        QuitAdapter quitAdapter = new QuitAdapter();

        glWindow.addKeyListener(new TraceKeyAdapter(quitAdapter));
        glWindow.addWindowListener(new TraceWindowAdapter(quitAdapter));

        glWindow.setSize(width, height);
        glWindow.setVisible(true);
        animator.start();

        while(!quitAdapter.shouldQuit() && animator.isAnimating() && animator.getDuration()<duration) {
            Thread.sleep(100);
        }

        animator.stop();
        glWindow.destroy(true);
    }

    @Test
    public void test01() throws InterruptedException {
        GLCapabilities caps = new GLCapabilities(GLProfile.getDefault());
        runTestGL(caps);
    }

    static long duration = 500; // ms

    public static void main(String args[]) {
        for(int i=0; i<args.length; i++) {
            if(args[i].equals("-time")) {
                i++;
                try {
                    duration = Integer.parseInt(args[i]);
                } catch (Exception ex) { ex.printStackTrace(); }
            }
        }
        org.junit.runner.JUnitCore.main(TestGearsNewtAWTWrapper.class.getName());
    }
}