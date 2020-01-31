# Overview

This is a hobby project to create a NodeJS native plugin to support a `PriorityQueue` class in NodeJS. The goal of this work is learn more about how the V8 JavaScript engine works, and therefore all code is written to directly use V8's API (instead of any wrappers such as N-API).

# Building

Install the `node-gyp` tools with:

```
$ npm install -g node-gyp
```

Then configure the build for your platform, with:

```
$ make configure
```

To compile the source code and create a native plugin, use:

```
$ make build
```

Finally, to run the test suite, use:

```
$ make test
```

# References

Here's all the documentation I could find on writing native plugins:

* [Official NodeJS Documentation on C++ Addons](https://nodejs.org/api/addons.html)
* [Writing Native Node.js Modules](https://blog.risingstack.com/writing-native-node-js-modules/)
* [V8 API Documentation](https://v8docs.nodesource.com/)

