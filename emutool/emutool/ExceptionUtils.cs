using System;
using System.Windows.Forms;
using System.Runtime.CompilerServices;

namespace emutool
{
    public static class ExceptionUtils
    {
        public static void LogExceptionMessage(Exception ex, [CallerMemberName] string fn_name = "<unknown>")
        {
            MessageBox.Show("Caught exception (" + ex.GetType().Name + ") at " + fn_name + ": " + ex.ToString(), "Caught exception!");
            Environment.Exit(1);
        }

        public static void Unless(bool cond, string message)
        {
            if(!cond)
            {
                throw new Exception(message);
            }
        }
    }
}
