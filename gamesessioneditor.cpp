#include "gamesessioneditor.h"
#include <QDir>
#include <QErrorMessage>
#include <QFileDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStandardPaths>
#include <QTabWidget>

#include <stdexcept>

#include <saveutil.h>

const QString GameSessionEditor::workspacePath = "workspace";

GameSessionEditor::GameSessionEditor(QWidget *parent) : QWidget(parent)
{
}

// Populate forms with extracted data
// Data should be located in directory dir
void GameSessionEditor::processSessionFiles(QString const& dir) {
    return;
}

// saves changes made in editor to directory dir
void GameSessionEditor::saveWorkspace(QString const& dir) {
    // unimplemented
}

void GameSessionEditor::enableAllChildWidgets() {
    for (QWidget* wp: findChildren<QWidget*>()) {
        wp->setEnabled(true);
    }
}

void GameSessionEditor::displayError(QString const& message) {
    QErrorMessage em(this);
    em.showMessage(message);
    em.exec();
}

void GameSessionEditor::on_saveButton_clicked() {
    saveFile();
}

void GameSessionEditor::on_addSubButton_clicked() {
    QString subPath = QFileDialog::getOpenFileName(
                this,
                tr("Add submarine file"),
                ".",
                tr("Submarine Files (*.sub)")
    );
    if (subPath == "")
        return;
    QFile subFile(subPath);
    QFileInfo subFileInfo(subFile);
    QString subFileName = subFileInfo.fileName();
    QString subFileExt = subFileInfo.suffix();
    QString dest = workspacePath + QDir::separator() + subFileName;
    if (QFile::exists(dest)) {
        displayError(tr("Submarine with this name already exists in current game session"));
        return;
    }
    subFile.copy(dest);
    QString subName(std::move(subFileName));
    subName.chop(subFileExt.size());
    // add sub to available list
    findChild<QListWidget*>("availableSubsList")->addItem("subName");
    // add sub to XML tree
    /// unimplemented
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
        displayError(e.what());
        return;
    }

    // process extracted data
    processSessionFiles(workspacePath);

    // save the edited file path on success
    openedFilePath = filePath;
    QLabel* labelFilename = findChild<QLabel*>("label_filename");
    labelFilename->setText(filePath);

    // enable editing of UI form
    enableAllChildWidgets(); // enable widgets inside QTabWidget
    findChild<QPushButton*>("saveButton")->setEnabled(true);
}

void GameSessionEditor::saveFile() {
    // write changes made in editor
    saveWorkspace(workspacePath);

    // compress directory and overwrite opened savegame
    try {
        SaveUtil::compressDirectory(workspacePath, openedFilePath);
    } catch(std::runtime_error const& e) {
        QErrorMessage em(this);
        em.showMessage(e.what());
        em.exec();
        return;
    }
}
