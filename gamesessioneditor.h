#ifndef GAMESESSIONEDITOR_H
#define GAMESESSIONEDITOR_H

#include <QWidget>

class GameSessionEditor : public QWidget
{
    Q_OBJECT
public:
    explicit GameSessionEditor(QWidget *parent = nullptr);

signals:

private slots:
    void on_saveButton_clicked();

public slots:
    void openFile();
    void saveFile();

private:
    QString savedGamePath;
};

#endif // GAMESESSIONEDITOR_H
