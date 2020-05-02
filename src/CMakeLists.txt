set(ORDER_BOOK_LIB_SRC
    OrderBook.cxx
	LimitOrder.cxx
	CommandProcessor.cxx
	Common.cxx
)

set(ORDER_BOOK_SRC 
	Main.cxx
)

set(BOOK_EXE order_book)

add_library(${BOOK_LIB} SHARED ${ORDER_BOOK_LIB_SRC})

add_executable(${BOOK_EXE} ${ORDER_BOOK_SRC})
target_link_libraries(${BOOK_EXE} ${BOOK_LIB})
