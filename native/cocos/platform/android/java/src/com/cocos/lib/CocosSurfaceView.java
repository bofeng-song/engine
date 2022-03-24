/****************************************************************************
 * Copyright (c) 2020 Xiamen Yaji Software Co., Ltd.
 *
 * http://www.cocos.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ****************************************************************************/

package com.cocos.lib;

import android.content.Context;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.inputmethod.EditorInfo;

import com.google.androidgamesdk.gametextinput.GameTextInput;
import com.google.androidgamesdk.gametextinput.InputConnection;

public class CocosSurfaceView extends SurfaceView {
    private CocosTouchHandler mTouchHandler;
    private CocosTextInput mCocosTextInput;

    public CocosSurfaceView(Context context, CocosTextInput cocosTextInput) {
        super(context);
        mTouchHandler = new CocosTouchHandler();
        mCocosTextInput = cocosTextInput;
    }

    private native void nativeOnSizeChanged(final int width, final int height);

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        CocosHelper.runOnGameThreadAtForeground(new Runnable() {
            @Override
            public void run() {
                nativeOnSizeChanged(w, h);
            }
        });
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        return mTouchHandler.onTouchEvent(event);
    }

    @Override
    public android.view.inputmethod.InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        InputConnection inputConnection = mCocosTextInput.getInputConnection();
        if (null == inputConnection) {
            return super.onCreateInputConnection(outAttrs);
        }
        if (outAttrs != null) {
            GameTextInput.copyEditorInfo(inputConnection.getEditorInfo(), outAttrs);
        }

        return inputConnection;
    }
}
