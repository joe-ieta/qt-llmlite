#include <QtTest>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "../../src/qtllm/core/llmconfig.h"
#include "../../src/qtllm/core/llmtypes.h"
#include "../../src/qtllm/providers/illmprovider.h"
#include "../../src/qtllm/providers/openaicompatibleprovider.h"
#include "../../src/qtllm/providers/providerfactory.h"
#include "../../src/qtllm/streaming/streamchunkparser.h"

using namespace qtllm;

class QtLlmCoreTests : public QObject
{
    Q_OBJECT

private slots:
    void providerFactoryCreatesKnownProviders();
    void providerFactoryRejectsUnknownProvider();
    void openAiCompatibleBuildRequestNormalizesPath();
    void openAiCompatibleBuildPayloadProducesJson();
    void openAiCompatibleParseResponse();
    void openAiCompatibleParseStreamTokens();
    void streamChunkParserHandlesFragmentedInput();
    void streamChunkParserTakePendingLine();
};

void QtLlmCoreTests::providerFactoryCreatesKnownProviders()
{
    std::unique_ptr<ILLMProvider> ollama = ProviderFactory::create(QStringLiteral("ollama"));
    QVERIFY(ollama != nullptr);
    QCOMPARE(ollama->name(), QStringLiteral("ollama"));

    std::unique_ptr<ILLMProvider> vllm = ProviderFactory::create(QStringLiteral("vllm"));
    QVERIFY(vllm != nullptr);
    QCOMPARE(vllm->name(), QStringLiteral("vllm"));

    std::unique_ptr<ILLMProvider> openai = ProviderFactory::create(QStringLiteral("openai"));
    QVERIFY(openai != nullptr);
    QCOMPARE(openai->name(), QStringLiteral("openai-compatible"));
}

void QtLlmCoreTests::providerFactoryRejectsUnknownProvider()
{
    const std::unique_ptr<ILLMProvider> unknown = ProviderFactory::create(QStringLiteral("unknown-provider"));
    QVERIFY(unknown == nullptr);
}

void QtLlmCoreTests::openAiCompatibleBuildRequestNormalizesPath()
{
    OpenAICompatibleProvider provider;

    LlmConfig config;
    config.baseUrl = QStringLiteral("http://127.0.0.1:11434/v1");
    config.apiKey = QStringLiteral("test-key");
    provider.setConfig(config);

    LlmRequest request;
    const QNetworkRequest networkRequest = provider.buildRequest(request);

    QCOMPARE(networkRequest.url().toString(), QStringLiteral("http://127.0.0.1:11434/v1/chat/completions"));
    QCOMPARE(networkRequest.rawHeader("Authorization"), QByteArray("Bearer test-key"));
}

void QtLlmCoreTests::openAiCompatibleBuildPayloadProducesJson()
{
    OpenAICompatibleProvider provider;

    LlmConfig config;
    config.model = QStringLiteral("default-model");
    provider.setConfig(config);

    LlmRequest request;
    request.stream = true;
    request.messages.append({QStringLiteral("user"), QStringLiteral("hello")});

    const QByteArray payload = provider.buildPayload(request);
    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    QVERIFY(doc.isObject());

    const QJsonObject obj = doc.object();
    QCOMPARE(obj.value(QStringLiteral("model")).toString(), QStringLiteral("default-model"));
    QCOMPARE(obj.value(QStringLiteral("stream")).toBool(), true);

    const QJsonArray messages = obj.value(QStringLiteral("messages")).toArray();
    QCOMPARE(messages.size(), 1);
    QCOMPARE(messages.first().toObject().value(QStringLiteral("role")).toString(), QStringLiteral("user"));
    QCOMPARE(messages.first().toObject().value(QStringLiteral("content")).toString(), QStringLiteral("hello"));
}

void QtLlmCoreTests::openAiCompatibleParseResponse()
{
    OpenAICompatibleProvider provider;

    const QByteArray okJson = R"({"choices":[{"message":{"content":"ok"}}]})";
    const LlmResponse ok = provider.parseResponse(okJson);
    QVERIFY(ok.success);
    QCOMPARE(ok.text, QStringLiteral("ok"));

    const QByteArray badJson = R"({"choices":[]})";
    const LlmResponse bad = provider.parseResponse(badJson);
    QVERIFY(!bad.success);
    QVERIFY(!bad.errorMessage.isEmpty());
}

void QtLlmCoreTests::openAiCompatibleParseStreamTokens()
{
    OpenAICompatibleProvider provider;

    const QByteArray chunk =
        "data: {\"choices\":[{\"delta\":{\"content\":\"Hel\"}}]}\n"
        "data: {\"choices\":[{\"delta\":{\"content\":\"lo\"}}]}\n"
        "data: [DONE]\n";

    const QList<QString> tokens = provider.parseStreamTokens(chunk);
    QCOMPARE(tokens.size(), 2);
    QCOMPARE(tokens.at(0), QStringLiteral("Hel"));
    QCOMPARE(tokens.at(1), QStringLiteral("lo"));
}

void QtLlmCoreTests::streamChunkParserHandlesFragmentedInput()
{
    StreamChunkParser parser;

    const QStringList first = parser.append("line1-part");
    QVERIFY(first.isEmpty());

    const QStringList second = parser.append("-end\nline2\n");
    QCOMPARE(second.size(), 2);
    QCOMPARE(second.at(0), QStringLiteral("line1-part-end"));
    QCOMPARE(second.at(1), QStringLiteral("line2"));
}

void QtLlmCoreTests::streamChunkParserTakePendingLine()
{
    StreamChunkParser parser;

    parser.append("partial");
    QCOMPARE(parser.takePendingLine(), QStringLiteral("partial"));
    QCOMPARE(parser.takePendingLine(), QString());
}

QTEST_MAIN(QtLlmCoreTests)
#include "tst_qtllm.moc"
