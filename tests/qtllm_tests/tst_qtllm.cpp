#include <QtTest>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>

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
    void providerFactoryCreatesVendorAliases();
    void providerFactoryRejectsUnknownProvider();
    void openAiCompatibleBuildRequestNormalizesPath();
    void openAiCompatibleBuildRequestAnthropic();
    void openAiCompatibleBuildRequestGoogle();
    void openAiCompatibleBuildPayloadProducesJson();
    void openAiCompatibleBuildPayloadAnthropicTools();
    void openAiCompatibleParseResponse();
    void openAiCompatibleParseAnthropicResponse();
    void openAiCompatibleParseGoogleResponse();
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

void QtLlmCoreTests::providerFactoryCreatesVendorAliases()
{
    QVERIFY(ProviderFactory::create(QStringLiteral("anthropic")) != nullptr);
    QVERIFY(ProviderFactory::create(QStringLiteral("google")) != nullptr);
    QVERIFY(ProviderFactory::create(QStringLiteral("gemini")) != nullptr);
    QVERIFY(ProviderFactory::create(QStringLiteral("deepseek")) != nullptr);
    QVERIFY(ProviderFactory::create(QStringLiteral("qwen")) != nullptr);
    QVERIFY(ProviderFactory::create(QStringLiteral("glm")) != nullptr);
    QVERIFY(ProviderFactory::create(QStringLiteral("zhipu")) != nullptr);
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

void QtLlmCoreTests::openAiCompatibleBuildRequestAnthropic()
{
    OpenAICompatibleProvider provider;

    LlmConfig config;
    config.baseUrl = QStringLiteral("https://api.anthropic.com/v1");
    config.apiKey = QStringLiteral("anthropic-key");
    config.modelVendor = QStringLiteral("anthropic");
    provider.setConfig(config);

    LlmRequest request;
    request.model = QStringLiteral("claude-3-7-sonnet");

    const QNetworkRequest networkRequest = provider.buildRequest(request);

    QCOMPARE(networkRequest.url().toString(), QStringLiteral("https://api.anthropic.com/v1/messages"));
    QCOMPARE(networkRequest.rawHeader("x-api-key"), QByteArray("anthropic-key"));
    QCOMPARE(networkRequest.rawHeader("anthropic-version"), QByteArray("2023-06-01"));
}

void QtLlmCoreTests::openAiCompatibleBuildRequestGoogle()
{
    OpenAICompatibleProvider provider;

    LlmConfig config;
    config.baseUrl = QStringLiteral("https://generativelanguage.googleapis.com");
    config.apiKey = QStringLiteral("google-key");
    config.modelVendor = QStringLiteral("google");
    config.model = QStringLiteral("gemini-2.0-flash");
    provider.setConfig(config);

    LlmRequest request;

    const QNetworkRequest networkRequest = provider.buildRequest(request);
    const QUrl url = networkRequest.url();

    QCOMPARE(url.path(), QStringLiteral("/v1beta/models/gemini-2.0-flash:generateContent"));
    QCOMPARE(url.query(), QStringLiteral("key=google-key"));
    QCOMPARE(networkRequest.rawHeader("x-goog-api-key"), QByteArray("google-key"));
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

void QtLlmCoreTests::openAiCompatibleBuildPayloadAnthropicTools()
{
    OpenAICompatibleProvider provider;

    LlmConfig config;
    config.modelVendor = QStringLiteral("anthropic");
    config.model = QStringLiteral("claude-3-7-sonnet");
    provider.setConfig(config);

    LlmRequest request;
    request.stream = false;
    request.messages.append({QStringLiteral("system"), QStringLiteral("You are an assistant")});
    request.messages.append({QStringLiteral("user"), QStringLiteral("What's weather?")});

    QJsonObject parameters;
    parameters.insert(QStringLiteral("type"), QStringLiteral("object"));
    parameters.insert(QStringLiteral("properties"), QJsonObject{{QStringLiteral("city"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}}}});

    QJsonObject function;
    function.insert(QStringLiteral("name"), QStringLiteral("current_weather"));
    function.insert(QStringLiteral("description"), QStringLiteral("Get weather"));
    function.insert(QStringLiteral("parameters"), parameters);

    QJsonObject tool;
    tool.insert(QStringLiteral("type"), QStringLiteral("function"));
    tool.insert(QStringLiteral("function"), function);

    request.tools.append(tool);

    const QJsonObject payload = QJsonDocument::fromJson(provider.buildPayload(request)).object();
    QCOMPARE(payload.value(QStringLiteral("model")).toString(), QStringLiteral("claude-3-7-sonnet"));
    QCOMPARE(payload.value(QStringLiteral("system")).toString(), QStringLiteral("You are an assistant"));

    const QJsonArray tools = payload.value(QStringLiteral("tools")).toArray();
    QCOMPARE(tools.size(), 1);
    QCOMPARE(tools.first().toObject().value(QStringLiteral("name")).toString(), QStringLiteral("current_weather"));
    QVERIFY(tools.first().toObject().contains(QStringLiteral("input_schema")));
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

void QtLlmCoreTests::openAiCompatibleParseAnthropicResponse()
{
    OpenAICompatibleProvider provider;

    LlmConfig config;
    config.modelVendor = QStringLiteral("anthropic");
    provider.setConfig(config);

    const QByteArray json = R"({
      "content": [
        {"type":"text","text":"Let me check."},
        {"type":"tool_use","id":"toolu_1","name":"current_time","input":{"timezone":"Asia/Shanghai"}}
      ],
      "stop_reason":"tool_use"
    })";

    const LlmResponse response = provider.parseResponse(json);
    QVERIFY(response.success);
    QCOMPARE(response.finishReason, QStringLiteral("tool_use"));
    QCOMPARE(response.assistantMessage.content, QStringLiteral("Let me check."));
    QCOMPARE(response.assistantMessage.toolCalls.size(), 1);
    QCOMPARE(response.assistantMessage.toolCalls.first().name, QStringLiteral("current_time"));
}

void QtLlmCoreTests::openAiCompatibleParseGoogleResponse()
{
    OpenAICompatibleProvider provider;

    LlmConfig config;
    config.modelVendor = QStringLiteral("google");
    provider.setConfig(config);

    const QByteArray json = R"({
      "candidates": [
        {
          "finishReason": "STOP",
          "content": {
            "parts": [
              {"text": "Checking weather."},
              {"functionCall": {"name":"current_weather","args":{"city":"Shanghai"}}}
            ]
          }
        }
      ]
    })";

    const LlmResponse response = provider.parseResponse(json);
    QVERIFY(response.success);
    QCOMPARE(response.finishReason, QStringLiteral("STOP"));
    QCOMPARE(response.assistantMessage.content, QStringLiteral("Checking weather."));
    QCOMPARE(response.assistantMessage.toolCalls.size(), 1);
    QCOMPARE(response.assistantMessage.toolCalls.first().name, QStringLiteral("current_weather"));
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
