/****************************************************************************
 Copyright (c) 2017-2022 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
****************************************************************************/


#include "CocosTextInput.h"
#include <android/looper.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string>
#include "java/jni/JniHelper.h"

#include "base/Log.h"
#include "base/UTF8.h"
#include "game-text-input/gametextinput.h"

#ifndef JCLS_COCOS_TEXT_INPUT
#define JCLS_COCOS_TEXT_INPUT "com/cocos/lib/CocosTextInput"
#endif

GameTextInput* cc::gameTextInput = nullptr;

namespace cc {

    GameTextInputState innerState;
    CocosTextInput *cocosTextInput = nullptr;

    /*
 * Write a command to be executed by the GameActivity on the application main
 * thread.
 */
    static void write_cmd(int fd, CmdInfo &work) {
        restart:
        int res = write(fd, &work, sizeof(work));
        if (res < 0 && errno == EINTR) {
            goto restart;
        }

        if (res == sizeof(work)) return;

        if (res < 0) {
            CC_LOG_WARNING("Failed writing to work fd: %s", strerror(errno));
        } else {
            CC_LOG_WARNING("Truncated writing to work fd: %d", res);
        }
    }

/*
 * Read commands to be executed by the GameActivity on the application main
 * thread.
 */
    static bool read_cmd(int fd, CmdInfo *outWork) {
        int res = read(fd, outWork, sizeof(CmdInfo));
        // no need to worry about EINTR, poll loop will just come back again.
        if (res == sizeof(CmdInfo)) return true;

        if (res < 0) {
            CC_LOG_WARNING("Failed reading work fd: %s", strerror(errno));
        } else {
            CC_LOG_WARNING("Truncated reading work fd: %d", res);
        }
        return false;
    }


    void setEditorInfo(const std::string &confirmType, const std::string &inputType, bool isMultiline) {
        JNIEnv *env = JniHelper::getEnv();
        jstring jstrCfrmType = cc::StringUtils::newStringUTFJNI(env, confirmType);
        jstring jstrInputType = cc::StringUtils::newStringUTFJNI(env, inputType);
//        JniHelper::callObjectVoidMethod((jobject)cc::cocosTextInputJObject, JCLS_COCOS_TEXT_INPUT, "setEditorInfo", jstrCfrmType, jstrInputType, isMultiline);

        if (jstrCfrmType) {
            env->DeleteLocalRef(jstrCfrmType);
        }
        if (jstrInputType) {
            env->DeleteLocalRef(jstrInputType);
        }
    }
    /*
 * Callback for handling native events on the application's main thread.
 */
    static int mainWorkCallback(int fd, int events, void *data) {
        CC_LOG_INFO("************** mainWorkCallback *********");
        CocosTextInput *input = (CocosTextInput *) data;
        if ((events & POLLIN) == 0) {
            return 1;
        }

        CmdInfo work;
        if (!read_cmd(input->mainThreadRead, &work)) {
            return 1;
        }
        CC_LOG_INFO("mainWorkCallback: cmd=%d", work.cmd);
        switch (work.cmd) {
            case CMD_SHOW_SOFT_INPUT: {
                setEditorInfo(work.confirmType, work.inputType, work.isMultiline);
                innerState.text_UTF8 = work.defaultTxt.c_str();
                innerState.text_length = work.defaultTxt.length();
                GameTextInputSpan span;
                span.end = span.start = work.defaultTxt.length() - 1;
                innerState.selection = span;
                GameTextInput_setState(input->gameTextInput, &innerState);
                GameTextInput_showIme(input->gameTextInput, ShowImeFlags::SHOW_FORCED);
            }
            break;
            case CMD_HIDE_SOFT_INPUT: {
                GameTextInput_hideIme(input->gameTextInput, HideImeFlags::HIDE_NOT_ALWAYS);
            }
            break;
            default:
                CC_LOG_WARNING("Unknown work command: %d", work.cmd);
                break;
        }

        return 1;
    }


    void showIme(const std::string &defaultTxt, const std::string &confirmType, const std::string &inputType, bool isMultiLine) {
        CmdInfo info;
        info.cmd = CMD_SHOW_SOFT_INPUT;
        info.confirmType = confirmType;
        info.inputType = inputType;
        info.defaultTxt = defaultTxt;
        info.isMultiline = isMultiLine;
        write_cmd(cc::cocosTextInput->mainThreadWrite, info);
    }

    void hideIme() {
        CmdInfo info;
        info.cmd = CMD_HIDE_SOFT_INPUT;
        write_cmd(cc::cocosTextInput->mainThreadWrite, info);
    }


    CocosTextInput::CocosTextInput() {
        mainThreadLooper = ALooper_forThread();
        gameTextInput = cc::gameTextInput;
        int msgpipe[2];
        if (pipe(msgpipe)) {
            std::string info = "could not create pipe: ";
            info += strerror(errno);

            CC_LOG_INFO("%s", info.c_str());
            return;
        }
        mainThreadRead = msgpipe[0];
        mainThreadWrite = msgpipe[1];
        int result = fcntl(mainThreadRead, F_SETFL, O_NONBLOCK);
        if (result != 0) {
            CC_LOG_WARNING("Could not make main work read pipe "
                           "non-blocking: %s",
                           strerror(errno));
        }

        result = fcntl(mainThreadWrite, F_SETFL, O_NONBLOCK);
        if (result != 0) {
            CC_LOG_WARNING("Could not make main work write pipe "
                           "non-blocking: %s",
                           strerror(errno));
        }
        ALooper_addFd(mainThreadLooper, mainThreadRead, 0, ALOOPER_EVENT_INPUT,
                      mainWorkCallback, this);
    }

    CocosTextInput::~CocosTextInput() {
        if (mainThreadLooper && mainThreadRead >= 0) {
            ALooper_removeFd(mainThreadLooper, mainThreadRead);
        }
        if (mainThreadLooper) {
            ALooper_release(mainThreadLooper);
        }
        if (mainThreadRead >= 0) {
            close(mainThreadRead);
        }
        if (mainThreadWrite >= 0) {
            close(mainThreadWrite);
        }
    }

}; // namespace cc

extern "C" {

//NOLINTNEXTLINE
JNIEXPORT void JNICALL Java_com_cocos_lib_CocosTextInput_initNative(JNIEnv *env, jobject obj, jobject cocosTextInput) {
    cc::gameTextInput = GameTextInput_init(env, 0);
    cc::cocosTextInput = new cc::CocosTextInput();
}

//NOLINTNEXTLINE
JNIEXPORT void JNICALL Java_com_cocos_lib_CocosTextInput_setInputConnectionNative(JNIEnv *env, jobject obj, jobject inputConnection, jobject cocosTextInput) {
    cc::gameTextInput = GameTextInput_init(env, 0);
    cc::cocosTextInput = new cc::CocosTextInput();
    cc::cocosTextInputJObject = env->NewGlobalRef(cocosTextInput);
    GameTextInput_setInputConnection(cc::gameTextInput, inputConnection);
    GameTextInput_setEventCallback(cc::gameTextInput, cc::onTextInputEventCallback, env);
}

//NOLINTNEXTLINE
JNIEXPORT void JNICALL Java_com_cocos_lib_CocosTextInput_onTextInputEventNative(JNIEnv *env, jobject obj, jobject soft_keyboard_event) {
    GameTextInput_processEvent(cc::gameTextInput, soft_keyboard_event);
}

//NOLINTNEXTLINE
JNIEXPORT void JNICALL Java_com_cocos_lib_CocosTextInput_onImeInsetsChangedNative(JNIEnv *env, jobject obj, jint left, jint top, jint right, jint bottom) {
    ARect rect{left, top, right, bottom};
    GameTextInput_processImeInsets(cc::gameTextInput, &rect);
}
}
