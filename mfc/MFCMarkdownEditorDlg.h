// MFCMarkdownEditorDlg.h: 头文件
//

#pragma once

#include "MarkdownParser.h"

// CMFCMarkdownEditorDlg 对话框
class CMFCMarkdownEditorDlg : public CDialogEx
{
// 构造
public:
    CMFCMarkdownEditorDlg(CWnd* pParent = nullptr);  // 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MFCMARKDOWNEDITOR_DIALOG };
#endif

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持


// 实现
protected:
    HICON m_hIcon;

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    DECLARE_MESSAGE_MAP()

public:
    // 控件变量
    CEdit m_editMarkdown;
    CHtmlView* m_pHtmlView;
    CWnd m_wndHtmlContainer;
    
    // Markdown 解析器
    CMarkdownParser m_markdownParser;
    
    // 当前文件路径
    CString m_strFilePath;
    // 修改标志
    BOOL m_bModified;
    
    // 更新预览
    void UpdatePreview();
    
    // 菜单处理函数
    afx_msg void OnFileNew();
    afx_msg void OnFileOpen();
    afx_msg void OnFileSave();
    afx_msg void OnFileSaveAs();
    afx_msg void OnFileExit();
    afx_msg void OnEditUndo();
    afx_msg void OnEditRedo();
    afx_msg void OnEditCut();
    afx_msg void OnEditCopy();
    afx_msg void OnEditPaste();
    afx_msg void OnEditSelectAll();
    afx_msg void OnViewFullscreen();
    afx_msg void OnViewRefresh();
    afx_msg void OnFormatBold();
    afx_msg void OnFormatItalic();
    afx_msg void OnFormatHeading1();
    afx_msg void OnFormatHeading2();
    afx_msg void OnFormatHeading3();
    afx_msg void OnFormatCode();
    afx_msg void OnFormatList();
    afx_msg void OnToolsOptions();
    afx_msg void OnHelpAbout();
};
