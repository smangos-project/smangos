#ifndef DATABASE_TOOL_SPEC_SHAPE_H
#define DATABASE_TOOL_SPEC_SHAPE_H

#include <map>
#include <string>

class DoltLiteDatabase;

struct ComputeShapeResult
{
    // Keyed by "source\ttable\tcolumn", one entry per srv_*/vdbc_* column of `reference`.
    std::map<std::string, std::string> shapes;
};

// phase0-plan.md step 4.4: for every srv_*/vdbc_* column, a "rows=<N> distinct=<N> null=<N>"
// shape string, plus " const=<value>" when every non-null value is the same, and
// " formula=<name>:<matched>/<total>" for the first candidate formula that applies:
// spell_template's `effectBaseDice<N> = (effectDieSides<N> != 0)`, or -- for any "array slot"
// column (a name ending in digits, e.g. `effectRadiusIndex2`) -- the sibling slot of the same
// stem (`effectRadiusIndex1`, `effectRadiusIndex3`, ...) with the highest equality rate. Both
// formulas are recorded whatever the rate, like Counterpart.cpp's matches: a hint, not a
// decision.
ComputeShapeResult ComputeShape(DoltLiteDatabase& reference);

#endif
