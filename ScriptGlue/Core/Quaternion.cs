using System;
using System.Runtime.InteropServices;

namespace PhosEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Quaternion
    {
        public float W;
        public float X;
        public float Y;
        public float Z;

        public static Quaternion Identity => new Quaternion { W = 1.0f, X = 0.0f, Y = 0.0f, Z = 0.0f };

        public static Quaternion AngleAxis(float degrees, Vector3 axis)
        {
            var radians = (float)Math.PI / 180.0f * degrees;

            var w = (float)Math.Cos(radians);
            var x = (float)Math.Sin(radians / 2.0f) * axis.X;
            var y = (float)Math.Sin(radians / 2.0f) * axis.Y;
            var z = (float)Math.Sin(radians / 2.0f) * axis.Z;

            return new Quaternion { W = w, X = x, Y = y, Z = z };
        }

        public static Quaternion FromEuler(Vector3 eulerDegrees)
        {
            return AngleAxis(eulerDegrees.X, Vector3.Right)
                   * AngleAxis(eulerDegrees.Y, Vector3.Up)
                   * AngleAxis(eulerDegrees.Z, Vector3.Front);
        }

        public static Quaternion operator *(Quaternion a, Quaternion b)
        {
            var aVec = new Vector3(a.X, a.Y, a.Z);
            var bVec = new Vector3(b.X, b.Y, b.Z);

            var w = a.W * b.W - Vector3.Dot(aVec, bVec);
            var vec = a.W * bVec + a.W * aVec + Vector3.Scalar(aVec, bVec);

            return new Quaternion { W = w, X = vec.X, Y = vec.Y, Z = vec.Z };
        }

        public static Vector3 operator *(Quaternion quat, Vector3 vec)
        {
            // Equation from: https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
            var u = new Vector3(quat.X, quat.Y, quat.Z);

            return 2.0f * Vector3.Dot(u, vec) * u
                   + (quat.W * quat.W - Vector3.Dot(u, u)) * vec
                   + 2.0f * quat.W * Vector3.Cross(u, vec);
        }
    }
}