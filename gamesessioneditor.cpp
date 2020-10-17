#include "gamesessioneditor.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QLabel>

GameSessionEditor::GameSessionEditor(QWidget *parent) : QWidget(parent)
{

}

void GameSessionEditor::on_saveButton_clicked() {
//    saveGame();
}

void GameSessionEditor::openFile() {
    QChar separator = QDir::separator();
    // save directory, taken from:
    // https://github.com/Regalis11/Barotrauma/blob/0002ad2c501a1a8df323b52edfc82a78d0afc6bc/Barotrauma/BarotraumaShared/SharedSource/Utils/SaveUtil.cs#L29
    QString save_directory =
            QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + separator +
            "Daedalic Entertainment GmbH" + separator +
            "Barotrauma";

    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open Saved Game"),
                save_directory,
                tr("Savegame files (*.save)")
    );
    QLabel* labelFilename = findChild<QLabel*>("label_filename");
    if (filename != "") {
        labelFilename->setText(filename);
    } else {
        labelFilename->setText(tr("No file chosen"));
    }
}
