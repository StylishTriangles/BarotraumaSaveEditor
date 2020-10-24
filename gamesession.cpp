#include "gamesession.h"

#include <QDebug>
#include <QFile>

#include <stdexcept>

static const QString availableSubsTagName = "AvailableSubs";
static const QString ownedSubsTagName = "ownedsubmarines";
static const QString gameSessionTagName = "Gamesession";

// from
// https://github.com/Regalis11/Barotrauma/blob/4978af3d602730de2e2742af8541ef43b227efe9/Barotrauma/BarotraumaShared/SharedSource/GameSession/GameModes/GameModePreset.cs#L11
static const QStringList gameModes{
    "SinglePlayerCampaign",
    "MultiPlayerCampaign",
    "Tutorial",
    "Mission",
    "TestMode",
    "Sandbox",
    "DevSandbox"
};

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
bool GameSession::addSubmarine(const QString &name, SubmarineType type) {
    if (containsSubmarine(name, type))
        return false;
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
    return true;
}

/*
 * @returns: true when submarine was removed successfully
 */
bool GameSession::removeSubmarine(const QString &name, SubmarineType type) {
    QDomNodeList nodeList;
    if (type == AvailableSubmarine)
        nodeList = xmlTree.elementsByTagName(availableSubsTagName);
    else
        nodeList = xmlTree.elementsByTagName(ownedSubsTagName);
    if (nodeList.isEmpty())
        return false;
    QDomNodeList subsList = nodeList.at(0).childNodes();
    for (int i = 0; i < subsList.size(); i++) {
        QDomNode subNode = subsList.item(i);
        QDomElement subElem = subNode.toElement();
        if (subElem.attribute("name") == name) {
            nodeList.at(0).removeChild(subNode);
            return true;
        }
    }
    return false;
}

/* Check if game session contains submarine with name "name"
 * More specifically, if a tag "sub" with attribute name=[name] exists
 * or submarine is the one currently in use
 * @returns: true when submarine is found in game session
 */
bool GameSession::containsSubmarine(const QString &name) {
    // currently used submarine is the one searched
    if (currentSubmarine() == name)
        return true;
    QDomNodeList subNodes = xmlTree.elementsByTagName("sub");
    for (int i = 0; i < subNodes.size(); i++) {
        QDomElement e = subNodes.item(i).toElement();
        if (e.attribute("name") == name) {
            return true;
        }
    }
    return false;
}

/* Check if game session contains submarine with specified name and type
 */
bool GameSession::containsSubmarine(QString const& name, SubmarineType type) {
    QStringList subNames = submarinesList(type);
    return subNames.contains(name);
}

/*
 * @returns: The submarine currently in use
 */
QString GameSession::currentSubmarine() {
    QDomNodeList nodeList = xmlTree.elementsByTagName(gameSessionTagName);
    QDomElement gsElem = nodeList.at(0).toElement();
    return gsElem.attribute("submarine");
}

// Get a list of submarines that match the specified type
// @returns: A list of submarine names
QStringList GameSession::submarinesList(SubmarineType type) const{
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

qint64 GameSession::getMoney() {
    for (QString const& mode: gameModes) {
        QDomNodeList nodeList = xmlTree.elementsByTagName(mode);
        if (nodeList.size() != 0) {
            return nodeList.at(0).toElement().attribute("money").toLongLong();
        }
    }
    return 0;
}

bool GameSession::setMoney(qint64 amount) {
    for (QString const& mode: gameModes) {
        QDomNodeList nodeList = xmlTree.elementsByTagName(mode);
        if (nodeList.size() != 0) {
            QDomElement elem = nodeList.at(0).toElement();
            elem.setAttribute("money", amount);
            return true;
        }
    }
    return false;
}
