#include <cassert>
#include <cmath>
#include <algorithm>

#include "OrderBook.hxx"
#include "Common.hxx"

using std::rbegin;
using std::rend;

template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin (reversion_wrapper<T> w) { return rbegin(w.iterable); }

template <typename T>
auto end (reversion_wrapper<T> w) { return rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse (T&& iterable) { return { iterable }; }
        
namespace trading
{
	namespace 
	{
	}

    OrderBook::OrderBook(const double _tickSize):
		tickSize(_tickSize)
    {
        if (tickSize <= 0.0)
        {
            LOG_AND_THROW("Tick size must be positive, but is " << tickSize);
        }
    }

	OrderBook::Fills OrderBook::add(LimitOrderPtr order)
	{
        if (openOrders.find(order->id) != openOrders.cend())
        {
            LOG_AND_THROW("Order with id=" << order->id << " already exists");
        }
        if (cancelledOrders.find(order->id) != cancelledOrders.cend())
        {
            LOG_AND_THROW("Order with id=" << order->id << " was already cancelled, cannot add again");
        }
        if (fullyFilledOrders.find(order->id) != fullyFilledOrders.cend())
        {
            LOG_AND_THROW("Order with id=" << order->id << " was already fully filled, cannot add again");
        }
        validateSide(order->side);
        validatePrice(order->price);
        validateQuantity(order->quantity);

		auto& otherSide = getSide(order->side == Side::Buy ? Side::Sell : Side::Buy);
		bool finished = false;
		Fills fills;
		std::list<int> ordersToRemove;
		for (auto item = otherSide.begin(); !finished && item != otherSide.end(); ++item) {
			for (auto& otherOrder: item->second) {
				if (!order->fullyFilled() && otherOrder->canCross(*order)) {
					auto fillQty = std::min(order->leaves(), otherOrder->leaves());
					fills.push_back(Fill { otherOrder->price, fillQty });
					order->addFill(fillQty);
					otherOrder->addFill(fillQty);
					if (otherOrder->fullyFilled())
					{
						ordersToRemove.push_back(otherOrder->id);
						fullyFilledOrders[otherOrder->id] = otherOrder;
					}
				}
			}
		}

		for (const auto id: ordersToRemove)
		{
			remove(id, /* flagCancelled */ false);
		}

		if (!order->fullyFilled())
		{
			insert(order);
		}
		else 
		{
			fullyFilledOrders[order->id] = order;
		}

		return fills;
	}

    void OrderBook::insert(LimitOrderPtr order)
    {
        const auto levelPrice = priceToLevel(order->price);
        auto& level = getSide(order->side)[levelPrice];
        level.push_back(order);
        openOrders[order->id] = order;
    }

    void OrderBook::amend(const int id, const int quantity)
    {
        validateQuantity(quantity);
        auto iOrder = openOrders.find(id);
        if (iOrder == openOrders.end())
        {
            LOG_AND_THROW("Order with id=" << id << " doesn't exists");
        }
        auto& order = iOrder->second;
		if (order->quantity != quantity) 
		{
			if (order->quantity < quantity)
			{
				order->quantity = quantity;
				remove(id, /*flagCancelled*/ false);
				insert(order);
			}
			else 
			{
				if (quantity <= order->filledQty)
				{
					LOG_AND_THROW("Cannot amend to below the filled level");
				}
		        order->quantity = quantity;
			}
		}
    }

    void OrderBook::cancel(const int id)
    {
		remove(id, /* flagCancelled */ true);
	}

    void OrderBook::remove(const int id, bool flagCancelled)
    {
        auto iOrder = openOrders.find(id);
        if (iOrder == openOrders.end())
        {
            LOG_AND_THROW("Order with id=" << id << " doesn't exists");
        }
        auto order = iOrder->second;
        const auto levelPrice = priceToLevel(order->price);
        auto& level = getSide(order->side)[levelPrice];
		auto iLevel = std::find_if(level.begin(), level.end(), [id](const auto& o){return o->id == id;});
		if (iLevel != level.end())
		{
        	level.erase(iLevel);
		}

        if (level.empty())
        {
            getSide(order->side).erase(levelPrice);
        }

        openOrders.erase(id);

		if (flagCancelled)
		{
			order->isCancelled = true;
			cancelledOrders[id] = order;
		}
    }

    double OrderBook::priceAt(const Side side, const int l) const
    {
        const auto& level = getLevel(side, l);
        if (level.empty())
        {
            LOG_AND_THROW("The level is unexpectedly empty");
        }
        return level.front()->price;
    }

    int OrderBook::sizeAt(const Side side, const int l) const
    {
        const auto& level = getLevel(side, l);
        int res = 0;
        for (const auto& order: level)
        {
            res += order->leaves();
        }
        return res;
    }

    void OrderBook::validatePrice(const double price) const
    {
        if (price <= 0.0)
        {
            LOG_AND_THROW("Price must be positive, " << price << " given");
        }
        if (!essentiallyEqual(price / tickSize, priceToLevel(price)))
        {
            LOG_AND_THROW("Price must be of given tick size " << tickSize << ", but it's not: " << price);
        }
    }

    void OrderBook::validateQuantity(const int quantity)
    {
        if (quantity <= 0)
        {
            LOG_AND_THROW("Quantity must be positive, " << quantity << " given");
        }
    }

    void OrderBook::validateSide(const Side side)
    {
        switch (side)
        {
            case Side::Buy:
            case Side::Sell:
                return;

            default:
                LOG_AND_THROW("Side must be Buy or Sell, " << side << " given");
        }
    }

    OrderBook::BookSide& OrderBook::getSide(const Side side)
    {
        const auto& res = const_cast<const OrderBook*>(this)->getSide(side);
        return const_cast<BookSide&>(res);
    }

    const OrderBook::BookSide& OrderBook::getSide(const Side side) const
    {
        switch (side)
        {
			case Side::Buy:
                return sides[0];

            case Side::Sell:
                return sides[1];

            default:
                LOG_AND_THROW("Side must be Buy or Sell, " << side << " given");
        }
    }

    int OrderBook::priceToLevel(const double price) const
    {
        return static_cast<int>(std::round(price / tickSize));
    }

    const OrderBook::PriceLevel& OrderBook::getLevel(const Side side, const int level) const
    {
        if (level < 0)
        {
            LOG_AND_THROW("Level must be non-negative, but " << level << " given");
        }

        auto count = level;

        switch (side)
        {
            case Side::Buy:
                for (const auto& item: reverse(getSide(side)))
                {
                    if (count == 0)
                    {
                        return item.second;
                    }
                    --count;
                }
                break;

            case Side::Sell:
                for (const auto& item: getSide(side))
                {
                    if (count == 0)
                    {
                        return item.second;
                    }
                    --count;
                }
                break;

            default:
                LOG_AND_THROW("Side must be Buy or Sell, " << side << " given");
        }

        LOG_AND_THROW("No such level in the book: " << level << " on side " << side);
    }


	QueryResult OrderBook::query(int id) const
	{
        const auto iOrder = openOrders.find(id);
        if (iOrder == openOrders.cend())
        {
			const auto iFilledOrder = fullyFilledOrders.find(id);
			if (iFilledOrder == fullyFilledOrders.cend()) 
			{
				const auto iCancelledOrder = cancelledOrders.find(id);
				if (iCancelledOrder == cancelledOrders.cend())
				{
					LOG_AND_THROW("Order with id=" << id << " doesn't exists");
				}

				return QueryResult { iCancelledOrder->second, -1 };
			}
			else 
			{
				return QueryResult { iFilledOrder->second, -1 };
			}
        }

        const auto order = iOrder->second;
        const auto levelPrice = priceToLevel(order->price);
		const auto& side = getSide(order->side);
		const auto iLevel = side.find(levelPrice);
		if (iLevel == side.cend())
		{
			LOG_AND_THROW("Order with id=" << id << " has no price level");
		}
        const auto& level = iLevel->second;
		int position = 0;
		for (const auto& o: level) 
		{
			if (o->id == id)
				break;
			++position;
		}
		return QueryResult { order, position };
	}
}
