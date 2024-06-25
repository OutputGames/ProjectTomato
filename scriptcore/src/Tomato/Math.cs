using System;

namespace Tomato
{
    
    public class Vector2
    {
        public float X,Y;

        public Vector2(float scalar)
        {
            X = scalar;
            Y = scalar;
        }

        public Vector2(float x, float y)
        {
            this.X = x;
            this.Y = y;
        }

        public override string ToString()
        {
            return X + "," + Y;
        }

        public static Vector2 Parse(string s)
        {
            string[] ps = s.Split(',');

            return new Vector2(float.Parse(ps[0]), float.Parse(ps[1]));
        }

    }
    
    public class Vector3
    {
        public float X = 0;
        public float Y = 0;
        public float Z = 0;

        
        public Vector3(float scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
        }
        
        public Vector3(Vector2 xy, float z)
        {
            X = xy.X;
            Y = xy.Y;
            this.Z = z;
        }
        public Vector3(float x, float y, float z)
        {
            //Console.WriteLine(x);
            //Console.WriteLine(y);
            //Console.WriteLine(z);
            X = x;
            Y = y;
            Z = z;
        }


        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        }

        public static Vector3 operator *(Vector3 a, Vector3 b)
        {
            return new Vector3(a.X * b.X, a.Y * b.Y, a.Z * b.Z);
        }

        public static Vector3 operator *(Vector3 vector, float scalar)
        {
            return new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

        public override string ToString()
        {
            return X + "," + Y + "," + Z;
        }

        public static Vector3 Parse(string s)
        {

            string[] ps = s.Split(',');

            return new Vector3(float.Parse(ps[0]), float.Parse(ps[1]), float.Parse(ps[2]));
        }

    }
    
    public class Vector4
    {
        public float X,Y,Z,W;

        public Vector4(float scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
            W = scalar;
        }

        public Vector4(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public Vector4(Vector2 xy,  Vector2 zw)
        {
            X = xy.X;
            Y = xy.Y;
            Z = zw.X;
            W = zw.Y;
        }

        public override string ToString()
        {
            return X + "," + Y + "," + Z + "," + W;
        }

        public static Vector4 Parse(string s)
        {
            string[] ps = s.Split(',');
            return new Vector4(float.Parse(ps[0]), float.Parse(ps[1]), float.Parse(ps[2]), float.Parse(ps[3]));
        }

    }

    public static class Math
    {
        public static float Sin(float v) => (float)System.Math.Sin(v);
        public static float Cos(float v) => (float)System.Math.Cos(v);
    }
    
}