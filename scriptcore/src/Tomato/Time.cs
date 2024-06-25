using System.Runtime.CompilerServices;

namespace Tomato
{
    public static class Time
    {
        public static extern float time { [MethodImpl(MethodImplOptions.InternalCall)] get; }
    }
}