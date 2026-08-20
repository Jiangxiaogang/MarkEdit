// MFCMarkdownEditor.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "framework.h"
#include "MFCMarkdownEditor.h"
#include "MFCMarkdownEditorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCMarkdownEditorApp

BEGIN_MESSAGE_MAP(CMFCMarkdownEditorApp, CWinApp)
    ON_COMMAND(ID_HELP_ABOUT, &CWinApp::OnHelpAbout)
END_MESSAGE_MAP()


// CMFCMarkdownEditorApp 构造

CMFCMarkdownEditorApp::CMFCMarkdownEditorApp()
{
    // 支持重新启动管理器
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
    // 如果应用程序是利用公共语言运行时支持 (/clr) 构建的，则:
    //     1) 必须有此附加设置，“重新启动管理器”支持才能正常工作。
    //     2) 在您的项目中，您必须按照生成顺序向 System.Windows.Forms 添加引用。
    System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

    // 将 Shell 管理器、任务栏列表等设置为 NULL
    m_pShellManager = nullptr;
    m_pTaskbarList = nullptr;
}

// 唯一的 CMFCMarkdownEditorApp 对象

CMFCMarkdownEditorApp theApp;


// CMFCMarkdownEditorApp 初始化

BOOL CMFCMarkdownEditorApp::InitInstance()
{
    // 如果一个运行在 Windows XP 上的应用程序清单指定要使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
    // 则需要 InitCommonControlsEx()；否则，将无法创建窗口。
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // 将它设置为包括所有要在应用程序中使用的公共控件类。
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();


    AfxEnableControlContainer();

    // 创建 shell 管理器，以防对话框包含任何 shell 树视图控件
    // 或 shell 列表视图控件。
    m_pShellManager = std::make_unique<CShellManager>();

    // 激活“视觉 Native Manager”可视化主题，允许在菜单中激活主题
    CVisualManager::SetDefaultClass(RUNTIME_CLASS(CMFCVisualManagerWindows));

    // 标准初始化
    // 如果未使用这些功能并希望减小最终可执行文件的大小，
    // 应删除不需要的以下特定初始化例程。
    // 更改用于存储设置的注册表项
    SetRegistryKey(_T("本地应用程序向导生成的应用程序"));

    CMFCMarkdownEditorDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
        // 使用“确定”关闭对话框时
    }
    else if (nResponse == IDCANCEL)
    {
        // 使用“取消”关闭对话框时
    }
    else if (nResponse == -1)
    {
        TRACE(traceAppMsg, 0, "警告：对话框创建失败，因此应用程序意外终止。\n");
        TRACE(traceAppMsg, 0, "警告：如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
    }

    // 删除上面创建的 shell 管理器。
    if (m_pShellManager != nullptr)
    {
        m_pShellManager.reset();
    }

#if defined(_AfxDLL) && !defined(_AFX_NO_KEEP_LIBSTATE)
    AfxTermThread(nullptr);
#endif

    // 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
    //  而不是启动应用程序的消息泵。
    return FALSE;
}
