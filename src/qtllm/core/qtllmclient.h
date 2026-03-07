#pragma once

#include "llmconfig.h"
#include "llmtypes.h"

#include <QObject>
#include <memory>

class ILLMProvider;
class HttpExecutor;

class QtLLMClient : public QObject
{
    Q_OBJECT
public:
    explicit QtLLMClient(QObject *parent = nullptr);
    ~QtLLMClient() override;

    void setConfig(const LlmConfig &config);
    void setProvider(std::unique_ptr<ILLMProvider> provider);
    void sendPrompt(const QString &prompt);

signals:
    void tokenReceived(const QString &token);
    void completed(const QString &text);
    void errorOccurred(const QString &message);

private:
    void wireExecutor();

private:
    LlmConfig m_config;
    std::unique_ptr<ILLMProvider> m_provider;
    HttpExecutor *m_executor;
    QString m_accumulatedText;
};
