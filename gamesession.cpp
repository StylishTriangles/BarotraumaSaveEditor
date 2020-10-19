#include "gamesession.h"

#include <QFile>

#include <stdexcept>

GameSession::GameSession(QString const& xmlPath)
{
    fromXML(xmlPath);
}

// dump XML to file it was read from
void GameSession::dumpXML() {
    dumpXML(xmlPath);
}

void GameSession::dumpXML(const QString &xmlPath) {
    QByteArray out = xmlTree.toByteArray(2);
    QFile file(xmlPath);
    file.open(QFile::WriteOnly | QFile::Truncate);
    file.write(out);
}

bool GameSession::fromXML(const QString &xmlPath) {
    this->xmlPath = xmlPath;
    QFile file(xmlPath);
    file.open(QFile::ReadOnly);
    return this->xmlTree.setContent(file.readAll());
}

void GameSession::addSubmarine(const QString &name, bool available, bool owned) {
    QDomNodeList nodeList;
    if (available == owned)
        throw std::runtime_error("Cannot make the submarine both available and owned");
    if (available)
        nodeList = xmlTree.elementsByTagName("AvailableSubs");
    else
        nodeList = xmlTree.elementsByTagName("ownedsubmarines");
    if (nodeList.length() > 1) {
        throw std::runtime_error("Could not add submarine - gamesession.xml "
                                 "contains too many <AvailableSubs> or <OwnedSubs tags");
    }
    if (nodeList.isEmpty()) {
        throw std::runtime_error("Could not add submarine - gamesession.xml "
                                 "doesn't have an <AvailableSubs> tag");
    }
    QDomNode subNode = xmlTree.createElement("sub");
    nodeList.at(0).appendChild(subNode);
    subNode.toElement().setAttribute("name", name);
}
