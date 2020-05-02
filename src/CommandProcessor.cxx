#include <sstream>

#include "Common.hxx"
#include "CommandProcessor.hxx"

namespace trading
{
	namespace 
	{
		auto string2side(const std::string& sideStr)
		{
			if (sideStr == "buy" || sideStr == "bid")
				return Side::Buy;
			else if (sideStr == "sell" || sideStr == "ask")
				return Side::Sell;
			else
				LOG_AND_THROW("Unknown side " << sideStr);
		}
	}

	CommandProcessor::CommandProcessor(OrderBook& _book, std::ostream& _out): 
		book(_book),
		out(_out)
	{}

	void CommandProcessor::handle(const std::string& input)
	{
		std::istringstream in(input);
		std::string cmd;
		in >> cmd;

		if (cmd == "order") {
			int id;
			int quantity;
			std::string sideStr;
			double price;
			in >> id >> sideStr >> quantity >> price;
			const auto side = string2side(sideStr);
			auto order = std::make_shared<LimitOrder>(LimitOrder { id, side, price, quantity });
			const auto&& fills = book.add(order);
			for (const auto& fill: fills)
			{
				out << "Fill: " << fill.filledQty << "@" << fill.filledPrice << std::endl;
			}
		}
		else if (cmd == "amend") {
			int id;
			int quantity;
			in >> id >> quantity;
			book.amend(id, quantity);
		}
		else if (cmd == "cancel") {
			int id;
			in >> id;
			book.cancel(id);
		}
		else if (cmd == "q") {
			std::string subCmd;
			in >> subCmd;
			if (subCmd == "level") {
				std::string sideStr;
				int level;
				in >> sideStr >> level;
				const auto side = string2side(sideStr);
				auto price = book.priceAt(side, level);
				auto totalSize = book.sizeAt(side, level);
				out << sideStr << ", " << level << ", " << price << ", " << totalSize << std::endl;
			}
			else if (subCmd == "order") {
				int id;
				in >> id;
				auto&& result = book.query(id);
				const auto& order = *result.order;
				out << order.status() << ", leaves=" << order.leaves() << ", filled=" << order.filledQty
					<< ", position=" << result.position
					<< std::endl;
			}
		}
	}
}

