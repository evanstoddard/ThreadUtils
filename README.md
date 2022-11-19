# ThreadUtils

ThreadUtils is a collection of header-only utilies to make multithreaded development easier.

## Table of Contents
- [ThreadUtils](#threadutils)
	- [Table of Contents](#table-of-contents)
	- [Dependencies](#dependencies)
	- [Usage](#usage)
	- [Examples](#examples)
		- [Creating Runnable](#creating-runnable)
		- [Threadpool](#threadpool)
	- [Demos](#demos)
	- [Contributing](#contributing)
	- [License](#license)

## Dependencies

ThreadUtils is built with `c++14` and all testing and development has been linked against `pthread`.

## Usage

Being a header only library, simply include the appropriate headers.

Everything is implemented in the `ThreadUtils` namespace.

## Examples

### Creating Runnable
There are various threadpool classes. They all operate on queues of runnables.

A `Runnable` is constructed with any invokable type (`std::function`, lambda, or function reference/pointer), and parameters to be passed to the invokable.  This is much like the constructor of `std::thread`.  The paremeter types of the runnable function are used as the template parameters of the runnable.
```
// Function definition
void doSomething(int, float);

// std::function
std::function<void(int, float)> functionObj(doSomething);
.
.
.
// Runnable instantiations
using namespace ThreadUtils;
Runnable<int, float> *runnableWithRef = new Runnable<int, float>(doSomething, 1, 3.14);
Runnable<int, float> *runnableWithStdFunc = new Runnable<int, float>(functionObj, 1, 3.14);
Runnable<int, float> *runnableWithLambda = new Runnable<int, float>([](int i, float f){}, 1, 3.14);
```

Runnables can also be contructed in place by other `ThreadUtils` classes.  See examples below.

### Threadpool

There are multiple types of threadpools which all inherit `Threadpool` as their basetype. A threadpool is constructed with the number of threads to be spun up.  The pool can be started with `threadpool.start()` and stopped with `threadpool.stop()`.

Runnables can be added with `threadpool.enqueue(runnable)`. Additionally a `Runnable` can be constructed inplace and enqueued with `enqueue_new`:

```
using namespace ThreadUtils;

void doSomething(int, float);
std::function<void(int, float)> funcObj(doSomething);
.
.
.
Threadpool threadpool(4);
Runnable<int, float> *runnable = new Runnable<int, float>(doSomething, 1, 3.14);
threadpool.enqueue(runnable);
threadpool.enqueue_new(funcObj, 1, 3.14);
threadpool.enqueue_new([&](int i, float f){}, 1, 3.14);
```


## Demos

Demos are built with cmake:
```
mkdir build && cd build
cmake .. && make
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License

[MIT](https://choosealicense.com/licenses/mit/)