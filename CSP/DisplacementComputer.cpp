#include <CSP/Model/Scenario.hpp>
#include <CSP/Model/TimeNode.hpp>
#include <CSP/Model/TimeRelation.hpp>
#include <Scenario/Process/Algorithms/Accessors.hpp>

#include <CSP/DisplacementComputer.hpp>

namespace CSP {

void compute(
	Scenario::ScenarioModel& scenario,
	const QVector<Id<Scenario::TimeNodeModel>>& draggedElements,
	const TimeValue& deltaTime,
	Scenario::ElementsProperties& elementsProperties)
{
    // get the csp scenario

    if(ScenarioModel* cspScenario = scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
	auto& solver = cspScenario->getSolver();

	// get the corresponding CSP elements and start editing vars
	for(auto curDraggedTimeNodeId : draggedElements)
	{
	    auto curDraggedCspTimeNode = cspScenario->m_timeNodes[curDraggedTimeNodeId];

	    // get the initial value
	    TimeValue initialDate;
	    if(elementsProperties.timenodes.contains(curDraggedTimeNodeId))
	    {
        initialDate = elementsProperties.timenodes[curDraggedTimeNodeId].oldDate;
	    }else
	    {
        initialDate = *(curDraggedCspTimeNode->getIscoreDate());
	    }

	    //weight
	    solver.addEditVariable(curDraggedCspTimeNode->getDate(),  kiwi::strength::strong + 1.0);

	    // suggest their new values
	    auto newDate = initialDate.msec() + deltaTime.msec();

	    try
	    {
        solver.suggestValue(curDraggedCspTimeNode->getDate(), newDate);
	    }catch(...)
	    {
        qDebug("lalalalalala j'ai même pas planté");
	    }
	}

	// solve system
	solver.updateVariables();

	// release edit vars
	for(auto curDraggedTimeNodeId : draggedElements)
	{
	    solver.removeEditVariable(cspScenario->m_timeNodes[curDraggedTimeNodeId]->getDate());
	}

	// look for changes // TODO : maybe find a more efficient way of doing that

	// - in timenodes :
	QHashIterator<Id<Scenario::TimeNodeModel>, TimeNodeModel*> timeNodeIterator(cspScenario->m_timeNodes);
	while (timeNodeIterator.hasNext())
	{
	    timeNodeIterator.next();

	    auto curTimeNodeId = timeNodeIterator.key();
	    auto curCspTimenode = timeNodeIterator.value();

	    // if updated TODO : if updated => curCspTimenode->dateChanged()
	    if(true)
	    {
		// if timenode NOT already in element properties, create new element properties and set the old date
		if(! elementsProperties.timenodes.contains(curTimeNodeId))
		{
		    elementsProperties.timenodes[curTimeNodeId] = Scenario::TimenodeProperties{};
		    elementsProperties.timenodes[curTimeNodeId].oldDate = *(curCspTimenode->getIscoreDate());
		}

		// put the new date
		elementsProperties.timenodes[curTimeNodeId].newDate = TimeValue::fromMsecs(curCspTimenode->getDate().value());
	    }
	}

	// - in time relations :
	QHashIterator<Id<Scenario::ConstraintModel>, TimeRelationModel*> timeRelationIterator(cspScenario->m_timeRelations);
	while(timeRelationIterator.hasNext())
	{
	    timeRelationIterator.next();

	    const auto& curTimeRelationId = timeRelationIterator.key();
	    auto& curCspTimerelation = timeRelationIterator.value();
	    const auto& curConstraint = scenario.constraint(curTimeRelationId);

	    // if osef TODO : if updated
	    if(true)
	    {
		// if timenode NOT already in element properties, create new element properties and set the old values
		if(! elementsProperties.constraints.contains(curTimeRelationId))
		{
		    elementsProperties.constraints[curTimeRelationId] = Scenario::ConstraintProperties{};
		    elementsProperties.constraints[curTimeRelationId].oldMin = curCspTimerelation->getIscoreMin();
		    elementsProperties.constraints[curTimeRelationId].oldMax = curCspTimerelation->getIscoreMax();

		    // Save the constraint display data START ----------------
		    QByteArray arr;
		    Visitor<Reader<DataStream>> jr{&arr};
		    jr.readFrom(curConstraint);

		    // Save for each view model of this constraint
		    // the identifier of the rack that was displayed
		    QMap<Id<Scenario::ConstraintViewModel>, Id<Scenario::RackModel>> map;
		    for(const Scenario::ConstraintViewModel* vm : curConstraint.viewModels())
		    {
                map[vm->id()] = vm->shownRack();
		    }

		    elementsProperties.constraints[curTimeRelationId].savedDisplay = {{iscore::IDocument::path(curConstraint), arr}, map};
		    // Save the constraint display data END ----------------
		}

		// put the new values
		/*
		qDebug() << "----       min : " << curCspTimerelation->m_min.value();
		qDebug() << "    min iscore : " << curCspTimerelation->m_iscoreMin->msec();

		qDebug() << "----       max : " << curCspTimerelation->m_max.value();
		qDebug() << "    max iscore : " << curCspTimerelation->m_iscoreMax->msec();
		*/

		elementsProperties.constraints[curTimeRelationId].newMin = TimeValue::fromMsecs(curCspTimerelation->getMin().value());
        if(!curCspTimerelation->getIscoreMax().isInfinite())
            elementsProperties.constraints[curTimeRelationId].newMax = TimeValue::fromMsecs(curCspTimerelation->getMax().value());
	    }
	}
    }else
    {
	std::runtime_error("No CSP scenario found for this model");
    }
}

void computeMin(Scenario::ScenarioModel& scenario, const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements, Scenario::ElementsProperties& elementsProperties)
{
    if(ScenarioModel* cspScenario = scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
        auto& solver = cspScenario->getSolver();
        QHashIterator<Id<Scenario::TimeNodeModel>, TimeNodeModel*> timeNodeIterator(cspScenario->m_timeNodes);
        auto tnIt = timeNodeIterator;
        while (timeNodeIterator.hasNext())
        {
            timeNodeIterator.next();

            auto& curTimeNodeId = timeNodeIterator.key();
            auto& curCspTimeNode = timeNodeIterator.value();

            if(!positionnedElements.contains(curTimeNodeId))
            {
                solver.addEditVariable(curCspTimeNode->getDate(),  kiwi::strength::strong + 1.0);
                solver.suggestValue(curCspTimeNode->getDate(), 0);
            }
        }
        solver.updateVariables();
        while(tnIt.hasNext())
        {
            tnIt.next();
            auto& curTimeNodeId = timeNodeIterator.key();
            auto& curCspTimeNode = timeNodeIterator.value();

            if(!positionnedElements.contains(curTimeNodeId))
                solver.removeEditVariable(curCspTimeNode->getDate());

            auto& iscore_tn = scenario.timeNode(curTimeNodeId);
            auto nextConstraints = Scenario::nextConstraints(iscore_tn, scenario);
            for(auto cstr : nextConstraints)
            {
                if(! elementsProperties.constraints.contains(cstr))
                {
                    auto curCstCstr = cspScenario->getTimeRelation(cstr);
                    elementsProperties.constraints[cstr] = Scenario::ConstraintProperties{};
                    elementsProperties.constraints[cstr].oldMin = curCstCstr->getIscoreMin();
                    elementsProperties.constraints[cstr].oldMax = curCstCstr->getIscoreMax();
                }
                auto& iscore_cstr = scenario.constraint(cstr);
                auto& prevTnId = Scenario::startTimeNode(iscore_cstr, scenario);
                auto prevCspTn = cspScenario->m_timeNodes.take(prevTnId.id());

                elementsProperties.constraints[cstr].newMin.setMSecs(curCspTimeNode->getDate().value() - prevCspTn->getDate().value());
            }
        }
    }
}

void computeMax(Scenario::ScenarioModel& scenario, const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements, Scenario::ElementsProperties& elementsProperties)
{

    if(ScenarioModel* cspScenario = scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
        auto& solver = cspScenario->getSolver();
        QHashIterator<Id<Scenario::TimeNodeModel>, TimeNodeModel*> timeNodeIterator(cspScenario->m_timeNodes);
        while (timeNodeIterator.hasNext())
        {
            timeNodeIterator.next();

            auto& curTimeNodeId = timeNodeIterator.key();
            auto& curCspTimeNode = timeNodeIterator.value();

            if(!positionnedElements.contains(curTimeNodeId))
            {
                solver.addEditVariable(curCspTimeNode->getDate(),  kiwi::strength::strong + 1.0);
                //solver.suggestValue(curCspTimeNode->getDate(), zero);
            }
        }
        solver.updateVariables();
    }
}

} // namespace CSP
