#pragma once

#include <QObject>
#include <QVector>
#include <CSP/Model/Scenario.hpp>
#include <kiwi/kiwi.h>

namespace CSP
{
class ConstraintHolder : public QObject
{
public:

    using QObject::QObject;
    ConstraintHolder(kiwi::Solver& solver, QObject* parent)
        :QObject::QObject{parent},
          m_solver(solver)
    {}

    virtual ~ConstraintHolder()
    {
        removeAllConstraints();
    }

    void removeAllConstraints()
    {
        for(auto constraint : m_constraints)
        {
           m_solver.removeConstraint(*constraint);
           delete constraint;
        }

        removeStays();
    }

    void addStay(kiwi::Constraint* stay)
    {
        m_solver.addConstraint(*stay);
        m_stays.push_back(stay);
    }

    void removeStays()
    {
        for(auto stay : m_stays)
        {
           m_solver.removeConstraint(*stay);
           delete stay;
        }

        m_stays.clear();//important
    }

    void putConstraintInSolver(kiwi::Constraint* cstr)
    {
        m_solver.addConstraint(*cstr);
        m_constraints.push_back(cstr);
    }


protected:
    QVector<kiwi::Constraint*> m_constraints; // all constraints to put in solver
    QVector<kiwi::Constraint*> m_stays; // only updatable constraints

    kiwi::Solver& m_solver;
};
}
