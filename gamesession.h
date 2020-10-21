#ifndef GAMESESSION_H
#define GAMESESSION_H

#include <QString>
#include <QDomDocument>

class GameSession
{
public:
    enum SubmarineType {
        AvailableSubmarine,
        OwnedSubmarine
    };
    GameSession() = default;
    GameSession(QString const& xmlPath);

    void dumpXML();
    void dumpXML(QString const& xmlPath);
    bool fromXML(QString const& xmlPath);
    void addSubmarine(QString const& name, SubmarineType type);
    bool removeSubmarine(QString const& name, SubmarineType type);
    bool containsSubmarine(QString const& name);
    QString getCurrentSubmarine();
    QStringList getSubmarines(SubmarineType type) const;

private:
    QString xmlPath;
    QDomDocument xmlTree;
};

#endif // GAMESESSION_H
