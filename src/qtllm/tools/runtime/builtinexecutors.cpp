#include "builtinexecutors.h"

#include <QDateTime>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimeZone>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace qtllm::tools::runtime {

QString CurrentTimeToolExecutor::toolId() const
{
    return QStringLiteral("current_time");
}

ToolExecutionResult CurrentTimeToolExecutor::execute(const ToolCallRequest &request,
                                                     const ToolExecutionContext &context)
{
    Q_UNUSED(context)

    ToolExecutionResult result;
    result.callId = request.callId;
    result.toolId = toolId();

    const QString timezoneName = request.arguments.value(QStringLiteral("timezone")).toString().trimmed();
    QTimeZone timezone;
    if (!timezoneName.isEmpty()) {
        timezone = QTimeZone(timezoneName.toUtf8());
        if (!timezone.isValid()) {
            result.errorCode = QStringLiteral("invalid_timezone");
            result.errorMessage = QStringLiteral("Invalid timezone: ") + timezoneName;
            result.retryable = false;
            return result;
        }
    }

    const QDateTime now = timezone.isValid()
        ? QDateTime::currentDateTimeUtc().toTimeZone(timezone)
        : QDateTime::currentDateTime();

    QJsonObject output;
    output.insert(QStringLiteral("iso8601"), now.toString(Qt::ISODateWithMs));
    output.insert(QStringLiteral("timezone"), timezone.isValid() ? QString::fromUtf8(timezone.id()) : QString::fromUtf8(QTimeZone::systemTimeZoneId()));
    output.insert(QStringLiteral("epochMs"), QString::number(now.toMSecsSinceEpoch()));

    result.success = true;
    result.output = output;
    return result;
}

QString CurrentWeatherToolExecutor::toolId() const
{
    return QStringLiteral("current_weather");
}

ToolExecutionResult CurrentWeatherToolExecutor::execute(const ToolCallRequest &request,
                                                        const ToolExecutionContext &context)
{
    Q_UNUSED(context)

    ToolExecutionResult result;
    result.callId = request.callId;
    result.toolId = toolId();

    const QJsonValue latValue = request.arguments.value(QStringLiteral("latitude"));
    const QJsonValue lonValue = request.arguments.value(QStringLiteral("longitude"));

    if (!latValue.isDouble() || !lonValue.isDouble()) {
        result.errorCode = QStringLiteral("missing_coordinates");
        result.errorMessage = QStringLiteral("current_weather requires numeric latitude and longitude");
        result.retryable = false;
        return result;
    }

    const double latitude = latValue.toDouble();
    const double longitude = lonValue.toDouble();
    const QString timezone = request.arguments.value(QStringLiteral("timezone")).toString().trimmed();

    QUrl url(QStringLiteral("https://api.open-meteo.com/v1/forecast"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("latitude"), QString::number(latitude, 'f', 6));
    query.addQueryItem(QStringLiteral("longitude"), QString::number(longitude, 'f', 6));
    query.addQueryItem(QStringLiteral("current"), QStringLiteral("temperature_2m,apparent_temperature,weather_code,wind_speed_10m"));
    query.addQueryItem(QStringLiteral("timezone"), timezone.isEmpty() ? QStringLiteral("auto") : timezone);
    url.setQuery(query);

    QNetworkAccessManager manager;
    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QNetworkReply *reply = manager.get(networkRequest);
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    QObject::connect(&timeout, &QTimer::timeout, &loop, [&]() {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
        loop.quit();
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timeout.start(10000);
    loop.exec();

    const bool timeoutHit = !timeout.isActive();
    timeout.stop();

    if (timeoutHit) {
        result.errorCode = QStringLiteral("weather_timeout");
        result.errorMessage = QStringLiteral("current_weather request timed out");
        result.retryable = true;
        reply->deleteLater();
        return result;
    }

    if (reply->error() != QNetworkReply::NoError) {
        result.errorCode = QStringLiteral("weather_http_error");
        result.errorMessage = reply->errorString();
        result.retryable = true;
        reply->deleteLater();
        return result;
    }

    const QByteArray payload = reply->readAll();
    reply->deleteLater();

    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isObject()) {
        result.errorCode = QStringLiteral("weather_parse_error");
        result.errorMessage = QStringLiteral("Invalid JSON response from weather service");
        result.retryable = true;
        return result;
    }

    const QJsonObject root = doc.object();
    const QJsonObject current = root.value(QStringLiteral("current")).toObject();
    if (current.isEmpty()) {
        result.errorCode = QStringLiteral("weather_data_missing");
        result.errorMessage = QStringLiteral("Weather response does not contain current weather data");
        result.retryable = true;
        return result;
    }

    QJsonObject output;
    output.insert(QStringLiteral("latitude"), root.value(QStringLiteral("latitude")).toDouble(latitude));
    output.insert(QStringLiteral("longitude"), root.value(QStringLiteral("longitude")).toDouble(longitude));
    output.insert(QStringLiteral("timezone"), root.value(QStringLiteral("timezone")).toString(timezone));
    output.insert(QStringLiteral("current"), current);

    result.success = true;
    result.output = output;
    return result;
}

} // namespace qtllm::tools::runtime
