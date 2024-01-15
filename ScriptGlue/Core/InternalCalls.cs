using System.Runtime.CompilerServices;

namespace PhosEngine
{
    internal static class InternalCalls
    {
        #region Logging

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Logging_Info(string content);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Logging_Warning(string content);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Logging_Error(string content);

        #endregion

        #region Entity

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_Instantiate(ulong prefabId, out ulong entityId);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_Destroy(ulong entityId);

        #endregion

        #region Input

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Input_IsKeyDown(uint key, out bool isDown);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Input_IsMouseButtonDown(uint mouseButton, out bool isDown);

        #endregion

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