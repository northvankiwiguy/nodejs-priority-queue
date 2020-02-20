/*
 * Tests for validating that PriorityQueue objects can be manipulated
 * with basic operations.
 */
const data_types = require('../build/Release/data_types.node')

let queue = undefined

beforeEach(() => {
  queue = new data_types.PriorityQueue()
})

describe("queue lengths", () => {
  test("queues are initially empty", () => {
    expect(queue.isEmpty()).toBe(true)
    expect(queue.length).toBe(0)
  })

  test("popping from an empty queue is still empty", () => {
    queue.pop()
    expect(queue.isEmpty()).toBe(true)
    expect(queue.length).toBe(0)
  })

  test("pushing nothing will leave the queue as empty", () => {
    queue.push()
    expect(queue.length).toBe(0)
  })

  test("pushing an item increases the length", () => {
    queue.push(1)
    expect(queue.isEmpty()).toBe(false)
    expect(queue.length).toBe(1)
  })

  test("pushing five items gives a queue of length 5", () => {
    queue.push(1, 2, 3, 4, 5)
    expect(queue.length).toBe(5)
  })
})

describe("pushing data items with default sorting", () => {
  test("popping from an empty queue return undefined", () => {
    expect(queue.pop()).toBeUndefined()
  })

  test("one item - push on, pop off", () => {
    queue.push(1234)
    expect(queue.pop()).toBe(1234)
  })

  test("multiple items returned in correct order", () => {
    queue.push(5, 4, 3, 2, 1)
    expect(queue.pop()).toBe(1)
    expect(queue.pop()).toBe(2)
    expect(queue.pop()).toBe(3)
    expect(queue.pop()).toBe(4)
    expect(queue.pop()).toBe(5)
    expect(queue.pop()).toBeUndefined()
  })
})