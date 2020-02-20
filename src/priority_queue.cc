#include <iostream>

#include "priority_queue.h"

using namespace std;

namespace priority_queue {

  /* the internal buffer is initially this big */
  const unsigned int InitialBufferSize = 100;

  /**
   * Create a new PriorityQueue.
   */
  PriorityQueue::PriorityQueue(
    int (*sorter)(void *a, void *b, void *optData),
    void *optData
  ) {
    length = 0;
    buffer = new void *[InitialBufferSize];
    this->sorter = sorter;
    this->optData = optData;
  }

  /**
   * Destroy a PriorityQueue.
   */
  PriorityQueue::~PriorityQueue() {
    delete buffer;
  }

  /**
   * Return the current number of items in the PriorityQueue.
   */
  unsigned int PriorityQueue::Length() {
    return length;
  }

  /**
   * Push a value into the queue.
   */
  void PriorityQueue::push(void *value) {
    // TODO: grow buffer if necessary.
    buffer[length] = value;
    siftUp(length);
    length++;
  }

  /**
   * Pop the lowest value off the queue (where lowest is defined
   * by the user-provided sorting function). Return null if the
   * queue is empty.
   */
  void *PriorityQueue::pop() {
    if (length == 0) {
      return nullptr;
    }
    void *poppedValue = buffer[0];

    if (--length > 0) {
      /* move the last element in the heap to the root, and siftDown */
      buffer[0] = buffer[length];
      siftDown(0);
    }

    return poppedValue;
  }

  /**
   * Given a newly-added node, compare it with its parent to see if it has a lowest
   * value. If so, swap them over and continue up the tree.
   */
  void PriorityQueue::siftUp(unsigned int position) {
    bool sorted = false;
    while (position != 0 && !sorted) {
      unsigned int parentPosition = (position - 1) >> 1;
      if (sorter(buffer[position], buffer[parentPosition], optData) > 0) {
        /* parent and child in wrong order, swap them and continue up the heap */
        void *tmp = buffer[position];
        buffer[position] = buffer[parentPosition];
        buffer[parentPosition] = tmp;
        position = parentPosition;
      } else {
        /* parent and child are in the correct order - we stop sifting upwards */
        sorted = true;
      }
    }
  }

  /**
   * Given a node that has recently been placed at the root, in order to replace
   * a node that was popped, we need to sift the new node downwards to maintain
   * the ordering of the heap.
   */
  void PriorityQueue::siftDown(unsigned int position) {
    bool sorted = false;

    while (!sorted) {
      unsigned int leftChildPosition = position * 2 + 1;
      unsigned int rightChildPosition = position * 2 + 2;

      /*
       * Algorithm: compare root with left child and right child, then determine which
       * is the lowest value. If the root isn't the lowest, then swap it with the lowest.
       */
      unsigned int lowest = position;

      /* is the left child lower than the root? */
      if (leftChildPosition < length &&
            sorter(buffer[lowest], buffer[leftChildPosition], optData) < 0) {
        lowest = leftChildPosition;
      }

      /* is the right child lower than the root and the left child? */
      if (rightChildPosition < length &&
            sorter(buffer[lowest], buffer[rightChildPosition], optData) < 0) {
        lowest = rightChildPosition;
      }

      /* if the root is lower than both child, the heap is already in order */
      if (lowest == position) {
        sorted = true;
      }

      /* else, swap the root with the lowest of the two child, and move down the tree */
      else {
        void *tmp = buffer[position];
        buffer[position] = buffer[lowest];
        buffer[lowest] = tmp;
        position = lowest;
      }
    }
  }
}
