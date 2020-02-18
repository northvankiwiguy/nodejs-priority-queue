#include <iostream>

#include "priority_queue.h"

using namespace std;

namespace priority_queue {

  /**
   * Create a new PriorityQueue.
   */
  PriorityQueue::PriorityQueue() {
    length = 0;
    cout << "PriorityQueue created\n";
  }

  /**
   * Destroy a PriorityQueue.
   */
  PriorityQueue::~PriorityQueue() {
    cout << "PriorityQueue destroyed\n";
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
    length++;
  }
}
