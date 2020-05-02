#pragma once

#include <unordered_map>
#include <map>
#include <list>
#include <array>

#include "LimitOrder.hxx"

namespace trading
{
	struct Fill
	{
		const double filledPrice;
		const int filledQty;
	};

	struct QueryResult
	{
		const LimitOrderPtr order;
		const int position;
	};

    class OrderBook
    {
    public:
		using Fills = std::list<Fill>;

        OrderBook(const double tickSize);

        Fills add(LimitOrderPtr order);

        void cancel(const int id);

        void amend(const int id, const int quantity);

        double priceAt(const Side side, const int level) const;

        int sizeAt(const Side side, const int level) const;

		QueryResult query(int id) const;

    private:
        using PriceLevel = std::list<LimitOrderPtr>;

        // map is ordered by the key
        using BookSide = std::map<int, PriceLevel>;

		const double tickSize;

        std::array<BookSide, 2> sides;
		std::unordered_map<int, LimitOrderPtr> openOrders;
		std::unordered_map<int, LimitOrderPtr> cancelledOrders;
		std::unordered_map<int, LimitOrderPtr> fullyFilledOrders;

        void insert(LimitOrderPtr order);

        void validatePrice(const double price) const;

        static void validateSide(const Side side);

        static void validateQuantity(const int quantity);

        int priceToLevel(const double price) const;

        const PriceLevel& getLevel(const Side side, int level) const;

        BookSide& getSide(const Side side);

        const BookSide& getSide(const Side side) const;

        void remove(const int id, bool flagCancelled);
    };
}
