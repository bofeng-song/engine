package com.cocos.lib;

import android.content.Context;
import android.os.Handler;
import android.text.InputType;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;

import androidx.core.graphics.Insets;

import com.google.androidgamesdk.gametextinput.InputConnection;
import com.google.androidgamesdk.gametextinput.Listener;
import com.google.androidgamesdk.gametextinput.Settings;
import com.google.androidgamesdk.gametextinput.State;

class CocosTextInput implements Listener {

    private static final int ACTION_ID = 1000;
    private InputConnection mInputConnection;
    private Context mApplicationContext;

    CocosTextInput(Context context) {
        mApplicationContext = context.getApplicationContext();
        initNative();
    }

    @Override
    public void stateChanged(State state, boolean dismissed) {
        onTextInputEventNative(state);
    }

    @Override
    public void onImeInsetsChanged(Insets insets) {
        onImeInsetsChangedNative(insets.left, insets.top, insets.right, insets.bottom);
        Log.d("dd", "bf test top=" + insets.top + " bottom=" + insets.bottom + "  left=" + insets.left + " right=" + insets.right);
    }

    void createInputConnection(View targetView) {
        EditorInfo editorInfo = new EditorInfo();
        editorInfo.inputType = InputType.TYPE_CLASS_TEXT ;
        editorInfo.packageName = mApplicationContext.getPackageName();
        editorInfo.actionLabel = "actionDone";
        editorInfo.actionId = EditorInfo.IME_ACTION_DONE;
        editorInfo.imeOptions = EditorInfo.IME_ACTION_DONE | EditorInfo.IME_FLAG_NO_FULLSCREEN;
        mInputConnection = (new InputConnection(mApplicationContext, targetView, new Settings(editorInfo, editorInfo.inputType == 0))).setListener(this);
        setInputConnectionNative(mInputConnection);
    }

    void destroy() {
        destroyNative();
    }

    void setEditorInfo(String confirmType, String inputType, boolean isMultiLine) {
        EditorInfo editorInfo = this.mInputConnection.getEditorInfo();
        if (confirmType.contentEquals("done")) {
            editorInfo.imeOptions = EditorInfo.IME_ACTION_DONE | EditorInfo.IME_FLAG_NO_FULLSCREEN;
        } else if (confirmType.contentEquals("next")) {
            editorInfo.imeOptions = EditorInfo.IME_ACTION_NEXT | EditorInfo.IME_FLAG_NO_EXTRACT_UI;
        } else if (confirmType.contentEquals("search")) {
            editorInfo.imeOptions = EditorInfo.IME_ACTION_SEARCH | EditorInfo.IME_FLAG_NO_EXTRACT_UI;
        } else if (confirmType.contentEquals("go")) {
            editorInfo.imeOptions = EditorInfo.IME_ACTION_GO | EditorInfo.IME_FLAG_NO_EXTRACT_UI;
        } else if (confirmType.contentEquals("send")) {
            editorInfo.imeOptions = EditorInfo.IME_ACTION_SEND | EditorInfo.IME_FLAG_NO_EXTRACT_UI;
        } else {
            editorInfo.imeOptions = EditorInfo.IME_NULL;
        }

        if (inputType.contentEquals("text")) {
            editorInfo.inputType = InputType.TYPE_CLASS_TEXT;
            if (isMultiLine) {
                editorInfo.inputType |= InputType.TYPE_TEXT_FLAG_MULTI_LINE;
            }
        } else if (inputType.contentEquals("email")) {
            editorInfo.inputType = InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS;
        } else if (inputType.contentEquals("number")) {
            editorInfo.inputType = InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_DECIMAL | InputType.TYPE_NUMBER_FLAG_SIGNED;
        } else if (inputType.contentEquals("phone")) {
            editorInfo.inputType = InputType.TYPE_CLASS_PHONE;
        } else if (inputType.contentEquals("password")) {
            editorInfo.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD;
        }
        Insets dd = mInputConnection.getImeInsets();
        Log.d("dd", "bf test top=" + dd.top + " bottom=" + dd.bottom);
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                Insets aa = mInputConnection.getImeInsets();
                Log.d("dd", "bf test top=" + aa.top + " bottom=" + aa.bottom + "left=" + aa.left + " right=" + aa.right);
            }
        }, 2000);
    }

    InputConnection getInputConnection() {
        return mInputConnection;
    }

    private static native void initNative();
    private static native void destroyNative();
    private native void onImeInsetsChangedNative(int left, int top, int right, int bottom);
    private native void onTextInputEventNative(State softKeyboardEvent);
    private native void setInputConnectionNative(InputConnection c);
}
