#ifndef TESTS_QUEUE_TESTS_HPP
#define TESTS_QUEUE_TESTS_HPP

#include "gtest/gtest.h"
#include "queue.hpp"
#include "fmt/format.h"

TEST(sky_queue, enq_full) {
    sky::queue<int, 8> q;
    for (std::size_t i = 0; i < q.capacity(); ++i) {
        q.enq(1);
    }

    for (std::size_t i = 0; i < q.capacity(); ++i) {
        EXPECT_EQ(q.peek(i), 1);
    }
}

TEST(sky_queue, enq_extra) {
    sky::queue<int, 8> q;
    for (std::size_t i = 0; i < q.capacity(); ++i) {
        q.enq(1);
    }
    q.enq(2);

    //for (std::size_t i = 0; i < q.capacity(); ++i) {
    //    fmt::print("{}", q.peek(i));
    //    if (i < q.capacity() - 1) fmt::print(" ");
    //    else fmt::print("\n");
    //}

    for (std::size_t i = 0; i < q.capacity() - 1; ++i) {
        EXPECT_EQ(q.peek(i), 1);
    }
    EXPECT_EQ(q.peek_back(q.capacity() - 1), 2);
}

#endif  // !TESTS_QUEUE_TESTS_HPP

