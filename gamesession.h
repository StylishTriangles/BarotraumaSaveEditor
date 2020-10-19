#ifndef GAMESESSION_H
#define GAMESESSION_H

#include <QString>
#include <QDomDocument>

class GameSession
{
public:
    GameSession() = default;
    GameSession(QString const& xmlPath);

    void dumpXML();
    void dumpXML(QString const& xmlPath);
    bool fromXML(QString const& xmlPath);
    void addSubmarine(QString const& name, bool available, bool owned);

private:
    QString xmlPath;
    QDomDocument xmlTree;
};

#endif // GAMESESSION_H
