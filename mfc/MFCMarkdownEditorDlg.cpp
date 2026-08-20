// MFCMarkdownEditorDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCMarkdownEditor.h"
#include "MFCMarkdownEditorDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCMarkdownEditorDlg 对话框



CMFCMarkdownEditorDlg::CMFCMarkdownEditorDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_MFCMARKDOWNEDITOR_DIALOG, pParent)
    , m_bModified(FALSE)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_pHtmlView = nullptr;
    m_strFilePath.Empty();
}

void CMFCMarkdownEditorDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_MARKDOWN, m_editMarkdown);
}

BEGIN_MESSAGE_MAP(CMFCMarkdownEditorDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_SIZE()
    ON_WM_TIMER()
    ON_COMMAND(ID_FILE_NEW, &CMFCMarkdownEditorDlg::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, &CMFCMarkdownEditorDlg::OnFileOpen)
    ON_COMMAND(ID_FILE_SAVE, &CMFCMarkdownEditorDlg::OnFileSave)
    ON_COMMAND(ID_FILE_SAVE_AS, &CMFCMarkdownEditorDlg::OnFileSaveAs)
    ON_COMMAND(ID_FILE_EXIT, &CMFCMarkdownEditorDlg::OnFileExit)
    ON_COMMAND(ID_EDIT_UNDO, &CMFCMarkdownEditorDlg::OnEditUndo)
    ON_COMMAND(ID_EDIT_REDO, &CMFCMarkdownEditorDlg::OnEditRedo)
    ON_COMMAND(ID_EDIT_CUT, &CMFCMarkdownEditorDlg::OnEditCut)
    ON_COMMAND(ID_EDIT_COPY, &CMFCMarkdownEditorDlg::OnEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, &CMFCMarkdownEditorDlg::OnEditPaste)
    ON_COMMAND(ID_EDIT_SELECT_ALL, &CMFCMarkdownEditorDlg::OnEditSelectAll)
    ON_COMMAND(ID_VIEW_FULLSCREEN, &CMFCMarkdownEditorDlg::OnViewFullscreen)
    ON_COMMAND(ID_VIEW_REFRESH, &CMFCMarkdownEditorDlg::OnViewRefresh)
    ON_COMMAND(ID_FORMAT_BOLD, &CMFCMarkdownEditorDlg::OnFormatBold)
    ON_COMMAND(ID_FORMAT_ITALIC, &CMFCMarkdownEditorDlg::OnFormatItalic)
    ON_COMMAND(ID_FORMAT_HEADING1, &CMFCMarkdownEditorDlg::OnFormatHeading1)
    ON_COMMAND(ID_FORMAT_HEADING2, &CMFCMarkdownEditorDlg::OnFormatHeading2)
    ON_COMMAND(ID_FORMAT_HEADING3, &CMFCMarkdownEditorDlg::OnFormatHeading3)
    ON_COMMAND(ID_FORMAT_CODE, &CMFCMarkdownEditorDlg::OnFormatCode)
    ON_COMMAND(ID_FORMAT_LIST, &CMFCMarkdownEditorDlg::OnFormatList)
    ON_COMMAND(ID_TOOLS_OPTIONS, &CMFCMarkdownEditorDlg::OnToolsOptions)
    ON_COMMAND(ID_HELP_ABOUT, &CMFCMarkdownEditorDlg::OnHelpAbout)
END_MESSAGE_MAP()


// CMFCMarkdownEditorDlg 消息处理程序

BOOL CMFCMarkdownEditorDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);         // 设置大图标
    SetIcon(m_hIcon, FALSE);        // 设置小图标

    // 设置窗口标题
    SetWindowText(_T("Markdown 编辑器"));

    // 创建 HTML 预览容器
    CRect rect;
    GetClientRect(&rect);
    
    // 计算右侧区域 (假设左侧编辑区占 50%)
    CRect htmlRect(rect.left + rect.Width() / 2 + 5, rect.top, rect.right, rect.bottom);
    
    // 创建 CHtmlView 用于 MSHTML 预览
    m_pHtmlView = new CHtmlView();
    m_pHtmlView->Create(NULL, _T("HTML Preview"), WS_CHILD | WS_VISIBLE | WS_BORDER, htmlRect, this, IDC_HTML_PREVIEW);
    
    // 初始化预览内容
    UpdatePreview();

    // 设置定时器用于实时预览 (每 500ms 更新一次)
    SetTimer(1, 500, NULL);

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCMarkdownEditorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        //CAboutDlg dlgAbout;
        //dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCMarkdownEditorDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

// 当用户拖动最小化窗口时系统调用此函数取得
//  光标显示。
HCURSOR CMFCMarkdownEditorDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CMFCMarkdownEditorDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    // 调整编辑控件和 HTML 预览的大小
    if (m_editMarkdown.GetSafeHwnd() != NULL && m_pHtmlView != NULL)
    {
        CRect rect;
        GetClientRect(&rect);
        
        // 左侧编辑区
        CRect editRect(rect.left, rect.top, rect.Width() / 2 - 5, rect.bottom);
        m_editMarkdown.MoveWindow(&editRect);
        
        // 右侧预览区
        CRect htmlRect(rect.left + rect.Width() / 2 + 5, rect.top, rect.right, rect.bottom);
        m_pHtmlView->MoveWindow(&htmlRect);
    }
}

void CMFCMarkdownEditorDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1)
    {
        // 定期更新预览
        UpdatePreview();
    }
    CDialogEx::OnTimer(nIDEvent);
}

void CMFCMarkdownEditorDlg::UpdatePreview()
{
    // 获取编辑框中的 Markdown 文本
    CString strMarkdown;
    m_editMarkdown.GetWindowText(strMarkdown);
    
    // 转换为 HTML
    CString strHtml = m_markdownParser.Parse(strMarkdown);
    
    // 在 MSHTML 控件中显示
    if (m_pHtmlView != NULL && m_pHtmlView->GetSafeHwnd() != NULL)
    {
        // 使用 Navigate2 显示 HTML 内容
        CString strUrl = _T("about:blank");
        m_pHtmlView->Navigate2(strUrl);
        
        // 获取 HTML 文档并写入内容
        LPDISPATCH lpDispatch = m_pHtmlView->GetDocument();
        if (lpDispatch != NULL)
        {
            IHTMLDocument2* pDoc = NULL;
            HRESULT hr = lpDispatch->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);
            if (SUCCEEDED(hr) && pDoc != NULL)
            {
                // 清除现有内容
                pDoc->write(COleVariant(strHtml));
                pDoc->close();
                pDoc->Release();
            }
            lpDispatch->Release();
        }
    }
}

// 菜单命令处理函数

void CMFCMarkdownEditorDlg::OnFileNew()
{
    m_editMarkdown.SetWindowText(_T(""));
    m_strFilePath.Empty();
    m_bModified = FALSE;
    SetWindowText(_T("Markdown 编辑器"));
    UpdatePreview();
}

void CMFCMarkdownEditorDlg::OnFileOpen()
{
    CFileDialog dlg(TRUE, _T("md"), NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("Markdown 文件 (*.md;*.markdown)|*.md;*.markdown|所有文件 (*.*)|*.*||"), this);
    
    if (dlg.DoModal() == IDOK)
    {
        m_strFilePath = dlg.GetPathName();
        
        CFile file;
        if (file.Open(m_strFilePath, CFile::modeRead | CFile::typeText))
        {
            ULONGLONG fileSize = file.GetLength();
            TCHAR* pBuffer = new TCHAR[fileSize + 1];
            
            file.Read(pBuffer, (UINT)fileSize);
            pBuffer[fileSize] = _T('\0');
            
            m_editMarkdown.SetWindowText(pBuffer);
            delete[] pBuffer;
            file.Close();
            
            m_bModified = FALSE;
            SetWindowText(_T("Markdown 编辑器 - ") + m_strFilePath);
            UpdatePreview();
        }
    }
}

void CMFCMarkdownEditorDlg::OnFileSave()
{
    if (m_strFilePath.IsEmpty())
    {
        OnFileSaveAs();
        return;
    }
    
    CString strContent;
    m_editMarkdown.GetWindowText(strContent);
    
    CFile file;
    if (file.Open(m_strFilePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
    {
        file.WriteString(strContent);
        file.Close();
        m_bModified = FALSE;
    }
}

void CMFCMarkdownEditorDlg::OnFileSaveAs()
{
    CFileDialog dlg(FALSE, _T("md"), _T("untitled.md"), OFN_OVERWRITEPROMPT,
        _T("Markdown 文件 (*.md;*.markdown)|*.md;*.markdown|所有文件 (*.*)|*.*||"), this);
    
    if (dlg.DoModal() == IDOK)
    {
        m_strFilePath = dlg.GetPathName();
        OnFileSave();
        SetWindowText(_T("Markdown 编辑器 - ") + m_strFilePath);
    }
}

void CMFCMarkdownEditorDlg::OnFileExit()
{
    OnCancel();
}

void CMFCMarkdownEditorDlg::OnEditUndo()
{
    m_editMarkdown.Undo();
}

void CMFCMarkdownEditorDlg::OnEditRedo()
{
    // CEdit 不直接支持 Redo，这里简单实现
    OnEditUndo(); // 简化处理
}

void CMFCMarkdownEditorDlg::OnEditCut()
{
    m_editMarkdown.Cut();
}

void CMFCMarkdownEditorDlg::OnEditCopy()
{
    m_editMarkdown.Copy();
}

void CMFCMarkdownEditorDlg::OnEditPaste()
{
    m_editMarkdown.Paste();
}

void CMFCMarkdownEditorDlg::OnEditSelectAll()
{
    m_editMarkdown.SetSel(0, -1);
}

void CMFCMarkdownEditorDlg::OnViewFullscreen()
{
    // 全屏切换逻辑 (简化)
    ModifyStyle(WS_CAPTION | WS_THICKFRAME, 0);
    ShowWindow(SW_MAXIMIZE);
}

void CMFCMarkdownEditorDlg::OnViewRefresh()
{
    UpdatePreview();
}

void CMFCMarkdownEditorDlg::OnFormatBold()
{
    // 在选中文本前后添加 **
    int nStart, nEnd;
    m_editMarkdown.GetSel(nStart, nEnd);
    
    if (nStart != nEnd)
    {
        CString strText;
        m_editMarkdown.GetWindowText(strText);
        CString strSelected = strText.Mid(nStart, nEnd - nStart);
        CString strNewText = strText.Left(nStart) + _T("**") + strSelected + _T("**") + strText.Mid(nEnd);
        m_editMarkdown.SetWindowText(strNewText);
        m_editMarkdown.SetSel(nStart, nEnd + 4);
        m_bModified = TRUE;
    }
}

void CMFCMarkdownEditorDlg::OnFormatItalic()
{
    // 在选中文本前后添加 *
    int nStart, nEnd;
    m_editMarkdown.GetSel(nStart, nEnd);
    
    if (nStart != nEnd)
    {
        CString strText;
        m_editMarkdown.GetWindowText(strText);
        CString strSelected = strText.Mid(nStart, nEnd - nStart);
        CString strNewText = strText.Left(nStart) + _T("*") + strSelected + _T("*") + strText.Mid(nEnd);
        m_editMarkdown.SetWindowText(strNewText);
        m_editMarkdown.SetSel(nStart, nEnd + 2);
        m_bModified = TRUE;
    }
}

void CMFCMarkdownEditorDlg::OnFormatHeading1()
{
    int nStart, nEnd;
    m_editMarkdown.GetSel(nStart, nEnd);
    
    CString strText;
    m_editMarkdown.GetWindowText(strText);
    
    // 找到当前行
    int nLineStart = m_editMarkdown.LineIndex(m_editMarkdown.LineFromChar(nStart));
    CString strLine = strText.Mid(nLineStart);
    int nLineEnd = strLine.Find(_T("\r\n"));
    if (nLineEnd == -1) nLineEnd = strLine.GetLength();
    else nLineEnd = nLineStart + nLineEnd;
    
    CString strNewText = strText.Left(nLineStart) + _T("# ") + strText.Mid(nLineStart, nLineEnd - nLineStart) + strText.Mid(nLineEnd);
    m_editMarkdown.SetWindowText(strNewText);
    m_bModified = TRUE;
}

void CMFCMarkdownEditorDlg::OnFormatHeading2()
{
    int nStart, nEnd;
    m_editMarkdown.GetSel(nStart, nEnd);
    
    CString strText;
    m_editMarkdown.GetWindowText(strText);
    
    int nLineStart = m_editMarkdown.LineIndex(m_editMarkdown.LineFromChar(nStart));
    CString strLine = strText.Mid(nLineStart);
    int nLineEnd = strLine.Find(_T("\r\n"));
    if (nLineEnd == -1) nLineEnd = strLine.GetLength();
    else nLineEnd = nLineStart + nLineEnd;
    
    CString strNewText = strText.Left(nLineStart) + _T("## ") + strText.Mid(nLineStart, nLineEnd - nLineStart) + strText.Mid(nLineEnd);
    m_editMarkdown.SetWindowText(strNewText);
    m_bModified = TRUE;
}

void CMFCMarkdownEditorDlg::OnFormatHeading3()
{
    int nStart, nEnd;
    m_editMarkdown.GetSel(nStart, nEnd);
    
    CString strText;
    m_editMarkdown.GetWindowText(strText);
    
    int nLineStart = m_editMarkdown.LineIndex(m_editMarkdown.LineFromChar(nStart));
    CString strLine = strText.Mid(nLineStart);
    int nLineEnd = strLine.Find(_T("\r\n"));
    if (nLineEnd == -1) nLineEnd = strLine.GetLength();
    else nLineEnd = nLineStart + nLineEnd;
    
    CString strNewText = strText.Left(nLineStart) + _T("### ") + strText.Mid(nLineStart, nLineEnd - nLineStart) + strText.Mid(nLineEnd);
    m_editMarkdown.SetWindowText(strNewText);
    m_bModified = TRUE;
}

void CMFCMarkdownEditorDlg::OnFormatCode()
{
    int nStart, nEnd;
    m_editMarkdown.GetSel(nStart, nEnd);
    
    if (nStart != nEnd)
    {
        CString strText;
        m_editMarkdown.GetWindowText(strText);
        CString strSelected = strText.Mid(nStart, nEnd - nStart);
        CString strNewText = strText.Left(nStart) + _T("`") + strSelected + _T("`") + strText.Mid(nEnd);
        m_editMarkdown.SetWindowText(strNewText);
        m_editMarkdown.SetSel(nStart, nEnd + 2);
        m_bModified = TRUE;
    }
}

void CMFCMarkdownEditorDlg::OnFormatList()
{
    int nStart, nEnd;
    m_editMarkdown.GetSel(nStart, nEnd);
    
    CString strText;
    m_editMarkdown.GetWindowText(strText);
    
    int nLineStart = m_editMarkdown.LineIndex(m_editMarkdown.LineFromChar(nStart));
    CString strNewText = strText.Left(nLineStart) + _T("- ") + strText.Mid(nLineStart);
    m_editMarkdown.SetWindowText(strNewText);
    m_bModified = TRUE;
}

void CMFCMarkdownEditorDlg::OnToolsOptions()
{
    AfxMessageBox(_T("选项功能待实现"));
}

void CMFCMarkdownEditorDlg::OnHelpAbout()
{
    AfxMessageBox(_T("Markdown 编辑器\n\n基于 MFC 和 MSHTML 内核\n不使用第三方库"));
}
