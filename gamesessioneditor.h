#ifndef GAMESESSIONEDITOR_H
#define GAMESESSIONEDITOR_H

#include <QWidget>

#include <gamesession.h>

namespace Ui {
    class GameSessionEditor;
}

class GameSessionEditor : public QWidget
{
    Q_OBJECT
public:
    explicit GameSessionEditor(QWidget *parent = nullptr);

private:
    void processSessionFiles(QString const& dir);
    void enableAllChildWidgets();
    void displayError(QString const& message);
    bool removeFromWorkspace(QString const& fileName);

signals:
    void sessionLoaded(bool);

private slots:
    void on_addSubButton_clicked();
    void on_removeAvailableSubsButton_clicked();
    void on_transferSubsButton_clicked();

public slots:
    void openFile();
    void saveFile();

private:
    Ui::GameSessionEditor* ui;
    static const QString workspacePath;
    QString openedFilePath; // path to the file being edited
    GameSession gameSession;
};

#endif // GAMESESSIONEDITOR_H
