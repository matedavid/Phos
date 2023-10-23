using System.Runtime.CompilerServices;

namespace PhosEngine
{
    public static class InternalCalls
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Sample();
    }
}