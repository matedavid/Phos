using System.Runtime.CompilerServices;

namespace PhosEngine
{
    internal static class InternalCalls
    {
        #region TransformComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetPosition(ulong entityId, out Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetPosition(ulong entityId, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetScale(ulong entityId, out Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetScale(ulong entityId, ref Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetRotation(ulong entityId, out Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetRotation(ulong entityId, ref Vector3 rotation);

        #endregion
    }
}