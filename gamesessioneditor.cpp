#include "gamesessioneditor.h"
#include "ui_gamesessioneditor.h"
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

GameSessionEditor::GameSessionEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameSessionEditor)
{
    ui->setupUi(this);
    connect(this, SIGNAL(sessionLoaded(bool)), ui->subTab, SLOT(setEnabled(bool)));
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
    // list available submarines
    ui->availableSubsList->addItems(gameSession.getSubmarines(GameSession::AvailableSubmarine));
    ui->ownedSubsList->addItems(gameSession.getSubmarines(GameSession::OwnedSubmarine));
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

void GameSessionEditor::on_addSubButton_clicked() {
    QString subPath = QFileDialog::getOpenFileName(
                this,
                tr("Add submarine file"),
                "",
                tr("Submarine File (*.sub)")
    );
    if (subPath == "")
        return;
    QFile subFile(subPath);
    QFileInfo subFileInfo(subFile);
    QString subFileName = subFileInfo.fileName();
    QString subFileExt = subFileInfo.suffix();
    QString dest = workspacePath + QDir::separator() + subFileName;
    QListWidget* availableSubsList = ui->availableSubsList;
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
        gameSession.addSubmarine(subName, GameSession::AvailableSubmarine);
    } catch (std::runtime_error const& e) {
        displayError(e.what());
    }
}

void GameSessionEditor::on_removeAvailableSubsButton_clicked() {
}

void GameSessionEditor::openFile() {
    if (!openedFilePath.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText(tr("Loading another save file will discard your changes."));
        msgBox.setInformativeText(tr("Do you want to proceed?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int ret = msgBox.exec();
        if (ret != QMessageBox::Yes)
            return;
    }
    QChar separator = QDir::separator();
    // save location, taken from:
    // https://github.com/Regalis11/Barotrauma/blob/0002ad2c501a1a8df323b52edfc82a78d0afc6bc/Barotrauma/BarotraumaShared/SharedSource/Utils/SaveUtil.cs#L29
    QString save_directory =
            QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + separator +
            "Daedalic Entertainment GmbH" + separator +
            "Barotrauma";

    QString filePath = QFileDialog::getOpenFileName(
                this,
                tr("Open Saved Game"),
                save_directory,
                tr("Savegame file (*.save)")
    );
    if (filePath == "")
        return;
    // extract save file to workspace
    try {
        SaveUtil::decompressToDirectory(filePath, workspacePath);
    } catch (std::runtime_error const& e){
        displayError(e.what());
        emit sessionLoaded(false);
        openedFilePath = QString();
        return;
    }

    // process extracted data
    processSessionFiles(workspacePath);

    // save the edited file path on success
    openedFilePath = filePath;
    QLabel* labelFilename = findChild<QLabel*>("label_filename");
    labelFilename->setText(filePath);

    // inform other widgets that the session was successfully loaded
    // so that they can be enabled for editing
    emit sessionLoaded(true);
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
