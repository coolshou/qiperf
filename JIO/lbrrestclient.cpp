#include "lbrrestclient.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QEventLoop> // For synchronous wait in sendRequest (not recommended for UI threads)

LBRRestClient::LBRRestClient(QString baseurl, QObject *parent) : QObject(parent)
{
    m_networkAccessManager = new QNetworkAccessManager(this);
    m_baseUrl = QUrl(baseurl); // Your Flask API base URL

    // Connect common reply handler for debugging (optional but good practice)
    connect(m_networkAccessManager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply) {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Network error for" << reply->url() << ":" << reply->errorString();
            // General error handling could go here or be specific to each reply slot
        }
    });
}

LBRRestClient::~LBRRestClient()
{
    // QNetworkAccessManager is parented, so it will be deleted automatically.
    // However, if you had a different parent strategy, you might need:
    // delete m_networkAccessManager;
}

// --- Helper for sending requests ---
void LBRRestClient::sendRequest(QNetworkRequest &request,
                               QNetworkAccessManager::Operation operation,
                               const QByteArray &data)
{
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!m_accessToken.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
    }

    QNetworkReply *reply = nullptr;
    switch (operation) {
        case QNetworkAccessManager::GetOperation:
            reply = m_networkAccessManager->get(request);
            break;
        case QNetworkAccessManager::PostOperation:
            reply = m_networkAccessManager->post(request, data);
            break;
        case QNetworkAccessManager::PutOperation:
            reply = m_networkAccessManager->put(request, data);
            break;
        case QNetworkAccessManager::DeleteOperation:
            reply = m_networkAccessManager->deleteResource(request);
            break;
        default:
            qWarning() << "Unsupported HTTP operation";
            return;
    }

    // Connect reply specific slots
    if (reply) {
        qDebug() << "TODO: reply";
        // if (request.url().path() == "/login") {
        //     connect(reply, &QNetworkReply::finished, this, &LBRRestClient::onLoginReplyFinished);
        // } else if (request.url().path() == "/logout") {
        //     connect(reply, &QNetworkReply::finished, this, &LBRRestClient::onLogoutReplyFinished);
        // } else if (request.url().path() == "/todos") {
        //     if (operation == QNetworkAccessManager::GetOperation) {
        //         connect(reply, &QNetworkReply::finished, this, &LBRRestClient::onGetTodosReplyFinished);
        //     } else if (operation == QNetworkAccessManager::PostOperation) {
        //         connect(reply, &QNetworkReply::finished, this, &LBRRestClient::onAddTodoReplyFinished);
        //     }
        // } else if (request.url().path().startsWith("/todos/")) {
        //      if (operation == QNetworkAccessManager::PutOperation) {
        //          connect(reply, &QNetworkReply::finished, this, &LBRRestClient::onUpdateTodoReplyFinished);
        //      } else if (operation == QNetworkAccessManager::DeleteOperation) {
        //          connect(reply, &QNetworkReply::finished, this, &LBRRestClient::onDeleteTodoReplyFinished);
        //      }
        // }
    }
}

// --- Generic Reply Handler ---
void LBRRestClient::handleReply(QNetworkReply *reply,
                               std::function<void(const QJsonDocument&)> successHandler)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

        if (parseError.error == QJsonParseError::NoError) {
            successHandler(doc);
        } else {
            qWarning() << "JSON parse error:" << parseError.errorString();
            emit apiError("JSON parse error: " + parseError.errorString());
        }
    } else {
        qWarning() << "API Error:" << reply->errorString() << "Status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        emit apiError(reply->errorString(), reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    }
    reply->deleteLater(); // Clean up the reply object
}

// --- Authentication Methods ---

void LBRRestClient::login(const QString &username, const QString &password)
{
    QUrl loginUrl = m_baseUrl;
    loginUrl.setPath("/login");
    QNetworkRequest request(loginUrl);

    QJsonObject loginData;
    loginData["username"] = username;
    loginData["password"] = password;
    QJsonDocument doc(loginData);

    sendRequest(request, QNetworkAccessManager::PostOperation,
                doc.toJson(QJsonDocument::Compact));
}

void LBRRestClient::onLoginReplyFinished(QNetworkReply *reply)
{
    handleReply(reply, [this](const QJsonDocument& doc) {
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("access_token") && obj["access_token"].isString()) {
                m_accessToken = obj["access_token"].toString();
                qDebug() << "Login successful! Token:" << m_accessToken;
                emit loginSuccess();
            } else if (obj.contains("message") && obj["message"].isString()) {
                qWarning() << "Login failed:" << obj["message"].toString();
                emit loginFailed(obj["message"].toString());
            } else {
                qWarning() << "Login failed: Unexpected response format.";
                emit loginFailed("Unexpected response format.");
            }
        } else {
            qWarning() << "Login failed: Response is not a JSON object.";
            emit loginFailed("Unexpected response format.");
        }
    });
}

void LBRRestClient::logout()
{
    QUrl logoutUrl = m_baseUrl;
    logoutUrl.setPath("/logout");
    QNetworkRequest request(logoutUrl);
    sendRequest(request, QNetworkAccessManager::PostOperation); // No body needed for logout
}

void LBRRestClient::onLogoutReplyFinished(QNetworkReply *reply)
{
    handleReply(reply, [this](const QJsonDocument& doc) {
        if (doc.isObject() && doc.object().contains("message")) {
            qDebug() << "Logout successful:" << doc.object()["message"].toString();
            m_accessToken.clear(); // Clear the stored token
            emit logoutSuccess();
        } else {
            qWarning() << "Logout failed or unexpected response.";
            emit apiError("Logout failed or unexpected response.");
        }
    });
}

// --- Protected API Calls ---

void LBRRestClient::getTodos()
{
    QUrl todosUrl = m_baseUrl;
    todosUrl.setPath("/todos");
    QNetworkRequest request(todosUrl);
    sendRequest(request, QNetworkAccessManager::GetOperation);
}

void LBRRestClient::onGetTodosReplyFinished(QNetworkReply *reply)
{
    handleReply(reply, [this](const QJsonDocument& doc) {
        if (doc.isObject() && doc.object().contains("todos") && doc.object()["todos"].isArray()) {
            QJsonArray todosArray = doc.object()["todos"].toArray();
            qDebug() << "Todos received:" << todosArray.size();
            emit todosReceived(todosArray);
        } else if (doc.isObject() && doc.object().contains("message") && doc.object()["message"].isString()) {
            // Handle cases where API returns message even on success (e.g., empty list)
            qWarning() << "Get Todos: " << doc.object()["message"].toString();
            emit apiError(doc.object()["message"].toString());
        }
        else {
            qWarning() << "Get Todos: Unexpected response format.";
            emit apiError("Unexpected response format for todos.");
        }
    });
}

void LBRRestClient::addTodo(const QString &task, bool completed)
{
    QUrl addTodoUrl = m_baseUrl;
    addTodoUrl.setPath("/todos");
    QNetworkRequest request(addTodoUrl);

    QJsonObject todoData;
    todoData["task"] = task;
    todoData["completed"] = completed;
    QJsonDocument doc(todoData);

    sendRequest(request, QNetworkAccessManager::PostOperation, doc.toJson(QJsonDocument::Compact));
}

void LBRRestClient::onAddTodoReplyFinished(QNetworkReply *reply)
{
    Q_UNUSED(reply)
    qDebug() << "TODO: onAddTodoReplyFinished";
    // handleReply(reply, [this](const QJsonDocument& doc) {
    //     if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 201 && doc.isObject()) {
    //         qDebug() << "Todo added successfully:" << doc.object()["task"].toString();
    //         emit addTodoSuccess(doc.object());
    //     } else {
    //         qWarning() << "Add Todo failed or unexpected response.";
    //         emit apiError("Failed to add todo or unexpected response.", reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    //     }
    // });
}

void LBRRestClient::updateTodo(int id, const QString &task, bool completed)
{
    QUrl updateTodoUrl = m_baseUrl;
    updateTodoUrl.setPath(QString("/todos/%1").arg(id));
    QNetworkRequest request(updateTodoUrl);

    QJsonObject todoData;
    todoData["task"] = task;
    todoData["completed"] = completed;
    QJsonDocument doc(todoData);

    sendRequest(request, QNetworkAccessManager::PutOperation, doc.toJson(QJsonDocument::Compact));
}

void LBRRestClient::onUpdateTodoReplyFinished(QNetworkReply *reply)
{
    Q_UNUSED(reply)
    qDebug() << "TODO onUpdateTodoReplyFinished";
    // handleReply(reply, [this](const QJsonDocument& doc) {
    //     if (doc.isObject()) {
    //         qDebug() << "Todo updated successfully:" << doc.object()["task"].toString();
    //         emit updateTodoSuccess(doc.object());
    //     } else {
    //         qWarning() << "Update Todo failed or unexpected response.";
    //         emit apiError("Failed to update todo or unexpected response.", reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    //     }
    // });
}

void LBRRestClient::deleteTodo(int id)
{
    QUrl deleteTodoUrl = m_baseUrl;
    deleteTodoUrl.setPath(QString("/todos/%1").arg(id));
    QNetworkRequest request(deleteTodoUrl);
    sendRequest(request, QNetworkAccessManager::DeleteOperation);
}

void LBRRestClient::onDeleteTodoReplyFinished(QNetworkReply *reply)
{
    Q_UNUSED(reply)
    qDebug() << "TODO onDeleteTodoReplyFinished";
    // handleReply(reply, [this](const QJsonDocument& doc) {
    //     if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200 && doc.isObject() && doc.object().contains("message")) {
    //         qDebug() << "Todo deleted successfully: " << doc.object()["message"].toString();
    //         // If your API returns the ID of the deleted item, you could pass it.
    //         // For now, we'll assume the deletion request was for a known ID.
    //         // You might need to adjust this based on your API's delete response.
    //         // For simplicity, we'll just emit success without the ID here.
    //         emit deleteTodoSuccess(0); // Emit 0 or the actual ID if extractable
    //     } else {
    //         qWarning() << "Delete Todo failed or unexpected response.";
    //         emit apiError("Failed to delete todo or unexpected response.", reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    //     }
    // });
}
