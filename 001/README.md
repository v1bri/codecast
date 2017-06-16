# Episode 001 - Handling MATLAB interrupts in Java

Matlab is a great tool for prototyping new ideas, running experiments and
visualizing the results. And its native support for Java libraries quickly takes
those ideas from prototype to production. However with a large enough Matlab and
Java code base, eventually one problem always rears its head. There is
effectively no way to terminate a long running or blocked Java method call! Just
try to `Ctrl-C` out of a Java method from Matlab and see for yourself.

**Demo**
```
$ matlab -nodesktop -nodisplay -nosplash

                                           < M A T L A B (R) >
                                 Copyright 1984-2017 The MathWorks, Inc.
                                  R2017a (9.2.0.538062) 64-bit (glnxa64)
                                            February 23, 2017

 
To get started, type one of these: helpwin, helpdesk, or demo.
For product information, visit www.mathworks.com.
 
>> % We'll start with a long computation. This could be a matrix multiply, a
>> % graphics plot, reading a socket or any other long running operation in native
>> % Matlab code.
>> long_computation = @() pause(2);
>> long_computation();
>> 
>> % That works. Let's refactor and increase our test coverage.
>> long_computation = @(n) pause(n);
>> long_computation(4);
>> 
>> % Now to run it on some big data.
>> long_computation(intmax('int32'));
Operation terminated by user during @(n)pause(n)
 
>> % When things take longer than usual, a user quickly learns to reach for
>> % `Ctrl-C` first. As you can see, it works as expected from native Matlab code.
>> clc

>> % Once the library is ready, we can port to Java.
>> long_java_computation = @(n) java.lang.Thread.sleep(n * 1000);
>> long_java_computation(2);
>> long_java_computation(4);
>> 
>> % The moment of truth...
>> long_java_computation(intmax('int32'));

[3]+  Stopped                 matlab -nodesktop -nodisplay -nosplash
$ # No luck. Go directly to `killall -9 MATLAB`.
$ killall -9 MATLAB
[3]+  Killed                  matlab -nodesktop -nodisplay -nosplash
```

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
```
$ emacs -nw CodeCast001.java
$
$ # We'll build and jar it for Matlab's JVM.
$ javac -source 1.7 -target 1.7 CodeCast001.java
warning: [options] bootstrap class path not set in conjunction with -source 1.7
1 warning
$ jar -cf codecast001.jar CodeCast001.class
$ matlab -nodesktop -nodisplay -nosplash

                                           < M A T L A B (R) >
                                 Copyright 1984-2017 The MathWorks, Inc.
                                  R2017a (9.2.0.538062) 64-bit (glnxa64)
                                            February 23, 2017

 
To get started, type one of these: helpwin, helpdesk, or demo.
For product information, visit www.mathworks.com.
 
>> javaaddpath('codecast001.jar');
>> 
>> % Let's quickly test for regressions.
>> CodeCast001.longComputation(2);
>> CodeCast001.longComputation(4);
>> CodeCast001.longComputation(intmax('int32'));

[3]+  Stopped                 matlab -nodesktop -nodisplay -nosplash
$ killall -9 MATLAB
[3]+  Killed                  matlab -nodesktop -nodisplay -nosplash
$ emacs -nw CodeCast001.java
$
$ # One more build.
$ javac -source 1.7 -target 1.7 CodeCast001.java
warning: [options] bootstrap class path not set in conjunction with -source 1.7
1 warning
$ jar -cf codecast001.jar CodeCast001.class
$ matlab -nodesktop -nodisplay -nosplash

                                           < M A T L A B (R) >
                                 Copyright 1984-2017 The MathWorks, Inc.
                                  R2017a (9.2.0.538062) 64-bit (glnxa64)
                                            February 23, 2017

 
To get started, type one of these: helpwin, helpdesk, or demo.
For product information, visit www.mathworks.com.
 
>> javaaddpath('codecast001.jar');
>> 
>> % Cross your fingers!
>> CodeCast001.longInterruptibleComputation(intmax('int32'));
Running in Matlab
Matlab Ctrl-C
>> % Sweet it works!!!!
>> exit
```

So that's it for now. Full source code for this codecast is available on
[my GitHub repo](https://github.com/v1bri/codecast).
If you enjoyed watching please leave a comment, like this video or subscribe to
[my YouTube channel](https://www.youtube.com/channel/UCuHL6s049fT6juv0z8Y9U0A)
for future codecasts. Thanks for watching!
