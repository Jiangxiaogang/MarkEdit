// MFCMarkdownEditor.h: 主头文件，用于应用程序类
//

#pragma once

#ifndef __AFXWIN_H__
    #error "在包含此文件之前包括 'pch.h' 以生成 PCH"
#endif

#include "Resource.h"       // 主符号


// CMFCMarkdownEditorApp:
// 请参阅 MFCMarkdownEditor.cpp 以了解此类的实现。

class CMFCMarkdownEditorApp : public CWinApp
{
public:
    CMFCMarkdownEditorApp();

// 重写
public:
    virtual BOOL InitInstance();

// 实现

    DECLARE_MESSAGE_MAP()
};

extern CMFCMarkdownEditorApp theApp;
