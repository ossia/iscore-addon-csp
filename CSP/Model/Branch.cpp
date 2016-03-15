#include "Branch.hpp"

#include <Scenario/Document/TimeNode/TimeNodeModel.hpp>
#include <Scenario/Document/Constraint/ConstraintModel.hpp>

namespace CSP
{
Branch::Branch(QObject* parent):
    QObject{parent}
{

}

Branch::Branch(const Branch& source, QObject* parent):
    Branch{parent}
{
    m_timenodes = source.m_timenodes;
    m_timeRelations = source.m_timeRelations;
}

void Branch::addTimenode(const Id<Scenario::TimeNodeModel>& tn)
{
    m_timenodes.push_back(tn);
}

void Branch::addTimeRelation(const Id<Scenario::ConstraintModel>& cstr)
{
    m_timeRelations.push_back(cstr);
}

int Branch::indexOfTimenode(const Id<Scenario::TimeNodeModel>& tn) const
{
    return m_timenodes.indexOf(tn);
}

}
