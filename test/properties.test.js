/*
 * Tests for validating that PriorityQueue objects have all the expected
 * properties, with all the correct descriptors.
 */
const data_types = require('../build/Release/data_types.node')

let queue = undefined

beforeAll(() => {
  queue = new data_types.PriorityQueue()
})

describe("the constructor", () => {
  test("has the expected name", () => {
    expect(queue.constructor.name).toBe("PriorityQueue")
  })
})

describe("the length property", () => {
  test("exists as an ownProperty", () => {
    expect(Object.getOwnPropertyNames(queue)).toContain("length")
  })

  test("starts with the numeric 0 value", () => {
    expect(typeof queue.length).toBe("number")
    expect(queue.length).toBe(0)
  })

  test("is enumerable", () => {
    expect(Object.getOwnPropertyDescriptor(queue, "length")).toStrictEqual({
      "configurable": false, 
      "enumerable": true, 
      "value": 0,
      "writable": false
    })
  })

  test("can not be modified", () => {
    queue.length = 42
    expect(queue.length).toBe(0)
  })

  test("can not be deleted", () => {
    delete queue.length
    expect(Object.getOwnPropertyNames(queue)).toContain("length")
  })
})

describe("the isEmpty function", () => {
  test("is a function", () => {
    expect(typeof queue.isEmpty).toBe("function")
  })

  test("is a prototype property, not an own property", () => {
    expect(Object.getPrototypeOf(queue)["isEmpty"]).not.toBeUndefined()
  })

  test("can be called (and will return true for an empty queue)", () => {
    expect(queue.isEmpty()).toBe(true)
  })
})
