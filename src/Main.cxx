#include <iostream>

#include "Common.hxx"
#include "OrderBook.hxx"
#include "CommandProcessor.hxx"


int main(int /*argc*/, const char** /*argv*/)
{
	using namespace trading;

	std::string cmd;

	OrderBook book(0.05);
	CommandProcessor processor(book, std::cout);

	while (std::getline(std::cin, cmd))
	{
		//std::cout << cmd << std::endl;
		try 
		{
			processor.handle(cmd);
		}
		catch (const TradingError&)
		{} // we ignore this error here, since it means the command can't be processed, so we did nothing
	}

	return 0;
}
