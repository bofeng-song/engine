package com.cocos.lib;

import android.content.Context;
import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.widget.TextView;

import com.google.androidgamesdk.gametextinput.GameTextInput;
import com.google.androidgamesdk.gametextinput.InputConnection;


public class CocosTextView extends TextView {
    private CocosTextInput mCocosTextInput;

    public CocosTextView(Context context, CocosTextInput cocosTextInput) {
        super(context);
        mCocosTextInput = cocosTextInput;
    }

    @Override
    public android.view.inputmethod.InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        return super.onCreateInputConnection(outAttrs);
//        InputConnection inputConnection = mCocosTextInput.getInputConnection();
//        if (null == inputConnection) {
//            return super.onCreateInputConnection(outAttrs);
//        }
//        if (outAttrs != null) {
//            GameTextInput.copyEditorInfo(inputConnection.getEditorInfo(), outAttrs);
//        }
//
//        return inputConnection;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return super.onKeyDown(keyCode, event);
    }
}
