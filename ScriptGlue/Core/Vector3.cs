using System.Runtime.InteropServices;

namespace PhosEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3
    {
        public float X;
        public float Y;
        public float Z;

        public static Vector3 Zero => new Vector3() { X = 0.0f, Y = 0.0f, Z = 0.0f };

        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public static Vector3 operator +(Vector3 a, Vector3 b) => new Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        public static Vector3 operator -(Vector3 a, Vector3 b) => new Vector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);

        public static Vector3 operator +(Vector3 a, float b) => new Vector3(a.X + b, a.Y + b, a.Z + b);
        public static Vector3 operator +(float b, Vector3 a) => new Vector3(a.X + b, a.Y + b, a.Z + b);

        public static Vector3 operator -(Vector3 a, float b) => new Vector3(a.X - b, a.Y - b, a.Z - b);
        public static Vector3 operator -(float b, Vector3 a) => new Vector3(a.X - b, a.Y - b, a.Z - b);

        public static Vector3 operator *(Vector3 a, float b) => new Vector3(a.X * b, a.Y * b, a.Z * b);
        public static Vector3 operator *(float b, Vector3 a) => new Vector3(a.X * b, a.Y * b, a.Z * b);
    }
}