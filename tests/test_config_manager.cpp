#include "../src/configmanager.h"

#include <QCoreApplication>
#include <QSettings>
#include <cassert>
#include <iostream>

static int failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { std::cerr << "FAIL: " << msg << "\n"; ++failures; } \
    else { std::cout << "PASS: " << msg << "\n"; } \
} while (0)

int main(int argc, char *argv[])
{
    // Use an isolated settings file for the test.
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("MarkEditTest");
    QCoreApplication::setApplicationName("MarkEditTest");

    // Clean previous test settings.
    QSettings().clear();

    ConfigManager *cfg = ConfigManager::instance();
    cfg->setShowLineNumbers(false);
    cfg->setShowWhitespace(true);
    cfg->setTabWidth(2);
    QFont f("TestFont");
    f.setPointSize(14);
    cfg->setEditorFont(f);
    cfg->setCssFilePath("custom.css");
    cfg->setSyncScroll(false);
    cfg->addRecentFile("/tmp/a.md");
    cfg->addRecentFile("/tmp/b.md");
    cfg->addRecentFile("/tmp/a.md"); // duplicate should be de-duplicated
    cfg->saveConfig();

    // Reload from a fresh instance.
    ConfigManager *cfg2 = new ConfigManager(nullptr);
    CHECK(cfg2->showLineNumbers() == false, "showLineNumbers persisted");
    CHECK(cfg2->showWhitespace() == true, "showWhitespace persisted");
    CHECK(cfg2->tabWidth() == 2, "tabWidth persisted");
    CHECK(cfg2->editorFont().family() == "TestFont", "font family persisted");
    CHECK(cfg2->editorFont().pointSize() == 14, "font size persisted");
    CHECK(cfg2->cssFilePath() == "custom.css", "css path persisted");
    CHECK(cfg2->syncScroll() == false, "syncScroll persisted");

    QStringList recent = cfg2->recentFiles(10);
    CHECK(recent.size() == 2, "recent files de-duplicated");
    CHECK(recent.first() == "/tmp/a.md", "most recent file first");

    delete cfg2;
    QSettings().clear();

    if (failures == 0) {
        std::cout << "All ConfigManager tests passed.\n";
        return 0;
    }
    std::cerr << failures << " test(s) failed.\n";
    return 1;
}
