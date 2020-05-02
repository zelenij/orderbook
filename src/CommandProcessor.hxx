#pragma once

#include <iostream>
#include <string>

#include "OrderBook.hxx"

namespace trading
{
	class CommandProcessor
	{
	public:
		CommandProcessor(OrderBook& _book, std::ostream& _out);

		void handle(const std::string& cmd);

	private:
		OrderBook& book;
		std::ostream& out;
	};
}
