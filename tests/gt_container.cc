// SPDX-License-Identifier: CC-BY-SA-4.0
// Copyright 2015-2019 Peter A. Bigot

#include <gtest/gtest.h>

#include <pabigot/container.hpp>

using namespace pabigot::container;

namespace {

TEST(RRAdaptor, BasicPushPop)
{
  std::array<uint8_t, 4> data;
  rr_adaptor<> rrb{&data[0], data.max_size()};

  ASSERT_EQ(4U, rrb.max_size());

  ASSERT_EQ(0U, rrb.size());
  ASSERT_TRUE(rrb.empty());
  ASSERT_FALSE(rrb.full());

  ASSERT_FALSE(rrb.push(1));
  ASSERT_EQ(1U, rrb.size());
  ASSERT_FALSE(rrb.empty());
  ASSERT_FALSE(rrb.full());

  ASSERT_FALSE(rrb.push(2));
  ASSERT_EQ(2U, rrb.size());
  ASSERT_FALSE(rrb.empty());
  ASSERT_FALSE(rrb.full());

  ASSERT_FALSE(rrb.push(3));
  ASSERT_EQ(3U, rrb.size());
  ASSERT_FALSE(rrb.empty());
  ASSERT_FALSE(rrb.full());

  ASSERT_FALSE(rrb.push(4));
  ASSERT_EQ(4U, rrb.size());
  ASSERT_FALSE(rrb.empty());
  ASSERT_TRUE(rrb.full());

  /* Discards element 1 */
  ASSERT_TRUE(rrb.push(5));
  ASSERT_EQ(4U, rrb.size());
  ASSERT_FALSE(rrb.empty());
  ASSERT_TRUE(rrb.full());

  /* Drain it */
  ASSERT_EQ(2, rrb.pop());
  ASSERT_EQ(3U, rrb.size());
  ASSERT_FALSE(rrb.empty());
  ASSERT_FALSE(rrb.full());

  ASSERT_EQ(3, rrb.pop());
  ASSERT_EQ(2U, rrb.size());
  ASSERT_FALSE(rrb.empty());
  ASSERT_FALSE(rrb.full());

  ASSERT_EQ(4, rrb.pop());
  ASSERT_EQ(1U, rrb.size());
  ASSERT_FALSE(rrb.empty());
  ASSERT_FALSE(rrb.full());

  ASSERT_EQ(5, rrb.pop());
  ASSERT_EQ(0U, rrb.size());
  ASSERT_TRUE(rrb.empty());
  ASSERT_FALSE(rrb.full());

  ASSERT_EQ(0, rrb.pop());
  ASSERT_EQ(0U, rrb.size());
  ASSERT_TRUE(rrb.empty());
  ASSERT_FALSE(rrb.full());
}

TEST(RRAdaptor, EbbAndFlow)
{
  std::array<uint8_t, 4> data;
  rr_adaptor<> rrb{&data[0], data.max_size()};

  ASSERT_EQ(4U, rrb.max_size());

  ASSERT_EQ(0U, rrb.size());
  ASSERT_TRUE(rrb.empty());
  ASSERT_FALSE(rrb.full());

  ASSERT_FALSE(rrb.push(1));
  ASSERT_FALSE(rrb.empty());
  ASSERT_EQ(1U, rrb.size());

  ASSERT_FALSE(rrb.push(2));
  ASSERT_FALSE(rrb.empty());
  ASSERT_EQ(2U, rrb.size());

  ASSERT_EQ(1, rrb.pop());
  ASSERT_FALSE(rrb.empty());
  ASSERT_EQ(1U, rrb.size());

  ASSERT_EQ(2, rrb.pop());
  ASSERT_TRUE(rrb.empty());
  ASSERT_EQ(0U, rrb.size());

  /* Now empty, but had only filled half way.  Make sure the tail and head are
   * in sync. */
  ASSERT_FALSE(rrb.push(3));
  ASSERT_FALSE(rrb.empty());
  ASSERT_EQ(1U, rrb.size());
}

TEST(RRAdaptor, Clear)
{
  std::array<uint8_t, 4> data;
  rr_adaptor<> rrb{&data[0], data.max_size()};
  ASSERT_TRUE(rrb.empty());
  rrb.push(1);
  ASSERT_FALSE(rrb.empty());
  rrb.clear();
  ASSERT_TRUE(rrb.empty());
}

class ForwardChainFixture : public ::testing::Test {
protected:

  struct element {
    struct ref_next {
      using pointer_type = element *;
      pointer_type& operator() (element &m) noexcept
      {
        return m.next;
      }
    };
    using queue_type = forward_chain<element, ref_next>;

    element (int id) :
      id{id}
    { }

    int const id;
    int value{};
    queue_type::pointer_type next{queue_type::unlinked_ptr()};
  };

  void populate_queue ()
  {
    queue.clear();
    queue.link_back(e1);
    queue.link_back(e2);
    queue.link_back(e3);
  }

  element::queue_type queue;

  element e1{1};
  element e2{2};
  element e3{3};
};

TEST_F(ForwardChainFixture, Basics)
{
  ASSERT_TRUE(queue.empty());
  ASSERT_EQ(e1.id, 1);
  ASSERT_TRUE(queue.is_unlinked(e1));
  ASSERT_EQ(e2.id, 2);
  ASSERT_EQ(e3.id, 3);
}

TEST_F(ForwardChainFixture, Front)
{
  ASSERT_TRUE(queue.empty());
  ASSERT_EQ(nullptr, queue.front());
  ASSERT_EQ(nullptr, queue.back());

  ASSERT_TRUE(queue.is_unlinked(e1));

  queue.link_front(e1);
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e1, queue.back());
  ASSERT_FALSE(queue.is_unlinked(e1));

  ASSERT_EQ(&e1, queue.unlink_front());
  ASSERT_TRUE(queue.empty());
  ASSERT_EQ(nullptr, queue.front());
  ASSERT_EQ(nullptr, queue.back());
  ASSERT_TRUE(queue.is_unlinked(e1));

  ASSERT_EQ(nullptr, queue.unlink_front());

  queue.link_front(e2);
  queue.link_front(e1);
  ASSERT_EQ(queue.front(), &e1);
  ASSERT_EQ(queue.next(e1), &e2);
  ASSERT_EQ(queue.next(e2), nullptr);

  ASSERT_EQ(&e1, queue.unlink_front());
  ASSERT_TRUE(queue.is_unlinked(e1));

  ASSERT_EQ(queue.front(), &e2);
  ASSERT_EQ(&e2, queue.unlink_front());
  ASSERT_TRUE(queue.empty());
}

TEST_F(ForwardChainFixture, Back)
{
  ASSERT_TRUE(queue.empty());
  ASSERT_EQ(nullptr, queue.front());
  ASSERT_EQ(nullptr, queue.back());

  queue.link_back(e1);
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e1, queue.back());

  ASSERT_EQ(&e1, queue.unlink_front());
  ASSERT_TRUE(queue.empty());
  ASSERT_EQ(nullptr, queue.front());
  ASSERT_EQ(nullptr, queue.back());

  queue.link_back(e1);
  queue.link_back(e2);
  ASSERT_EQ(queue.front(), &e1);
  ASSERT_EQ(queue.next(e1), &e2);
  ASSERT_EQ(queue.next(e2), nullptr);
  ASSERT_EQ(queue.back(), &e2);

  ASSERT_EQ(&e1, queue.unlink_front());
  ASSERT_TRUE(queue.is_unlinked(e1));

  ASSERT_EQ(queue.front(), &e2);
  ASSERT_EQ(&e2, queue.unlink_front());
  ASSERT_TRUE(queue.empty());
}

TEST_F(ForwardChainFixture, Unlink)
{
  ASSERT_TRUE(queue.empty());
  populate_queue();
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e2, queue.next(e1));
  ASSERT_EQ(&e3, queue.next(e2));
  ASSERT_EQ(nullptr, queue.next(e3));
  ASSERT_EQ(&e3, queue.back());

  ASSERT_EQ(&e2, queue.unlink(e2));
  ASSERT_TRUE(queue.is_unlinked(e2));
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e3, queue.next(e1));
  ASSERT_EQ(&e3, queue.back());

  ASSERT_EQ(&e3, queue.unlink(e3));
  ASSERT_TRUE(queue.is_unlinked(e3));
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e1, queue.back());
  ASSERT_EQ(nullptr, queue.next(e1));

  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(nullptr, queue.unlink(e3));
  ASSERT_EQ(&e1, queue.unlink(e1));
  ASSERT_TRUE(queue.empty());
  ASSERT_EQ(nullptr, queue.front());
  ASSERT_EQ(nullptr, queue.back());
}

TEST_F(ForwardChainFixture, LinkAfter)
{
  ASSERT_TRUE(queue.empty());

  queue.link_front(e1);
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e1, queue.back());

  queue.link_after(e1, e3);
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e3, queue.next(e1));
  ASSERT_EQ(&e3, queue.back());

  queue.link_after(e1, e2);
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e2, queue.next(e1));
  ASSERT_EQ(&e3, queue.next(e2));
  ASSERT_EQ(&e3, queue.back());
}

TEST_F(ForwardChainFixture, LinkBefore)
{
  ASSERT_TRUE(queue.empty());

  queue.link_before(e3, [](auto& e){return false;});
  ASSERT_EQ(&e3, queue.front());
  ASSERT_EQ(&e3, queue.back());

  queue.link_before(e1, [](auto& e){return e.id == 3;});
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e3, queue.next(e1));
  ASSERT_EQ(&e3, queue.back());

  queue.link_before(e2, [](auto& e){return e.id == 3;});
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e2, queue.next(e1));
  ASSERT_EQ(&e3, queue.next(e2));
  ASSERT_EQ(&e3, queue.back());
}

TEST_F(ForwardChainFixture, Clear)
{
  ASSERT_TRUE(queue.empty());
  queue.link_back(e1);
  queue.link_back(e2);
  queue.link_back(e3);
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e2, queue.next(e1));
  ASSERT_EQ(&e3, queue.next(e2));
  ASSERT_EQ(nullptr, queue.next(e3));
  ASSERT_EQ(&e3, queue.back());

  queue.clear();
  ASSERT_EQ(nullptr, queue.front());
  ASSERT_TRUE(queue.is_unlinked(e1));
  ASSERT_TRUE(queue.is_unlinked(e2));
  ASSERT_TRUE(queue.is_unlinked(e3));
  ASSERT_EQ(nullptr, queue.back());
}

TEST_F(ForwardChainFixture, RangeFor)
{
  ASSERT_TRUE(queue.empty());

  unsigned int mask{};
  for (auto& elt: queue) {
    mask |= (1U << elt.id);
  }
  ASSERT_EQ(0U, mask);

  populate_queue();
  mask = {};
  for (auto& elt: queue) {
    mask |= (1U << elt.id);
    elt.value += 1;
  }
  ASSERT_EQ(0x0Eu, mask);
  ASSERT_EQ(1, e1.value);

  mask = {};
  for (auto& elt: queue) {
    if (2 == elt.id) {
      ASSERT_EQ(&e2, queue.unlink(e2));
    } else {
      mask |= (1U << elt.id);
      elt.value += 1;
    }
  }
  ASSERT_EQ(0x0Au, mask);
  ASSERT_EQ(2, e1.value);
  ASSERT_EQ(1, e2.value);
  ASSERT_EQ(2, e3.value);

  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e3, queue.next(e1));
  ASSERT_EQ(&e3, queue.back());

  const auto& q2{queue};
  mask = {};
  for (auto& elt: q2) {
    mask |= (1U << elt.id);
    elt.value += 1;
  }
  ASSERT_EQ(0x0Au, mask);
  ASSERT_EQ(3, e1.value);
  ASSERT_EQ(1, e2.value);
  ASSERT_EQ(3, e3.value);

  /* Whitebox test of invalid use: chain has 1, 3; 3 is removed while visiting
   * 1. 3 is still visited, which is a problem because it's unlinked. */
  mask = {};
  for (auto& elt: queue) {
    if (1 == elt.id) {
      ASSERT_EQ(&e3, queue.unlink(e3));
      ASSERT_TRUE(queue.is_unlinked(e3));
    }
    mask |= (1U << elt.id);
    elt.value += 1;
  }
  ASSERT_EQ(0x0Au, mask);
  ASSERT_EQ(4, e1.value);
  ASSERT_EQ(1, e2.value);
  ASSERT_EQ(4, e3.value);

  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e1, queue.back());
  queue.link_back(e2);

  /* Whitebox test of invalid use: an added element is visted. */
  mask = {};
  for (auto& elt: queue) {
    if (1 == elt.id) {
      ASSERT_EQ(&e2, queue.back());
      e3.value = 0;
      queue.link_back(e3);
      ASSERT_EQ(&e3, queue.back());
    }
    mask |= (1U << elt.id);
    elt.value += 1;
  }
  ASSERT_EQ(0x0Eu, mask);
  ASSERT_EQ(5, e1.value);
  ASSERT_EQ(2, e2.value);
  ASSERT_EQ(1, e3.value);
}

TEST_F(ForwardChainFixture, MoveCtor)
{
  populate_queue();
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e2, queue.next(e1));
  ASSERT_EQ(&e3, queue.next(e2));
  ASSERT_EQ(nullptr, queue.next(e3));
  ASSERT_EQ(&e3, queue.back());

  auto pfx{std::move(queue)};
  ASSERT_TRUE(queue.empty());
  ASSERT_EQ(nullptr, queue.front());
  ASSERT_EQ(nullptr, queue.back());
  ASSERT_FALSE(pfx.empty());
  ASSERT_EQ(&e1, pfx.front());
  ASSERT_EQ(&e2, pfx.next(e1));
  ASSERT_EQ(&e3, pfx.next(e2));
  ASSERT_EQ(nullptr, pfx.next(e3));
  ASSERT_EQ(&e3, pfx.back());
}

TEST_F(ForwardChainFixture, MoveAssign)
{
  populate_queue();
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e2, queue.next(e1));
  ASSERT_EQ(&e3, queue.next(e2));
  ASSERT_EQ(nullptr, queue.next(e3));
  ASSERT_EQ(&e3, queue.back());

  element::queue_type pfx;
  ASSERT_TRUE(pfx.empty());

  pfx = std::move(queue);
  ASSERT_TRUE(queue.empty());
  ASSERT_EQ(nullptr, queue.front());
  ASSERT_EQ(nullptr, queue.back());

  ASSERT_FALSE(pfx.empty());
  ASSERT_EQ(&e1, pfx.front());
  ASSERT_EQ(&e2, queue.next(e1));
  ASSERT_EQ(&e3, pfx.next(e2));
  ASSERT_EQ(nullptr, queue.next(e3));
  ASSERT_EQ(&e3, pfx.back());
}

TEST_F(ForwardChainFixture, SplitAtFalse)
{
  populate_queue();

  // Nothing satisfies predicate: returned chain is empty
  ASSERT_EQ(&e1, queue.front());
  auto pfx = queue.split_through([](auto){return false;});

  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(&e1, queue.front());
  ASSERT_EQ(&e2, queue.next(e1));
  ASSERT_EQ(&e3, queue.next(e2));
  ASSERT_EQ(nullptr, queue.next(e3));
  ASSERT_EQ(&e3, queue.back());

  ASSERT_TRUE(pfx.empty());
  ASSERT_EQ(nullptr, pfx.front());
  ASSERT_EQ(nullptr, pfx.back());
}

TEST_F(ForwardChainFixture, SplitAtEmpty)
{
  ASSERT_TRUE(queue.empty());
  auto pfx = queue.split_through([](auto){return true;});
  ASSERT_TRUE(pfx.empty());
}

TEST_F(ForwardChainFixture, SplitAtTrue)
{
  populate_queue();

  // Everything satisfies predicate: essentially a move
  ASSERT_EQ(&e1, queue.front());
  auto pfx = queue.split_through([](auto){return true;});

  ASSERT_TRUE(queue.empty());
  ASSERT_EQ(nullptr, queue.front());
  ASSERT_EQ(nullptr, queue.back());

  ASSERT_FALSE(pfx.empty());
  ASSERT_EQ(&e1, pfx.front());
  ASSERT_EQ(&e2, pfx.next(e1));
  ASSERT_EQ(&e3, pfx.next(e2));
  ASSERT_EQ(nullptr, pfx.next(e3));
  ASSERT_EQ(&e3, pfx.back());
}

TEST_F(ForwardChainFixture, SplitAfterFirst)
{
  populate_queue();

  // e1 satisfies, nothing else does
  ASSERT_EQ(&e1, queue.front());
  auto pfx = queue.split_through([](auto& obj){return 1 >= obj.id;});

  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(&e2, queue.front());
  ASSERT_EQ(&e3, queue.next(e2));
  ASSERT_EQ(nullptr, queue.next(e3));
  ASSERT_EQ(&e3, queue.back());

  ASSERT_FALSE(pfx.empty());
  ASSERT_EQ(&e1, pfx.front());
  ASSERT_EQ(nullptr, queue.next(e1));
  ASSERT_EQ(&e1, pfx.back());
}

TEST_F(ForwardChainFixture, SplitAtLast)
{
  populate_queue();

  // e1 and e2 satisfies, e3 does not
  ASSERT_EQ(&e3, queue.back());
  auto pfx = queue.split_through([](auto& obj){return 3 > obj.id;});

  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(&e3, queue.front());
  ASSERT_EQ(nullptr, queue.next(e3));
  ASSERT_EQ(&e3, queue.back());

  ASSERT_FALSE(pfx.empty());
  ASSERT_EQ(&e1, pfx.front());
  ASSERT_EQ(&e2, pfx.next(e1));
  ASSERT_EQ(nullptr, pfx.next(e2));
  ASSERT_EQ(&e2, pfx.back());
}

} // ns anonymous
