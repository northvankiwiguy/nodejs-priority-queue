/*
 * Test cases using a custom sort function.
 */
const data_types = require('../build/Release/data_types.node')

let queue = undefined

/* Basic sorting by numbers */
function numberSortFunction(a, b) {
  return b - a;
}

/* Basic sorting by string */
function stringSortFunction(a, b) {
  if (a < b) {
    return 1
  } else if (a === b) {
    return 0
  } else {
    return -1
  }
}

/* Invalid sort function that doesn't return a number */
function invalidSortFunction(a, b) {
  return 'bad'
}

describe("custom sort function", () => {
  
  test("non-function is rejected", () => {
    expect(() => new data_types.PriorityQueue(42)).toThrow()
    expect(() => new data_types.PriorityQueue('Hello')).toThrow()
  })

  test('correct sort function will not throw', () => {
    expect(() => new data_types.PriorityQueue(numberSortFunction)).not.toThrow()
  })

  test('sort function that does not return a number throws an exception', () => {
    queue = new data_types.PriorityQueue(invalidSortFunction)
    queue.push(5)
    expect(() => queue.push(10)).toThrow()
  })

  test('sort order is correct for numeric data', () => {
    queue = new data_types.PriorityQueue(numberSortFunction)
    queue.push(10);
    queue.push(5)
    queue.push(2)

    expect(queue.pop()).toBe(2)
    expect(queue.pop()).toBe(5)
    expect(queue.pop()).toBe(10)
  })

  test('sort order is correct for string data', () => {
    queue = new data_types.PriorityQueue(stringSortFunction)

    queue.push('camel')
    queue.push('aardvark')
    queue.push('pig')

    expect(queue.pop()).toBe('aardvark')
    expect(queue.pop()).toBe('camel')
    expect(queue.pop()).toBe('pig')
  })
})