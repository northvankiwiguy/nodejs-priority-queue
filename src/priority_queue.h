#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_

namespace priority_queue {
  /**
   * This class provides a generic implementation of priority queues. It's
   * independent of any framework that it might be included in.
   */
  class PriorityQueue {

    public:
      /**
       * @param sorter A user-provided function for comparing `a` and `b`, returning -1, 0, or +1.
       * @param optData Optional user-provided data that's passed to the `sorter` function.
       */
      PriorityQueue(
        int (*sorter)(void *a, void *b, void *optData),
        void *optData = nullptr);
      ~PriorityQueue();

      unsigned int Length();
      void push(void *value);
      void *pop();

    private:
      unsigned int length;
      unsigned int bufferLength;
      void **buffer;
      int (*sorter)(void *a, void *b, void *optData);
      void *optData;

      void siftUp(unsigned int position);
      void siftDown(unsigned int position);
  };
}

#endif /* _PRIORITY_QUEUE_H_ */
