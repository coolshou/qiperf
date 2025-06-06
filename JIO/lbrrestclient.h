#ifndef LBRRESTCLIENT_H
#define LBRRESTCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>

class LBRRestClient : public QObject
{
    Q_OBJECT

public:
    explicit LBRRestClient(QString baseurl="https://127.0.0.1:5000", QObject *parent = nullptr);
    ~LBRRestClient();

    // --- Authentication ---
    void login(const QString &username, const QString &password);
    void logout();
    QString accessToken() const { return m_accessToken; }
    bool isLoggedIn() const { return !m_accessToken.isEmpty(); }

    // --- Protected API Calls (requires JWT) ---
    void getTodos();
    void addTodo(const QString &task, bool completed);
    void updateTodo(int id, const QString &task, bool completed);
    void deleteTodo(int id);

signals:
    // Authentication signals
    void loginSuccess();
    void loginFailed(const QString &error);
    void logoutSuccess();

    // API response signals
    void todosReceived(const QJsonArray &todos);
    void addTodoSuccess(const QJsonObject &newTodo);
    void updateTodoSuccess(const QJsonObject &updatedTodo);
    void deleteTodoSuccess(int id);
    void apiError(const QString &message, int statusCode = -1);

private slots:
    // --- Slots to handle network replies ---
    void onLoginReplyFinished(QNetworkReply *reply);
    void onLogoutReplyFinished(QNetworkReply *reply);
    void onGetTodosReplyFinished(QNetworkReply *reply);
    void onAddTodoReplyFinished(QNetworkReply *reply);
    void onUpdateTodoReplyFinished(QNetworkReply *reply);
    void onDeleteTodoReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkAccessManager;
    QUrl m_baseUrl;
    QString m_accessToken;

    void sendRequest(QNetworkRequest &request, QNetworkAccessManager::Operation operation, const QByteArray &data = QByteArray());
    void handleReply(QNetworkReply *reply, std::function<void(const QJsonDocument&)> successHandler);
};

#endif // LBRRESTCLIENT_H
