using System;
using Tomato;
using TomatoEngine;

namespace TomatoScript.Tomato
{
    public class Transform : Object
    {
        private Actor _actor;

        public Vector3 Position
        {
            get => InternalCalls.GetTransformValue(Actor.GetId(), 0);
            set
            {
                //Console.WriteLine(value);
                InternalCalls.SetTransformValue(Actor.GetId(), value, 0);
            }
        }

        public Vector3 Rotation
        {
            get => InternalCalls.GetTransformValue(Actor.GetId(), 1);
            set => InternalCalls.SetTransformValue(Actor.GetId(), value, 1);
        }

        public Vector3 Scale
        {
            get => InternalCalls.GetTransformValue(Actor.GetId(), 2);
            set => InternalCalls.SetTransformValue(Actor.GetId(), value, 2);
        }

        public Actor Actor
        {
            get => _actor;
            internal set => _actor = value;
        }
    }
}