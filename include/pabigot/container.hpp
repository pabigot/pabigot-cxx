/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2015-2019 Peter A. Bigot */

/** Special-purpose containers, primarily for embedded systems use.
 *
 * These may or may not satisfy any standard C++ container concepts.
 *
 * @file */

#ifndef PABIGOT_CONTAINER_HPP
#define PABIGOT_CONTAINER_HPP
#pragma once

#include <array>
#include <cinttypes>
#include <functional>
#include <limits>

#include <pabigot/common.hpp>

namespace pabigot {

/** Special purpose containers. */
namespace container {

/** A basic round-robin (circular) homogeneous buffer with externally-allocated
 * capacity.
 *
 * Intended for use in embedded systems.  You don't get to make copies or move
 * them so you probably want a static allocation as a global variable instead
 * of defining one on a stack somewhere.
 *
 * @tparam T the type of the elements in the buffer. */
template <typename T = uint8_t>
class rr_adaptor
{
public:
  /** Type alias for the elements of the buffer */
  using value_type = T;

  /** Type alias for representing the size of the buffer.
   *
   * This class is intended to be used in embedded systems, so the capacity is
   * limited.  Most applications would work with a maximum of 255 elements, but
   * none should require more than 65535.
   *
   * Really, if you want bigger buffers you should be using std::queue. */
  using size_type = uint16_t;

  /** Type alias for signed size values. */
  using ssize_type = std::make_signed<size_type>::type;

  /** Marker value stored in #head_ to indicate that the buffer is empty. */
  static constexpr size_type EMPTY_HEAD = -1;

  /** Create an adaptor for round-robin access to a fixed buffer.
   *
   * @param data pointer to a contiguous sequence of @p count instances of
   * #value_type.
   *
   * @param count the number of instances of #value_type available for
   * storage. */
  constexpr rr_adaptor (value_type* data,
                        size_type count) :
    data_{data},
    count_{count}
  { }

  rr_adaptor () = delete;
  rr_adaptor (const rr_adaptor&) = delete;
  rr_adaptor& operator= (const rr_adaptor&) = delete;
  rr_adaptor (rr_adaptor&&) = delete;
  rr_adaptor& operator= (rr_adaptor&&) = delete;

  /** `true` iff the buffer has no data in it. */
  bool empty () const noexcept
  {
    return EMPTY_HEAD == head_;
  }

  /** `true` if the buffer cannot receive more data without discarding an
   * element. */
  bool full () const noexcept
  {
    return size() == max_size();
  }

  /** The maximum number of values that can be stored in the buffer.
   *
   * This is one fewer than provided to the constructor so we can distinguish
   * empty() from full() without maintaining a separate flag. */
  size_type max_size () const noexcept
  {
    return count_;
  }

  /** The number of values currently stored in the buffer. */
  size_type size () const noexcept
  {
    size_type rv = 0;
    if (!empty()) {
      /* Get the signed difference between the head and tail.  Negative means the
       * range wraps. */
      ssize_type n = head_ - tail_;
      if (0 >= n) {
        n += count_;
      }
      rv = n;
    }
    return rv;
  }

  /** Push a new value at the front of the buffer.
   *
   * If the buffer is full() this overwrites a previously-pushed value.  The
   * caller is responsible for preventing that if desired.
   *
   * @return `true` iff the store resulted in discarding a previously-pushed
   * value. */
  bool push (value_type v) noexcept
  {
    bool rv = false;

    if (empty()) {
      head_ = tail_ = 0;
    } else if (tail_ == head_) {
      rv = true;
      (void)pop();
    }
    auto nh = next_index_(head_);
    data_[head_] = v;
    head_ = nh;
    return rv;
  }

  /** Pop a value from the back of the buffer.
   *
   * If the buffer is empty() this returns a default-initialized object of
   * #value_type. */
  value_type pop () noexcept
  {
    if (empty()) {
      return value_type{};
    }
    size_type tail = tail_;
    tail_ = next_index_(tail_);
    if (head_ == tail_) {
      head_ = EMPTY_HEAD;
    }
    return data_[tail];
  }

  /** Restore the buffer to an empty state. */
  void clear () noexcept
  {
    head_ = EMPTY_HEAD;
  }

private:
  size_type next_index_ (size_type v) const
  {
    if (++v >= count_) {
      v = 0;
    }
    return v;
  }

  value_type* data_;
  size_type const count_;
  size_type head_ = EMPTY_HEAD;
  size_type tail_ = 0;
};

/** Container used to link objects into a sequence.
 *
 * The container is intended for use in embedded systems where objects may need
 * to be present in one or more sequences and dynamic memory allocation for
 * `std::forward_list` and similar containers is not desired.  Links are
 * instead stored as fields within the linked structures, accessed through a
 * function object provided to the container on construction.
 *
 * Supports:
 * * constant time LIFO operations via link_front() and unlink_front();
 * * constant time FIFO operations via link_back() and unlink_front();
 * * constant time link_after() an object already in the sequence;
 * * linear time link_before() the first object satisfying a predicate,
 *   supporting order sequences;
 * * linear time split_after() to remove the predicate-satisfying prefix of a list;
 * * linear time unlink();
 * * forward iteration including [range-for](https://en.cppreference.com/w/cpp/language/range-for);
 * * detection of values that are not in a list via is_unlinked() as long as
 *   the link field is initialized to unlinked_ptr().
 *
 * Container instances may be moved, but cannot be copied.
 *
 * @note References stored in the sequence are not `const`-qualified: iterating
 * over a `const forward_chain` instance allows non-const operations to be
 * performed on the objects themselves.
 *
 * @note Iterator invalidation with this sequence is **not** the same as with
 * `std::forward_list`.  It is specifically safe to @ref unlink the current
 * value from the sequence within a range-for statement body: iteration will
 * continue with the next value in the original sequence.  Removing other
 * values, or adding values, may produce anomalous behavior if an existing
 * iterator is used after the sequence is modified.  Iterators are also
 * invalidated if the sequence is moved.
 *
 * @warning The application is responsible for ensuring that instances are only
 * added to the chain when they are @link is_unlinked not already in a chain
 * @endlink.  Violation of this may result in corrupted data structures that
 * include cycles or cross-linked chains.
 *
 * @tparam T the type of the instance.
 *
 * @tparam REF_NEXT a function object type where `operator()` converts a `T&`
 * to a `T*&` which is the lvalue for the pointer to the next `T` in the chain.
 *
 * Example:
 *
 *     struct object {
 *       // Function object type used to reference field next
 *       struct ref_next {
 *         using pointer_type = object *;
 *         pointer_type& operator() (object& m) noexcept
 *         {
 *           return m.next;
 *         }
 *       };
 *       using chain_type = forward_chain<object>;
 *
 *       // Declare link used when in a chain, initalized as unlinked
 *       chain_type::pointer_type next{chain_type::unlinked_ptr()};
 *
 *       // Declare the chain that links objects
 *       static chain_type chain;
 *     };
 */
template <typename T,
          typename REF_NEXT>
class forward_chain
{
public:
  /** The type of this chain */
  using chain_type = forward_chain<T, REF_NEXT>;

  /** The type of object linked by this chain */
  using value_type = T;

  /** The type for a pointer to @ref value_type */
  using pointer_type = value_type *;

  /** The type for a function that tests a @ref value_type for a condition. */
  using predicate_type = std::function<bool(const value_type&)>;

  /** Integral value of an invalid pointer used to denote unlinked objects. */
  static constexpr uintptr_t UNLINKED_PTR = -1;

  /** Test whether a pointer is unlinked. */
  static pointer_type unlinked_ptr () noexcept
  {
    return reinterpret_cast<pointer_type>(UNLINKED_PTR);
  }

  /** Test whether a pointer is unlinked. */
  bool is_unlinked (pointer_type ptr) const noexcept
  {
    return unlinked_ptr() == ptr;
  }

  /** Test whether a value is unlinked. */
  bool is_unlinked (value_type& value) const noexcept
  {
    return is_unlinked(ref_next(value));
  }

  /* You can default-construct these. */
  forward_chain () noexcept = default;

  /* You can't copy them. */
  forward_chain (const forward_chain&) = delete;
  forward_chain& operator= (const forward_chain&) = delete;

  /* You can move them. */
  forward_chain (forward_chain&& from) noexcept :
    front_{from.front_},
    back_{from.back_}
  {
    from.front_ = from.back_ = nullptr;
  }

  forward_chain& operator= (forward_chain&& from) noexcept
  {
    front_ = from.front_;
    back_ = from.back_;
    from.front_ = from.back_ = nullptr;
    return *this;
  }

  /** Sentinal type used as end-of-chain iterator value. */
  class end_iterator_type { };

  /** Iterator type designed to support range-for.
   *
   * An iterator can be dereferenced to produce a reference to a value within
   * the chain.  The reference is not `const`-qualified.
   *
   * An iterator can be pre-incremented, which will update it so that when
   * dereferenced it will reference the value that was the successor of the
   * value referenced by the iterator at the time the iterator was last
   * updated.
   *
   * I.e., if you have an iterator that references `e`, and `f` follows `e` in
   * the chain, then removing `e` from the chain and incrementing the iterator
   * will reference `f`. */
  class chain_iterator_type
  {
    friend chain_type;

    chain_iterator_type (const chain_type& chain) :
      chain{chain},
      current{chain.front_},
      next{current ? chain.ref_next(*current) : nullptr}
    { }

    const chain_type& chain;

    /** Pointer to the node we're looking at. */
    pointer_type current = nullptr;

    /** Pointer to the node that followed current when we advanced to current.
     *
     * This preserves the ability to advance when the current node is removed
     * from the sequence (invalidating its `next` field) during iteration. */
    pointer_type next = nullptr;

  public:
    /** Return `true` iff this iterator is past the end of its chain. */
    bool operator== (const end_iterator_type& )
    {
      return !current;
    }

    /** Minimum operator required for range-for support.
     *
     * Returns `false` iff this iterator is past the end of its chain. */
    bool operator!= (const end_iterator_type& rhs)
    {
      return !operator==(rhs);
    }

    /** Increment to reference the cached successor of the current. */
    chain_iterator_type& operator++ ()
    {
      current = next;
      if (current) {
        next = chain.ref_next(*current);
        /* Avoid segfault if somebody removed a value that we iterated into.
         * See RangeFor whitebox testing. */
        if (chain.is_unlinked(next)) {
          next = nullptr;
        }
      }
      return *this;
    }

    /** Get a reference to the value in the chain.
     *
     * @warning Invoking this in a situation where `iter != end()` is false
     * will dereference a null pointer. */
    value_type& operator* () noexcept
    {
      return *current;
    }
  };

  /** Get an iterator that starts at the beginning of the chain. */
  chain_iterator_type begin () const noexcept
  {
    return {*this};
  }

  /** Get a value `end` for which `iter != end` will return false only when
   * `iter` is a @ref chain_iterator_type that is past the end of its chain. */
  end_iterator_type end () const noexcept
  {
    return {};
  }

  /** Indicate whether the sequence is empty. */
  bool empty () const noexcept
  {
    return !front_;
  }

  /** Return a pointer to the first value in the sequence. */
  pointer_type front () const noexcept
  {
    return front_;
  }

  /** Return a pointer to the next value in the sequence.
   *
   * @param ptr an object known to be in the chain.
   *
   * @return a pointer to the next object in the chain, or `nullptr` if @p elt
   * is last. */
  pointer_type next (value_type& elt) const noexcept
  {
    return ref_next(elt);
  }

  /** Return a pointer to the last value in the sequence. */
  pointer_type back () const noexcept
  {
    return back_;
  }

  /** Add the value to the front of the sequence.
   *
   * @warning @p value must not already be in the sequence. */
  void link_front (value_type &value) noexcept
  {
    if (empty()) {
      back_ = &value;
    }
    ref_next(value) = front_;
    front_ = &value;
  }

  /** Remove and return a pointer to the first value of the sequence. */
  pointer_type unlink_front () noexcept
  {
    auto rv = front_;
    if (rv) {
      auto& next = ref_next(*rv);
      front_ = next;
      next = unlinked_ptr();
      if (back_ == rv) {
        back_ = nullptr;
      }
    }
    return rv;
  }

  /** Add the value immediately after an value already in the sequence.
   *
   * @param pos the value already in the sequence.
   * @param value the value to add after @p pos. */
  void link_after (value_type &pos,
                   value_type &value) noexcept
  {
    ref_next(value) = ref_next(pos);
    ref_next(pos) = &value;
    if (back_ == &pos) {
      back_ = &value;
    }
  }

  /** Add the value immediately before the first value in the sequence for
   * which @p pred is `true`.
   *
   * If no value in the sequence satisfies the predicate the new value is
   * linked at the end.
   *
   * @param value the value to insert into the chain.
   *
   * @param pred the predicate that identifies the linked member before which
   * @p value will be inserted. */
  void link_before (value_type &value,
                    predicate_type pred) noexcept
  {
    auto npp = &front_;
    while (*npp) {
      auto np = *npp;
      if (pred(*np)) {
        ref_next(value) = np;
        *npp = &value;
        return;
      }
      npp = &ref_next(*np);
    }
    return link_back(value);
  }

  /** Strip off a chain of all leading elements that satisfy a predicate.
   *
   * This may be used on a chain linking objects by some ordinal function via
   * insert_before(), to extract the prefix list of items that are ready to
   * process.
   *
   * @param pred the predicate that identifies elements that are to be removed
   * from the list.
   *
   * @return a new chain containing only the elements that satisfy @p pred.
   * The chain in which this method is invoked holds any remaining elements. */
  chain_type split_through (predicate_type pred) noexcept
  {
    if (empty()
        || (!pred(*front_))) {
      return {};
    }
    auto prev = front_;
    auto npp = &ref_next(*prev);
    while ((*npp) && pred(**npp)) {
      prev = *npp;
      npp = &ref_next(*prev);
    };
    auto front = front_;
    front_ = *npp;
    if (!front_) {
      back_ = nullptr;
    }
    *npp = nullptr;
    return {front, prev};
  }

  /** Add the value to the end of the sequence.
   *
   * @warning @p value must not already be in the sequence.  */
  void link_back (value_type &value) noexcept
  {
    if (empty()) {
      return link_front(value);
    }
    ref_next(value) = nullptr;
    ref_next(*back_) = &value;
    back_ = &value;
  }

  /** Remove an value from the sequence at any position.
   *
   * @param value the value to be removed
   *
   * @return the address of @p value if @p value was in the sequence, otherwise a null
   * pointer. */
  pointer_type unlink (value_type& value) noexcept
  {
    auto lpp = &front_;
    pointer_type prev = nullptr;

    while (*lpp) {
      auto lp = *lpp;
      if (&value == lp) {
        auto& next = ref_next(value);
        *lpp = next;
        next = unlinked_ptr();
        if (&value == back_) {
          back_ = prev;
        }
        return &value;
      }
      prev = lp;
      lpp = &ref_next(*lp);
    }
    return nullptr;
  }

  /** Remove all values from the sequence. */
  void clear () noexcept
  {
    auto p = front_;
    while (p) {
      auto& next = ref_next(*p);
      p = next;
      next = unlinked_ptr();
    }
    front_ = back_ = nullptr;
  }

private:
  /** Private constructor for move support. */
  forward_chain (pointer_type front,
                 pointer_type back) noexcept :
    front_{front},
    back_{back}
  { }

  /** Get the non-const reference to the pointer field associated with @p
   * elt. */
  pointer_type& ref_next (value_type & elt) const noexcept
  {
    return REF_NEXT{}(elt);
  }

  /** First item in the chain. */
  pointer_type front_{};

  /** Last item in the chain. */
  pointer_type back_{};
};

} // ns container

} // ns pabigot

#endif /* PABIGOT_CONTAINER_HPP */
