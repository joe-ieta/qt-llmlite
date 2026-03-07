#pragma once

#include "llmconfig.h"
#include "llmtypes.h"

#include <QObject>
#include <memory>

class ILLMProvider;
class HttpExecutor;
class StreamChunkParser;

class QtLLMClient : public QObject
{
    Q_OBJECT
public:
    explicit QtLLMClient(QObject *parent = nullptr);
    ~QtLLMClient() override;

    void setConfig(const LlmConfig &config);
    void setProvider(std::unique_ptr<ILLMProvider> provider);
    bool setProviderByName(const QString &providerName);

    void sendPrompt(const QString &prompt);
    void cancelCurrentRequest();

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
    std::unique_ptr<StreamChunkParser> m_streamParser;
    QString m_accumulatedText;
};