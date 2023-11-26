namespace PhosEngine
{
    public class Logging
    {
        public static void Info(string content)
        {
            InternalCalls.Logging_Info(content);
        }

        public static void Warning(string content)
        {
            InternalCalls.Logging_Warning(content);
        }

        public static void Error(string content)
        {
            InternalCalls.Logging_Error(content);
        }
    }
}