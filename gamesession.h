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

    // data loading/dumping

    void dumpXML();
    void dumpXML(QString const& xmlPath);
    bool fromXML(QString const& xmlPath);

    // submarine management

    bool addSubmarine(QString const& name, SubmarineType type);
    bool removeSubmarine(QString const& name, SubmarineType type);
    bool containsSubmarine(QString const& name);
    bool containsSubmarine(QString const& name, SubmarineType type);
    QString currentSubmarine();
    QStringList submarinesList(SubmarineType type) const;

    // general info

    qint64 getMoney();
    bool setMoney(qint64 amount);

private:
    QString xmlPath;
    QDomDocument xmlTree;
};

#endif // GAMESESSION_H
