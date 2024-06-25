using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using Tomato;
using TomatoScript.Tomato;

namespace TomatoEngine
{
    public static class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern Vector3 GetTransformValue(uint id, int type);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void SetTransformValue(uint id, Vector3 p, int type);
    }
}