#include "gamesessioneditor.h"
#include <QErrorMessage>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QLabel>

#include <stdexcept>

#include <saveutil.h>

const QString GameSessionEditor::workspacePath = "workspace";

GameSessionEditor::GameSessionEditor(QWidget *parent) : QWidget(parent)
{

}

void GameSessionEditor::on_saveButton_clicked() {
    saveFile();
}

void GameSessionEditor::openFile() {
    QChar separator = QDir::separator();
    // save directory, taken from:
    // https://github.com/Regalis11/Barotrauma/blob/0002ad2c501a1a8df323b52edfc82a78d0afc6bc/Barotrauma/BarotraumaShared/SharedSource/Utils/SaveUtil.cs#L29
    QString save_directory =
            QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + separator +
            "Daedalic Entertainment GmbH" + separator +
            "Barotrauma";

    QString filePath = QFileDialog::getOpenFileName(
                this,
                tr("Open Saved Game"),
                save_directory,
                tr("Savegame files (*.save)")
    );
    if (filePath == "")
        return;
    // extract save file to workspace
    try {
        SaveUtil::decompressToDirectory(filePath, workspacePath);
    } catch (std::runtime_error const& e){
        QErrorMessage em(this);
        em.showMessage(e.what());
        em.exec();
        return;
    }
    // save the edited file path on success
    openedFilePath = filePath;
    QLabel* labelFilename = findChild<QLabel*>("label_filename");
    labelFilename->setText(filePath);
}

void GameSessionEditor::saveFile() {

}
