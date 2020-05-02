#pragma once

#include <iostream>
#include <memory>

namespace trading
{
    enum class Side: char 
    {
        Buy = 'B',
        Sell = 'O'
    };

	std::ostream& operator<<(std::ostream& out, const Side side);

    struct LimitOrder
    {
        int id;
        Side side;
        double price;
        int quantity;
		int filledQty;
		bool isCancelled = false;

		std::string status() const;

		int leaves() const;

		bool fullyFilled() const;

		void addFill(int fillQty);

		bool canCross(const LimitOrder& other) const;
    };

	using LimitOrderPtr = std::shared_ptr<LimitOrder>;
}
