#include "gamesession.h"

#include <QDebug>
#include <QFile>

#include <stdexcept>

static const QString availableSubsTagName = "AvailableSubs";
static const QString ownedSubsTagName = "ownedsubmarines";

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

// Load game session from XML file located in xmlPath
bool GameSession::fromXML(const QString &xmlPath) {
    this->xmlPath = xmlPath;
    QFile file(xmlPath);
    file.open(QFile::ReadOnly);
    return this->xmlTree.setContent(file.readAll());
}

/* Add submarine to game session
 * @param name: Submarine name (without .sub)
 * @param type: Submarine type
 */
void GameSession::addSubmarine(const QString &name, SubmarineType type) {
    QDomNodeList nodeList;
    if (type == AvailableSubmarine)
        nodeList = xmlTree.elementsByTagName(availableSubsTagName);
    else
        nodeList = xmlTree.elementsByTagName(ownedSubsTagName);
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

// Get a list of submarines that match the specified type
// @returns: A list of submarine names
QStringList GameSession::getSubmarines(SubmarineType type) const{
    QDomNodeList nodeList;
    QStringList names;
    if (type == AvailableSubmarine)
        nodeList = xmlTree.elementsByTagName(availableSubsTagName);
    else
        nodeList = xmlTree.elementsByTagName(ownedSubsTagName);

    if (nodeList.isEmpty())
        return names;
    QDomNodeList subsList = nodeList.at(0).childNodes();
    for (int i = 0; i < subsList.size(); i++) {
        QDomNode sub = subsList.item(i); // sub node
        if (sub.nodeName().toLower() == "sub") {
            QDomElement elSub = sub.toElement();
            names.push_back(elSub.attribute("name"));
        } else {
            qDebug() << "Invalid sub tag name" << sub.nodeName();
        }
    }
    return names;
}
