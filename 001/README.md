# Episode 001 - Handling MATLAB interrupts in Java

Matlab is a great tool for prototyping new ideas, running experiments and
visualizing the results. And its native support for Java libraries quickly takes
those ideas from prototype to production. However with a large enough Matlab and
Java code base, eventually one problem always rears its head. There is
effectively no way to terminate a long running or blocked Java method call! Just
try to `Ctrl-C` out of a Java method from Matlab and see for yourself.

**Demo**

Not the best user experience. The `Ctrl-C` gets placed on Matlab's event queue,
which is handled by the thread that is currently blocked forever inside the Java
method. This problem has come up
[again](https://www.mathworks.com/matlabcentral/answers/77663),
and [again](http://undocumentedmatlab.com/blog/waiting-for-asynchronous-events),
and [*again*](https://www.mathworks.com/matlabcentral/newsreader/view_thread/61152)
over the years.

We simply need a way to periodically process Matlab's event queue, while blocked
inside the Java method. As luck would have it, one night after a few drinks I
was struck by some inspiration from
[this blog post](http://undocumentedmatlab.com/blog/jmi-java-to-matlab-interface)
and came up with something new.

The trick is to call back into Matlab via JMI (the Java Matlab Interface) and
trigger event queue processing. If a `Ctrl-C` is waiting on the queue it will
get picked up and generate an exception that we can handle in our Java code. We
just need to choose a suitable Matlab method. `pause()` will work but always
incurs a slight delay. `drawnow` will process pending callbacks without
explicitly pausing, which could also work. However, `drawnow limitrate` is even
better because it defers pending graphics operations after achieving a target
frame rate.

**Demo**

So that's it for now. Full source code for this codecast is available on
[my GitHub repo](https://github.com/v1bri/codecast).
If you enjoyed watching please leave a comment, like this video or subscribe to
[my YouTube channel](https://www.youtube.com/channel/UCuHL6s049fT6juv0z8Y9U0A)
for future codecasts. Thanks for watching!
