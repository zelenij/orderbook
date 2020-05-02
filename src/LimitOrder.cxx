#include "Common.hxx"
#include "LimitOrder.hxx"

namespace trading
{
	std::ostream& operator<<(std::ostream& out, const Side side)
	{
		switch (side) {
			case Side::Buy:
				return out << "buy";
			case Side::Sell:
				return out << "sell";

			default:
				LOG_AND_THROW("Bad side");
		}
	}

	std::string LimitOrder::status() const
	{
		if (isCancelled) 
		{
			return "cancelled";
		}
		else if (fullyFilled()) 
		{
			return "filled";
		}
		else if (filledQty > 0) 
		{
			return "partial";
		}
		else 
		{
			return "open";
		}
	}

	int LimitOrder::leaves() const
	{
		if (isCancelled)
		{
			return 0;
		}
		return quantity - filledQty;
	}

	bool LimitOrder::fullyFilled() const
	{
		return quantity == filledQty;
	}

	void LimitOrder::addFill(int fillQty)
	{
		filledQty += fillQty;
	}

	bool LimitOrder::canCross(const LimitOrder& other) const
	{
		if (side == other.side)
			return false;

		switch (side) {
			case Side::Buy:
				return price >= other.price;
			case Side::Sell:
				return price <= other.price;

			default:
				LOG_AND_THROW("Bad side");
		}
	}
}
