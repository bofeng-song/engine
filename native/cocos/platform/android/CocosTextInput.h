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

#pragma once

#include <string>

class GameTextInput;

class GameTextInputState;

struct ALooper;

namespace cc {

    extern GameTextInput *gameTextInput;

    // run in mainThread
    extern void onTextInputEventCallback(void *context, const GameTextInputState *current_state);

    // run in gameThread
    extern void showIme(const std::string &defaultTxt,  const std::string &confirmType, const std::string &inputType, bool isMultiLine);

    // run in gameThread
    extern void hideIme();

    extern void setTextInputState(const GameTextInputState *current_state);

    struct CmdInfo {
        std::string defaultTxt;
        std::string confirmType;
        std::string inputType;
        int cmd;
        bool isMultiline;
    };

    class CocosTextInput {
    public:
        CocosTextInput();

        ~CocosTextInput();


        GameTextInput *gameTextInput;
        ALooper *mainThreadLooper{nullptr};
        int mainThreadRead{-1};
        int mainThreadWrite{-1};
    };

    enum {
        CMD_FINISH = 1,
        CMD_SET_WINDOW_FORMAT,
        CMD_SET_WINDOW_FLAGS,
        CMD_SHOW_SOFT_INPUT,
        CMD_HIDE_SOFT_INPUT,
        CMD_SET_SOFT_INPUT_STATE
    };

} // namespace cc
