#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_

namespace priority_queue {
  /**
   * This class provides a generic implementation of priority queues. It's
   * independent of any framework that it might be included in.
   */
  class PriorityQueue {

    public:
      PriorityQueue();
      // TODO: do we need a destructor?

      unsigned int Length();

    private:
      unsigned int length;
  };
}

#endif /* _PRIORITY_QUEUE_H_ */
