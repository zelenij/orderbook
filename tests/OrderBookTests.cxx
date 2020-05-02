#include <atomic>
#include <gtest/gtest.h>

#include "Common.hxx"
#include "OrderBook.hxx"
#include "CommandProcessor.hxx"

using namespace trading;

namespace
{
    static std::atomic<int> g_id(0);

    auto makeOrder(const Side side, const double price, const int quantity)
    {
        return std::make_shared<LimitOrder>(LimitOrder{++g_id, side, price, quantity, 0 });
    }

    auto buy(const double price, const int quantity = 10)
    {
        return makeOrder(Side::Buy, price, quantity);
    }

    auto sell(const double price, const int quantity = 10)
    {
        return makeOrder(Side::Sell, price, quantity);
    }
}


TEST(OrderBookTest, create_order_book)
{
    ASSERT_NO_THROW(OrderBook(0.1));
    ASSERT_NO_THROW(OrderBook(0.5));
    ASSERT_NO_THROW(OrderBook(0.01));
    ASSERT_NO_THROW(OrderBook(1));
    ASSERT_NO_THROW(OrderBook(6));

    ASSERT_THROW(OrderBook(0), TradingError);
    ASSERT_THROW(OrderBook(-1), TradingError);
    ASSERT_THROW(OrderBook(-10), TradingError);
}

TEST(OrderBookTest, insert_into_order_book)
{
    OrderBook book(0.1);

    auto order1 = buy(2.0);
    auto order2 = sell(2.0);
    book.add(order1);
    const auto&& fills = book.add(order2);
	ASSERT_EQ(1, fills.size());
	ASSERT_THROW(book.sizeAt(Side::Buy, 0), TradingError);
	ASSERT_THROW(book.sizeAt(Side::Sell, 0), TradingError);

    ASSERT_THROW(book.add(order1), TradingError);
    ASSERT_THROW(book.add(order2), TradingError);
    ASSERT_THROW(book.add(buy(0.0)), TradingError);
    ASSERT_THROW(book.add(buy(1.0, -1)), TradingError);

	auto result = book.query(order1->id);
	ASSERT_EQ("filled", result.order->status());
	ASSERT_EQ("filled", book.query(order2->id).order->status());

    order1->side = order2->side;
    ASSERT_THROW(book.add(order1), TradingError);
    ASSERT_THROW(book.add(order2), TradingError);
}

TEST(OrderBookTest, fills_in_order_book)
{
    OrderBook book(0.1);
	auto o3 = buy(10.0, 100);
	auto o4 = sell(10, 250);
	ASSERT_EQ(0, book.add(o3).size());
	ASSERT_EQ(100, book.sizeAt(Side::Buy, 0));
	ASSERT_THROW(book.sizeAt(Side::Sell, 0), TradingError);
	auto&& fills = book.add(o4);
	ASSERT_EQ(1, fills.size());
	ASSERT_EQ(100, fills.front().filledQty);
	ASSERT_EQ(150, book.sizeAt(Side::Sell, 0));
	ASSERT_THROW(book.sizeAt(Side::Buy, 0), TradingError);

	auto result = book.query(o4->id);
	ASSERT_EQ("partial", result.order->status());
	ASSERT_EQ(100, result.order->filledQty);
	ASSERT_EQ(150, result.order->leaves());
}

TEST(OrderBookTest, fills_in_order_book1)
{
    OrderBook book(0.1);
	auto o3 = buy(10.0, 100);
	auto o4 = sell(10, 250);
	ASSERT_EQ(0, book.add(o4).size());
	auto&& fills = book.add(o3);
	ASSERT_EQ(1, fills.size());
	ASSERT_EQ(100, fills.front().filledQty);
	ASSERT_EQ(150, book.sizeAt(Side::Sell, 0));
	ASSERT_THROW(book.sizeAt(Side::Buy, 0), TradingError);

	auto result = book.query(o4->id);
	ASSERT_EQ("partial", result.order->status());
	ASSERT_EQ(100, result.order->filledQty);
	ASSERT_EQ(150, result.order->leaves());

	auto result1 = book.query(o3->id);
	ASSERT_EQ("filled", result1.order->status());
}

TEST(OrderBookTest, tick_size_into_order_book)
{
    OrderBook book(0.2);
    book.add(buy(1.0));
    book.add(buy(1.2));
    book.add(buy(1.4));
    book.add(sell(1.4));
    ASSERT_THROW(book.add(buy(1.1)), TradingError);
    ASSERT_THROW(book.add(sell(1.1)), TradingError);
    ASSERT_THROW(book.add(sell(3.5)), TradingError);
    ASSERT_THROW(book.add(sell(-1)), TradingError);
    ASSERT_THROW(book.add(sell(0)), TradingError);
}

TEST(OrderBookTest, cancel_from_order_book)
{
    OrderBook book(0.5);

    auto order1 = buy(2.0);
    auto order2 = sell(2.5);
    book.add(order1);
    book.add(order2);

    book.cancel(order1->id);
    book.cancel(order2->id);
    ASSERT_THROW(book.cancel(order1->id), TradingError);
    ASSERT_THROW(book.cancel(order2->id), TradingError);
    ASSERT_THROW(book.add(order1), TradingError);
    ASSERT_THROW(book.add(order2), TradingError);
}

TEST(OrderBookTest, modify_in_order_book_and_query)
{
    OrderBook book(0.5);

    auto order1 = buy(2.0, 20);
    auto order2 = sell(2.5, 30);
    book.add(order1);
	book.add(buy(order1->price));
	book.add(buy(order1->price));
	book.add(buy(order1->price));
	book.add(buy(order1->price));
    book.add(order2);

    book.amend(order1->id, 50);
    ASSERT_THROW(book.amend(order1->id, 0), TradingError);
    ASSERT_THROW(book.amend(order1->id, -10), TradingError);
    book.amend(order2->id, 60);

	const auto result1 = book.query(order1->id);
	const auto result2 = book.query(order2->id);
    ASSERT_EQ(result1.order->quantity, 50);
    ASSERT_EQ(result1.position, 4);
    ASSERT_EQ(result2.order->quantity, 60);
    ASSERT_EQ(result2.position, 0);
}

TEST(OrderBookTest, data_at_level_of_order_book)
{
    OrderBook book(0.5);
    ASSERT_THROW(book.priceAt(Side::Buy, 0), TradingError);

    book.add(buy(20.0, 5));
    book.add(buy(20.0, 15));
    book.add(buy(20.0, 1));
    book.add(buy(19.0, 5));
    book.add(buy(19.0, 5));
    book.add(buy(18.0, 5));
    book.add(buy(17.0, 5));
    book.add(buy(17.0, 5));
    book.add(buy(16.0, 5));
    book.add(buy(15.0, 5));

    ASSERT_THROW(book.priceAt(Side::Sell, 0), TradingError);

    book.add(sell(21.0, 5));
    book.add(sell(21.0, 15));
    book.add(sell(21.0, 1));
    book.add(sell(22.0, 5));
    book.add(sell(22.0, 5));
    book.add(sell(23.0, 5));
    book.add(sell(23.0, 5));
    book.add(sell(23.0, 5));
    book.add(sell(24.0, 5));
    book.add(sell(24.0, 5));

    ASSERT_EQ(book.priceAt(Side::Buy, 0), 20.0);
    ASSERT_EQ(book.priceAt(Side::Buy, 1), 19.0);
    ASSERT_EQ(book.priceAt(Side::Buy, 2), 18.0);
    ASSERT_EQ(book.sizeAt(Side::Buy, 0), 21);
    ASSERT_EQ(book.sizeAt(Side::Buy, 1), 10);

    ASSERT_EQ(book.priceAt(Side::Sell, 0), 21.0);
    ASSERT_EQ(book.priceAt(Side::Sell, 1), 22.0);
    ASSERT_EQ(book.priceAt(Side::Sell, 2), 23.0);
    ASSERT_EQ(book.sizeAt(Side::Sell, 0), 21);
    ASSERT_EQ(book.sizeAt(Side::Sell, 2), 15);

    ASSERT_THROW(book.priceAt(Side::Sell, -1), TradingError);
    ASSERT_THROW(book.priceAt(Side::Sell, 10), TradingError);
    ASSERT_THROW(book.sizeAt(Side::Buy, 10), TradingError);
}


TEST(CommandProcessorTest, process_standard_commands)
{
	OrderBook book(0.05);
	CommandProcessor processor(book, std::cout);

	processor.handle("order 1001 buy 100 12.30");
	processor.handle("q order 1001");
	processor.handle("q level bid 0");
	ASSERT_THROW(processor.handle("q level ask 0"), TradingError);
}
