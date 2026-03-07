#pragma once

#include <QWidget>

class QTextEdit;
class QLineEdit;
class QPushButton;
class QtLLMClient;

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWindow(QWidget *parent = nullptr);

private slots:
    void onSendClicked();

private:
    QTextEdit *m_output;
    QLineEdit *m_input;
    QPushButton *m_sendButton;
    QtLLMClient *m_client;
};
