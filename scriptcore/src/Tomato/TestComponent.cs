using System;
using Tomato;
using Math = Tomato.Math;

namespace TomatoScript.Tomato
{
    public class TestComponent : MonoComponent
    {
        protected override void Start()
        {
            base.Start();
            
            Console.WriteLine("Started test component");
            Console.WriteLine("Actor: "+Actor.Name);
            Console.WriteLine("Id: "+Actor.GetId());
            
            Console.WriteLine(Actor.Transform);
            
            Console.WriteLine("Position: "+Actor.Transform.Position);
            
        }

        protected override void Update()
        {
            
            //Console.WriteLine("Updated test component");

            //Console.WriteLine(Time.time);

            float radius = 10;
            float speed = 0.01f;
            
            Actor.Transform.Position = new Vector3(Math.Sin(Time.time*speed)*radius, 0,Math.Cos(Time.time*speed)*radius);
            
            base.Update();
        }
    }
}