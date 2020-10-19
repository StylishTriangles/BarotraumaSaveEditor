#ifndef GAMESESSIONEDITOR_H
#define GAMESESSIONEDITOR_H

#include <QWidget>

#include <gamesession.h>

class GameSessionEditor : public QWidget
{
    Q_OBJECT
public:
    explicit GameSessionEditor(QWidget *parent = nullptr);

private:
    void processSessionFiles(QString const& dir);
    void enableAllChildWidgets();
    void displayError(QString const& message);

signals:

private slots:
    void on_saveButton_clicked();
    void on_addSubButton_clicked();

public slots:
    void openFile();
    void saveFile();

private:
    static const QString workspacePath;
    QString openedFilePath; // path to the file being edited
    GameSession gameSession;
};

#endif // GAMESESSIONEDITOR_H
