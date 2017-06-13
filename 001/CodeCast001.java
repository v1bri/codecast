public class CodeCast001 {
    // This is our long running function, ported to its own Java class.
    public static void longComputation(double n) throws InterruptedException {
        Thread.sleep((long)(n * 1000.0));
    }

    // Our long running function, enhanced to handle pending Matlab operations while
    // blocked.
    public static void longInterruptibleComputation(double n) {
        java.lang.reflect.Method mtEval = null;
        Class matlabException = null;

        try {
            // When this code runs in Matlab's JVM, it will have access to the JMI classes via
            // reflection. We'll use them to obtain a reference to the `mtEval` method.
            Class<?> ML = java.lang.Class.forName("com.mathworks.jmi.Matlab");
            mtEval = ML.getMethod("mtEval", String.class, int.class);

            // The `MatlabException` class lets us differentiate `Ctrl-C` from other interruptions.
            matlabException = java.lang.Class.forName("com.mathworks.jmi.MatlabException");
            System.out.println("Running in Matlab");
        } catch (ReflectiveOperationException ex) {
            System.out.println("Not running in Matlab");
        }

        if (mtEval == null) {
            try {
                // When not running in Matlab we can code in our usual style.
                Thread.sleep((long)(n * 1000.0));
            } catch (Exception ex) {
                System.out.println(ex);
            }
        } else {
            try {
                // When running in Matlab we will poll the event queue via `drawnow limitrate` to
                // remain responsive to interruptions.
                for (long i = 0; i < (long)n; ++i) {
                    mtEval.invoke(null, "drawnow limitrate", 0);
                    Thread.sleep(1000);
                }
            }
            catch (Exception ex) {
                Throwable t = ex.getCause();
                if (t != null && t.getClass().equals(matlabException)) {
                    System.out.println("Matlab Ctrl-C");
                } else {
                    System.out.println(ex);
                }
            }
        }
    }
}
