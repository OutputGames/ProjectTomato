using System;
using Tomato;
using Math = Tomato.Math;

namespace TomatoScript.Tomato
{
    public class TestComponent : MonoComponent
    {

        public float Radius = 10;
        public float Speed = 0.01f;

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

            //Console.WriteLine(Radius);
            
            Actor.Transform.Position = new Vector3((float)(Math.Sin(Time.time*Speed)*Radius), 0,(float)(Math.Cos(Time.time*Speed)*Radius));
            
            base.Update();
        }
    }
}