#include "gamesessioneditor.h"
#include <QDebug>
#include <QDir>
#include <QErrorMessage>
#include <QFileDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
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
    QDir sessionDir(dir);
    QStringList subNames;
    bool gameSessionFound = false;
    for (QString const& str: sessionDir.entryList(QDir::Files)) {
         QFileInfo fileInfo(dir + QDir::separator() + str);
         if (fileInfo.fileName() == "gamesession.xml") {
             gameSessionFound = true;
             bool success = gameSession.fromXML(fileInfo.absoluteFilePath());
             if (!success)
                 qDebug() << "Error processing gamesession.xml";
         }
    }
    if (!gameSessionFound) {
        displayError("Could not find gamesession.xml in workspace");
    }
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
    QListWidget* availableSubsList = findChild<QListWidget*>("availableSubsList");
    QString subName(std::move(subFileName));
    subName.chop(subFileExt.size()+1); // remove extension and dot
    if (QFile::exists(dest) || !availableSubsList->findItems(subName, Qt::MatchExactly).empty()) {
        displayError(tr("Submarine with this name already exists in current game session"));
        return;
    }
    // copy submarine to workspace
    subFile.copy(dest);
    // add sub to available list
    availableSubsList->addItem(subName);
    // add sub to XML tree
    try {
        gameSession.addSubmarine(subName, true, false);
    } catch (std::runtime_error const& e) {
        displayError(e.what());
    }
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
    // write changes made to session in editor
    gameSession.dumpXML();

    // compress directory and overwrite opened savegame
    try {
        SaveUtil::compressDirectory(workspacePath, openedFilePath);
    } catch(std::runtime_error const& e) {
        QErrorMessage em(this);
        em.showMessage(e.what());
        em.exec();
        return;
    }
    QMessageBox::information(
                this,
                tr("Save successful"),
                tr("The game session was saved succesfully!")
    );
}
