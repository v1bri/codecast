# Epiode 002 - An opinionated start to a C++ project

Thanks in part to fierce competition over the past decade, C++ has seen a huge
ramp in language features and batteries included library support. Build tools
have also improved so much that starting a new project could almost be described
as user-friendly. Most projects are standardizing around CMake which, with a
style ironically similar to C++, can be quite powerful when restricted to a
pleasant subset of the overall language. This is a quick guide to starting your
own C++ programming project. Use a few of these concepts and you'll be off to a
great start.

**Demo**

My C++ projects always start with two extremely useful libraries:
[gflags](https://github.com/gflags/gflags) and
[glog](https://github.com/google/glog). Both are open source libraries from
Google that have seen heavy production use inside and outside the company.
Gflags provides a quick and easy way to integrate command line flags into your
application, while glog delivers flexible, high performance logging with
built-in assertions and stack traces. These libraries are under active
development on GitHub and very responsive to issues and pull requests.

To add the libraries to our project we'll use the popular CMake package manager
[Hunter](https://github.com/ruslo/hunter). The jury is still out on which
package manager will become the de-facto choice for C++, but Hunter is a great
choice today. It's written in native CMake script, so there are no additional
dependencies (like Python, node.js or another scripting language). All Hunter
requires is one CMake module and a few lines in `CMakeLists.txt` to enable it.

**Demo**

So that's it for now. Full source code for this codecast is available on
[my GitHub repo](https://github.com/v1bri/codecast).
If you enjoyed watching please leave a comment, like this video or subscribe to
[my YouTube channel](https://www.youtube.com/channel/UCuHL6s049fT6juv0z8Y9U0A)
for future codecasts. Thanks for watching!
