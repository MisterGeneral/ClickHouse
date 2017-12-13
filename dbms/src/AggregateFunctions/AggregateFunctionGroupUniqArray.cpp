#include <AggregateFunctions/AggregateFunctionFactory.h>
#include <AggregateFunctions/AggregateFunctionGroupUniqArray.h>
#include <AggregateFunctions/Helpers.h>

namespace DB
{

namespace
{

/// Substitute return type for Date and DateTime
class AggregateFunctionGroupUniqArrayDate : public AggregateFunctionGroupUniqArray<DataTypeDate::FieldType>
{
    DataTypePtr getReturnType() const override { return std::make_shared<DataTypeArray>(std::make_shared<DataTypeDate>()); }
};

class AggregateFunctionGroupUniqArrayDateTime : public AggregateFunctionGroupUniqArray<DataTypeDateTime::FieldType>
{
    DataTypePtr getReturnType() const override { return std::make_shared<DataTypeArray>(std::make_shared<DataTypeDateTime>()); }
};


static IAggregateFunction * createWithExtraTypes(const DataTypes & argument_types)
{
    const IDataType& argument_type = *argument_types[0];

         if (typeid_cast<const DataTypeDate *>(&argument_type)) return new AggregateFunctionGroupUniqArrayDate;
    else if (typeid_cast<const DataTypeDateTime *>(&argument_type)) return new AggregateFunctionGroupUniqArrayDateTime;
    else
    {
        IAggregateFunction* fun;

        /// Check that we can use plain version of AggreagteFunctionGroupUniqArrayGeneric
        if (argument_type.isValueUnambiguouslyRepresentedInContiguousMemoryRegion())
            fun = new AggreagteFunctionGroupUniqArrayGeneric<true>;
        else
            fun = new AggreagteFunctionGroupUniqArrayGeneric<false>;

        fun->setArguments(argument_types);
        return fun;
    }
}

AggregateFunctionPtr createAggregateFunctionGroupUniqArray(const std::string & name, const DataTypes & argument_types, const Array & /*parameters*/)
{
    if (argument_types.size() != 1)
        throw Exception("Incorrect number of arguments for aggregate function " + name,
            ErrorCodes::NUMBER_OF_ARGUMENTS_DOESNT_MATCH);

    AggregateFunctionPtr res(createWithNumericType<AggregateFunctionGroupUniqArray>(*argument_types[0]));

    if (!res)
        res = AggregateFunctionPtr(createWithExtraTypes(argument_types));

    if (!res)
        throw Exception("Illegal type " + argument_types[0]->getName() +
            " of argument for aggregate function " + name, ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT);

    return res;
}

}

void registerAggregateFunctionGroupUniqArray(AggregateFunctionFactory & factory)
{
    factory.registerFunction("groupUniqArray", createAggregateFunctionGroupUniqArray);
}

}
