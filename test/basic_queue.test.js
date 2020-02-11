/*
 * Tests for validating that PriorityQueue objects can be manipulated
 * with basic operations.
 */
const data_types = require('../build/Release/data_types.node')

let queue = undefined

beforeAll(() => {
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

  test.skip("pushing an item increases the length", () => {
    queue.push(1)
    expect(queue.isEmpty()).toBe(false)
    expect(queue.length).toBe(1)
  })
})
