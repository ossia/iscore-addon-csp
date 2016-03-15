#pragma once

#include <QObject>
#include <QVector>
#include <iscore/tools/SettableIdentifier.hpp>

namespace Scenario
{
    class TimeNodeModel;
    class ConstraintModel;
}

namespace CSP {

class Branch final : public QObject
{
        Q_OBJECT
    public:
        Branch(QObject* parent);

        Branch(const Branch& source, QObject* parent);

        const QVector<Id<Scenario::TimeNodeModel>>& timenodes() {return m_timenodes;}
        const QVector<Id<Scenario::ConstraintModel>>& constraints() {return m_timeRelations;}
        void addTimenode(const Id<Scenario::TimeNodeModel>&);
        void addTimeRelation(const Id<Scenario::ConstraintModel>&);

        int indexOfTimenode(const Id<Scenario::TimeNodeModel>& ) const;

    private:
        QVector<Id<Scenario::TimeNodeModel>> m_timenodes;
        QVector<Id<Scenario::ConstraintModel>> m_timeRelations;
};

}
