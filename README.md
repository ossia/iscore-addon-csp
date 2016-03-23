# iscore-addon-csp
CSP plug-in for i-score

i-score is an interactive intermedia sequencer, available on  https://github.com/OSSIA/i-score or http://i-score.org/.

This plug-in is about computing how a change in a part of a temporal structure of a scenario impact the entire scenario. 
We separate two cases :
 * edition
 * execution
 
## Edition
On edition mode, user may want to change duration of a constraint. So the end timenode of this constraint is translated, and so on. But what if others constraints ended on a potentially translated timenode ?

Many strategies exist, the CSP-addon implement some of them. As exemple :
 * "hard bounded" strategy : default duration can be changed but min or max value can not be updated by the algorithm (so rigid constraint cannot be resize)
 * "soft bounded" strategy : first do as "hard bounded" but min can be decrease and max increase when no other solutions.
 * "flexible" strategy : keep the same min/max relatively to default (the entire interval move when resizing default duration). All next timenodes are translated, resizing their previous constraints.

## Execution
The semantic of the model says that a timenode has only one date. So it means that the previous constraints have to end on the same time. But what if there were interaction and flexible time before ?

User give values for min and max but some of this values may not be reachable. Exemple : a scenario with two constraints, one rigid the other flexible, ending on the same timenode. The flexible one seems to be actually rigid, and we may want the CSP to do the update. But interaction is powerful and the rigid constraint can be disable at execution time and then the flexible one can actually be flexible ...

So what should CSP do :
 * find values that may be unreacheable and warn user,
 * fix problem if execution is forced (many strategies),
 * compute again each time something happen (timenode triggered : real date and maybe some things disabled)

## Using i-score without the CSP
It's possible, that's why it is just a plugin and not an integrated component. But displacement is provided with only one strategy (a flexible one). And the execution doesn't respect the semantic of the model : each timenode wait for all its previous constraints before beeing triggered

## More
Scientific documents, more explicit, exist. Differents strategies are explained, algo are described. 

Ask developers to obtain them.

CAUTION : contains a lot of mathematical stuff. You are averted.
