#include "multiclientwindow.h"

#include "../../qtllm/logging/logtypes.h"
#include "../../qtllm/logging/qtllmlogger.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>

namespace {

QString findWorkspaceRoot(const QString &startPath)
{
    QDir dir(startPath);
    while (dir.exists()) {
        if (QFileInfo(dir.filePath(QStringLiteral("qt-llm.pro"))).exists()) {
            return dir.absolutePath();
        }
        if (!dir.cdUp()) {
            break;
        }
    }
    return startPath;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qRegisterMetaType<qtllm::logging::LogEvent>();

    const QString workspaceRoot = findWorkspaceRoot(QDir::currentPath());
    qtllm::logging::FileLogSinkOptions logOptions;
    logOptions.workspaceRoot = workspaceRoot;
    logOptions.maxBytesPerFile = 8 * 1024 * 1024;
    logOptions.maxFilesPerClient = 50;
    qtllm::logging::QtLlmLogger::instance().setMinimumLevel(qtllm::logging::LogLevel::Debug);
    qtllm::logging::QtLlmLogger::instance().installFileSink(logOptions);

    MultiClientWindow window;
    window.show();

    return app.exec();
}
