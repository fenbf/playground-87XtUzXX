# How not_null can improve your code? 

![not_null examples](https://1.bp.blogspot.com/-x8ZG7XOykSk/WeNZ8kqeJmI/AAAAAAAADGU/aVlO5lPGn4kpV2NxDXxAkSFEbgu9Uj4ZgCLcBGAs/s1600/not_null.png)

One of the key points of modern C++, as I observe, is to be expressive and use proper types. For example, regarding null pointers, rather than just writing a comment:

```cpp
void Foo(int* pInt); // pInt cannot be null
```

I should actually use `not_null<int *> pInt`. 

The code looks great now, isn't it? Let's investigate what `not_null` (from the Core Guidelines/Guideline Support Library) can do for us. 

## Intro

In your application, there are probably lots of places where you have to check if a pointer is not null before you process it. How many times do you write similar code:

```cpp
if (pMyData)
    pMyData->Process();
```
or:

```cpp
auto result = pObj ? pObj->Compute() : InvalidVal;
```

or

```cpp
void Foo(Object* pObj)
{
    if (!pObj)
        return;

    // Do stuff...
}
```

What are the problems with the code?

* It's error-prone: you might forget about if statements and then you might end up with AV (Memory Access Violation), or some other strange errors.
* Code duplication
* Error handling might be on a wrong level. Some functions must accept the null object, but some should depend on the caller to do the checks.
* Performance hit. One additional check might not be a huge deal, but in some projects, I see hundreds or more of such tests.

What if we could forget about most of those safety checks and just be sure the pointer is always valid? How can we enforce such contract?

As you know, writing a simple comment, like `"this argument cannot be null"` won't do the job :)

There's a simple solution suggested in Core Guidelines:

> [I.12](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Ri-nullptr): Declare a pointer that must not be null as `not_null`.

So what's that `not_null` type? How can it help us?

The article was inspired mostly by Kate Gregory's original article: [Using the not_null Template for Pointers That Must Never Be Nul](https://visualstudiomagazine.com/articles/2016/06/01/using-the-not_null-template.aspx). Moreover, Kate's done a great course about core guidelines, where she also experimented with `not_null`. Check it here: [First Look: C++ Core Guidelines and the Guideline Support Library @Pluralsight](http://www.shareasale.com/m-pr.cfm?merchantID=53701&userID=1600409&productID=687369677).

## The basics

`not_null` is a class that can wrap a pointer (or a smart pointer) and guarantees that it will hold only not null values.

The helper class can be found in the Guideline Support Library (GSL, not GLS :))

We can use Microsoft's implementation:

[github.com/Microsoft/GSL/include/gsl/gsl](https://github.com/Microsoft/GSL/blob/master/include/gsl/gsl)

```cpp
//
// not_null
//
// Restricts a pointer or smart pointer to only hold non-null values.
```

(Strangely the class itself is located not in a separate header but in the core header for GSL, so you cannot include only that class without including all other stuff. There's a reported issue that might solve that problem: [#issue 502](https://github.com/Microsoft/GSL/issues/502)).

The basic idea is that you can write:

```cpp
not_null<int *> pIntPtr = nullptr;
```

And you'll get a compile-time error as it's not possible to assign `nullptr` to the pointer. When you have such pointer, you can be sure it's valid and can be accessed. 

For a function:

```cpp
void Foo(not_null<Object*> pObj)
{
    // Do stuff...
}
```

Inside `Foo` you are guaranteed to have a valid pointer, and the additional checks might be removed.

That's some basic theory, and now let's consider a few more examples. 

I divided examples into two sections: compile time and runtime. While it would be cool to handle `nullptr` at compile time only, we won't get away with issues happening at runtime.

## Compile time

The wrapper class won't allow to construct a `not_null` object from `nullptr`, nor it allows to assign null. That's useful in several situations:

* When you have not null pointer and want to clear it:

```cpp
not_null<int *> pInt = new int(10);
// ...
delete pInt;
pInt = nullptr; // error!
```

In the above case you'll get:

    error C2280: 
    'not_null<int *> &not_null<int *>::operator =(nullptr_t)': 
    attempting to reference a deleted function

I really advise not to use raw new/delete (my code is only for a demonstration!). Still, `not_null` gives here an strong hint: "don't mess with the pointer!". Such use case is also a topic of the ownership of such pointer. Since we have only a raw pointer (just wrapped with `not_null`), we can only observe it and not change the pointer itself. Of course, the code will compile when you only delete the pointer and don't clear it. But the consequences of such approach might be dangerous.

* When you want to pass null to a function requiring a not null input parameter.

Violation of a contract!

```cpp
void RunApp(gsl::not_null<App *> pApp) { }

RunApp(nullptr); // error!
```

You'd get the following:

    function "gsl::not_null<T>::not_null(std::nullptr_t) [with T=App *]" cannot be referenced -- it is a deleted function

In other words, you cannot invoke such function, as there's no option to create such param from `nullptr`. With marking input arguments with `not_null`, you get a stronger guarantee. Much better than just a comment :)

* Another reason to initialise when declaring a pointer variable.

While you can always initialize a pointer variable to `nullptr`, maybe it's better just to init it properly (with some real address/value/object) ?

Sometimes it will force you to rethink the code and move the variable to be declared later in the code.

```cpp
int* pInt = nullptr;
// ...
pInt = ComputeIntPtr();
if (pInt) {
    // ...
}
```

Write:

```cpp
// ...
not_null<int *> pInt = CompueInt();
// ...
```

You can play with the code below. Uncomment the code and see what errors you'll get... 

@[Compile time]({"stubs": ["cpp_compile_time.cpp"],"command": "sh ./run.sh cpp_compile_time.cpp"})

Compile time is relatively easy. The compiler will reject the code, and we have just to redesign/fix it. But what about runtime? 

## Runtime

Unfortunately, the compiler cannot predict when a pointer becomes null. It might happen for various reasons. So how to get away with the `if (pPtr) { }` checks?

### The expectations

For example:

```cpp
void RunApp(not_null<App *> pApp);

App* pFakePtr = nullptr;
RunApp(pFakePtr);
```

By default we'll get (Under VS 2017, Windows):

![Error](https://4.bp.blogspot.com/-fEhU4j8tGBQ/WeRY0P3rmOI/AAAAAAAADGk/WcWy-5-13B0Wy3OPEEQlf5HxvsDZPZIywCLcBGAs/s1600/not_null_abort_called.png)

Under that condition the wrapper class can do the following:

1. Terminate app
2. Throw an exception
3. Do nothing

### How to control

You can control the behaviour using a proper `#define`.

See gsl_assert file: [github.com/Microsoft/GSL/include/gsl/gsl_assert](https://github.com/Microsoft/GSL/blob/master/include/gsl/gsl_assert).

```
// 1. GSL_TERMINATE_ON_CONTRACT_VIOLATION: 
//       std::terminate will be called (default)
// 2. GSL_THROW_ON_CONTRACT_VIOLATION: 
//       a gsl::fail_fast exception will be thrown
// 3. GSL_UNENFORCED_ON_CONTRACT_VIOLATION: 
//       nothing happens
```

I probably prefer to use `GSL_THROW_ON_CONTRACT_VIOLATION` and that way we can use exceptions to check the null state.

### Code rewrite

Let's look at the following example. When we have only a single pointer param it's simple anyway, but what if we have more:

So this (2 params):

```cpp
void TestApp(App* pApp, TestParams* pParams)
{
    if (pApp && pParams)
    {
        // ...
    }
    else
        ReportError("null input params");
}
```

can become:

```cpp
void TestApp(not_null<App *> pApp), not_null<TestParams *> pParams)
{
    // input pointers are valid
}
```

But now, all of the checks need to go to the caller:

```cpp
// using
// #define GSL_THROW_ON_CONTRACT_VIOLATION

auto myApp = std::make_unique<App>("Poker");
auto myParams = std::make_unique<TestParams>();

try
{
    TestApp(myApp.get(), myParams.get());
    RunApp(myApp.get());
}
catch (std::exception& e)
{
    std::cout << e.what() << "\n";
    ReportError("null input params");
}
```

Is this better?

* Might be, as we can handle `nullptr` pointer in only one place, shared for several 'child' functions.
* We can move the checks up and up in the code, and in theory have only one test for null pointers.

You can play with the code below:

@[Runtime]({"stubs": ["cpp_runtime.cpp"],"command": "sh ./run.sh cpp_runtime.cpp"})

## Issues

* Smart pointers? The type is prepared to be used with smart pointers, but when I tried to use it, it looked strange. For now, I am not convinced. Although, the 'ownership' of a pointer and null state seems to be orthogonal.
  * See issues like [Core#225](https://github.com/isocpp/CppCoreGuidelines/issues/225), [GSL#89](https://github.com/Microsoft/GSL/issues/89)
* Using with Spans
  * [Core#399](https://github.com/isocpp/CppCoreGuidelines/issues/399)
* Converting constructors 
  * [GSL#395](https://github.com/Microsoft/GSL/issues/395)
* Any difference between [`reference_wrapper`](http://en.cppreference.com/w/cpp/utility/functional/reference_wrapper)? In C++ we have references that were designed not to hold null values, there's also a reference_wrapper class that are copyable and assignable. So cannot we just use ref wrapper instead of `not_null`? 
  *  [Stack Overflow: gsl::not_null<T*> vs. std::reference_wrapper<T> vs. T&](https://stackoverflow.com/questions/33306553/gslnot-nullt-vs-stdreference-wrappert-vs-t?rq=1).

## Summary

Should we immediately use `not_null` everywhere in our code?
The answer is not that obvious.

For sure, I am waiting to see such class in the Standard Library, not just in GSL. When it's included in STL, it would be perceived as a solid standardized helper to our code. I haven't seen any papers on that, however... maybe you know something about it?

Still, I believe it can help in many places. It won't do the magic on its own, but at least it forces us to rethink the design. Functions might become smaller (as they won't have to check for nulls), but on the other hand, the caller might require being updated.

It's definitely worth a try, so I plan to write more code with `not_null`.

### Call to action:

* Play with `not_null` for some time. Share your feedback.

This playground is adapted from my blog: [Bartek's coding blog: How not_null can improve your code?](http://www.bfilipek.com/2017/10/notnull.html)

Visit the blog if you're looking for more good stuff about C++ :)

	