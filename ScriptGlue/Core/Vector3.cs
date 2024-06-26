using System;
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

        public static Vector3 Left => new Vector3() { X = -1.0f, Y = 0.0f, Z = 0.0f };
        public static Vector3 Right => new Vector3() { X = 1.0f, Y = 0.0f, Z = 0.0f };
        public static Vector3 Up => new Vector3() { X = 0.0f, Y = 1.0f, Z = 0.0f };
        public static Vector3 Down => new Vector3() { X = 0.0f, Y = -1.0f, Z = 0.0f };
        public static Vector3 Front => new Vector3() { X = 0.0f, Y = 0.0f, Z = 1.0f };
        public static Vector3 Back => new Vector3() { X = 0.0f, Y = 0.0f, Z = -1.0f };

        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public static float Dot(Vector3 a, Vector3 b) => a.X * b.X + a.Y * b.Y + a.Z * b.Z;
        public static Vector3 Scalar(Vector3 a, Vector3 b) => new Vector3(a.X * b.X, a.Y * b.Y, a.Z * b.Z);

        public static Vector3 Cross(Vector3 a, Vector3 b) =>
            new Vector3(
                a.Y * b.Z - a.Z * b.Y,
                a.Z * b.X - a.X * b.Z,
                a.X * b.Y - a.Y * b.X
            );

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